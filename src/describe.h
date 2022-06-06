#pragma once

#include <vector>
#include <ArduinoJson.h>

typedef std::function<void(JsonObject &obj)> DeviceDescFn;

class DeviceDesc {
private:
    static DeviceDesc *head;
    DeviceDesc *next;
    DeviceDescFn fn;

public:
    static const std::vector<const char *> groups;

    DeviceDesc(DeviceDescFn fn);
    static void describe(JsonObject &obj);
    static size_t printTo(Print &out);
    static size_t printTo(String &out);
};
