#pragma once

#define HOSTNAME "espbase"

#define WIFI_SSID "Pretty Fly for a WiFi"
#define WIFI_PASSWORD "password123"

#define OTA_PASSWORD "password123"

// Optional MQTT.
#define MQTT_HOST IPAddress(127, 0, 0, 1)
#define MQTT_PORT 1883
#define MQTT_USER "espbase"
#define MQTT_PASSWORD "password123"

// Optional InfluxDB.
#define INFLUXDB_URL "http://127.0.0.1:8086"
#define INFLUXDB_TOKEN "user:pass"
#define INFLUXDB_ORG "org name/id"
#define INFLUXDB_BUCKET "metrics"

// Optional NTP.
#define USE_NTP
