#include "espbase.h"

void setupNetwork();
void setupOta();
void setupHttp();
void handleNetwork();
void handleOta();

void setupEspbase() {
    setupNetwork();
    setupOta();
    setupHttp();
    Setting::load();
}

void handleEspbase() {
    handleNetwork();
    handleOta();
}
