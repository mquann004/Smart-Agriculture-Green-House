<div align="center">

# 🌿 Smart Greenhouse System

### Hệ thống nhà kính thông minh: Giải pháp tối ưu cho nông nghiệp hiệu suất cao

[![Platform](https://img.shields.io/badge/Platform-ESP32-blue?style=for-the-badge&logo=espressif&logoColor=white)](https://www.espressif.com/)
[![AI](https://img.shields.io/badge/AI-XiaoZhi_LLM-orange?style=for-the-badge&logo=openai&logoColor=white)](#-trợ-lý-ảo-ai-lyli)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)
[![Arduino](https://img.shields.io/badge/Framework-Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)

<br/>

*Đề tài Nghiên cứu Khoa học Sinh viên năm 2025*
*Học viện Công nghệ Bưu chính Viễn thông – Cơ sở TP.HCM*

<br/>

<img src="https://img.shields.io/badge/Sensors-5-brightgreen?style=flat-square" alt="Sensors"/>
<img src="https://img.shields.io/badge/Actuators-3-blue?style=flat-square" alt="Actuators"/>
<img src="https://img.shields.io/badge/API_Endpoints-15+-purple?style=flat-square" alt="APIs"/>
<img src="https://img.shields.io/badge/Voice_Control-✓-ff69b4?style=flat-square" alt="Voice"/>

</div>

---

## 📖 Giới thiệu

**Smart Greenhouse** là hệ thống nhà kính thông minh sử dụng **ESP32** kết hợp **AI** để tự động giám sát và điều khiển các yếu tố môi trường (nhiệt độ, độ ẩm, ánh sáng, CO₂), đảm bảo điều kiện sinh trưởng tối ưu cho cây trồng.

Hệ thống hỗ trợ:
- 🤖 **Điều khiển bằng giọng nói** thông qua trợ lý ảo AI (LYLI)
- 🌐 **Giám sát từ xa** qua giao diện Web Dashboard real-time
- ⚙️ **Tự động hóa hoàn toàn** với cơ chế fail-safe thông minh
- 📊 **Lưu trữ lịch sử** dữ liệu cảm biến trên SPIFFS

---

## 🏗️ Kiến trúc hệ thống

```
                        ┌─────────────────────────────────┐
                        │       ☁️  AI SERVER              │
                        │  ┌─────┐  ┌─────┐  ┌─────┐     │
                        │  │ ASR │→ │ LLM │→ │ TTS │     │
                        │  │funASR│  │OpenAI│  │Google│    │
                        │  └─────┘  └──┬──┘  └─────┘     │
                        │              │ Tool Calling      │
                        └──────┬───────┼──────────────────┘
                               │       │
                    HTTPS API  │       │  HTTPS API
                               │       │
              ┌────────────────┴─┐   ┌─┴──────────────────┐
              │   ESP32-S3       │   │     ESP32           │
              │   (MCU 2)        │   │     (MCU 1)         │
              │                  │   │                     │
              │  🎤 INMP441      │   │  🌡️ DHT22           │
              │  🔊 MAX98357A    │   │  💡 BH1750           │
              │  🖥️ ILI9341      │   │  🌱 Soil Sensor     │
              │                  │   │  💨 MQ-135           │
              │  Voice Assistant │   │                     │
              └──────────────────┘   │  ┌─────────────┐   │
                                     │  │ Relay Module │   │
                                     │  └──┬──┬──┬────┘   │
                                     └─────┼──┼──┼────────┘
                                           │  │  │
                                     ┌─────┘  │  └─────┐
                                     🌀       💧       💡
                                    Fan     Pump    Light
```

---

## 🔧 Phần cứng

### Linh kiện chính

| Thành phần | Model | Giao thức | Chức năng |
|:---:|:---:|:---:|:---|
| **MCU chính** | ESP32 | — | Trung tâm xử lý, WiFi, Web Server |
| **MCU phụ** | ESP32-S3 | — | Xử lý AI, giọng nói, hiển thị |
| **Nhiệt độ & Độ ẩm** | DHT22 | Digital (GPIO 4) | Đo nhiệt độ & độ ẩm không khí |
| **Ánh sáng** | BH1750 | I²C | Đo cường độ ánh sáng (lux) |
| **Độ ẩm đất** | Capacitive Sensor | ADC (GPIO 34) | Đo độ ẩm đất |
| **CO₂** | MQ-135 | ADC (GPIO 35) | Đo nồng độ CO₂ (ppm) |
| **Quạt** | DC 5V | Relay (GPIO 26) | Thông gió, giảm nhiệt |
| **Bơm nước** | DC 12V | Relay (GPIO 25) | Tưới cây tự động |
| **Đèn LED** | 3W | Relay (GPIO 13) | Chiếu sáng quang hợp |
| **Micro** | INMP441 | I²S | Thu âm giọng nói |
| **Loa** | MAX98357A | I²S | Phát phản hồi giọng nói |
| **Màn hình** | ILI9341 2.8" | SPI | Hiển thị thông tin |

### Sơ đồ kết nối ESP32

| Chân GPIO | Kết nối | Ghi chú |
|:---:|:---|:---|
| GPIO 4 | DHT22 (DATA) | Pull-up 4.7kΩ |
| GPIO 21 (SDA) | BH1750 (SDA) | I²C Bus |
| GPIO 22 (SCL) | BH1750 (SCL) | I²C Bus |
| GPIO 34 | Soil Sensor (AO) | ADC input |
| GPIO 35 | MQ-135 (AO) | ADC input |
| GPIO 25 | Relay CH1 (IN) | → Bơm nước |
| GPIO 26 | Relay CH2 (IN) | → Quạt |
| GPIO 13 | Relay CH3 (IN) | → Đèn LED |

---

## 💻 Cấu trúc phần mềm

Firmware được thiết kế theo kiến trúc **7 tầng (Layered Architecture)**, mỗi module tách biệt trong file riêng:

```
full/
│
├── 📄 full.ino                  # Entry point — setup() & loop()
│
├── ⚙️ config.h                  # Tầng 2: Cấu hình (GPIO, WiFi, Thresholds)
├── 🌐 globals.h                 # Biến toàn cục (extern declarations)
│
├── 📡 SensorManager.h/.cpp      # Tầng 3: Đọc cảm biến (DHT22, BH1750, Soil, CO₂)
├── 🔌 DeviceManager.h/.cpp      # Tầng 3: Điều khiển thiết bị (Fan, Pump, Light)
│
├── 🔄 AutoModeManager.h/.cpp    # Tầng 4: Quản lý Auto/Manual + timeout 30s
├── 🧠 AutomationManager.h/.cpp  # Tầng 4: Logic tự động hóa thông minh
│
├── 💾 HistoryManager.h/.cpp     # Tầng 5: Lưu trữ SPIFFS (JSON)
│
├── 🌍 WebManager.h/.cpp         # Tầng 6: Web Server + REST API
├── 🎨 page_dashboard.h          # Tầng 6: HTML Dashboard (PROGMEM)
└── 📊 page_history.h            # Tầng 6: HTML History page (PROGMEM)
```

### Sơ đồ phụ thuộc giữa các module

```
    ┌──────────┐
    │ full.ino │  ← Entry Point
    └────┬─────┘
         │ includes
         ▼
    ┌──────────┐     ┌──────────┐
    │  config  │────▶│ globals  │
    └────┬─────┘     └────┬─────┘
         │                │
    ┌────┴────────────────┴────────────────┐
    │                                      │
    ▼                  ▼                   ▼
┌──────────┐  ┌──────────────┐  ┌──────────────────┐
│  Device  │  │    Sensor    │  │   AutoMode       │
│  Manager │  │    Manager   │  │   Manager        │
└────┬─────┘  └──────┬───────┘  └────────┬─────────┘
     │               │                   │
     └───────┬───────┘                   │
             ▼                           │
     ┌───────────────┐                   │
     │    History    │                   │
     │    Manager    │                   │
     └───────┬───────┘                   │
             │                           │
             ▼                           │
     ┌───────────────────┐               │
     │   Automation      │◀──────────────┘
     │   Manager         │
     └───────┬───────────┘
             │
             ▼
     ┌───────────────────┐
     │    WebManager     │
     │  ┌─────────────┐  │
     │  │ Dashboard   │  │
     │  │ History Page│  │
     │  │ REST APIs   │  │
     │  └─────────────┘  │
     └───────────────────┘
```

---

## 🤖 Trợ lý ảo AI (LYLI)

Hệ thống tích hợp trợ lý ảo **LYLI** sử dụng pipeline **ASR → LLM → TTS**:

| Bước | Công nghệ | Chức năng |
|:---:|:---:|:---|
| 1️⃣ | **funASR** | Chuyển giọng nói → văn bản |
| 2️⃣ | **OpenAI LLM** | Phân tích ý định + Tool Calling |
| 3️⃣ | **Google TTS** | Chuyển văn bản → giọng nói phản hồi |

### Tool Calling — Điều khiển bằng ngôn ngữ tự nhiên

Thay vì bắt từ khóa cứng, hệ thống sử dụng **LLM Tool Calling** để hiểu ngữ cảnh:

```
👤 "Nóng quá!"          → 🤖 LLM hiểu → gọi control_fan(state="on")
👤 "Tưới cây đi"        → 🤖 LLM hiểu → gọi control_pump(state="on")  
👤 "Nhiệt độ bao nhiêu?" → 🤖 LLM hiểu → gọi get_sensor_data() → trả lời
```

### AutoModeManager — Cơ chế an toàn (Fail-safe)

```
 Chế độ Tự động (mặc định)
         │
         │  Người dùng ra lệnh thủ công
         ▼
 Chế độ Thủ công ──── Timer 30s bắt đầu đếm
         │
         │  Không có lệnh mới trong 30s?
         ▼
 Tự động quay về Chế độ Tự động ✅
```

> 💡 **Giá trị:** Nhà kính không bao giờ bị "bỏ quên" ở trạng thái thủ công.  
> Cây trồng luôn được chăm sóc liên tục ngay cả khi người dùng vắng mặt.

---

## 🔌 REST API

ESP32 cung cấp **15+ REST API endpoints** trên port 80:

### Dữ liệu & Giám sát

| Method | Endpoint | Mô tả |
|:---:|:---|:---|
| `GET` | `/` | Dashboard chính |
| `GET` | `/data` | Tất cả dữ liệu cảm biến + trạng thái thiết bị |
| `GET` | `/api/sensor` | Dữ liệu cảm biến + ngưỡng chi tiết |
| `GET` | `/history` | Trang lịch sử dữ liệu |
| `GET` | `/api/history` | API lấy lịch sử (JSON) |

### Điều khiển thiết bị

| Method | Endpoint | Mô tả |
|:---:|:---|:---|
| `GET` | `/mode?mode=auto\|manual` | Chuyển chế độ |
| `GET` | `/device?device=fan&action=on` | Điều khiển 1 thiết bị |
| `POST` | `/api/command` | Điều khiển qua JSON body |
| `GET` | `/bulkControl?fan=on&pump=off` | Điều khiển nhiều thiết bị |

### Cấu hình

| Method | Endpoint | Mô tả |
|:---:|:---|:---|
| `GET` | `/api/thresholds` | Lấy ngưỡng cảm biến hiện tại |
| `POST` | `/api/thresholds` | Cập nhật ngưỡng |
| `GET` | `/api/plant` | Lấy loại cây trồng |
| `POST` | `/api/plant` | Đặt loại cây trồng |

### AI Integration

| Method | Endpoint | Mô tả |
|:---:|:---|:---|
| `POST` | `/api/ai_command` | Nhận lệnh từ AI |
| `POST` | `/tool` | API tổng hợp cho AI Tool Calling |

<details>
<summary>📋 Ví dụ request/response</summary>

**GET `/data`** — Response:
```json
{
  "temperature": 28.5,
  "humidity": 65.2,
  "soilMoisture": 3500,
  "lightLevel": 150.0,
  "co2Level": 450,
  "fan": true,
  "pump": false,
  "light": false,
  "autoMode": true,
  "lightMessage": "Ánh sáng bình thường",
  "aiCommand": "Chờ lệnh..."
}
```

**POST `/api/command`** — Bật quạt:
```json
{ "device": "fan", "action": "on" }
```

**POST `/api/thresholds`** — Cập nhật ngưỡng:
```json
{
  "tempHigh": 32.0,
  "tempLow": 25.0,
  "soilDry": 4000,
  "soilWet": 3000,
  "lightDark": 50,
  "lightBright": 300,
  "co2High": 1000,
  "co2Low": 400
}
```

</details>

---

## 🚀 Hướng dẫn cài đặt

### Yêu cầu

- **Phần cứng:** ESP32 DevKit v1 (hoặc tương đương)
- **Phần mềm:** Arduino IDE 2.x hoặc PlatformIO
- **Thư viện Arduino cần cài:**

| Thư viện | Phiên bản | Cài đặt |
|:---|:---:|:---|
| `DHT sensor library` | ≥ 1.4.4 | Library Manager |
| `BH1750` | ≥ 1.3.0 | Library Manager |
| `ArduinoJson` | ≥ 6.x | Library Manager |
| `NTPClient` | ≥ 3.2.1 | Library Manager |
| `WebServer` | built-in | Có sẵn trong ESP32 core |

### Các bước cài đặt

#### 1. Clone repository

```bash
git clone https://github.com/mquann004/Smart-Agriculture-Green-House.git
cd Smart-Agriculture-Green-House
```

#### 2. Cấu hình WiFi

Mở file `full/full.ino` và chỉnh sửa thông tin WiFi:

```cpp
const char* WIFI_SSID = "Tên_WiFi_của_bạn";
const char* WIFI_PASSWORD = "Mật_khẩu_WiFi";

// Cấu hình IP tĩnh (tùy chỉnh theo mạng của bạn)
IPAddress local_IP(192, 168, 1, 100);    // IP cho ESP32
IPAddress gateway_IP(192, 168, 1, 1);    // Gateway router
IPAddress subnet(255, 255, 255, 0);
```

#### 3. Cài đặt thư viện

Trong Arduino IDE:
1. Vào **Sketch** → **Include Library** → **Manage Libraries**
2. Tìm và cài đặt: `DHT sensor library`, `BH1750`, `ArduinoJson`, `NTPClient`

#### 4. Upload firmware

1. Chọn Board: **ESP32 Dev Module**
2. Chọn Port: COM tương ứng
3. Mở file `full/full.ino`
4. Nhấn **Upload** (→)

#### 5. Truy cập Dashboard

Sau khi upload thành công, mở Serial Monitor (115200 baud) để xem IP:

```
Smart Greenhouse - Khoi dong...
Da ket noi WiFi!
IP: 192.168.1.100
Smart Greenhouse da khoi dong!
```

Mở trình duyệt và truy cập: **`http://192.168.1.100`**

---

## 📊 Logic tự động hóa

Hệ thống áp dụng các quy tắc điều khiển **liên kết chéo** thông minh:

| Điều kiện | Hành động | Ghi chú |
|:---|:---|:---|
| Nhiệt độ > **30°C** | 🌀 Bật quạt | Làm mát |
| Nhiệt độ < **27°C** & CO₂ ổn định | 🌀 Tắt quạt | Tiết kiệm điện |
| Độ ẩm đất > **4000** | 💧 Bật bơm | Đất khô |
| Độ ẩm đất < **3000** | 💧 Tắt bơm | Đất đủ ẩm |
| Ánh sáng < **50 lux** & giờ 6h–18h | 💡 Bật đèn | Bổ sung quang hợp |
| Ánh sáng > **300 lux** | 💡 Tắt đèn | Đủ sáng |
| Ngoài giờ quang hợp (18h–6h) | 💡 Luôn tắt đèn | Nghỉ quang hợp |
| CO₂ > **1000 ppm** hoặc < **400 ppm** | 🌀 Bật quạt | Thông khí |

> ⚠️ **Liên kết chéo:** Quạt chỉ tắt khi **cả nhiệt độ VÀ CO₂** đều trong ngưỡng an toàn, tránh xung đột giữa các quy tắc.

---

## 🛠 Tùy chỉnh

### Thay đổi ngưỡng mặc định

Chỉnh sửa trong `config.h`:

```cpp
struct SensorThresholds {
  float tempHigh = 30.0;     // Ngưỡng nhiệt độ cao (°C)
  float tempLow = 27.0;      // Ngưỡng nhiệt độ thấp (°C)
  int soilDry = 4000;         // Ngưỡng đất khô
  int soilWet = 3000;         // Ngưỡng đất ướt
  int lightDark = 50;         // Ánh sáng yếu (lux)
  int lightBright = 300;      // Ánh sáng mạnh (lux)
  int co2High = 1000;         // CO₂ cao (ppm)
  int co2Low = 400;           // CO₂ thấp (ppm)
};
```

### Thay đổi GPIO

Chỉnh sửa trong `config.h`:

```cpp
#define DHT_PIN 4       // Chân cảm biến DHT22
#define FAN_PIN 26      // Chân relay quạt
#define PUMP_PIN 25     // Chân relay bơm
#define LIGHT_PIN 13    // Chân relay đèn
#define SOIL_PIN 34     // Chân cảm biến độ ẩm đất
#define CO2_PIN 35      // Chân cảm biến CO₂
```

---

## 📁 Cấu trúc Repository

```
Smart-Agriculture-Green-House/
│
├── 📂 full/                         # Firmware nhà kính thông minh
│   ├── full.ino                     # Entry point (setup + loop)
│   ├── config.h                     # Cấu hình hệ thống
│   ├── globals.h                    # Biến toàn cục
│   ├── AutoModeManager.h/.cpp       # Quản lý Auto/Manual mode
│   ├── DeviceManager.h/.cpp         # Điều khiển thiết bị
│   ├── SensorManager.h/.cpp         # Đọc cảm biến
│   ├── HistoryManager.h/.cpp        # Lưu trữ lịch sử (SPIFFS)
│   ├── AutomationManager.h/.cpp     # Logic tự động hóa
│   ├── WebManager.h/.cpp            # Web Server + REST API
│   ├── page_dashboard.h             # HTML Dashboard
│   └── page_history.h               # HTML History page
│
├── 📄 .gitignore
└── 📄 README.md
```

---

## 🔮 Hướng phát triển

- [ ] 📸 Tích hợp camera + AI nhận diện sâu bệnh (YOLO / TensorFlow Lite)
- [ ] 📱 Phát triển ứng dụng Mobile App
- [ ] ☁️ Lưu trữ dữ liệu trên Cloud (Firebase / InfluxDB)
- [ ] 🧠 Machine Learning tự tối ưu ngưỡng theo giai đoạn sinh trưởng
- [ ] 🔇 Nhận diện giọng nói offline (Edge AI)
- [ ] 🏢 Mở rộng quản lý nhiều nhà kính qua mạng cảm biến không dây (WSN)

---

## 🏫 Thông tin đề tài

|  |  |
|:---|:---|
| **Tên đề tài** | Hệ thống nhà kính thông minh: Giải pháp tối ưu cho nông nghiệp hiệu suất cao |
| **Mã đề tài** | 04-SV-2025-VT2 |
| **Đơn vị** | Khoa Viễn thông 2 – Học viện Công nghệ Bưu chính Viễn thông (PTIT) |
| **GVHD** | ThS. Nguyễn Quang Sang |
| **Lĩnh vực** | Công nghệ Internet vạn vật (IoT) & Trí tuệ nhân tạo (AI) |

---

## 📚 Tài liệu tham khảo

1. Espressif Systems – [ESP32 Technical Reference Manual](https://www.espressif.com/en/support/documents/technical-docs)
2. Arduino – [ESP32 Arduino Core API Reference](https://docs.arduino.cc)
3. OpenAI – [API Documentation](https://platform.openai.com/docs)
4. Lakshmanan, S., et al. (2020). *IoT-Based Smart Hydroponic System*. IEEE Access.
5. Rayhana, R., et al. (2020). *IoT-Based Greenhouse Monitoring*. Journal of Agricultural Informatics.

---

<div align="center">

**Được phát triển với ❤️ bởi sinh viên PTIT**

*Tham khảo mã nguồn XiaoZhi ESP32 tại:* [78/xiaozhi-esp32](https://github.com/78/xiaozhi-esp32)

*Nếu dự án hữu ích, hãy cho một ⭐ nhé!*

</div>
