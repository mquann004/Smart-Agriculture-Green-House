#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include "config.h"

// ===== SENSOR MANAGER =====
// Đọc dữ liệu từ tất cả cảm biến
class SensorManager {
public:
    static void initialize();

    static float getTemperature();
    static float getHumidity();
    static int getSoilMoisture();
    static float getLightLevel();
    static int getCO2Level();

private:
    static DHT _dht;
    static BH1750 _lightMeter;
    static bool _dhtInitialized;
};

#endif
