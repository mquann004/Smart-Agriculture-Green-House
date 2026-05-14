// ===== SMART GREENHOUSE - MAIN =====
// Tất cả logic đã được tách thành các thư viện riêng biệt.
// File này chỉ chứa: includes, biến toàn cục, setup() và loop().

#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "config.h"
#include "globals.h"
#include "AutoModeManager.h"
#include "DeviceManager.h"
#include "SensorManager.h"
#include "HistoryManager.h"
#include "AutomationManager.h"
#include "WebManager.h"

// ===== GLOBAL VARIABLE DEFINITIONS =====
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000); // UTC+7 cho Việt Nam

String currentPlant = "Tomato";
String lastAiCommand = "Chờ lệnh...";

const char* WIFI_SSID = ".";
const char* WIFI_PASSWORD = "12345678";

IPAddress local_IP(192, 168, 137, 191);
IPAddress gateway_IP(192, 168, 137, 1);
IPAddress subnet(255, 255, 255, 0);

// ===== SETUP =====
void setup() {
    Serial.begin(115200);
    Serial.println("Smart Greenhouse - Khoi dong...");

    // Khởi tạo AutoModeManager trước
    AutoModeManager::getInstance().initialize();

    HistoryManager::initialize();
    SensorManager::initialize();
    DeviceManager::initialize();
    AutomationManager::initialize();

    WiFi.config(local_IP, gateway_IP, subnet);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Dang ket noi WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nDa ket noi WiFi!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    timeClient.begin();
    timeClient.update();
    Serial.println("NTP time synchronized");

    WebManager::initialize();
    Serial.println("Smart Greenhouse da khoi dong!");
}

// ===== LOOP =====
void loop() {
    WebManager::handleClient();
    AutomationManager::update();
    delay(1000);
}