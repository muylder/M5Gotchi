#include <cstdio>
#include <string>
#include "logger.h"
#include "M5Cardputer.h"
#include "Arduino.h"
#include "settings.h"
#include "SD.h"

void logMessage(String message) {
    #ifdef SERIAL_LOGS
    // Format: [I][logger.cpp:logMessage][time:12345] Message
    printf("[%lu][I][logger.cpp:11] %s\n", millis(), message.c_str());

    if(sd_logging)
    {if(SD.exists("/log.txt")) {
        File logFile = SD.open("/log.txt", FILE_APPEND);
        if (logFile) {
            logFile.printf("[%lu][I][logger.cpp:11] %s\n", millis(), message.c_str());
            logFile.close();
        }
    } else {
        File logFile = SD.open("/log.txt", FILE_WRITE);
        if (logFile) {
            logFile.printf("[%lu][I][logger.cpp:11] %s\n", millis(), message.c_str());
            logFile.close();
        }
    }}
    #endif
}