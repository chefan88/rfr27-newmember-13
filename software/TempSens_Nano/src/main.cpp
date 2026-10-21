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
static const int8_t Mux_en = 9;
static const int8_t Sens_en = 10;
static const int8_t pin_LED = 8;
static const int8_t pin_button = 7;
static const int8_t pin_ADC = A0;

int mux_shift = 2;
int mux_area = (0x0F << mux_shift);

//I2C

static int16_t adcCounts[diodes];
static int8_t tempvaluesx10[diodes];
static int8_t channelstatus[diodes]; 

//LED
static bool ledON = false;
static int32_t lastLEDtoggle = 0;
static bool lastButton = false;
static int32_t lastButtonChange = 0;



//side funcs

static void bigEndian(int8_t *final, int16_t value){
  final[0] = (int8_t)((int16_t)value >> 8);
  final[1] = (int8_t)((int16_t)value & 0xFF);
}

static void muxSelect(int8_t channel){
  PORTD = (int8_t)((PORTD & ~mux_area) | ((channel & 0x0F) << mux_shift));
}

//LED funcs
static void updateLED(bool fault){
    int32_t now = millis();
    if (!fault) {
      ledON = false;
      digitalWrite(pin_LED, LOW);
      return;
    }
    else if (now - lastLEDtoggle >= led_period){
      lastLEDtoggle = now;
      ledON = !ledON;
      digitalWrite(pin_LED, ledON ? HIGH : LOW);
    }
}


void setup() {
  // put your setup code here, to run once:
  
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}