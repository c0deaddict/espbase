#pragma once

// Required

#ifndef HOSTNAME
#error "HOSTNAME is not configured"
#endif

#ifndef WIFI_SSID
#error "WIFI_SSID is not configured"
#endif

#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD is not configured"
#endif

#ifndef OTA_PASSWORD
#error "OTA_PASSWORD is not configured"
#endif

// MQTT is optional.
#ifdef MQTT_HOST

#ifndef MQTT_USER
#error "MQTT_USER is not configured"
#endif

#ifndef MQTT_PASSWORD
#error "MQTT_PASSWORD is not configured"
#endif

// Optional

#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif

#endif // MQTT_HOST

// InfluxDB is optional.
#ifdef INFLUXDB_URL

#ifndef INFLUXDB_DB
#error "INFLUXDB_DB is not configured"
#endif

#endif // INFLUXDB_URL

#ifndef SETTINGS_MAX_SIZE
#define SETTINGS_MAX_SIZE 1024
#endif

#ifndef DEVICE_DESC_MAX_SIZE
#define DEVICE_DESC_MAX_SIZE 1024
#endif

#ifndef DEVICE_CONFIG_MAX_SIZE
#define DEVICE_CONFIG_MAX_SIZE 1024
#endif

#ifndef GROUPS
#define GROUPS {"all"}
#endif
