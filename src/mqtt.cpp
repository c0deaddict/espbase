#include "config.h"
#include "espbase.h"
#include <stdarg.h>

#ifdef MQTT_HOST
Mqtt mqtt;

Counter mqttDisconnected("esp_mqtt_disconnected", "Number of times MQTT is disconnected.");
Counter mqttMessagesReceived("esp_mqtt_messages_received", "Number of messages received over MQTT.");

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

#ifdef ESP32
void Mqtt::startReconnectTimer() {
    xTimerStart(reconnectTimer, 0);
}

void Mqtt::stopReconnectTimer() {
    xTimerStop(reconnectTimer, 0);
}
#else
void Mqtt::startReconnectTimer() {
    reconnectTimer.once_ms(2000, [this]() {
        connect();
    });
}

void Mqtt::stopReconnectTimer() {
    reconnectTimer.detach();
}
#endif // ESP32

MqttSub *MqttSub::head = NULL;

MqttSub::MqttSub(const char *pattern, MqttHandler handler) : pattern(pattern), handler(handler) {
    // Insert at front of definitions list.
    next = head;
    head = this;
}

bool MqttSub::match(const char *topic) {
    // / = topic separator
    // # = multi level wildcard
    // + = single level wildcard
    const char *p = pattern;
    for (; *p != 0 && *topic != 0; p++, topic++) {
        if (*p == '#') {
            return true;
        } else if (*p == '+') {
            const char *sep = strchr(topic, '/');
            if (sep == NULL) {
                // Pattern matches until the end.
                return true;
            } else {
                // Step back one, because loop will increment topic.
                topic = sep - 1;
            }
        } else if (*p != *topic) {
            return false;
        }
    }

    return *p == 0 && *topic == 0;
}

void MqttSub::subscribe(AsyncMqttClient *client) {
    logger->printf("MQTT: subscribed to %s\n\r", pattern);
    client->subscribe(pattern, 0);
}

void Mqtt::connect() {
    logger->println("MQTT: connecting...");
    client.connect();
}

void Mqtt::disconnect() {
    state = false;
    stopReconnectTimer();
    client.disconnect();
}

void Mqtt::setup() {
    #ifdef ESP32
    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connect));
    #endif

    client.setCredentials(MQTT_USER, MQTT_PASSWORD);
    client.setServer(MQTT_HOST, MQTT_PORT);

    client.onConnect([this](bool sessionPresent) {
        logger->println("MQTT: connected");
        for (MqttSub *sub = MqttSub::head; sub != NULL; sub = sub->next) {
            sub->subscribe(&client);
        }
    });

    client.onDisconnect([this](AsyncMqttClientDisconnectReason reason) {
        mqttDisconnected.inc();
        logger->printf("MQTT: disconnected: %s\n\r", mqttDisconnectReasonToStr(reason).c_str());

        if (state && WiFi.isConnected()) {
            startReconnectTimer();
        }
    });    

    client.onMessage([this](const char *topic, const char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
        logger->printf("MQTT: received message on topic %s\n\r", topic);
        mqttMessagesReceived.inc();
        for (MqttSub *sub = MqttSub::head; sub != NULL; sub = sub->next) {
            if (sub->match(topic)) {
                logger->printf("MQTT: topic %s matched %s\n\r", topic, sub->pattern);
                if (len != total) {
                    logger->printf("MQTT: received too large payload: %u != %u (ignoring message)\n\r", len, total);
                } else {
                    sub->handler(topic, payload, len);
                }
                return;
            }
        }

        // Should never happen!
        logger->printf("MQTT: no handler matched topic %s\n\r", topic);
    });
}

void Mqtt::publish(const char *topic, uint8_t qos, bool retain, const char *payload, size_t len) {
    client.publish(topic, qos, retain, payload, len);
}

#endif // MQTT_HOST
