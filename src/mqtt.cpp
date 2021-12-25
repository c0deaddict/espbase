#include "config.h"
#include "espbase.h"

using namespace std::placeholders;

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
    reconnectTimer.once_ms(2000, std::bind(&Mqtt::connect, this));
}

void Mqtt::stopReconnectTimer() {
    reconnectTimer.detach();
}
#endif // ESP32

MqttSub *MqttSub::head = NULL;

MqttSub::MqttSub(const char *pattern, bool targeted, MqttHandler handler)
    : pattern(pattern), targeted(targeted), handler(handler) {
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

bool MqttSub::isTarget(const char *topic) {
    if (!targeted) {
        return true;
    }

    const char *target = strrchr(topic, '/');
    if (target == NULL) return false;
    target++; // Skip the slash

    if (!strcmp(target, HOSTNAME)) {
        return true;
    }

    for (const char *group : DeviceDesc::groups) {
        if (!strcmp(target, group)) {
            return true;
        }
    }

    return false;
}

void MqttSub::subscribe(AsyncMqttClient *client) {
    logger->printf("MQTT: subscribed to %s\n\r", pattern);
    client->subscribe(pattern, 0);
}

Mqtt::Mqtt() : state(true) {
}

void Mqtt::connect() {
    state = true;
    logger->println("MQTT: connecting...");
    client.connect();
}

void Mqtt::disconnect() {
    stopReconnectTimer();
    state = false;
    client.disconnect();
}

void Mqtt::onConnect(bool sessionPresent) {
    logger->println("MQTT: connected");
    for (MqttSub *sub = MqttSub::head; sub != NULL; sub = sub->next) {
        sub->subscribe(&client);
    }

    Device::announceAll();
}

void Mqtt::onDisconnect(AsyncMqttClientDisconnectReason reason) {
    mqttDisconnected.inc();
    logger->printf("MQTT: disconnected: %s\n\r", mqttDisconnectReasonToStr(reason).c_str());

    if (state && WiFi.isConnected()) {
        startReconnectTimer();
    }
}

void Mqtt::onMessage(const char *topic, const char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    logger->printf("MQTT: received message on topic %s\n\r", topic);
    mqttMessagesReceived.inc();
    for (MqttSub *sub = MqttSub::head; sub != NULL; sub = sub->next) {
        if (sub->match(topic)) {
            // logger->printf("MQTT: topic %s matched %s\n\r", topic, sub->pattern);
            if (len != total) {
                logger->printf("MQTT: received too large payload: %u != %u (ignoring message)\n\r", len, total);
            } else if (sub->isTarget(topic)) {
                sub->handler(topic, payload, len);
            } else {
                // logger->printf("MQTT: topic %s does not match target\n\r", topic);
            }
            return;
        }
    }

    // Should never happen!
    logger->printf("MQTT: no handler matched topic %s\n\r", topic);
}

void Mqtt::setup() {
    #ifdef ESP32
    reconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)this, [](TimerHandle_t timer) {
        Mqtt *mqtt = (Mqtt *)pvTimerGetTimerID(timer);
        mqtt->connect();
    });
    #endif

    client.setCredentials(MQTT_USER, MQTT_PASSWORD);
    client.setServer(MQTT_HOST, MQTT_PORT);

    client.onConnect(std::bind(&Mqtt::onConnect, this, _1));
    client.onDisconnect(std::bind(&Mqtt::onDisconnect, this, _1));
    client.onMessage(std::bind(&Mqtt::onMessage, this, _1, _2, _3, _4, _5, _6));
}

bool Mqtt::publish(const char *topic, uint8_t qos, bool retain, const char *payload, size_t len, int retries) {
    if (!client.connected()) {
        return false;
    }

    int sleep = 10;

    do {
        int result = client.publish(topic, qos, retain, payload, len);
        if (result != 0) {
            return true;
        }

        delay(sleep);
        sleep *= 2;
    } while (retries-- > 0);

    logger->printf("MQTT publish to %s failed after 3 retries\r\n", topic);
    return false;
}

MqttSub mqttDiscover(
    "esp/discovery/request", false,
    [](const char *topic, const char *payload, size_t len) {
        String buf;
        size_t buflen = DeviceDesc::printTo(buf);
        mqtt.publish("esp/discovery/response", 0, false, buf.c_str(), buflen);
    }
);

MqttSub mqttPatchSettings(
    "esp/settings/patch/+", true,
    [](const char *topic, const char *payload, size_t len) {
        Setting::patch(payload, len);
    }
);

MqttSub mqttSaveSettings(
    "esp/settings/save/+", true,
    std::bind(&Setting::save)
);

#endif // MQTT_HOST
