TimerHandle_t wifiReconnectTimer;

void connectToWifi();
void onWifiConnect();
void onWifiDisconnect();

void startWifiReconnectTimer() {
    xTimerStart(wifiReconnectTimer, 0);
}

void setHostname(const char *hostname) {
    WiFi.setHostname(hostname);
}

#ifdef MQTT_HOST
void connectToMqtt();

TimerHandle_t mqttReconnectTimer;

void startMqttReconnectTimer() {
    xTimerStart(mqttReconnectTimer, 0);
}

void stopMqttReconnectTimer() {
    xTimerStop(mqttReconnectTimer, 0);
}
#endif

void onWifiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n\r", event);

    switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
        onWifiConnect();
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        onWifiDisconnect();
        break;

    default:
        break;
    }
}

void setupWifi() {
    #ifdef MQTT_HOST
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    #endif
    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

    WiFi.onEvent(onWifiEvent);
}
