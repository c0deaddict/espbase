#pragma once

#include <ArduinoJson.h>
#include <AsyncMqttClient.h>

typedef std::function<void(const char *topic, const char *payload, size_t len)> MqttHandler;

class MqttSub {
private:
    friend class Mqtt;

    static MqttSub *head;
    MqttSub *next;
    const char *pattern;
    bool targeted;
    MqttHandler handler;

    void subscribe(AsyncMqttClient *client);

public:
    MqttSub(const char *pattern, bool targeted, MqttHandler handler);
    bool match(const char *topic);
    bool isTarget(const char *topic);
};

class Mqtt {
private:
    volatile bool state;
    AsyncMqttClient client;

    #ifdef ESP32
    TimerHandle_t reconnectTimer;
    #else
    Ticker reconnectTimer;
    #endif

    void startReconnectTimer();

    void onConnect(bool sessionPresent);
    void onDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMessage(const char *topic, const char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

public:
    Mqtt();
    void setup();
    void connect();
    void disconnect();
    void stopReconnectTimer();
    bool publish(const char *topic, uint8_t qos, bool retain, const char *payload, size_t len = 0, int retries = 0);
};

extern Mqtt mqtt;
