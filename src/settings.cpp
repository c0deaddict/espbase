#include <Arduino.h>
#include <StreamUtils.h>

#include "config.h"
#include "settings.h"

#define SETTINGS_VERSION 1

DynamicJsonDocument settings(SETTINGS_MAX_SIZE);

Setting *head = NULL;

Setting::Setting(const char *name, SettingSetDefaultFn setDefault, SettingChangeFn change)
    : name(name), setDefault(setDefault), change(change) {
    // Set default
    JsonVariant value = settings.getOrAddMember(name);
    setDefault(value);

    // Insert at front of definitions list.
    next = head;
    head = this;
}

void loadSettings() {
    EEPROM.begin(SETTINGS_MAX_SIZE);

    settings["version"] = SETTINGS_VERSION;

    DynamicJsonDocument doc(SETTINGS_MAX_SIZE);
    EepromStream eepromStream(0, SETTINGS_MAX_SIZE);

    DeserializationError err = deserializeJson(doc, eepromStream);
    if (err) {
        Serial.print(F("loadSettings: deserializeJson() failed: "));
        Serial.println(err.c_str());
    } else if (!doc.is<JsonObject>()) {
        Serial.println("loadSettings: saved settings is not an object");
    } else {
        if (doc["version"] != SETTINGS_VERSION) {
            // TODO: migrate settings
        }
        doc.remove("version");

        Serial.println("loaded settings:");
        serializeJson(doc, Serial);
        Serial.println();

        JsonObject obj = doc.as<JsonObject>();
        mergeSettings(&obj);
    }
}

void saveSettings() {
    Serial.println("Saving settings..");

    // https://arduinojson.org/v6/issues/memory-leak/
    settings.garbageCollect();

    EepromStream eepromStream(0, SETTINGS_MAX_SIZE);
    serializeJson(settings, eepromStream);
    eepromStream.flush();
}

bool setSetting(const char *name, JsonVariant value) {
    for (Setting *s = head; s != NULL; s = s->next) {
        if (!strcmp(name, s->name)) {
            if (!s->change(value)) {
                String serializedValue;
                serializeJson(value, serializedValue);
                Serial.printf("Invalid value '%s' for setting %s\n\r", serializedValue.c_str(), name);
                return false;
            } else {
                settings[name] = value;
                return true;
            }
        }
    }

    Serial.printf("Trying to set undefined setting %s\n\r", name);
    return false;
}

bool mergeSettings(JsonObject *object) {
    bool ok = false;
    for (auto kvp : *object) {
        ok |= setSetting(kvp.key().c_str(), kvp.value());
    }
    return ok;
}

bool mergeSettings(const char *str, size_t len) {
    DynamicJsonDocument doc(SETTINGS_MAX_SIZE);
    DeserializationError err = deserializeJson(doc, str, len);
    if (err) {
        Serial.print(F("mergeSettings: deserializeJson() failed: "));
        Serial.println(err.c_str());
        return false;
    }

    JsonObject obj = doc.as<JsonObject>();
    return mergeSettings(&obj);
}

String getSettingsAsJson() {
    String output;
    serializeJson(settings, output);
    return output;
}
