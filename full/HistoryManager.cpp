#include "HistoryManager.h"
#include "SensorManager.h"

void HistoryManager::initialize() {
    if (!SPIFFS.begin(true)) {
      Serial.println("Failed to mount SPIFFS");
      return;
    }
    Serial.println("SPIFFS initialized");
}

void HistoryManager::saveData() {
    SensorData data;
    data.temperature = SensorManager::getTemperature();
    data.humidity = SensorManager::getHumidity();
    data.soilMoisture = SensorManager::getSoilMoisture();
    data.lightLevel = SensorManager::getLightLevel();
    data.co2Level = SensorManager::getCO2Level();
    data.timestamp = millis(); // Hoặc sử dụng thời gian thực nếu có RTC

    File file = SPIFFS.open("/sensor_history.json", FILE_APPEND);
    if (!file) {
      Serial.println("Failed to open file for appending");
      return;
    }

    StaticJsonDocument<200> doc;
    doc["temperature"] = data.temperature;
    doc["humidity"] = data.humidity;
    doc["soilMoisture"] = data.soilMoisture;
    doc["lightLevel"] = data.lightLevel;
    doc["co2Level"] = data.co2Level;
    doc["timestamp"] = data.timestamp;

    String json;
    serializeJson(doc, json);
    file.println(json);
    file.close();
    Serial.println("Saved sensor data to history");
}

String HistoryManager::getHistoryData(int maxRecords) {
    File file = SPIFFS.open("/sensor_history.json", FILE_READ);
    if (!file) {
      Serial.println("Failed to open history file");
      return "[]";
    }

    StaticJsonDocument<1024> doc;
    JsonArray array = doc.to<JsonArray>();
    int count = 0;

    while (file.available() && count < maxRecords) {
      String line = file.readStringUntil('\n');
      StaticJsonDocument<200> entry;
      if (deserializeJson(entry, line) == DeserializationError::Ok) {
        array.add(entry);
        count++;
      }
    }
    file.close();

    String json;
    serializeJson(array, json);
    return json;
}
