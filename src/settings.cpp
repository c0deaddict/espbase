#include <Arduino.h>
#include <StreamUtils.h>

#include "config.h"
#include "settings.h"
#include "metrics.h"
#include "logger.h"

#define SETTINGS_VERSION 1

size_t memory_usage = 0;
Setting *Setting::head = NULL;

// TODO: look into https://github.com/xoseperez/eeprom_rotate
// https://platformio.org/lib/show/5512/EEPROM32_Rotate
// https://platformio.org/lib/show/5456/EEPROM_Rotate

Setting::Setting(const char *name, SettingGetFn getter, SettingSetFn setter)
    : name(name), getter(getter), setter(setter) {
    // Insert at front of definitions list.
    next = head;
    head = this;
}

void Setting::load() {
    EEPROM.begin(SETTINGS_MAX_SIZE);

    DynamicJsonDocument doc(SETTINGS_MAX_SIZE);
    EepromStream eepromStream(0, SETTINGS_MAX_SIZE);

    DeserializationError err = deserializeJson(doc, eepromStream);
    if (err) {
        logger->print("Settings: load: deserializeJson() failed: ");
        logger->println(err.c_str());
    } else if (!doc.is<JsonObject>()) {
        logger->println("Settings: saved settings is not an object");
    } else {
        memory_usage = doc.memoryUsage();

        int version = doc["version"].as<int>();
        if (version != SETTINGS_VERSION) {
            logger->printf("Settings: version is unexpected: %d\n\r", version);
            // TODO: migrate settings?
        }
        doc.remove("version");

        logger->print("Settings: loaded: ");
        serializeJson(doc, *logger);
        logger->println();

        JsonObject obj = doc.as<JsonObject>();
        patch(&obj);
    }
}

void Setting::save() {
    EepromStream eepromStream(0, SETTINGS_MAX_SIZE);
    printTo(eepromStream);
    eepromStream.flush();
    logger->println("Settings: saved");
}

bool Setting::set(const char *name, JsonVariant value) {
    for (Setting *s = Setting::head; s != NULL; s = s->next) {
        if (!strcmp(name, s->name)) {
            if (!s->setter(value)) {
                String serializedValue;
                serializeJson(value, serializedValue);
                logger->printf("Settings: invalid value '%s' for %s\n\r", serializedValue.c_str(), name);
                return false;
            }

            return true;
        }
    }

    logger->printf("Settings: trying to set undefined setting %s\n\r", name);
    return false;
}

bool Setting::patch(JsonObject *object) {
    bool ok = false;
    for (auto kvp : *object) {
        ok |= set(kvp.key().c_str(), kvp.value());
    }
    return ok;
}

bool Setting::patch(const char *str, size_t len) {
    DynamicJsonDocument doc(SETTINGS_MAX_SIZE);
    DeserializationError err = deserializeJson(doc, str, len);
    if (err) {
        logger->print("Settings: patch: deserializeJson() failed: ");
        logger->println(err.c_str());
        return false;
    }

    JsonObject obj = doc.as<JsonObject>();
    return patch(&obj);
}

void Setting::toJson(JsonObject &obj) {
    for (Setting *s = Setting::head; s != NULL; s = s->next) {
        s->getter(obj, s->name);
    }

    obj["version"] = SETTINGS_VERSION;
}

void Setting::printTo(Print &out) {
    DynamicJsonDocument doc(SETTINGS_MAX_SIZE);
    JsonObject obj = doc.to<JsonObject>();
    toJson(obj);
    memory_usage = doc.memoryUsage();
    serializeJson(doc, out);
}

const MetricProxy settingsBytes(
    "esp_settings_bytes", "gauge",
    "Amount of bytes used for storing the settings.",
    [](const char *name, Print *out){
        out->printf("%s %u\n", name, memory_usage);
    }
);
