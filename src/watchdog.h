#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "esp_task_wdt.h"
#include "logger.h"

class Watchdog {
private:
    static const uint32_t TIMEOUT_SECONDS = 30;
    static bool initialized;
    
public:
    static void init() {
        if (initialized) return;
        
        esp_err_t err = esp_task_wdt_init(TIMEOUT_SECONDS, true);
        if (err == ESP_OK) {
            esp_task_wdt_add(NULL);
            initialized = true;
            logMessage("Watchdog timer initialized (" + String(TIMEOUT_SECONDS) + "s timeout)");
        } else {
            logMessage("Failed to initialize watchdog timer");
        }
    }
    
    static void feed() {
        if (initialized) {
            esp_task_wdt_reset();
        }
    }
    
    static void disable() {
        if (initialized) {
            esp_task_wdt_delete(NULL);
            esp_task_wdt_deinit();
            initialized = false;
            logMessage("Watchdog timer disabled");
        }
    }
};

bool Watchdog::initialized = false;

#endif // WATCHDOG_H
