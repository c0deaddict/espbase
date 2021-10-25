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
    logger->println("WiFi: connecting...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    #ifdef ESP8266
    wifi_set_sleep_type(NONE_SLEEP_T);
    #endif
    #ifdef ESP32
    WiFi.setSleep(false);
    // https://github.com/espressif/arduino-esp32/issues/3438
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
    #endif
    setHostname(HOSTNAME);
}

void onWifiConnect() {
    logger->println("WiFi: connected");
    logger->printf("WiFi: IP address: %s\n\r", WiFi.localIP().toString().c_str());
    logger->printf("WiFi: signal strength (RSSI): %d\n\r", WiFi.RSSI());

    #ifdef MQTT_HOST
    mqtt.connect();
    #endif

    #ifdef INFLUXDB_URL
    // Check server connection
    if (influx.validateConnection()) {
        logger->printf("InfluxDB: connected to InfluxDB: %s\n\r", influx.getServerUrl().c_str());
    } else {
        logger->printf("InfluxDB: connection failed: %s\n\r", influx.getLastErrorMessage().c_str());
    }
    #endif
}

void onWifiDisconnect() {
    wifiDisconnected.inc();
    logger->println("WiFi: lost connection");
    #ifdef MQTT_HOST
    mqtt.stopReconnectTimer(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
    #endif

    startWifiReconnectTimer();
}

void setupNetwork() {
    #ifdef MQTT_HOST
    mqtt.setup();
    #endif

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
