#include "espbase.h"

void setupNetwork();
void setupOta();
void setupHttp();
void handleNetwork();
void handleOta();
void loadSettings();

void setupEspbase() {
    Serial.begin(115200);

    setupNetwork();
    setupOta();
    setupHttp();
    loadSettings();
}

void handleEspbase() {
    handleNetwork();
    handleOta();
}
