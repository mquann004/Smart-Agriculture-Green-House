#include "AutomationManager.h"
#include "globals.h"

SensorThresholds AutomationManager::_thresholds;
unsigned long AutomationManager::_lastUpdate = 0;
String AutomationManager::_lightMessage = "Ánh sáng bình thường";
bool AutomationManager::_forceUpdate = false;

void AutomationManager::initialize() {
    // GÁN GIÁ TRỊ NGƯỠNG MẶC ĐỊNH NGAY KHI KHỞI TẠO
    _thresholds.tempHigh = 30.0;
    _thresholds.tempLow = 27.0;
    _thresholds.soilDry = 4000;
    _thresholds.soilWet = 3000;
    _thresholds.lightDark = 50;
    _thresholds.lightBright = 300;
    _thresholds.co2High = 1000;
    _thresholds.co2Low = 400;
    _lastUpdate = millis();
    _forceUpdate = false;
    // Set callback cho AutoModeManager
    AutoModeManager::getInstance().setTimeoutCallback([]() {
        Serial.println("Auto mode resumed after timeout");
    });
}

void AutomationManager::update() {
    unsigned long currentTime = millis();
    if (currentTime - _lastUpdate >= SENSOR_READ_INTERVAL || _forceUpdate) {
        if (AutoModeManager::getInstance().isAutoMode()) {
            checkTemperatureControl();
            checkSoilMoistureControl();
            checkLightControl();
            checkCO2Control();
        }
        HistoryManager::saveData(); // Lưu dữ liệu cảm biến
        _lastUpdate = currentTime;
        _forceUpdate = false; // Reset flag after update
    }

    // Update AutoModeManager
    AutoModeManager::getInstance().update();
}

void AutomationManager::setAutoMode(bool enabled) {
    if (enabled) {
        AutoModeManager::getInstance().setAutoMode();
    } else {
        AutoModeManager::getInstance().setManualMode();
    }
    Serial.println("Chuyen sang che do: " + String(enabled ? "AUTO" : "MANUAL"));
}

bool AutomationManager::isAutoMode() { return AutoModeManager::getInstance().isAutoMode(); }
String AutomationManager::getLightMessage() { return _lightMessage; }
SensorThresholds AutomationManager::getThresholds() { return _thresholds; }
void AutomationManager::setThresholds(const SensorThresholds& t) { _thresholds = t; }

// ===== PRIVATE: CONTROL LOGIC =====

void AutomationManager::checkTemperatureControl() {
    float temp = SensorManager::getTemperature();
    if (temp > _thresholds.tempHigh) {
        if (!DeviceManager::getFanState()) {
            DeviceManager::fanOn();
            Serial.println("AUTO: Bat quat do nhiet do cao");
        }
    } else if (temp < _thresholds.tempLow) {
        // Chỉ tắt quạt nếu nhiệt độ thấp VÀ CO2 trong ngưỡng
        int co2 = SensorManager::getCO2Level();
        if (DeviceManager::getFanState() && co2 >= _thresholds.co2Low && co2 <= _thresholds.co2High) {
            DeviceManager::fanOff();
            Serial.println("AUTO: Tat quat do nhiet do thap va CO2 trong nguong");
        }
    }
}

void AutomationManager::checkSoilMoistureControl() {
    int soil = SensorManager::getSoilMoisture();
    if (soil > _thresholds.soilDry && !DeviceManager::getPumpState()) {
        DeviceManager::pumpOn();
        Serial.println("AUTO: Bat bom do dat kho");
    } else if (soil < _thresholds.soilWet && DeviceManager::getPumpState()) {
        DeviceManager::pumpOff();
        Serial.println("AUTO: Tat bom do dat du am");
    }
}

void AutomationManager::checkLightControl() {
    timeClient.update();
    int currentHour = timeClient.getHours();
    if (currentHour < 6 || currentHour >= 18) {
        _lightMessage = "Ngoài giờ quang hợp, cảm biến ánh sáng tắt";
        if (DeviceManager::getLightState()) {
            DeviceManager::lightOff();
            Serial.println("AUTO: Tắt đèn vì ngoài giờ quang hợp");
        }
        return;
    }
    float light = SensorManager::getLightLevel();
    if (light < _thresholds.lightDark) {
        _lightMessage = "Ánh sáng quá thấp, yêu cầu bật đèn";
        if (!DeviceManager::getLightState()) {
            DeviceManager::lightOn();
            Serial.println("AUTO: Bat den do troi toi");
        }
    } else if (light > _thresholds.lightBright) {
        _lightMessage = "Ánh sáng quá cao, vui lòng tắt đèn";
        if (DeviceManager::getLightState()) {
            DeviceManager::lightOff();
            Serial.println("AUTO: Tat den do du sang");
        }
    } else {
        _lightMessage = "Ánh sáng bình thường";
    }
}

void AutomationManager::checkCO2Control() {
    int co2 = SensorManager::getCO2Level();
    if (co2 > _thresholds.co2High || co2 < _thresholds.co2Low) {
        if (!DeviceManager::getFanState()) {
            DeviceManager::fanOn();
            Serial.println("AUTO: Bat quat do CO2 ngoai nguong");
        }
    } else {
        // Chỉ tắt quạt nếu CO2 trong ngưỡng VÀ nhiệt độ cũng trong ngưỡng
        float temp = SensorManager::getTemperature();
        if (DeviceManager::getFanState() && temp >= _thresholds.tempLow && temp <= _thresholds.tempHigh) {
            DeviceManager::fanOff();
            Serial.println("AUTO: Tat quat do CO2 va nhiet do trong nguong");
        }
    }
}
