#include <cstdio>
#include <string>
#include "logger.h"
#include "M5Cardputer.h"
#include "Arduino.h"
#include "settings.h"

void logMessage(String message) {
    #ifdef SERIAL_LOGS
    printf("%s\n", message.c_str());
    #endif
}