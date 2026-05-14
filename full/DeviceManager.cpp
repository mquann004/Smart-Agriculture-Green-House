#include "DeviceManager.h"

bool DeviceManager::_fanState = false;
bool DeviceManager::_pumpState = false;
bool DeviceManager::_lightState = false;

void DeviceManager::initialize() {
    pinMode(FAN_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(LIGHT_PIN, OUTPUT);
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LIGHT_PIN, LOW);
}

void DeviceManager::fanOn() { digitalWrite(FAN_PIN, HIGH); _fanState = true; }
void DeviceManager::fanOff() { digitalWrite(FAN_PIN, LOW); _fanState = false; }
void DeviceManager::pumpOn() { digitalWrite(PUMP_PIN, HIGH); _pumpState = true; }
void DeviceManager::pumpOff() { digitalWrite(PUMP_PIN, LOW); _pumpState = false; }
void DeviceManager::lightOn() { digitalWrite(LIGHT_PIN, HIGH); _lightState = true; }
void DeviceManager::lightOff() { digitalWrite(LIGHT_PIN, LOW); _lightState = false; }

bool DeviceManager::getFanState() { return _fanState; }
bool DeviceManager::getPumpState() { return _pumpState; }
bool DeviceManager::getLightState() { return _lightState; }
