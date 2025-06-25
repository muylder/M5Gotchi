#include <cstdio>
#include <string>
#include "logger.h"
#include "M5Cardputer.h"
#include "Arduino.h"
#include "settings.h"

void logMessage(String message) {
    #ifdef SERIAL_LOGS
    // Format: [I][logger.cpp:logMessage][time:12345] Message
    printf("[I][logger.cpp:logMessage][time:%lu] %s\n", millis(), message.c_str());
    #endif
}