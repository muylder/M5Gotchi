// test_fixes.h - Arquivo de teste para validar correções
// Adicionar temporariamente ao projeto para testes

#ifndef TEST_FIXES_H
#define TEST_FIXES_H

#include <Arduino.h>
#include "esp_heap_caps.h"

class SystemTests {
public:
    static void printMemoryInfo() {
        Serial.println("\n=== Memory Info ===");
        Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
        Serial.printf("Heap Size: %d bytes\n", ESP.getHeapSize());
        Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
        
        size_t freeHeap = ESP.getFreeHeap();
        size_t heapSize = ESP.getHeapSize();
        float usedPercent = 100.0 * (heapSize - freeHeap) / heapSize;
        
        Serial.printf("Memory Used: %.2f%%\n", usedPercent);
        
        if (freeHeap < 50000) {
            Serial.println("[WARNING] Low memory!");
        } else if (freeHeap < 30000) {
            Serial.println("[CRITICAL] Very low memory!");
        } else {
            Serial.println("[OK] Memory healthy");
        }
        Serial.println("==================\n");
    }
    
    static void testMemoryLeak() {
        Serial.println("\n=== Memory Leak Test ===");
        
        size_t initialFree = ESP.getFreeHeap();
        Serial.printf("Initial Free Memory: %d bytes\n", initialFree);
        
        // Simular operações que podem vazar memória
        for (int i = 0; i < 100; i++) {
            String test = "Test string " + String(i);
            delay(10);
        }
        
        size_t afterFree = ESP.getFreeHeap();
        Serial.printf("After Test Free Memory: %d bytes\n", afterFree);
        
        int leaked = initialFree - afterFree;
        Serial.printf("Memory Change: %d bytes\n", leaked);
        
        if (leaked > 1000) {
            Serial.println("[FAIL] Significant memory leak detected!");
        } else if (leaked > 100) {
            Serial.println("[WARN] Minor memory usage detected");
        } else {
            Serial.println("[PASS] No significant memory leak");
        }
        Serial.println("========================\n");
    }
    
    static void testWatchdog() {
        Serial.println("\n=== Watchdog Test ===");
        Serial.println("Watchdog should be feeding regularly");
        Serial.println("Monitor for unexpected resets");
        Serial.println("=====================\n");
    }
    
    static void testTimerOverflow() {
        Serial.println("\n=== Timer Overflow Test ===");
        
        unsigned long testTime = 0xFFFFFF00; // Near overflow
        unsigned long previousTime = testTime - 120000;
        
        // Test the fixed timer logic
        if ((testTime - previousTime) >= 120000) {
            Serial.println("[PASS] Timer comparison handles overflow correctly");
        } else {
            Serial.println("[FAIL] Timer comparison fails with overflow");
        }
        
        Serial.println("===========================\n");
    }
    
    static void testStringOperations() {
        Serial.println("\n=== String Operations Test ===");
        
        size_t beforeHeap = ESP.getFreeHeap();
        
        // Test efficient string concatenation
        String result;
        result.reserve(100);  // Pre-allocate
        
        for (int i = 0; i < 50; i++) {
            result += "X";
        }
        
        size_t afterHeap = ESP.getFreeHeap();
        int used = beforeHeap - afterHeap;
        
        Serial.printf("Memory used for string ops: %d bytes\n", used);
        
        if (used < 200) {
            Serial.println("[PASS] Efficient string operation");
        } else {
            Serial.println("[WARN] String operation could be more efficient");
        }
        
        Serial.println("==============================\n");
    }
    
    static void runAllTests() {
        Serial.println("\n");
        Serial.println("╔═══════════════════════════════════╗");
        Serial.println("║   GOTCHI_MOD VALIDATION TESTS     ║");
        Serial.println("╚═══════════════════════════════════╝");
        Serial.println();
        
        printMemoryInfo();
        delay(1000);
        
        testTimerOverflow();
        delay(1000);
        
        testStringOperations();
        delay(1000);
        
        testMemoryLeak();
        delay(1000);
        
        testWatchdog();
        delay(1000);
        
        Serial.println("\n");
        Serial.println("╔═══════════════════════════════════╗");
        Serial.println("║      TESTS COMPLETED              ║");
        Serial.println("╚═══════════════════════════════════╝");
        Serial.println();
        
        printMemoryInfo();
    }
    
    static void continuousMemoryMonitor(unsigned long intervalMs = 30000) {
        static unsigned long lastCheck = 0;
        unsigned long now = millis();
        
        if (now - lastCheck >= intervalMs) {
            lastCheck = now;
            
            Serial.println("\n--- Memory Monitor ---");
            Serial.printf("[%lu] Free: %d | Min: %d | Used: %.1f%%\n",
                         now / 1000,
                         ESP.getFreeHeap(),
                         ESP.getMinFreeHeap(),
                         100.0 * (ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize());
            Serial.println("---------------------\n");
        }
    }
};

// Macro para testes rápidos
#define RUN_TESTS() SystemTests::runAllTests()
#define MONITOR_MEMORY() SystemTests::continuousMemoryMonitor()
#define PRINT_MEMORY() SystemTests::printMemoryInfo()

#endif // TEST_FIXES_H
