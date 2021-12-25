#include "config.h"
#include "espbase.h"

Device *Device::head = NULL;

Device::Device(const char *configTopic, DeviceAnnounceFn announce)
    : announce(announce)
    , configTopic(configTopic) {

    // Insert at front of definitions list.
    next = head;
    head = this;
}

void Device::announceAll() {
    DynamicJsonDocument doc(DEVICE_CONFIG_MAX_SIZE);
    char buf[DEVICE_CONFIG_MAX_SIZE];

    for (Device *d = head; d != NULL; d = d->next) {
        logger->printf("Announcing device %s\n\r", d->configTopic);
        JsonObject obj = doc.to<JsonObject>();
        d->announce(obj);

        size_t len = serializeJson(obj, buf, DEVICE_CONFIG_MAX_SIZE);
        mqtt.publish(d->configTopic, 0, false, buf, len, 5);
    }
}


MqttSub mqttHassStatus(
    "homeassistant/status", false,
    [](const char *topic, const char *payload, size_t len) {
        if (len == 6 && !strncmp(payload, "online", 6)) {
            Device::announceAll();
        }
    }
);
