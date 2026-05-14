#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "globals.h"
#include "AutoModeManager.h"
#include "DeviceManager.h"
#include "SensorManager.h"
#include "HistoryManager.h"
#include "AutomationManager.h"

// ===== WEB MANAGER =====
// Web server và tất cả REST API handlers
class WebManager {
public:
    static void initialize();
    static void handleClient();

private:
    static WebServer _server;

    // Page handlers
    static void handleRoot();
    static void handleHistory();

    // Data API handlers
    static void handleData();
    static void handleApiHistory();
    static void handleApiSensor();

    // Control handlers
    static void handleMode();
    static void handleDeviceControl();
    static void handleApiCommand();
    static void handleBulkControl();

    // Threshold handlers
    static void handleApiThresholdsGet();
    static void handleApiThresholdsPost();

    // Plant handlers
    static void handleApiPlantGet();
    static void handleApiPlantPost();

    // AI handlers
    static void handleApiAiCommand();
    static void handleTool();

    // Error handler
    static void handleNotFound();
};

#endif
