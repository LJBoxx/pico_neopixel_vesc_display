#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>

#define PIN_NEOPIXEL 0
#define BRIGHTNESS 16

float current_percentage = 0;
float battery_percentage = 0;
float speed = 0;

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

int number = 0;
int number_units = 0;
int number_dozens = 0;
uint16_t color_bar = matrix.Color(0,0,0);

void loop() {
    number += 1;
    battery_percentage += 10;
    if (battery_percentage>100) battery_percentage = 0;
    if (number > 199) number = 0; //change to 199 to clamp it later on (bc display 99+ but whatever who would go 200+ on an ev)
    
    number_dozens = number/10;
    number_units = number % 10;

    if (battery_percentage < 40) {
        color_bar = matrix.Color(255,0,0);
    } else if (battery_percentage >= 40 && battery_percentage <= 60) {
        color_bar = matrix.Color(128,128,0);
    } else if (battery_percentage > 60){
        color_bar = matrix.Color(0,200,0);
    }
    
    matrix.fillScreen(0);
    draw_digit(5,0,number_units,colors[1]);
    if (number < 100) {
        draw_digit(1,0,number_dozens,colors[1]);
    } else {
        number_dozens -= 10;
        draw_digit(1,0,number_dozens,colors[0]);
    }
    draw_bar(7,battery_percentage, color_bar);
    matrix.setBrightness(BRIGHTNESS);
    matrix.show();
    delay(100);
}