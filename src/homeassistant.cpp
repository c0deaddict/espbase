// listen to homeassistant/status
// if a message is published with 'online', announce sensors etc. on config topics.

// per sensor:
// {device_class: "temperature", "name": "temperature", "state_topic": "homeassistant/sensor/<node-id>/state", "unit_of_measurement": "*C", value_template: "{{ value_json.temperature }}" }
// {device_class": "humidity", "name": "Humidity", "state_topic": "homeassistant/sensor/sensorBedroom/state", "unit_of_measurement": "%", "value_template": "{{ value_json.humidity}}" }
// { "temperature": 23.20, "humidity": 43.70 }

// switch:
// {"name": "garden", "command_topic": "homeassistant/switch/irrigation/set", "state_topic": "homeassistant/switch/irrigation/state"}
// mosquitto_pub -h 127.0.0.1 -p 1883 -t "homeassistant/switch/irrigation/set" -m ON

// https://community.home-assistant.io/t/configuring-wled-with-mqtt/160918

// {"~": "homeassistant/switch/irrigation", "name": "garden", "cmd_t": "~/set", "stat_t": "~/state"}

/*
{
  "~": "homeassistant/light/kitchen",
  "name": "Kitchen",
  "unique_id": "kitchen_light",
  "cmd_t": "~/set",
  "stat_t": "~/state",
  "schema": "json",
  "brightness": true
}
 */

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

    for (Device *d = head; d != NULL; d = d->next) {
        JsonObject obj = doc.to<JsonObject>();
        d->announce(obj);

        String buf;
        size_t len = serializeJson(doc, buf);
        mqtt.publish(d->configTopic, 0, false, buf.c_str(), len);
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
