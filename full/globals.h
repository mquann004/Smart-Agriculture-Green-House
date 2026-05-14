#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// ===== GLOBAL VARIABLE DECLARATIONS =====
// Actual definitions are in full.ino

extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

extern String currentPlant;
extern String lastAiCommand;

extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;

extern IPAddress local_IP;
extern IPAddress gateway_IP;
extern IPAddress subnet;

#endif
