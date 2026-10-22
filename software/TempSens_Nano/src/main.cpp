#include <Arduino.h>
#include <Wire.h>
#include "temp_converter.h"
#include <stdint.h>
// put function declarations here:


static bool sensor_error = true;

#define I2C_Address 0x42
const int diodes = 16;
int fault = 600;
int nofault = 500;

int mux_ms = 1500;
int ADC_read = 4;
int ADC_shift = 2; // log2(ADC_read) = 2;

int led_period = 250;

//Pin Assignments
static const int8_t Mux_en = 9;
static const int8_t Sens_en = 10;
static const int8_t pin_LED = 8;
static const int8_t pin_ADC = A0;

int mux_shift = 2;
int mux_area = (0x0F << mux_shift);

//I2C

#define summary   0x00   
#define channel_Data       0x01
#define summary_len   8
#define all_len       (diodes * 2)

#define maxtemp     0x01   // max temp above limit
#define sensErrChan 0x02   //at least one channel open or short
#define statvalid   0x04   //at least one scan done
#define invalid     0x7FFF // value for a bad channel

//----

static uint8_t tempSummary[summary_len];
static uint8_t tempAllChannel[all_len];
static volatile uint8_t pointer = summary;

static int16_t adcCounts[diodes];
static int16_t tempvaluesx10[diodes];
static int8_t channelstatus[diodes]; 

static bool overTemp = false;
static bool sensorError = false;
static bool haveValidScan = false;



//LED
static bool ledON = false;
static int32_t lastLEDtoggle = 0;



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

static bool faultCheck(void){
    if (sensor_error) 
      return overTemp || sensorError;
    else
      return overTemp;
    
}

//ADC Reading Func

static int16_t readChannel(int8_t channel1){
  muxSelect(channel1);
  delayMicroseconds(mux_ms);
  (void)analogRead(pin_ADC);

  int16_t sum = 0;
  for (int i = 0; i < ADC_read; i++){
    sum += analogRead(pin_ADC);
  }
  return (sum + (ADC_read >> 1)) >> ADC_shift;
}

// Full 16 Channel Scan
static void scanChannels(void){
  int32_t sum10 = 0;
  int16_t max10 = -32768, min10 = 32767;
  int8_t valid1 = 0, hotchannel = 0;
  bool anyError = false;

  for (uint8_t channel2 = 0; channel2 < diodes; channel2++){
    adcCounts[channel2] = readChannel(channel2);
    channelstatus[channel2] = count_to_temp(adcCounts[channel2], &tempvaluesx10[channel2]);

    if (channelstatus[channel2] == Normal){
      int16_t t = tempvaluesx10[channel2];
      if(t>max10) {max10 = t; hotchannel = channel2;}
      if(t<min10) {min10 = t;}
      sum10 += t;
      valid1++;
    }
    else{
      anyError = true;
    }
    updateLED(faultCheck());
  }

  sensor_error = anyError;

  if (valid1 > 0){
    if (max10 > fault)  overTemp = true;
    else if (max10 < nofault) overTemp = false;
  }

  int8_t sum8[summary_len];
  int8_t channel_data[all_len];

  int8_t status = statvalid;
  if (overTemp) status |= maxtemp;
  if (sensor_error) status |= sensErrChan;

  int16_t avg = (valid1 > 0) ? (int16_t)(sum10 / valid1) : (int16_t)invalid;
  if (valid1 == 0) {max10 = invalid; min10 = invalid;}

  sum8[0] = status;
  bigEndian(&sum8[1], max10);
  bigEndian(&sum8[3], min10);
  bigEndian(&sum8[5], avg);
  sum8[7] = hotchannel;

  for (uint8_t channel3 = 0; channel3 < diodes; channel3++){
    int16_t value2 = (channelstatus[channel3] == Normal) ? tempvaluesx10[channel3] : (int16_t)invalid;
    bigEndian(&channel_data[channel3 * 2], value2);
  }

  noInterrupts();
  memcpy(tempSummary,sum8,summary_len);
  memcpy(tempAllChannel,channel_data,all_len);
  interrupts();
  
  haveValidScan = true;

}

static void onI2CReceive (int numBytes){
  if (numBytes >= 1){
    pointer = (uint8_t)Wire.read();
    while (Wire.available() > 0) {
      (void)Wire.read();
    }
  }
}

static void onI2CRequest (void){
  if (pointer == channel_Data){
    Wire.write(tempAllChannel, all_len);
  }
  else{
    Wire.write(tempSummary, summary_len);
  }
}


void setup(void) {
  DDRD |= mux_area;
  muxSelect(0);
  pinMode(Mux_en, OUTPUT);
  digitalWrite(Mux_en, HIGH);\
  
  //LED
  pinMode(pin_LED, OUTPUT);
  digitalWrite(pin_LED, LOW);

  pinMode(Sens_en, OUTPUT);
  digitalWrite(Sens_en, LOW);

  analogReference(DEFAULT);

  delay(20);
  digitalWrite(Mux_en, LOW);

  memset(tempSummary, 0, summary_len);
  bigEndian((int8_t *)&tempSummary[1], invalid);
  bigEndian((int8_t *)&tempSummary[3], invalid);
  bigEndian((int8_t *)&tempSummary[5], invalid);
  for (uint8_t channel4 = 0; channel4 < diodes; channel4++){
    bigEndian((int8_t *)&tempAllChannel[channel4 * 2], invalid);
  }

  Wire.begin(I2C_Address);
  Wire.onReceive(onI2CReceive);
  Wire.onRequest(onI2CRequest);

}

void loop() {
  scanChannels();
  updateLED(faultCheck());
}
