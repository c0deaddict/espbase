void connectToWifi();
void onWifiConnect();
void onWifiDisconnect();

Ticker wifiReconnectTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

void startWifiReconnectTimer() {
    wifiReconnectTimer.once(2, connectToWifi);
}

void setHostname(const char *hostname) {
    WiFi.hostname(hostname);
}

void esp8266_onWifiConnect(const WiFiEventStationModeGotIP& event) {
    onWifiConnect();
}

void esp8266_onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
    onWifiDisconnect();
}

#ifdef MQTT_HOST
void mqttReconnect();

Ticker mqttReconnectTimer;

void startMqttReconnectTimer() {
    mqttReconnectTimer.once(2, mqttReconnect);
}

void stopMqttReconnectTimer() {
    mqttReconnectTimer.detach();
}
#endif

void setupWifi() {
    wifiConnectHandler = WiFi.onStationModeGotIP(esp8266_onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(esp8266_onWifiDisconnect);
}
