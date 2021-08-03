#include <ArduinoOTA.h>

#include "config.h"
#include "logger.h"
#include "network.h"

extern void stop();

void setupOta() {
    ArduinoOTA.setPort(8266);
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);

    ArduinoOTA.onStart([]() {
        // Prepare for uploading.
        stop();

        #ifdef MQTT_HOST
        mqtt.disconnect();
        #endif

        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        logger->println("OTA: start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        logger->println("\nOTA: end");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        logger->printf("OTA: progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        logger->printf("OTA: error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) logger->println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) logger->println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) logger->println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) logger->println("Receive Failed");
        else if (error == OTA_END_ERROR) logger->println("End Failed");
    });
    ArduinoOTA.begin();
    logger->println("OTA: configured");
}

void handleOta() {
    ArduinoOTA.handle();
}
