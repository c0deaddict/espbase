#include "config.h"
#include "espbase.h"

#ifdef MQTT_HOST

volatile bool mqttState = true;
void connectToMqtt();

#ifdef ESP32
TimerHandle_t mqttReconnectTimer;

void startMqttReconnectTimer() {
    xTimerStart(mqttReconnectTimer, 0);
}

void stopMqttReconnectTimer() {
    xTimerStop(mqttReconnectTimer, 0);
}
#else
Ticker mqttReconnectTimer;

void startMqttReconnectTimer() {
    mqttReconnectTimer.once_ms(2000, connectToMqtt);
}

void stopMqttReconnectTimer() {
    mqttReconnectTimer.detach();
}
#endif // ESP32

AsyncMqttClient mqtt;
Counter mqttDisconnected("esp_mqtt_disconnected", "Number of times MQTT is disconnected.");

String mqttDisconnectReasonToStr(AsyncMqttClientDisconnectReason reason) {
    switch (reason) {
    case AsyncMqttClientDisconnectReason::TCP_DISCONNECTED: return "TCP_DISCONNECTED";
    case AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION: return "MQTT_UNACCEPTABLE_PROTOCOL_VERSION";
    case AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED: return "MQTT_IDENTIFIER_REJECTED";
    case AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE: return "MQTT_SERVER_UNAVAILABLE";
    case AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS: return "MQTT_MALFORMED_CREDENTIALS";
    case AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED: return "MQTT_NOT_AUTHORIZED";
    case AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE: return "ESP8266_NOT_ENOUGH_SPACE";
    case AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT: return "TLS_BAD_FINGERPRINT";
    default: return "unknown";
    }
}

void connectToMqtt() {
    Serial.println("Connecting to MQTT...");
    mqtt.connect();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    mqttDisconnected.inc();
    Serial.printf("Disconnected from MQTT: %s\n\r", mqttDisconnectReasonToStr(reason).c_str());

    if (mqttState && WiFi.isConnected()) {
        startMqttReconnectTimer();
    }
}

void disconnectMqtt() {
    mqttState = false;
    stopMqttReconnectTimer();
    mqtt.disconnect();
}

void setupMqtt() {
    #ifdef ESP32
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    #endif

    mqtt.onDisconnect(onMqttDisconnect);
    mqtt.setCredentials(MQTT_USER, MQTT_PASSWORD);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
}

#endif // MQTT_HOST
