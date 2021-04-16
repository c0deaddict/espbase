#include <Arduino.h>

#include "metrics.h"

#ifdef ESP32
#define ESP_VERSION 32
#else
#define ESP_VERSION 8266
extern "C"{
  #include "user_interface.h"
}
#endif

Metric* Metric::head = NULL;

const MetricProxy version(
    "esp_version", "gauge",
    "ESP version (8266 or 32) and some other metadata.",
    [](const char *name, Print *out) {
        out->printf("%s{sdk=\"%s\",chipid=%d} %u\n",
                    ESP.getSdkVersion(),
                    ESP.getChipId(),
                    ESP_VERSION);
    }
);

const MetricProxy freeHeap(
    "esp_free_heap", "gauge",
    "Current size of free heap memory in bytes.",
    [](const char *name, Print *out){
        out->printf("%s %u\n", name, ESP.getFreeHeap());
    }
);

const MetricProxy heapFragmentation(
    "esp_heap_fragmentation", "gauge",
    "Percentage of heap fragmentation (0-100).",
    [](const char *name, Print *out){
        out->printf("%s %u\n", name, ESP.getHeapFragmentation());
    }
);

#ifdef ESP32
const MetricProxy heapSize(
    "esp_heap_size", "gauge",
    "Total size of heap memory in bytes.",
    [](const char *name, Print *out){
        out->printf("%s %u\n", name, ESP.getHeapSize());
    }
);
#endif

const MetricProxy freeSketchSpace(
    "esp_free_sketch_space", "gauge",
    "Amount of bytes free for program.",
    [](const char *name, Print *out){
        out->printf("%s %u\n", name, ESP.getFreeSketchSpace());
    }
);

const MetricProxy uptime(
    "esp_uptime", "counter",
    "Microseconds since boot.",
    [](const char *name, Print *out){
        uint64_t time;
        #ifdef ESP32
        time = esp_timer_get_time();
        #else
        time = 1000L * (uint64_t)system_get_time();
        #endif
        out->printf("%s %lld\n", name, time);
    }
);

#ifdef ESP32
const MetricProxy taskCount(
    "esp_task_count",
    "gauge",
    "Number of tasks the FreeRTOS kernel is managing.",
    [](const char *name, Print *out){
        out->printf("%s %u\n", name, uxTaskGetNumberOfTasks());
    }
);
#endif

void printMetrics(Print *out) {
    Metric::writeMetrics(out);
}
