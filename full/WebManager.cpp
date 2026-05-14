#include "WebManager.h"
#include "page_dashboard.h"
#include "page_history.h"

WebServer WebManager::_server(80);

// ===== KHỞI TẠO SERVER VÀ CÁC ROUTE =====
void WebManager::initialize() {
    _server.on("/", handleRoot);
    _server.on("/data", handleData);
    _server.on("/mode", handleMode);
    _server.on("/device", handleDeviceControl);
    _server.on("/api/sensor", handleApiSensor);
    _server.on("/api/command", HTTP_POST, handleApiCommand);
    _server.on("/api/thresholds", HTTP_GET, handleApiThresholdsGet);
    _server.on("/api/thresholds", HTTP_POST, handleApiThresholdsPost);
    _server.on("/api/plant", HTTP_GET, handleApiPlantGet);
    _server.on("/api/plant", HTTP_POST, handleApiPlantPost);
    _server.on("/api/ai_command", HTTP_POST, handleApiAiCommand);
    _server.on("/bulkControl", handleBulkControl);
    _server.on("/tool", HTTP_POST, handleTool);
    _server.on("/history", handleHistory);
    _server.on("/api/history", handleApiHistory);
    _server.onNotFound(handleNotFound);

    _server.begin();
    Serial.println("Web server da khoi dong!");
}

void WebManager::handleClient() {
    _server.handleClient();
}

// ===== PAGE HANDLERS =====

void WebManager::handleRoot() {
    _server.send(200, "text/html; charset=UTF-8", PAGE_DASHBOARD);
}

void WebManager::handleHistory() {
    _server.send(200, "text/html; charset=UTF-8", PAGE_HISTORY);
}

// ===== DATA API HANDLERS =====

void WebManager::handleData() {
    StaticJsonDocument<512> doc;
    doc["temperature"] = SensorManager::getTemperature();
    doc["humidity"] = SensorManager::getHumidity();
    doc["soilMoisture"] = SensorManager::getSoilMoisture();
    doc["lightLevel"] = SensorManager::getLightLevel();
    doc["co2Level"] = SensorManager::getCO2Level();
    doc["lightMessage"] = AutomationManager::getLightMessage();
    doc["fan"] = DeviceManager::getFanState();
    doc["pump"] = DeviceManager::getPumpState();
    doc["light"] = DeviceManager::getLightState();
    doc["autoMode"] = AutomationManager::isAutoMode();
    doc["aiCommand"] = lastAiCommand;

    String json;
    serializeJson(doc, json);
    _server.send(200, "application/json", json);
}

void WebManager::handleApiHistory() {
    String json = HistoryManager::getHistoryData();
    _server.send(200, "application/json", json);
}

void WebManager::handleApiSensor() {
    StaticJsonDocument<1024> doc;
    doc["temperature"] = SensorManager::getTemperature();
    doc["humidity"] = SensorManager::getHumidity();
    doc["soilMoisture"] = SensorManager::getSoilMoisture();
    doc["lightLevel"] = SensorManager::getLightLevel();
    doc["co2Level"] = SensorManager::getCO2Level();
    doc["lightMessage"] = AutomationManager::getLightMessage();
    doc["fan"] = DeviceManager::getFanState();
    doc["pump"] = DeviceManager::getPumpState();
    doc["light"] = DeviceManager::getLightState();
    doc["autoMode"] = AutomationManager::isAutoMode();

    SensorThresholds thresholds = AutomationManager::getThresholds();
    doc["tempHigh"] = thresholds.tempHigh;
    doc["tempLow"] = thresholds.tempLow;
    doc["soilDry"] = thresholds.soilDry;
    doc["soilWet"] = thresholds.soilWet;
    doc["lightDark"] = thresholds.lightDark;
    doc["lightBright"] = thresholds.lightBright;
    doc["co2High"] = thresholds.co2High;
    doc["co2Low"] = thresholds.co2Low;

    String json;
    serializeJson(doc, json);
    _server.send(200, "application/json", json);
}

// ===== CONTROL HANDLERS =====

void WebManager::handleMode() {
    if (_server.hasArg("mode")) {
        String mode = _server.arg("mode");
        AutomationManager::setAutoMode(mode == "auto");
        _server.send(200, "text/plain", "OK");
    }
}

void WebManager::handleDeviceControl() {
    if (_server.hasArg("device") && _server.hasArg("action")) {
        String device = _server.arg("device");
        String action = _server.arg("action");
        AutomationManager::setAutoMode(false); // Tự động chuyển sang manual
        if (device == "fan") { if (action == "on") DeviceManager::fanOn(); else DeviceManager::fanOff(); }
        else if (device == "pump") { if (action == "on") DeviceManager::pumpOn(); else DeviceManager::pumpOff(); }
        else if (device == "light") { if (action == "on") DeviceManager::lightOn(); else DeviceManager::lightOff(); }
        _server.send(200, "text/plain", "OK");
    }
}

void WebManager::handleApiCommand() {
    if (!_server.hasArg("plain")) { _server.send(400, "application/json", "{\"error\":\"Missing body\"}"); return; }
    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, _server.arg("plain"))) { _server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}"); return; }
    const char* device = doc["device"];
    const char* action = doc["action"];
    if (!device || !action) { _server.send(400, "application/json", "{\"error\":\"Missing device or action\"}"); return; }

    AutomationManager::setAutoMode(false); // Tự động chuyển sang manual
    String deviceStr = String(device);
    String actionStr = String(action);
    if (deviceStr == "fan") { if (actionStr == "on") DeviceManager::fanOn(); else if (actionStr == "off") DeviceManager::fanOff(); }
    else if (deviceStr == "pump") { if (actionStr == "on") DeviceManager::pumpOn(); else if (actionStr == "off") DeviceManager::pumpOff(); }
    else if (deviceStr == "light") { if (actionStr == "on") DeviceManager::lightOn(); else if (actionStr == "off") DeviceManager::lightOff(); }
    else { _server.send(400, "application/json", "{\"error\":\"Invalid device\"}"); return; }
    _server.send(200, "application/json", "{\"status\":\"OK\"}");
}

void WebManager::handleBulkControl() {
    AutomationManager::setAutoMode(false);
    bool handled = false;
    if (_server.hasArg("fan")) { String val = _server.arg("fan"); if (val == "on") DeviceManager::fanOn(); else if (val == "off") DeviceManager::fanOff(); handled = true; }
    if (_server.hasArg("pump")) { String val = _server.arg("pump"); if (val == "on") DeviceManager::pumpOn(); else if (val == "off") DeviceManager::pumpOff(); handled = true; }
    if (_server.hasArg("light")) { String val = _server.arg("light"); if (val == "on") DeviceManager::lightOn(); else if (val == "off") DeviceManager::lightOff(); handled = true; }
    if (handled) { _server.send(200, "text/plain", "OK"); }
    else { _server.send(400, "text/plain", "No valid parameters"); }
}

// ===== THRESHOLD HANDLERS =====

void WebManager::handleApiThresholdsGet() {
    StaticJsonDocument<256> doc;
    SensorThresholds thresholds = AutomationManager::getThresholds();
    doc["tempHigh"] = thresholds.tempHigh;
    doc["tempLow"] = thresholds.tempLow;
    doc["soilDry"] = thresholds.soilDry;
    doc["soilWet"] = thresholds.soilWet;
    doc["lightDark"] = thresholds.lightDark;
    doc["lightBright"] = thresholds.lightBright;
    doc["co2High"] = thresholds.co2High;
    doc["co2Low"] = thresholds.co2Low;

    String json;
    serializeJson(doc, json);
    _server.send(200, "application/json", json);
}

void WebManager::handleApiThresholdsPost() {
    if (!_server.hasArg("plain")){
        _server.send(400, "application/json", "{\"error\":\"Missing body\"}");
        return;
    }

    String body = _server.arg("plain");
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        _server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    // Lấy một bản sao của các ngưỡng hiện tại để không làm mất giá trị cũ nếu JSON thiếu
    SensorThresholds updatedThresholds = AutomationManager::getThresholds();

    // Cập nhật các giá trị từ JSON nhận được một cách an toàn
    if (doc.containsKey("tempHigh"))    updatedThresholds.tempHigh    = doc["tempHigh"].as<float>();
    if (doc.containsKey("tempLow"))     updatedThresholds.tempLow     = doc["tempLow"].as<float>();
    if (doc.containsKey("soilDry"))     updatedThresholds.soilDry     = doc["soilDry"].as<int>();
    if (doc.containsKey("soilWet"))     updatedThresholds.soilWet     = doc["soilWet"].as<int>();
    if (doc.containsKey("lightDark"))   updatedThresholds.lightDark   = doc["lightDark"].as<int>();
    if (doc.containsKey("lightBright")) updatedThresholds.lightBright = doc["lightBright"].as<int>();
    if (doc.containsKey("co2High"))     updatedThresholds.co2High     = doc["co2High"].as<int>();
    if (doc.containsKey("co2Low"))      updatedThresholds.co2Low      = doc["co2Low"].as<int>();

    AutomationManager::setThresholds(updatedThresholds);
    AutomationManager::update();

    Serial.println("--- NGUONG MOI DA DUOC CAP NHAT ---");
    _server.send(200, "application/json", "{\"status\":\"thresholds updated\"}");
}

// ===== PLANT HANDLERS =====

void WebManager::handleApiPlantGet() {
    _server.send(200, "application/json", "{\"plant\":\"" + currentPlant + "\"}");
}

void WebManager::handleApiPlantPost() {
    if (!_server.hasArg("plain")) { _server.send(400, "application/json", "{\"error\":\"Missing body\"}"); return; }
    StaticJsonDocument<100> doc;
    if (deserializeJson(doc, _server.arg("plain"))) { _server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}"); return; }
    if (doc.containsKey("plant")) {
        currentPlant = doc["plant"].as<String>();
        _server.send(200, "application/json", "{\"status\":\"plant updated\"}");
    } else {
        _server.send(400, "application/json", "{\"error\":\"Missing plant\"}");
    }
}

// ===== AI HANDLERS =====

void WebManager::handleApiAiCommand() {
    if (!_server.hasArg("plain")) { _server.send(400, "application/json", "{\"error\":\"Missing body\"}"); return; }
    StaticJsonDocument<200> doc;
    if (deserializeJson(doc, _server.arg("plain"))) { _server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}"); return; }
    const char* device = doc["device"];
    const char* action = doc["action"];
    if (!device || !action) { _server.send(400, "application/json", "{\"error\":\"Missing device or action\"}"); return; }

    String deviceStr = String(device);
    String actionStr = String(action);
    if (deviceStr == "fan") { if (actionStr == "on") DeviceManager::fanOn(); else if (actionStr == "off") DeviceManager::fanOff(); }
    else if (deviceStr == "pump") { if (actionStr == "on") DeviceManager::pumpOn(); else if (actionStr == "off") DeviceManager::pumpOff(); }
    else if (deviceStr == "light") { if (actionStr == "on") DeviceManager::lightOn(); else if (actionStr == "off") DeviceManager::lightOff(); }
    else { _server.send(400, "application/json", "{\"error\":\"Invalid device\"}"); return; }
    lastAiCommand = "Lệnh từ AI: " + deviceStr + " " + actionStr;
    _server.send(200, "application/json", "{\"status\":\"OK\"}");
}

void WebManager::handleTool() {
    if (!_server.hasArg("plain")) { _server.send(400, "application/json", "{\"error\":\"Missing body\"}"); return; }
    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, _server.arg("plain"))) { _server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}"); return; }

    AutomationManager::setAutoMode(false);
    AutoModeManager::getInstance().resetTimer();

    if (doc.containsKey("fan")) { String val = doc["fan"]; if (val == "on") DeviceManager::fanOn(); else if (val == "off") DeviceManager::fanOff(); }
    if (doc.containsKey("pump")) { String val = doc["pump"]; if (val == "on") DeviceManager::pumpOn(); else if (val == "off") DeviceManager::pumpOff(); }
    if (doc.containsKey("light")) { String val = doc["light"]; if (val == "on") DeviceManager::lightOn(); else if (val == "off") DeviceManager::lightOff(); }

    if (doc.containsKey("thresholds")) {
        JsonObject thresholds = doc["thresholds"];
        SensorThresholds newThresholds = AutomationManager::getThresholds();
        if (thresholds.containsKey("tempHigh")) newThresholds.tempHigh = thresholds["tempHigh"];
        if (thresholds.containsKey("tempLow")) newThresholds.tempLow = thresholds["tempLow"];
        if (thresholds.containsKey("soilDry")) newThresholds.soilDry = thresholds["soilDry"];
        if (thresholds.containsKey("soilWet")) newThresholds.soilWet = thresholds["soilWet"];
        if (thresholds.containsKey("lightDark")) newThresholds.lightDark = thresholds["lightDark"];
        if (thresholds.containsKey("lightBright")) newThresholds.lightBright = thresholds["lightBright"];
        AutomationManager::setThresholds(newThresholds);
        AutomationManager::update();
    }

    StaticJsonDocument<256> responseDoc;
    responseDoc["autoMode"] = AutomationManager::isAutoMode();
    responseDoc["timeout"] = AutoModeManager::getInstance().getRemainingTime();
    responseDoc["fan"] = DeviceManager::getFanState();
    responseDoc["pump"] = DeviceManager::getPumpState();
    responseDoc["light"] = DeviceManager::getLightState();
    String jsonResponse;
    serializeJson(responseDoc, jsonResponse);
    _server.send(200, "application/json", jsonResponse);
}

// ===== ERROR HANDLER =====

void WebManager::handleNotFound() {
    _server.send(404, "text/plain", "Not Found");
}
