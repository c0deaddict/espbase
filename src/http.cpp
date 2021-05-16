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

void handlePostControl(AsyncWebServerRequest *request) {
    int numParams = request->params();
    for (int i = 0; i < numParams; i++) {
        AsyncWebParameter *p = request->getParam(i);
        if (!p->isFile()) { // accept GET or POST parameters
            DynamicJsonDocument doc(256);
            DeserializationError err = deserializeJson(doc, p->value());
            if (err) {
                Serial.print(F("handlePostControl: deserializeJson() failed: "));
                Serial.println(err.c_str());
            }
            else if (setSetting(p->name().c_str(), doc.as<JsonVariant>())) {
                saveSettings();
            }
        }
    }

    request->send(200, "application/json", getSettingsAsJson());
}

void handleGetSettings(AsyncWebServerRequest *request) {
    request->send(200, "application/json", getSettingsAsJson());
}

void handlePostSettings(AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject obj = json.as<JsonObject>();
    if (mergeSettings(&obj)) {
        saveSettings();
    }
    request->send(200, "application/json", getSettingsAsJson());
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

void setupHttp() {
    http.on("/metrics", HTTP_GET, handleGetMetrics);
    http.on("/control", HTTP_POST, handlePostControl);
    http.on("/settings", HTTP_GET, handleGetSettings);
    http.on("/restart", HTTP_POST, handleRestart);

    AsyncCallbackJsonWebHandler* postSettings =
        new AsyncCallbackJsonWebHandler("/settings", handlePostSettings);
    postSettings->setMethod(HTTP_POST);
    http.addHandler(postSettings);

    http.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not Found");
    });
 
    http.begin();
}
