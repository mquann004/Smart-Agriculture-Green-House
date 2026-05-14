#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// ===== SENSOR DATA STRUCT =====
struct SensorData {
  float temperature;
  float humidity;
  int soilMoisture;
  float lightLevel;
  int co2Level;
  unsigned long timestamp;
};

// ===== HISTORY MANAGER =====
// Lưu trữ và đọc lịch sử dữ liệu cảm biến trên SPIFFS
class HistoryManager {
public:
  static void initialize();
  static void saveData();
  static String getHistoryData(int maxRecords = 100);
};

#endif
