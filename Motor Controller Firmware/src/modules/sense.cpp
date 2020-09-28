/**
 * Motor Controller Firmware
 * Copyright (c) 2020 Applied Engineering
 * 
 * Based on software from various contributors.
 * Copyright (c) 2020 Andrew Berkun / Alex Liu / William Zhou
 * 
 */

/**
 * sense.cpp
 * 
 * Detects electrical operating conditions.
 * 
 */

#include "../Configuration.h"
#include "sense.h"

int deviceCount;
float tempC[2];

int senseSamples;
int avgCount = 16;
unsigned int sumCounter;
const float conversionFactor = 5.0/1024/1000;
int zeroISenseVADC; // (ADC)
int iSenseVADC;     // (ADC)
float current;      // (A)
int vSenseADC;      // (ADC)
float voltage;      // (V)
float power;        // (W)
float powerA[100];  // (W)
float mx = 0;       // (W)

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);
// Pass oneWire reference to DallasTemperature library
DallasTemperature tempSensors(&oneWire);

// Sense temperatures every second.
NonBlockingTask tempUpdate(1000);
// Sense current every millisecond.
NonBlockingTask iSenseUpdate(1);
// Sense voltage every 10 milliseconds.
NonBlockingTask vSenseUpdate(10);
// Sense power every 100 milliseconds.
NonBlockingTask pSenseUpdate(100);

// Moving average uses last avgCount samples.
RunningAverage movAvgCurrent(avgCount);

void configureTempSensors() {
  // Start up the DallasTemperature library
  tempSensors.begin();
  // Locate devices on the bus
  deviceCount = tempSensors.getDeviceCount();
}

void senseTemperatures() {
  // Send command to all the sensors for temperature conversion
  tempSensors.requestTemperatures();
  // Get temperature from each sensor
  for (senseSamples = 0; senseSamples < deviceCount; senseSamples++) {
    tempC[senseSamples] = tempSensors.getTempCByIndex(senseSamples);
  }
}

// Get offset current, I_IS(offset).
void senseZeroCurrent() {
  for (senseSamples = 0; senseSamples < 5; senseSamples++) {
    zeroISenseVADC += analogRead(IS_1);
  }
  zeroISenseVADC /= 5;
}

// Get sense current, I_IS.
void senseCurrent() {
  movAvgCurrent.addValue(analogReadFast(IS_1));
  sumCounter++;
  if (sumCounter != 65535) { 
    iSenseVADC = movAvgCurrent.getFastAverage();
  } else {
    // Update MA internal sum to prevent accumulating errors.
    iSenseVADC = movAvgCurrent.getAverage();
  }
  current = conversionFactor * (iSenseVADC - zeroISenseVADC) * 19500;
}

void senseVoltage() {
  for (senseSamples = 0; senseSamples < 5; senseSamples++) {
    vSenseADC += analogReadFast(VBAT);
  }
  vSenseADC /= 5;
  voltage = vSenseADC * (44.8/1024);
}

void sensePower() {
  power = voltage * current;
}

void trackMPPT() {
	/*
    power array with last 100 values of power
    find max out of last 100 samples of power array
    if power > max, increase PWM
    if power == max, keep PWM output stable
    if power < max && powerChange = -1, decrease PWM
    if power < max && powerChange = 1, increase PWM

    lastPower = powerA[98], power = powerA[99];
    if power > lastPower, powerChange = 1;
    if power = lastPower, powerChange = 0;
    if power < lastPower, powerChange = -1;
    }
  */

	// problem: check if array contains less than 100 data points

	for (int i : powerA) {
	    mx = max(mx, i);
  }
  if (power > mx) {
      // increase PWM
  } else if (power == mx) {
      // keep stable
  } else {
    // if powerChange = -1
    if (powerA[99] < powerA[98]) {
      // decrease PWM
    } else {
      // increase PWM
    }
  }
}