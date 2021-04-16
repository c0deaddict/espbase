#pragma once

#include <Arduino.h>

typedef void (*WriteMetricProxy)(const char *name, Print *out);

class Metric {
protected:
    static Metric *head;

    const Metric *next = NULL;

    const char *name;
    const char *type;
    const char *help;

    virtual void write(Print *out) const = 0;

public:
    Metric(const char *name, const char *type, const char *help)
        : name(name), type(type), help(help) {
        next = head;
        head = this;
    }

    static void writeMetrics(Print *out) {
        for (const Metric *m = head; m != NULL; m = m->next) {
            out->printf("# HELP %s %s\n", m->name, m->help);
            out->printf("# TYPE %s %s\n", m->name, m->type);
            m->write(out);
        }
    }
};

class MetricProxy : public Metric {
protected:
    WriteMetricProxy writeProxy;

    void write(Print *out) const {
        writeProxy(name, out);
    }

public:
    MetricProxy(const char *name, const char *type, const char *help, WriteMetricProxy writeProxy)
        : Metric(name, type, help), writeProxy(writeProxy) {
    }
};

class Counter : public Metric {
protected:
    unsigned long long value = 0;

    void write(Print *out) const {
        out->printf("%s %llu\n", name, value);
    }

public:
    Counter(const char *name, const char *help)
        : Metric(name, "counter", help) {
    }

    void inc() {
        value++;
    }

    void inc(unsigned long long i) {
        value += i;
    }
};

class Gauge : public Metric {
protected:
    long long value = 0;

    void write(Print *out) const {
        out->printf("%s %lld\n", name, value);
    }

public:
    Gauge(const char *name, const char *help)
        : Metric(name, "gauge", help) {
    }

    void inc() {
        value++;
    }

    void dec() {
        value--;
    }

    void inc(long long i) {
        value += i;
    }

    void dec(long long i) {
        value -= i;
    }

    void set(long long i) {
        value = i;
    }
};    

void printMetrics(Print *out);
