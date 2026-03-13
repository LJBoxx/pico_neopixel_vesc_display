#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <mcp_can.h>
#include <SPI.h>

#define PIN_NEOPIXEL 0
#define BRIGHTNESS 16
#define CAN_INT 9 //gp9 int mcp on hw spi1
#define CAN_CS 13
#define BTN_POWER 2
#define BTN_SELECT 3
#define BTN_UP 4
#define BTN_DOWN 5

float current_percentage = 0; //float is heavy on mcu. need to change to int.
float battery_percentage = 0;
float speed = 0;
float vbattmin = 30;
float vbattmax = 42; //values for 10s batt (on bench test)
int poles = 14;
int diameter = 279;//279mm about 11inch 

uint32_t T_MOS = 0;
uint32_t T_MOT = 0;

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
//char msgString[128]; //idontneed that lol

MCP_CAN CAN(&SPI1, CAN_CS);

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PIN_NEOPIXEL,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800); 

const uint16_t colors[] = {matrix.Color(255,0,0), matrix.Color(0,255,0), matrix.Color(0,0,255)};

const uint8_t digits[10][5] = {
    {0b111,
    0b101,
    0b101,
    0b101,
    0b111}, // 0
    {0b010,
    0b110,
    0b010,
    0b010,
    0b111}, // 1
    {0b111,
    0b001,
    0b111,
    0b100,
    0b111}, // 2
    {0b111,
    0b001,
    0b011,
    0b001,
    0b111}, // 3
    {0b101,
    0b101,
    0b111,
    0b001,
    0b001}, // 4
    {0b111,
    0b100,
    0b111,
    0b001,
    0b111}, // 5
    {0b111,
    0b100,
    0b111,
    0b101,
    0b111}, // 6
    {0b111,
    0b001,
    0b001,
    0b001,
    0b001}, // 7
    {0b111,
    0b101,
    0b111,
    0b101,
    0b111}, // 8
    {0b111,
    0b101,
    0b111,
    0b001,
    0b111}  // 9
};

void draw_digit(int x, int y, int num, uint16_t color) {
    if (num<0 || num>9) return;

    for (int row = 0; row <5; row ++){
        for (int col = 0; col < 3; col ++){
            if (digits[num][row] & (1 << (2-col))) {
                matrix.drawPixel(x+col, y+row, color);
            } 
        }
    }
}

void draw_bar(int y, float percent, uint16_t color) {
    float length = (int)((percent/100.0) * matrix.width());
    if (length > 8) length = 8;
    for (int i =0; i<length; i++) {
        matrix.drawPixel(i, y, color);
    }
}

void setup() {
    matrix.begin();
    matrix.setTextWrap(false);
    matrix.setBrightness(40);
    matrix.setTextColor(colors[0]);
}

int number_units = 0;
int number_dozens = 0;
uint16_t color_bar = matrix.Color(0,0,0);

void setup1() {
    SPI1.setRX(12);
    SPI1.setTX(11);
    SPI1.setSCK(10);
    SPI1.begin();
    CAN.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ);
    CAN.setMode(MCP_NORMAL);
    pinMode(CAN_INT, INPUT_PULLUP);
    pinMode(BTN_POWER, INPUT_PULLUP);
    pinMode(BTN_SELECT, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    
    //add query for config (idk how yet ;-;)
}

void loop() {
    //speed += 1;
    //battery_percentage += 10; for testing 
    if (battery_percentage>100) battery_percentage = 0;
    if (speed > 199) speed = 0; //change to 199 to clamp it later on (bc display 99+ but whatever who would go 200+ on an ev)
    
    number_dozens = speed/10;
    number_units = (int)speed % 10;

    if (battery_percentage < 40) {
        color_bar = matrix.Color(255,0,0);
    } else if (battery_percentage >= 40 && battery_percentage <= 60) {
        color_bar = matrix.Color(128,128,0);
    } else if (battery_percentage > 60){
        color_bar = matrix.Color(0,200,0);
    }
    
    matrix.fillScreen(0);
    draw_digit(5, 0, number_units, colors[1]);
    if (speed < 100) {
        draw_digit(1, 0, number_dozens, colors[1]);
    } else {
        number_dozens -= 10;
        draw_digit(1, 0, number_dozens, colors[0]);
    }
    draw_bar(7, battery_percentage, color_bar); //batt
    draw_bar(6, current_percentage, colors[2]); //power
    matrix.setBrightness(BRIGHTNESS);
    matrix.show();
    delay(100);
}

void loop1() {
    if (!digitalRead(CAN_INT)) {
        CAN.readMsgBuf(&rxId, &len, rxBuf);
        int vesc_id = rxId & 0xFF; //b0-7 for vescid
        int command_id = (rxId>>8) & 0xFF; //b8-15 for command id
        if (command_id == 9) { //status 1
            int32_t erpm = 0; //rpm
            int32_t current = 0; //A scale 10
            int32_t duty = 0; //percentage scale 1000
            for (int i = 0; i < 8; i++) {
                if (i < 4) erpm = (erpm<<8) | rxBuf[i]; //packet b0-3 (4byte)
                if (i >= 4 && i < 6) current = (current<<8) | rxBuf[i];
                if (i >= 6) duty = (duty<<8) | rxBuf[i];
            }
            if (duty != 0) duty /= 1000;
            if (current != 0) current /= 10;
            speed = ((erpm / (poles/2)) * (314 * diameter)/10000) * 60;//speed in km/m(distance/time) = r/m * distance/r (and units conv)
            //mayb a filter so its not too jumpy ? i mean polling rate can be changed and default @10hz iirc
        }
        if (command_id == 16) { //status 4
            for (int i = 0; i < 8; i++) {
                if (i < 2) T_MOS = ((T_MOS<<8) | rxBuf[i]);
                if (i >= 2 && i <4) T_MOT = ((T_MOT<<8) | rxBuf[i]);
            }
            if (T_MOS!=0) T_MOS /= 10;
            if (T_MOT!=0) T_MOT /= 10;
        }
        if (command_id == 27) { //status 5
            int32_t voltage = 0;
            for (int i = 0; i < 6; i++) {
                if (i >= 3) voltage = (voltage<<8) | rxBuf[i]; 
            }
            voltage /= 10; //scale 10
            battery_percentage = (voltage-vbattmin)/(vbattmax-vbattmin);
        }
        
        /*
        process frame looooooooooooong ass if/else check lol 
        also might need to make something to appaear on the canbus scan that would be cool!
        alt mode : uart canbus is broken on a lot of vesc devices, maybe fallback mode ?
        */
    }
    if (!digitalRead(BTN_POWER)) {
        //send shutdown command
    }
    if (!digitalRead(BTN_SELECT)) {
        //idk man
    }
    if (!digitalRead(BTN_UP)) {
        //idfk
    }
    if (!digitalRead(BTN_DOWN)) {
        //same
    }
    if (!digitalRead(BTN_DOWN) && !digitalRead(BTN_UP)) {
        //speedlimiter logic here :p
    }
}