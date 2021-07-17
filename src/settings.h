#pragma once

#include <ArduinoJson.h>
#include <functional>

typedef std::function<void(JsonVariant& value)> SettingSetDefaultFn;
typedef std::function<bool(JsonVariant value)> SettingChangeFn;

class Setting {
protected:
    const char *name;
    SettingSetDefaultFn setDefault;
    SettingChangeFn change;
    Setting *next;

public:
    Setting(const char *name, SettingSetDefaultFn setDefault, SettingChangeFn change);

    friend bool setSetting(const char *name, JsonVariant value);
    friend void loadSettings();
};

void saveSettings();
bool setSetting(const char *name, JsonVariant value);
void printSettings(Print &out);
bool mergeSettings(JsonObject *object);
bool mergeSettings(const char *str, size_t len);
