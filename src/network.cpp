#include "config.h"
#include "espbase.h"

#ifdef MQTT_HOST
#include "mqtt.h"
#endif

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

#ifdef INFLUXDB_URL
InfluxDBClient influx(INFLUXDB_URL, INFLUXDB_DB);
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

    #ifdef INFLUXDB_URL
    // Check server connection
    if (influx.validateConnection()) {
        Serial.print("Connected to InfluxDB: ");
        Serial.println(influx.getServerUrl());
    } else {
        Serial.print("InfluxDB connection failed: ");
        Serial.println(influx.getLastErrorMessage());
    }
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

    setupMqtt();
    setupWifi();

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
