#include "config.h"
#include "espbase.h"

const std::vector<const char *> DeviceDesc::groups = GROUPS;

DeviceDesc *DeviceDesc::head = NULL;

DeviceDesc::DeviceDesc(DeviceDescFn fn) : fn(fn) {
    // Insert at front of definitions list.
    next = head;
    head = this;
}

void DeviceDesc::describe(JsonObject &obj) {
    obj["hostname"] = HOSTNAME;
    obj["ip"] = WiFi.localIP().toString();

    JsonArray groups = obj.createNestedArray("groups");
    for (const char *g : DeviceDesc::groups) {
        groups.add(g);
    }

    JsonObject settings = obj.createNestedObject("settings");
    Setting::toJson(settings);

    for (DeviceDesc *d = head; d != NULL; d = d->next) {
        d->fn(obj);
    }
}

size_t DeviceDesc::printTo(Print &out) {
    DynamicJsonDocument doc(DEVICE_DESC_MAX_SIZE);
    JsonObject obj = doc.to<JsonObject>();
    describe(obj);
    return serializeJson(obj, out);
}

size_t DeviceDesc::printTo(String &out) {
    DynamicJsonDocument doc(DEVICE_DESC_MAX_SIZE);
    JsonObject obj = doc.to<JsonObject>();
    describe(obj);
    return serializeJson(doc, out);
}
