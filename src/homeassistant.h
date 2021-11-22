#pragma once

#include <ArduinoJson.h>

typedef std::function<void(JsonObject &obj)> DeviceAnnounceFn;

class Device {
private:
    static Device *head;

    Device *next;
    const DeviceAnnounceFn announce;
    const char *configTopic;

public:
    static void announceAll();

    Device(const char *configTopic, DeviceAnnounceFn announce);
};

#define HASS_TOPIC(component, id) "homeassistant/" component "/" HOSTNAME "/" id
