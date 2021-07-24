#pragma once

#include <ArduinoJson.h>
#include <functional>

typedef std::function<void(JsonDocument &doc, const char *name)> SettingGetFn;
typedef std::function<bool(JsonVariant value)> SettingSetFn;

class Setting {
protected:
    const char *name;
    SettingGetFn getter;
    SettingSetFn setter;
    Setting *next;

public:
    Setting(const char *name, SettingGetFn getter, SettingSetFn setter);

    static void load();
    static void save();
    static void printTo(Print &out);
    static bool set(const char *name, JsonVariant value);
    static bool patch(JsonObject *object);
    static bool patch(const char *str, size_t len);
};
