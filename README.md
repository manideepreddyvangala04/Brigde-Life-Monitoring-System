# ESP32 Structural Health Monitoring (SHM) System

An open-source, embedded IoT telemetry system designed to monitor bridges and civil structures. This device tracks mechanical stress, extreme impacts, and dangerous road conditions (like black ice) using sensor fusion and publishes real-time data via MQTT.

## 🚀 Features
* **Structural Tilt Tracking:** Utilizes a Complementary Filter to fuse gyroscope and accelerometer data, providing stable Pitch and Roll calculations to monitor foundation shifting or mechanical sag.
* **Impact & Resonance Detection:** Converts high-frequency Z-axis acceleration into G-forces to detect sudden, heavy impacts (e.g., collisions or extreme load stress).
* **Black Ice Warning:** Uses contactless infrared thermopile technology to measure the exact road surface temperature, triggering alarms if conditions drop below freezing.
* **Real-Time Telemetry:** Connects to local Wi-Fi and streams JSON-formatted structural data and critical string alerts to an MQTT broker for cloud dashboard integration.

---

## 🛠️ Hardware Requirements
* **Microcontroller:** ESP32 Development Board
* **IMU Sensor:** MPU6050 (6-DOF Accelerometer & Gyroscope)
* **Infrared Sensor:** MLX90614 (Contactless IR Temperature Sensor)
* **Power:** 5V/3.3V Power Supply suitable for the ESP32

### Wiring Diagram
Both sensors communicate via the $I^2C$ protocol and share the same bus on the ESP32.

| Sensor Pin | ESP32 Pin | Function |
| :--- | :--- | :--- |
| **VIN / VCC** | 3.3V | Power |
| **GND** | GND | Common Ground |
| **SCL** | GPIO 22 | $I^2C$ Clock Line |
| **SDA** | GPIO 21 | $I^2C$ Data Line |

---

## 📦 Software Dependencies
This project is built using the Arduino C++ framework. Before compiling, ensure the following libraries are installed via the Arduino IDE Library Manager:
1. `Adafruit MPU6050` by Adafruit
2. `Adafruit MLX90614` by Adafruit
3. `Adafruit Unified Sensor` by Adafruit
4. `PubSubClient` by Nick O'Leary

---

## ⚙️ Installation & Setup

1. **Clone the Repository:**
   ```bash
   git clone https://github.com/manideepreddyvangala04/Brigde-Life-Monitoring-System.git
   ```

2. **Open the Project in Arduino IDE:**
   - Launch your Arduino IDE.
   - Open the cloned directory. Note: You may need to rename `main.ino` to `Brigde-Life-Monitoring-System.ino` or create a containing folder named `main` as required by the Arduino IDE environment.

3. **Install Dependencies:**
   - Navigate to **Sketch** -> **Include Library** -> **Manage Libraries...**
   - Search for and install the 4 required libraries listed in the **Software Dependencies** section.

4. **Network and Security Configuration:**
   - Open `config.h`.
   - Update your Wi-Fi credentials:
     ```cpp
     const char* WIFI_SSID     = "YOUR_WIFI_SSID";
     const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
     ```
   - Customize the safety thresholds if needed:
     ```cpp
     const float VIBRATION_THRESHOLD_G = 2.5;  // G-force limit for impact
     const float TILT_THRESHOLD_DEG    = 5.0;  // Maximum allowable structural tilt
     const float ICE_WARNING_TEMP_C    = 2.0;  // Temperature for black ice warning
     ```

5. **Upload the Code:**
   - Connect your ESP32 board to your computer.
   - Select your target ESP32 board in **Tools** -> **Board** (e.g., "ESP32 Dev Module").
   - Select the matching serial port in **Tools** -> **Port**.
   - Click **Upload** (or press `Ctrl+U`).

---

## 📡 Telemetry & MQTT Protocol

The device connects to an MQTT broker to publish structured JSON payloads and immediate event-based safety alarms.

### MQTT Topics & Payloads

1. **Telemetry Topic (`bridge/status/telemetry`):**
   - **Interval:** Published every 2 seconds.
   - **Format:** JSON object.
   - **Example Payload:**
     ```json
     {
       "pitch": 0.45,
       "roll": -1.12,
       "z_gforce": 1.02,
       "surface_temp_c": 1.50,
       "ambient_temp_c": 12.40
     }
     ```
   - **Field Descriptions:**
     - `pitch`: Current pitch in degrees (front-to-back tilt).
     - `roll`: Current roll in degrees (side-to-side tilt).
     - `z_gforce`: Normalised vertical acceleration in Gs ($1.0\text{ G}$ at rest).
     - `surface_temp_c`: Exact road surface temperature from the MLX90614 IR sensor.
     - `ambient_temp_c`: Ambient air temperature.

2. **Alarms Topic (`bridge/status/alerts`):**
   - **Interval:** Published immediately when safety thresholds are violated.
   - **Alert Strings:**
     - `IMPACT_DETECTED`: Triggered when vertical G-force exceeds `VIBRATION_THRESHOLD_G` (e.g., vehicle collision or structural fracture).
     - `TILT_LIMIT_EXCEEDED`: Triggered when absolute pitch or roll exceeds `TILT_THRESHOLD_DEG`.
     - `SURFACE_ICE_WARNING`: Triggered when road temperature is at or below `ICE_WARNING_TEMP_C`.

---

## 🧠 Sensor Fusion Math

To ensure stable readings on dynamic structures (like bridges with constant traffic vibrations), simple raw accelerometer readings are insufficient. The system applies a **Complementary Filter** to fuse high-frequency gyroscope data with low-frequency accelerometer data.

### Complementary Filter Formula
$$\theta_{\text{fused}} = \alpha \cdot (\theta_{\text{previous}} + \omega_{\text{gyro}} \cdot dt) + (1 - \alpha) \cdot \theta_{\text{accel}}$$

Where:
* $\alpha = 0.98$ (high-pass filter weight for the gyroscope to eliminate accelerometer vibration noise).
* $1 - \alpha = 0.02$ (low-pass filter weight for the accelerometer to prevent long-term gyro drift).
* $dt$ is the sample time interval.
* $\theta_{\text{accel}}$ is calculated directly from gravity components:
  $$\text{Pitch}_{\text{accel}} = \text{atan2}(a_y, \sqrt{a_x^2 + a_z^2}) \cdot \frac{180}{\pi}$$
  $$\text{Roll}_{\text{accel}} = \text{atan2}(-a_x, a_z) \cdot \frac{180}{\pi}$$

### ⚠️ Critical Note on Gyroscope Units
The `Adafruit_MPU6050` library returns gyroscope readings in **radians per second** ($\text{rad/s}$). Because the accelerometer angles are computed in **degrees**, integrating the gyro directly as `g.gyro.x * dt` introduces a major units mismatch (adding radians to degrees). 

To ensure the filter works correctly, the gyro inputs must be converted to degrees per second ($\text{deg/s}$) by multiplying by $\frac{180}{\pi}$:
```cpp
pitch = 0.98 * (pitch + g.gyro.x * dt * 180 / PI) + 0.02 * accelPitch;
roll  = 0.98 * (roll  + g.gyro.y * dt * 180 / PI) + 0.02 * accelRoll;
```
*(The open-source code in `sensor_math.h` should be updated to include this conversion factor for correct operation).*

