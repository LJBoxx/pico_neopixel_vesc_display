#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <mcp_can.h>
#include <SPI.h>

#define PIN_NEOPIXEL 0
#define BRIGHTNESS 16
#define CAN_INT 9 //gp9 int mcp on hw spi1
#define CAN_CS 13

float current_percentage = 0; //float is heavy on mcu. need to change to int.
float battery_percentage = 0;
float speed = 0;

char msgString[128]; 

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
    CAN.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ);
    CAN.setMode(MCP_NORMAL);
    pinMode(CAN_INT, INPUT);
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
        /*
        process frame looooooooooooong ass if/else check lol 
        also might need to make something to appaear on the canbus scan that would be cool!
        alt mode : uart canbus is broken on a lot of vesc devices, maybe fallback mode ?
        */
    }
}