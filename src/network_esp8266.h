void connectToWifi();
void onWifiConnect();
void onWifiDisconnect();

Ticker wifiReconnectTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

void startWifiReconnectTimer() {
    wifiReconnectTimer.once(2, connectToWifi);
}

void esp8266_onWifiConnect(const WiFiEventStationModeGotIP& event) {
    onWifiConnect();
}

void esp8266_onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
    onWifiDisconnect();
}

void setupWifi() {
    wifiConnectHandler = WiFi.onStationModeGotIP(esp8266_onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(esp8266_onWifiDisconnect);

    WiFi.persistent(false);
}
