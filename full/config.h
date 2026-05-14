#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ===== PIN DEFINITIONS =====
#define DHT_PIN 4
#define FAN_PIN 26
#define PUMP_PIN 25
#define LIGHT_PIN 13
#define SOIL_PIN 34
#define CO2_PIN 35

// ===== TIMING =====
#define SENSOR_READ_INTERVAL 5000

// ===== SENSOR THRESHOLDS =====
struct SensorThresholds {
  float tempHigh = 30.0;
  float tempLow = 27.0;
  int soilDry = 4000;
  int soilWet = 3000;
  int lightDark = 50;
  int lightBright = 300;
  int co2High = 1000;
  int co2Low = 400;
};

#endif
