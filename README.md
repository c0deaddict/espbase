# Espbase Arduino library

Base for my Platformio projects running on a ESP8266 or ESP32.

## Include

Include the library in `platformio.ini`:

```ini
[env]
lib_deps = https://github.com/c0deaddict/espbase.git#main
```

## Config

Espbase requires configuration preprocessor variables to be declared at build
time. This can be accomplished by adding your config to the `build_flags` in
`platformio.ini`.

```ini
build_flags = -include "src/config.h"
```

See ./src/config.example.h for an example configuration.

## Main

Initialize espbase in `main.cpp`:

```c
#include <espbase.h>

void setup() {
    Serial.begin(115200);

    setupEspbase();
}

void loop() {
    handleEspbase();
}

void stop() {
    // Called before OTA is started, detach interrupts and stop timers.
}
```

## NTP

NTP is disabled by default. Enable it by defining `USE_NTP` in your config.h.
The ESP8266 seems to have a limit of 1 listening UDP socket, so if you want to
use that in your application you must disable NTP.

## HTTP

HTTP handlers must be registered before calling `setupEspbase`, for example:

```c
#include <espbase.h>

void handleGetTime(AsyncWebServerRequest *request);

void setup() {
    Serial.begin(115200);

    http.on("/time", HTTP_GET, handleGetTime);

    setupEspbase();
}

void loop() {
    handleEspbase();
}

void handleGetTime(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", ntp.getFormattedTime().c_str());
}
```

## MQTT

If you want to subscribe to MQTT messages, you must do so before calling
`setupEspbase`:

```c
#include <espbase.h>

void prepareMqtt();


void setup() {
    Serial.begin(115200);

    prepareMqtt();
    setupEspbase();
}

void loop() {
    handleEspbase();
}

void mqttOnMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    logger->printf("Received mqtt message on %s: %.*s\n\r", topic, len, payload);
}

// Must be called before setupEspbase()
void prepareMqtt() {
    mqtt.onConnect([](bool sessionPresent) {
        logger->println("Connected to MQTT!");
        mqtt.subscribe("sensors/+/pir", 0);
    });
    mqtt.onMessage(mqttOnMessage);
}
```
