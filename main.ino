#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_MLX90614.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Include our custom local files
#include "config.h"
#include "sensor_math.h"

// ==========================================
// GLOBAL VARIABLES
// ==========================================
Adafruit_MPU6050 mpu;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

WiFiClient espClient;
PubSubClient client(espClient);

float pitch = 0;
float roll = 0;
unsigned long lastTime = 0;
unsigned long lastMsgTime = 0;

// ==========================================
// NETWORK SETUP
// ==========================================
void setup_wifi() {
  delay(10);
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32BridgeMonitor-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

// ==========================================
// MAIN SETUP
// ==========================================
void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);

  Serial.println("Initializing Sensors...");
  
  if (!mpu.begin()) {
    Serial.println("MPU6050 connection failed!");
    while (1) { delay(10); }
  }
  
  if (!mlx.begin()) {
    Serial.println("MLX90614 connection failed!");
    while (1) { delay(10); }
  }

  // Optimize MPU6050 for structural tracking
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("System Ready.");
  lastTime = millis();
}

// ==========================================
// MAIN LOOP
// ==========================================
void loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();

  // 1. Calculate time delta (dt)
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  // 2. Read Sensors
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  
  float surfaceTemp = mlx.readObjectTempC();
  float ambientTemp = mlx.readAmbientTempC();
  float z_gforce = 0;

  // 3. Process Math (Calls the function from sensor_math.h)
  updateStructuralTilt(a, g, dt, pitch, roll, z_gforce);

  // 4. Threshold Alarms
  if (z_gforce > VIBRATION_THRESHOLD_G) {
    client.publish("bridge/status/alerts", "IMPACT_DETECTED");
  }
  if (abs(pitch) > TILT_THRESHOLD_DEG || abs(roll) > TILT_THRESHOLD_DEG) {
    client.publish("bridge/status/alerts", "TILT_LIMIT_EXCEEDED");
  }
  if (surfaceTemp <= ICE_WARNING_TEMP_C) {
    client.publish("bridge/status/alerts", "SURFACE_ICE_WARNING");
  }

  // 5. Telemetry Transmission (Every 2 seconds)
  if (currentTime - lastMsgTime > 2000) {
    lastMsgTime = currentTime;
    
    char payload[256];
    snprintf(payload, sizeof(payload), 
             "{\"pitch\":%.2f, \"roll\":%.2f, \"z_gforce\":%.2f, \"surface_temp_c\":%.2f, \"ambient_temp_c\":%.2f}", 
             pitch, roll, z_gforce, surfaceTemp, ambientTemp);
    
    Serial.print("Publishing telemetry: ");
    Serial.println(payload);
    client.publish("bridge/status/telemetry", payload);
  }
}
