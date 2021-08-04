#include "espbase.h"
#include <AsyncJson.h>

#ifdef ESP8266
#include <Ticker.h>
Ticker restartTimer;
#endif

AsyncWebServer http(80);

void handleGetMetrics(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/plain");
    printMetrics(response);
    request->send(response);
}

bool parseSaveSettingsHeader(AsyncWebServerRequest *request) {
    bool save = false;

    if (request->hasHeader("X-Save-Settings")) {
        const char *value = request->getHeader("X-Save-Settings")->value().c_str();
        save = !strcmp(value, "1");
    }

    return save;
}

void handlePostControl(AsyncWebServerRequest *request) {
    int numParams = request->params();
    bool changed = false;
    for (int i = 0; i < numParams; i++) {
        AsyncWebParameter *p = request->getParam(i);
        if (!p->isFile()) { // accept GET or POST parameters
            StaticJsonDocument<256> doc;
            DeserializationError err = deserializeJson(doc, p->value());
            if (err) {
                logger->print(F("handlePostControl: deserializeJson() failed: "));
                logger->println(err.c_str());
            }
            else if (Setting::set(p->name().c_str(), doc.as<JsonVariant>())) {
                changed = true;
            }
        }
    }

    if (changed && parseSaveSettingsHeader(request)) {
        Setting::save();
    }

    request->send(200, "text/plain", "Ok");
}

void handleGetSettings(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    Setting::printTo(*response);
    request->send(response);
}

void handlePostSettings(AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject obj = json.as<JsonObject>();
    if (Setting::patch(&obj) && parseSaveSettingsHeader(request)) {
        Setting::save();
    }

    AsyncResponseStream *response = request->beginResponseStream("application/json");
    Setting::printTo(*response);
    request->send(response);
}

void handleSaveSettings(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Ok");
    Setting::save();
}

void handleRestart(AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Ok");
    #ifdef ESP32
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP.restart();
    #else
    restartTimer.once_ms(100, [](){ ESP.restart(); });
    #endif
}

void handleDescribe(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DeviceDesc::printTo(*response);
    request->send(response);
}

void setupHttp() {
    http.on("/metrics", HTTP_GET, handleGetMetrics);
    http.on("/control", HTTP_POST, handlePostControl);
    http.on("/settings", HTTP_GET, handleGetSettings);
    http.on("/save", HTTP_POST, handleSaveSettings);
    http.on("/restart", HTTP_POST, handleRestart);
    http.on("/describe", HTTP_GET, handleDescribe);

    AsyncCallbackJsonWebHandler* postSettings =
        new AsyncCallbackJsonWebHandler("/settings", handlePostSettings);
    postSettings->setMethod(HTTP_POST);
    http.addHandler(postSettings);

    http.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not Found");
    });
 
    http.begin();
}
