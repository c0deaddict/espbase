#include "config.h"
#include "espbase.h"

#ifdef ESP32
#include "network_esp32.h"
#else
#include "network_esp8266.h"
#endif

#ifdef USE_NTP
const long utcOffsetSeconds = 3600;
WiFiUDP ntpUDP;
NTPClient ntp(NTPClient(ntpUDP, "europe.pool.ntp.org", utcOffsetSeconds));
#endif

Counter wifiDisconnected("esp_wifi_disconnected", "Number of times WiFi is diconnected.");

MetricProxy wifiRssi(
    "esp_wifi_rssi",
    "gauge",
    "Signal strength of WiFi.",
    [](const char *name, Print *out) {
        out->printf("%s %d\n", name, WiFi.RSSI());
    }
);

#ifdef MQTT_HOST
AsyncMqttClient mqtt;
Counter mqttDisconnected("esp_mqtt_disconnected", "Number of times MQTT is disconnected.");

String mqttDisconnectReasonToStr(AsyncMqttClientDisconnectReason reason) {
    switch (reason) {
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED: return "TCP_DISCONNECTED";
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION: return "MQTT_UNACCEPTABLE_PROTOCOL_VERSION";
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED: return "MQTT_IDENTIFIER_REJECTED";
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE: return "MQTT_SERVER_UNAVAILABLE";
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS: return "MQTT_MALFORMED_CREDENTIALS";
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED: return "MQTT_NOT_AUTHORIZED";
    case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE: return "ESP8266_NOT_ENOUGH_SPACE";
    case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT: return "TLS_BAD_FINGERPRINT";
    default: return "unknown";
    }
}

void connectToMqtt() {
    Serial.println("Connecting to MQTT...");
    mqtt.connect();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    mqttDisconnected.inc();
    Serial.println("Disconnected from MQTT.");

    if (WiFi.isConnected()) {
        startMqttReconnectTimer();
    }
}
#endif // MQTT_HOST

void connectToWifi() {
    Serial.print("Connecting to WiFi ");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    #ifdef ESP32
    // https://github.com/espressif/arduino-esp32/issues/3438
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    #endif
    setHostname(HOSTNAME);
}

void onWifiConnect() {
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    #ifdef MQTT_HOST
    connectToMqtt();
    #endif
}

void onWifiDisconnect() {
    wifiDisconnected.inc();
    Serial.println("WiFi lost connection");
    #ifdef MQTT_HOST
    stopMqttReconnectTimer(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    #endif

    startWifiReconnectTimer();
}

void setupNetwork() {
    Serial.println("Setting up network..");

    setupWifi();

    #ifdef MQTT_HOST
    mqtt.onDisconnect(onMqttDisconnect);
    mqtt.setCredentials(MQTT_USER, MQTT_PASSWORD);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    #endif

    connectToWifi();

    #ifdef USE_NTP
    ntp.begin();
    #endif
}

void handleNetwork() {
    #ifdef USE_NTP
    ntp.update();
    #endif
}
