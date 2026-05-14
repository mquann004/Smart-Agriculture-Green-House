#ifndef AUTOMATION_MANAGER_H
#define AUTOMATION_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "AutoModeManager.h"
#include "SensorManager.h"
#include "DeviceManager.h"
#include "HistoryManager.h"

// ===== AUTOMATION MANAGER =====
// Logic tự động hóa: kiểm tra cảm biến và điều khiển thiết bị
class AutomationManager {
public:
    static void initialize();
    static void update();
    static void setAutoMode(bool enabled);
    static bool isAutoMode();
    static String getLightMessage();
    static SensorThresholds getThresholds();
    static void setThresholds(const SensorThresholds& t);

private:
    static void checkTemperatureControl();
    static void checkSoilMoistureControl();
    static void checkLightControl();
    static void checkCO2Control();

    static SensorThresholds _thresholds;
    static unsigned long _lastUpdate;
    static String _lightMessage;
    static bool _forceUpdate;
};

#endif
