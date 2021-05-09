#pragma once

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#elif ESP8266
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ESPAsyncTCP.h>
#include <WiFiUdp.h>
#endif

#include <ESPAsyncWebServer.h>

#include <AsyncMqttClient.h>

#ifdef MQTT_HOST
extern AsyncMqttClient mqtt;
#endif

#ifdef USE_NTP
#include <NTPClient.h>
extern NTPClient ntp;
#endif

#ifdef INFLUXDB_URL
#include <InfluxDbClient.h>
extern InfluxDBClient influx;
#endif

extern AsyncWebServer http;
