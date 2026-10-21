#include <Arduino.h>
#include <Wire.h>
#include "temp_converter.h"
#include <stdint.h>
// put function declarations here:
int myFunction(int, int);

const int diodes = 16;
int fault = 600;
int nofault = 500;

int ADC_read = 4;


int led_period = 250;
int button_debounce = 30;

//Pin Assignments
static const int Mux_en = 9;
static const int Sens_en = 10;
static const int pin_LED = 8;
static const int pin_button = 7;
static const int pin_ADC = A0;

int mux_shift = 2;
int mux_area = (0x0F << mux_shift);

//I2C

static int16_t adcCounts[diodes];
void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}