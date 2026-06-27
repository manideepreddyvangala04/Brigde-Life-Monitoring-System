#ifndef CONFIG_H
#define CONFIG_H

// ==========================================
// NETWORK CONFIGURATION
// ==========================================
const char* WIFI_SSID     = "REPLACE_WITH_YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "REPLACE_WITH_YOUR_WIFI_PASSWORD";

const char* MQTT_SERVER   = "broker.hivemq.com"; 
const int   MQTT_PORT     = 1883;

// ==========================================
// SAFETY THRESHOLDS
// ==========================================
const float VIBRATION_THRESHOLD_G = 2.5;  // G-force limit for impact
const float TILT_THRESHOLD_DEG    = 5.0;  // Maximum allowable structural tilt
const float ICE_WARNING_TEMP_C    = 2.0;  // Temperature for black ice warning

#endif // CONFIG_H
