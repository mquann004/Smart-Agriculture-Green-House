#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <time.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 25200, 60000); // UTC+7 cho Việt Nam

String currentPlant = "Tomato";
String lastAiCommand = "Chờ lệnh...";

// ===== AUTO MODE MANAGER =====
class AutoModeManager {
public:
    static AutoModeManager& getInstance() {
        static AutoModeManager instance;
        return instance;
    }

    void initialize() {
        _autoMode = true;
        _lastManualTime = 0;
        _timeoutDuration = 30 * 1000; // 30 giây
        Serial.println("AutoModeManager initialized");
    }

    void setManualMode() {
        _autoMode = false;
        _lastManualTime = millis();
        Serial.println("Switched to MANUAL mode - Auto mode will resume in 30 seconds");
    }

    void setAutoMode() {
        _autoMode = true;
        _lastManualTime = 0;
        Serial.println("Switched to AUTO mode");
    }

    void resetTimer() {
        if (!_autoMode) {
            _lastManualTime = millis();
            Serial.println("Manual mode timer reset");
        }
    }

    void update() {
        if (!_autoMode && _lastManualTime > 0) {
            unsigned long currentTime = millis();
            if (currentTime - _lastManualTime >= _timeoutDuration) {
                setAutoMode();
                if (_timeoutCallback) {
                    _timeoutCallback();
                }
            }
        }
    }

    bool isAutoMode() const { return _autoMode; }

    void setTimeoutCallback(std::function<void()> callback) {
        _timeoutCallback = callback;
    }

    unsigned long getRemainingTime() const {
        if (_autoMode || _lastManualTime == 0) return 0;
        unsigned long elapsed = millis() - _lastManualTime;
        return elapsed >= _timeoutDuration ? 0 : (_timeoutDuration - elapsed) / 1000;
    }

private:
    AutoModeManager() = default;
    ~AutoModeManager() = default;

    bool _autoMode = true;
    unsigned long _lastManualTime = 0;
    unsigned long _timeoutDuration = 30000; // 30 giây
    std::function<void()> _timeoutCallback = nullptr;
};

// ===== CONFIGURATION =====
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define FAN_PIN 26
#define PUMP_PIN 25
#define LIGHT_PIN 13
#define SOIL_PIN 34
#define CO2_PIN 35

const char* WIFI_SSID = ".";
const char* WIFI_PASSWORD = "12345678";

// Static IP configuration
IPAddress local_IP(192, 168, 137, 191);
IPAddress gateway(192, 168, 137, 1);
IPAddress subnet(255, 255, 255, 0);

struct SensorThresholds {
  float tempHigh = 30.0;
  float tempLow = 27.0;
  int soilDry = 4000;
  int soilWet = 3000;
  int lightDark = 50;
  int lightBright = 300;
  int co2High = 1000;
  int co2Low = 400;
};

#define SENSOR_READ_INTERVAL 5000

// ===== DEVICE MANAGER =====
class DeviceManager {
public:
    static void initialize() {
        pinMode(FAN_PIN, OUTPUT);
        pinMode(PUMP_PIN, OUTPUT);
        pinMode(LIGHT_PIN, OUTPUT);
        digitalWrite(FAN_PIN, LOW);
        digitalWrite(PUMP_PIN, LOW);
        digitalWrite(LIGHT_PIN, LOW);
    }

    static void fanOn() { digitalWrite(FAN_PIN, HIGH); _fanState = true; }
    static void fanOff() { digitalWrite(FAN_PIN, LOW); _fanState = false; }
    static void pumpOn() { digitalWrite(PUMP_PIN, HIGH); _pumpState = true; }
    static void pumpOff() { digitalWrite(PUMP_PIN, LOW); _pumpState = false; }
    static void lightOn() { digitalWrite(LIGHT_PIN, HIGH); _lightState = true; }
    static void lightOff() { digitalWrite(LIGHT_PIN, LOW); _lightState = false; }

    static bool getFanState() { return _fanState; }
    static bool getPumpState() { return _pumpState; }
    static bool getLightState() { return _lightState; }

private:
    static bool _fanState;
    static bool _pumpState;
    static bool _lightState;
};

bool DeviceManager::_fanState = false;
bool DeviceManager::_pumpState = false;
bool DeviceManager::_lightState = false;

// ===== SENSOR MANAGER =====
class SensorManager {
public:
    static void initialize() {
        _dht.begin();
        _dhtInitialized = true;
        Wire.begin();
        _lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
    }

    static float getTemperature() {
        if (!_dhtInitialized) return 25.0;
        float temp = _dht.readTemperature();
        return isnan(temp) ? 25.0 : temp;
    }

    static float getHumidity() {
        if (!_dhtInitialized) return 60.0;
        float humi = _dht.readHumidity();
        return isnan(humi) ? 60.0 : humi;
    }

    static int getSoilMoisture() { return analogRead(SOIL_PIN); }
    static float getLightLevel() { return _lightMeter.readLightLevel(); }
    static int getCO2Level() { return analogRead(CO2_PIN); }

private:
    static DHT _dht;
    static BH1750 _lightMeter;
    static bool _dhtInitialized;
};

DHT SensorManager::_dht(DHT_PIN, DHT_TYPE);
BH1750 SensorManager::_lightMeter;
bool SensorManager::_dhtInitialized = false;

// ===== HISTORY MANAGER =====
struct SensorData {
  float temperature;
  float humidity;
  int soilMoisture;
  float lightLevel;
  int co2Level;
  unsigned long timestamp;
};

class HistoryManager {
public:
  static void initialize() {
    if (!SPIFFS.begin(true)) {
      Serial.println("Failed to mount SPIFFS");
      return;
    }
    Serial.println("SPIFFS initialized");
  }

  static void saveData() {
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

  static String getHistoryData(int maxRecords = 100) {
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
};

// ===== AUTOMATION MANAGER =====
class AutomationManager {
public:
    static void initialize() {
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

    static void update() {
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

    static void setAutoMode(bool enabled) {
        if (enabled) {
            AutoModeManager::getInstance().setAutoMode();
        } else {
            AutoModeManager::getInstance().setManualMode();
        }
        Serial.println("Chuyen sang che do: " + String(enabled ? "AUTO" : "MANUAL"));
    }

    static bool isAutoMode() { return AutoModeManager::getInstance().isAutoMode(); }
    static String getLightMessage() { return _lightMessage; }

    static SensorThresholds getThresholds() { return _thresholds; }
    static void setThresholds(const SensorThresholds& t) { _thresholds = t; }

private:
    static void checkTemperatureControl() {
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

    static void checkSoilMoistureControl() {
        int soil = SensorManager::getSoilMoisture();
        if (soil > _thresholds.soilDry && !DeviceManager::getPumpState()) {
            DeviceManager::pumpOn();
            Serial.println("AUTO: Bat bom do dat kho");
        } else if (soil < _thresholds.soilWet && DeviceManager::getPumpState()) {
            DeviceManager::pumpOff();
            Serial.println("AUTO: Tat bom do dat du am");
        }
    }

    static void checkLightControl() {
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

    static void checkCO2Control() {
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

    static SensorThresholds _thresholds;
    static unsigned long _lastUpdate;
    static String _lightMessage;
    static bool _forceUpdate;
};

SensorThresholds AutomationManager::_thresholds;
unsigned long AutomationManager::_lastUpdate = 0;
String AutomationManager::_lightMessage = "Ánh sáng bình thường";
bool AutomationManager::_forceUpdate = false;

// ==========================================================================
// ===== WEB MANAGER (PHIÊN BẢN NÂNG CẤP HOÀN CHỈNH - GIỮ NGUYÊN LOGIC) =====
// ==========================================================================
class WebManager {
public:
    // --- KHỞI TẠO SERVER VÀ CÁC ROUTE (GIỮ NGUYÊN TỪ CODE CỦA BẠN) ---
    static void initialize() {
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
        _server.on("/tool", HTTP_POST, handleTool); // Sửa lại: handleTool nên là POST
        _server.on("/history", handleHistory);
        _server.on("/api/history", handleApiHistory);
        _server.onNotFound(handleNotFound);

        _server.begin();
        Serial.println("Web server da khoi dong!");
    }

    // --- HÀM XỬ LÝ KẾT NỐI TỪ CLIENT ---
    static void handleClient() { 
        _server.handleClient(); 
    }

private:
    // --- KHAI BÁO BIẾN SERVER ---
    static WebServer _server;

    // --- TRANG CHỦ (DASHBOARD) - GIAO DIỆN MỚI ---
    static void handleRoot() {
        String html = F(R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Greenhouse</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.1.1/css/all.min.css">
    <style>
        :root{--primary-color:#28a745;--secondary-color:#f8f9fa;--card-bg:#ffffff;--text-color:#333;--border-radius:12px;--box-shadow:0 4px 12px rgba(0,0,0,0.08);}
        body{font-family:system-ui,-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,Oxygen,Ubuntu,Cantarell,'Open Sans','Helvetica Neue',sans-serif;background-color:var(--secondary-color);color:var(--text-color);margin:0;padding:20px;}
        .container{max-width:1200px;margin:auto;}
        header{text-align:center;margin-bottom:20px;color:var(--primary-color);}
        .grid{display:grid;grid-template-columns:2fr 1fr;gap:20px;}
        .card{background:var(--card-bg);border-radius:var(--border-radius);padding:20px;box-shadow:var(--box-shadow);margin-bottom:20px;}
        .card-title{display:flex;align-items:center;margin-top:0;font-size:1.2em;border-bottom:1px solid #eee;padding-bottom:10px;margin-bottom:15px;}
        .card-title i{margin-right:8px;color:var(--primary-color);}
        .sensor-grid{display:grid;grid-template-columns:1fr 1fr;gap:15px;}
        .sensor-item{background:var(--secondary-color);padding:15px;border-radius:8px;text-align:center;}
        .sensor-item .icon{font-size:1.8em;margin-bottom:5px;}
        .sensor-item .temp{color:#ff6347;} .sensor-item .humi{color:#1e90ff;} .sensor-item .soil{color:#8B4513;} .sensor-item .light{color:#FFD700;}
        .sensor-value{font-size:1.5em;font-weight:bold;margin-top:5px;}
        .sensor-alert{margin-top:15px;padding:10px;border-radius:8px;background-color:#fff3cd;color:#856404;text-align:center;display:none;}
        .control-grid, .device-grid{display:flex;flex-direction:column;gap:15px;}
        .control-item{display:flex;align-items:center;padding:10px;border-radius:8px;background:var(--secondary-color);}
        .device-item{display:flex;align-items:center;justify-content:space-between;}
        .device-info{display:flex;align-items:center;gap:10px;}
        .device-status{font-weight:bold;margin-left:auto;margin-right:15px;}
        button.ctrl{padding:8px 16px;border:none;border-radius:5px;cursor:pointer;font-weight:bold;color:white;transition:background-color 0.2s;}
        button.on{background-color:#28a745;} button.off{background-color:#f44336;}
        .toggle-switch{position:relative;display:inline-block;width:50px;height:28px;margin-left:auto;}
        .toggle-switch input{opacity:0;width:0;height:0;}
        .slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#ccc;transition:.4s;border-radius:28px;}
        .slider:before{position:absolute;content:"";height:20px;width:20px;left:4px;bottom:4px;background-color:white;transition:.4s;border-radius:50%;}
        input:checked+.slider{background-color:var(--primary-color);} input:checked+.slider:before{transform:translateX(22px);}
        #mode-text{font-weight:bold;}
        form .form-grid{display:grid;grid-template-columns:1fr 1fr;gap:10px;}
        form label{display:flex;flex-direction:column;font-size:0.9em;color:#555;}
        form input{width:100%;padding:8px;box-sizing:border-box;border:1px solid #ddd;border-radius:5px;margin-top:4px;}
        form button{width:100%;padding:10px;margin-top:15px;background:var(--primary-color);color:white;border:none;border-radius:5px;cursor:pointer;font-size:1em;font-weight:bold;}
        @media (max-width:900px){.grid{grid-template-columns:1fr;}}
        @media (max-width:500px){.sensor-grid{grid-template-columns:1fr;}}
    </style>
</head>
<body>
    <div class="container">
        <header><h1><i class="fas fa-seedling"></i> Smart Greenhouse</h1></header>
        <main class="grid">
            <div class="main-column">
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-tachometer-alt"></i> Cảm biến</h2>
                    <div class="sensor-grid">
                        <div class="sensor-item"><i class="fas fa-thermometer-half icon temp"></i><div>Nhiệt độ</div><div class="sensor-value" id="temp">-- &deg;C</div></div>
                        <div class="sensor-item"><i class="fas fa-tint icon humi"></i><div>Độ ẩm KK</div><div class="sensor-value" id="humi">-- %</div></div>
                        <div class="sensor-item"><i class="fas fa-water icon soil"></i><div>Độ ẩm đất</div><div class="sensor-value" id="soil">--</div></div>
                        <div class="sensor-item"><i class="fas fa-sun icon light"></i><div>Ánh sáng</div><div class="sensor-value" id="lightLux">-- lx</div></div>
                        <div class="sensor-item"><i class="fas fa-wind icon co2"></i><div>CO2</div><div class="sensor-value" id="co2">-- ppm</div></div>
                    </div>
                    <div class="sensor-alert" id="lightMsg"></div>
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-toggle-on"></i> Điều Khiển</h2>
                    <div class="control-item"><span id="mode-text">TỰ ĐỘNG</span><div class="toggle-switch"><input type="checkbox" id="mode-toggle"><label for="mode-toggle" class="slider"></label></div></div>
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-cogs"></i> Thiết bị</h2>
                    <div class="device-grid">
                        <div class="device-item"><div class="device-info"><i class="fas fa-fan icon"></i> Quạt</div> <span id="fan-status">TẮT</span> <div><button class="ctrl on" onclick="controlDevice('fan','on')">Bật</button><button class="ctrl off" onclick="controlDevice('fan','off')">Tắt</button></div></div>
                        <div class="device-item"><div class="device-info"><i class="fas fa-water icon"></i> Bơm</div> <span id="pump-status">TẮT</span> <div><button class="ctrl on" onclick="controlDevice('pump','on')">Bật</button><button class="ctrl off" onclick="controlDevice('pump','off')">Tắt</button></div></div>
                        <div class="device-item"><div class="device-info"><i class="far fa-lightbulb icon"></i> Đèn</div> <span id="light-status">TẮT</span> <div><button class="ctrl on" onclick="controlDevice('light','on')">Bật</button><button class="ctrl off" onclick="controlDevice('light','off')">Tắt</button></div></div>
                    </div>
                </div>
            </div>
            <div class="side-column">
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-sliders"></i> Ngưỡng cảm biến</h2>
                    <form onsubmit="saveThresholds(event)">
                        <div class="form-grid">
                            <label>Nhiệt độ cao<input type="number" id="tempHigh" step="0.1"></label><label>Nhiệt độ thấp<input type="number" id="tempLow" step="0.1"></label>
                            <label>Đất khô<input type="number" id="soilDry"></label><label>Đất ướt<input type="number" id="soilWet"></label>
                            <label>Sáng yếu<input type="number" id="lightDark"></label><label>Sáng mạnh<input type="number" id="lightBright"></label>
                            <label>CO2 cao<input type="number" id="co2High"></label><label>CO2 thấp<input type="number" id="co2Low"></label>
                        </div>
                        <button type="submit">Lưu</button>
                    </form>
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-leaf"></i> Loại cây trồng</h2>
                    <input type="text" id="plantInput" placeholder="Nhập tên loại cây..." oninput="setPlant()" style="width:100%; padding: 8px; box-sizing: border-box;">
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-robot"></i> Lệnh từ AI</h2><div id="aiStatus">Chờ lệnh...</div>
                </div>
                <div class="card">
                    <h2 class="card-title"><i class="fas fa-history"></i> Lịch sử dữ liệu</h2><button onclick="window.location.href='/history'" style="width:100%; padding:10px;">Xem chi tiết</button>
                </div>
            </div>
        </main>
    </div>
    <script>
        function setDeviceStatus(id,state){const el=document.getElementById(id);if(state){el.textContent='BẬT';el.style.color='var(--primary-color)';}else{el.textContent='TẮT';el.style.color='#f44336';}}
        async function updateData(){const r=await fetch('/data');const d=await r.json();
            document.getElementById('temp').innerHTML=`${d.temperature.toFixed(1)} &deg;C`;
            document.getElementById('humi').textContent=`${d.humidity.toFixed(1)} %`;
            document.getElementById('soil').textContent=d.soilMoisture;
            document.getElementById('lightLux').textContent=`${d.lightLevel.toFixed(1)} lx`;
            document.getElementById('co2').textContent=`${d.co2Level} ppm`;
            const alertEl=document.getElementById('lightMsg');
            if(d.lightMessage&&d.lightMessage.length>0){alertEl.textContent=d.lightMessage;alertEl.style.display='block';}else{alertEl.style.display='none';}
            setDeviceStatus('fan-status',d.fan); setDeviceStatus('pump-status',d.pump); setDeviceStatus('light-status',d.light);
            document.getElementById('mode-toggle').checked=d.autoMode;
            document.getElementById('mode-text').textContent=d.autoMode?'TỰ ĐỘNG':'THỦ CÔNG';
            document.getElementById('aiStatus').textContent=d.aiCommand;
        }
        function setMode(){const isAuto=document.getElementById('mode-toggle').checked;fetch(`/mode?mode=${isAuto?'auto':'manual'}`).then(()=>setTimeout(updateData,200));}
        function controlDevice(device,action){fetch(`/device?device=${device}&action=${action}`).then(()=>setTimeout(updateData,200));}
        async function loadPlant(){const r=await fetch('/api/plant');const d=await r.json();document.getElementById('plantInput').value=d.plant;}
        function setPlant(){const plant=document.getElementById('plantInput').value.trim();if(plant){fetch('/api/plant',{method:'POST',body:JSON.stringify({plant:plant}),headers:{'Content-Type':'application/json'}});}}
        async function loadThresholds(){const r=await fetch('/api/thresholds');const t=await r.json();
            document.getElementById('tempHigh').value=t.tempHigh; document.getElementById('tempLow').value=t.tempLow;
            document.getElementById('soilDry').value=t.soilDry; document.getElementById('soilWet').value=t.soilWet;
            document.getElementById('lightDark').value=t.lightDark; document.getElementById('lightBright').value=t.lightBright;
            document.getElementById('co2High').value=t.co2High; document.getElementById('co2Low').value=t.co2Low;
        }
        async function saveThresholds(e){e.preventDefault();const btn=e.target.querySelector('button');btn.textContent='Đang lưu...';
            const t={tempHigh:parseFloat(document.getElementById('tempHigh').value),tempLow:parseFloat(document.getElementById('tempLow').value),soilDry:parseInt(document.getElementById('soilDry').value),soilWet:parseInt(document.getElementById('soilWet').value),lightDark:parseInt(document.getElementById('lightDark').value),lightBright:parseInt(document.getElementById('lightBright').value),co2High:parseInt(document.getElementById('co2High').value),co2Low:parseInt(document.getElementById('co2Low').value)};
            await fetch('/api/thresholds',{method:'POST',body:JSON.stringify(t),headers:{'Content-Type':'application/json'}});
            btn.textContent='Đã lưu!';setTimeout(()=>btn.textContent='Lưu',2000);
        }
        document.addEventListener('DOMContentLoaded',()=>{
            document.getElementById('mode-toggle').addEventListener('change',setMode);
            updateData();loadPlant();loadThresholds();setInterval(updateData,3000);
        });
    </script>
</body></html>
)rawliteral");
        _server.send(200, "text/html; charset=UTF-8", html);
    }

    // --- TRANG LỊCH SỬ - GIAO DIỆN MỚI ---
    static void handleHistory() {
        String html = F(R"rawliteral(
<!DOCTYPE html><html><head><title>Lịch sử dữ liệu</title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<style>
    body{font-family:system-ui, sans-serif;margin:0;padding:20px;background:#f8f9fa;}
    .container{max-width:1000px;margin:0 auto;background:white;padding:20px;border-radius:12px;box-shadow:0 4px 12px rgba(0,0,0,0.08);}
    h1{color:#28a745;text-align:center;}
    table{width:100%;border-collapse:collapse;margin-top:20px;}
    th,td{border:1px solid #ddd;padding:12px;text-align:left;}
    th{background:#28a745;color:white;text-align:center;}
    tr:nth-child(even){background-color:#f2f2f2;}
    button{padding:10px 20px;border:none;border-radius:5px;cursor:pointer;background:#2196F3;color:white;font-size:1em;}
</style>
</head><body>
<div class='container'><h1>Lịch sử dữ liệu cảm biến</h1>
<button onclick="window.location.href='/'">Quay lại Dashboard</button>
<table><thead><tr><th>Thời gian</th><th>Nhiệt độ (&deg;C)</th><th>Độ ẩm (%)</th><th>Độ ẩm đất</th><th>Ánh sáng (lx)</th><th>CO2 (ppm)</th></tr></thead>
<tbody id='historyTable'></tbody></table>
<script>
    function updateHistory(){fetch('/api/history').then(r=>r.json()).then(data=>{
        let tbody=document.getElementById('historyTable');tbody.innerHTML='';
        data.forEach(d=>{
            let row=tbody.insertRow();
            let time=new Date(d.timestamp).toLocaleString('vi-VN'); // Giả sử timestamp là milliseconds
            row.innerHTML=`<td>${time}</td><td>${d.temperature.toFixed(1)}</td><td>${d.humidity.toFixed(1)}</td><td>${d.soilMoisture}</td><td>${d.lightLevel.toFixed(1)}</td><td>${d.co2Level}</td>`;
        });
    });}
    updateHistory(); setInterval(updateHistory, 5000);
</script>
</body></html>
)rawliteral");
        _server.send(200, "text/html; charset=UTF-8", html);
    }

    // --- API CUNG CẤP DỮ LIỆU JSON CHO GIAO DIỆN (DÙNG ArduinoJson) ---
    static void handleData() {
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

    // --- API LẤY LỊCH SỬ DỮ LIỆU ---
    static void handleApiHistory() {
        String json = HistoryManager::getHistoryData();
        _server.send(200, "application/json", json);
    }

    // --- API LẤY TẤT CẢ THÔNG TIN CẢM BIẾN VÀ NGƯỠNG (DÙNG ArduinoJson) ---
    static void handleApiSensor() {
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

    // --- API LẤY NGƯỠNG HIỆN TẠI (DÙNG ArduinoJson) ---
    static void handleApiThresholdsGet() {
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
// --- API CẬP NHẬT NGƯỠNG 
static void handleApiThresholdsPost() {
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

    // Cập nhật các giá trị từ JSON nhận được một cách an toàn, sử dụng .as<type>()
    if (doc.containsKey("tempHigh"))    updatedThresholds.tempHigh    = doc["tempHigh"].as<float>();
    if (doc.containsKey("tempLow"))     updatedThresholds.tempLow     = doc["tempLow"].as<float>();
    if (doc.containsKey("soilDry"))     updatedThresholds.soilDry     = doc["soilDry"].as<int>();
    if (doc.containsKey("soilWet"))     updatedThresholds.soilWet     = doc["soilWet"].as<int>();
    if (doc.containsKey("lightDark"))   updatedThresholds.lightDark   = doc["lightDark"].as<int>();
    if (doc.containsKey("lightBright")) updatedThresholds.lightBright = doc["lightBright"].as<int>();
    if (doc.containsKey("co2High"))     updatedThresholds.co2High     = doc["co2High"].as<int>();
    if (doc.containsKey("co2Low"))      updatedThresholds.co2Low      = doc["co2Low"].as<int>();

    // Gọi hàm để thiết lập lại giá trị ngưỡng cho AutomationManager
    AutomationManager::setThresholds(updatedThresholds);

    // Đánh thức hệ thống để kiểm tra ngay lập tức
    AutomationManager::update();

    // In ra Serial để xác nhận giá trị đã được cập nhật thành công
    Serial.println("--- NGUONG MOI DA DUOC CAP NHAT ---");
    SensorThresholds current = AutomationManager::getThresholds();
      _server.send(200, "application/json", "{\"status\":\"thresholds updated\"}");
}
    // =================================================================
    // CÁC HÀM HANDLE CÒN LẠI GIỮ NGUYÊN LOGIC GỐC CỦA BẠN
    // =================================================================
    static void handleMode() {
        if (_server.hasArg("mode")) {
            String mode = _server.arg("mode");
            AutomationManager::setAutoMode(mode == "auto");
            _server.send(200, "text/plain", "OK");
        }
    }

    static void handleDeviceControl() {
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

    static void handleApiCommand() {
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

    static void handleApiPlantGet() {
        _server.send(200, "application/json", "{\"plant\":\"" + currentPlant + "\"}");
    }

    static void handleApiPlantPost() {
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

    static void handleApiAiCommand() {
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

    static void handleBulkControl() {
        AutomationManager::setAutoMode(false);
        bool handled = false;
        if (_server.hasArg("fan")) { String val = _server.arg("fan"); if (val == "on") DeviceManager::fanOn(); else if (val == "off") DeviceManager::fanOff(); handled = true; }
        if (_server.hasArg("pump")) { String val = _server.arg("pump"); if (val == "on") DeviceManager::pumpOn(); else if (val == "off") DeviceManager::pumpOff(); handled = true; }
        if (_server.hasArg("light")) { String val = _server.arg("light"); if (val == "on") DeviceManager::lightOn(); else if (val == "off") DeviceManager::lightOff(); handled = true; }
        if (handled) { _server.send(200, "text/plain", "OK"); }
        else { _server.send(400, "text/plain", "No valid parameters"); }
    }

    static void handleTool() {
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
            // Đánh thức hệ thống để kiểm tra ngay lập tức sau khi cập nhật ngưỡng
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
    
    static void handleNotFound() {
        _server.send(404, "text/plain", "Not Found");
    }
};

WebServer WebManager::_server(80);
// ===== MAIN SETUP & LOOP =====
void setup() {
    Serial.begin(115200);
    Serial.println("Smart Greenhouse - Khoi dong...");

    // Khởi tạo AutoModeManager trước
    AutoModeManager::getInstance().initialize();

    HistoryManager::initialize();
    SensorManager::initialize();
    DeviceManager::initialize();
    AutomationManager::initialize();

    WiFi.config(local_IP, gateway, subnet);
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

void loop() {
    WebManager::handleClient();
    AutomationManager::update();
    delay(1000);
}