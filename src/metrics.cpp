#include <Arduino.h>

#include "metrics.h"

Metric* Metric::head = NULL;

const MetricProxy freeHeap(
    "esp_free_heap", "gauge",
    "Current size of free heap memory in bytes.",
    [](const char *name, Print *out){
        out->printf("%s %u\n", name, ESP.getFreeHeap());
    }
);

const MetricProxy heapSize(
    "esp_heap_size", "gauge",
    "Total size of heap memory in bytes.",
    [](const char *name, Print *out){
        out->printf("%s %u\n", name, ESP.getHeapSize());
    }
);

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
        out->printf("%s %lld\n", name, esp_timer_get_time());
    }
);

const MetricProxy taskCount(
    "esp_task_count",
    "gauge",
    "Number of tasks the FreeRTOS kernel is managing.",
    [](const char *name, Print *out){
        out->printf("%s %u\n", name, uxTaskGetNumberOfTasks());
    }
);

/**
 * Gather info about all running tasks.
 *
 * https://www.freertos.org/uxTaskGetSystemState.html#TaskStatus_t
 */
void printTaskMetrics(Print *response) {
    // Arduino SDK does not have: configUSE_TRACE_FACILITY=1
    #if false
    // Take a snapshot of the number of tasks in case it changes while this
    // function is executing.
    size_t numTasks = uxTaskGetNumberOfTasks();

    // Allocate a TaskStatus_t structure for each task.
    TaskStatus_t *taskStatusArray = (TaskStatus_t *)malloc(numTasks * sizeof(TaskStatus_t));
    if (taskStatusArray != NULL) {
        uint32_t totalRuntime;
        numTasks = uxTaskGetSystemState(taskStatusArray, numTasks, &totalRuntime);

        response->print("# HELP esp_task_runtime_total FreeRTOS total runtime for all tasks.\n");
        response->print("# TYPE esp_task_runtime_total counter\n");
        response->printf("esp_task_runtime_total %u\n", totalRuntime);

        response->print("# HELP esp_task_runtime Runtime for a task.\n");
        response->print("# TYPE esp_task_runtime counter\n");
        
        for (int x = 0; x < numTasks; x++) {
            TaskStatus_t *status = &taskStatusArray[x];
            response->printf("esp_task_runtime{task=\"%s\"} %u\n",
                             status->pcTaskName, status->ulRunTimeCounter);

            // TODO: could report on status->usStackHighWaterMark
            /* The minimum amount of stack space that has remained for the task since
               the task was created.  The closer this value is to zero the closer the task
               has come to overflowing its stack. */
        }
    }
    #endif
}

void printMetrics(Print *out) {
    Metric::writeMetrics(out);
    printTaskMetrics(out);
}
