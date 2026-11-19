#include <Wire.h>

#define LED_PIN 10
#define S0_PIN 2
#define S1_PIN 3
#define S2_PIN 4
#define ADC_PIN A0
#define NUM_TEMP_SENSORS 8
#define NUM_TEMP_RECORDINGS 10
#define NUM_MUX_SELECT_LINES 3
#define NUM_DATA_VALUES 33
#define ADC_MAX_COUNT 1023
#define ADC_MAX_VOLT_MV 5000
#define TEMP_FAULT_THRESHOLD 60 // in celsius
#define AMP_GAIN 2
#define SLAVE_ADDRESS 0x77    // unknown for now, change later

const int tempVsVoltTable[NUM_DATA_VALUES][2] =                      // in the form of millivolts vs celsius
  {{1300, 120}, {1310, 115}, {1320, 110}, {1330, 105}, {1340, 100},
  {1350, 95}, {1370, 90}, {1380, 85}, {1400, 80}, {1430, 75},
  {1450, 70}, {1480, 65}, {1510, 60}, {1550, 55}, {1590, 50},
  {1630, 45}, {1680, 40}, {1740, 35}, {1800, 30}, {1860, 25},
  {1920, 20}, {1990, 15}, {2050, 10}, {2110, 5}, {2170, 0},
  {2230, -5}, {2270, -10}, {2320, -15}, {2350, -20}, {2380, -25},
  {2400, -30}, {2420, -35}, {2440, -40}};

const int muxSelectLineValues[NUM_TEMP_SENSORS][NUM_MUX_SELECT_LINES] =
  {{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, 
  {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}};

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(S0_PIN, OUTPUT);
  pinMode(S1_PIN, OUTPUT);
  pinMode(S2_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
}

void loop() {
  int highestTempReading = -128;  // lowest 8-bit signed integer
  static double tempRecordings[NUM_TEMP_SENSORS][NUM_TEMP_RECORDINGS];

  for(int sensor=0; sensor < NUM_TEMP_SENSORS; sensor++){
    selectMuxChannel(sensor);
    delay(5);
    shiftLeft(tempRecordings[sensor], NUM_TEMP_RECORDINGS);       //sliding window algorithm
    tempRecordings[sensor][NUM_TEMP_RECORDINGS-1] = adcCountToTempConverter(analogRead(ADC_PIN));
    int currentTempReading = average(tempRecordings[sensor], NUM_TEMP_RECORDINGS);
    if(currentTempReading > highestTempReading){
      highestTempReading = currentTempReading;
    }
  }

  if(highestTempReading > TEMP_FAULT_THRESHOLD){
    digitalWrite(LED_PIN, HIGH);
  }else{
    digitalWrite(LED_PIN, LOW);
  }

  Serial.println(highestTempReading);

  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(highestTempReading);
  Wire.endTransmission();
}

void selectMuxChannel(int muxChannel){
    digitalWrite(S0_PIN, muxSelectLineValues[muxChannel][0]);
    digitalWrite(S1_PIN, muxSelectLineValues[muxChannel][1]);
    digitalWrite(S2_PIN, muxSelectLineValues[muxChannel][2]);
}

double adcCountToTempConverter(int adcCount){
  int tempSensorVoltReading =
    round(adcCount * ((double)ADC_MAX_VOLT_MV / ADC_MAX_COUNT));

    if(tempSensorVoltReading <= (tempVsVoltTable[0][0] * AMP_GAIN)){
      return tempVsVoltTable[0][1];
    }else if(tempSensorVoltReading >= (tempVsVoltTable[NUM_DATA_VALUES-1][0] * AMP_GAIN)){
      return tempVsVoltTable[NUM_DATA_VALUES-1][1];
    }

  int upperInterpolationBoundIndex = findUpperInterpolationBoundIndex(tempSensorVoltReading);
  int x_min = tempVsVoltTable[upperInterpolationBoundIndex-1][0] * AMP_GAIN;
  int y_min = tempVsVoltTable[upperInterpolationBoundIndex-1][1];     // y is temperature, x is voltage
  int x_max = tempVsVoltTable[upperInterpolationBoundIndex][0] * AMP_GAIN;
  int y_max = tempVsVoltTable[upperInterpolationBoundIndex][1];
  double estimatedTemp = 
    y_min + ((tempSensorVoltReading - x_min) * ((double)(y_max - y_min) / (x_max - x_min)));   // linear interpolation formula
  return estimatedTemp;
}

int findUpperInterpolationBoundIndex(int tempSensorVoltReading){
  for(int i=1; i<NUM_DATA_VALUES; i++){
    if((tempVsVoltTable[i][0] * AMP_GAIN) > tempSensorVoltReading){
      return i;
    }
  }
}

void shiftLeft(double arr[], int size){
  for(int i=0; i+1 < size; i++){
    arr[i] = arr[i+1];
  }
  arr[size-1] = 0;
}

int average(double arr[], int size){
  double sum = 0;
  for(int i=0; i<size; i++){
    sum = sum + arr[i];
  }
  return round(sum / size);
}
