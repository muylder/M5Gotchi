#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "logger.h"

extern "C" {
  #include "esp_heap_caps.h"
}
void printHeapInfo() {
    logMessage("Free heap: " + String(ESP.getFreeHeap()));
    logMessage("Chip PSRAM: " + String(psramFound() ? "yes" : "no"));
    if (psramFound()) {
        logMessage("Free PSRAM (approx): " + String(ESP.getPsramSize()));
    }
}


#define CURRENT_VERSION "0.4.3"
#define NORMAL_JSON_URL "https://devsur11.github.io/M5Gotchi/firmware/firmware.json"
#define LITE_JSON_URL   "https://devsur11.github.io/M5Gotchi/firmware/lite.json"
#define TEMP_DIR        "/temp"
#define TEMP_JSON_PATH  TEMP_DIR "/update.json"
#define TEMP_BIN_PATH   TEMP_DIR "/update.bin"
#define OLD_CONFIG_FILE "/config.conf"
#define NEW_CONFIG_FILE "/m5gothi.conf"
#define PERSONALITY_FILE "/personality.conf"
//#define LITE_VERSION
#define SERIAL_LOGS
//#define SKIP_AUTO_UPDATE
//#define BYPASS_SD_CHECK
//#define USE_EXPERIMENTAL_APPS
#define SD_CS    12  // G12
#define SD_MOSI  14  // G14
#define SD_SCK   40  // G40
#define SD_MISO  39  // G39
#define MAX_PKT_SIZE 3000
#define ROW_SIZE 40
#define PADDING 10

struct personality{
    uint16_t nap_time;
    uint16_t delay_after_wifi_scan;
    uint16_t delay_after_no_networks_found;
    uint16_t delay_after_attack_fail;
    uint16_t delay_after_successful_attack;
    uint16_t deauth_packets_sent;
    uint16_t delay_after_deauth;
    uint16_t delay_after_picking_target;
    uint16_t delay_before_switching_target;
    uint16_t delay_after_client_found;
    bool sound_on_events;
    bool deauth_on;
    uint16_t handshake_wait_time;
    bool add_to_whitelist_on_success;
    bool add_to_whitelist_on_fail;
    bool activate_sniffer_on_deauth;
    uint16_t client_sniffing_time;
    uint16_t deauth_packet_delay;
    uint16_t delay_after_no_clients_found;
    uint16_t client_discovery_timeout;
};

bool initVars();
bool saveSettings();
bool initPersonality();
bool savePersonality();

extern String hostname;
extern bool sound;
extern int brightness;
extern uint16_t pwned_ap;
extern SPIClass sdSPI;
extern String savedApSSID;
extern String savedAPPass;
extern String whitelist;
extern bool pwnagothiMode;
extern uint8_t sessionCaptures;
extern bool pwnagothiModeEnabled;
extern String bg_color;
extern String tx_color;
extern bool skip_eapol_check;
extern String wpa_sec_api_key;
extern personality pwnagotchi;
extern bool sd_logging;
extern bool toogle_pwnagothi_with_gpio0;
extern bool lite_mode_wpa_sec_sync_on_startup;
extern String lastPwnedAP;