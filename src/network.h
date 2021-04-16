#pragma once

#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
extern "C" {
	#include "freertos/FreeRTOS.h"
	#include "freertos/timers.h"
}
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ESPAsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>

#include <AsyncMqttClient.h>
#include <NTPClient.h>

#ifdef MQTT_HOST
extern AsyncMqttClient mqtt;
#endif

extern NTPClient ntp;

extern AsyncWebServer http;
