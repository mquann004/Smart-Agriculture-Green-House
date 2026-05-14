#include "SensorManager.h"

DHT SensorManager::_dht(DHT_PIN, DHT22);
BH1750 SensorManager::_lightMeter;
bool SensorManager::_dhtInitialized = false;

void SensorManager::initialize() {
    _dht.begin();
    _dhtInitialized = true;
    Wire.begin();
    _lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
}

float SensorManager::getTemperature() {
    if (!_dhtInitialized) return 25.0;
    float temp = _dht.readTemperature();
    return isnan(temp) ? 25.0 : temp;
}

float SensorManager::getHumidity() {
    if (!_dhtInitialized) return 60.0;
    float humi = _dht.readHumidity();
    return isnan(humi) ? 60.0 : humi;
}

int SensorManager::getSoilMoisture() { return analogRead(SOIL_PIN); }
float SensorManager::getLightLevel() { return _lightMeter.readLightLevel(); }
int SensorManager::getCO2Level() { return analogRead(CO2_PIN); }
