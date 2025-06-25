#pragma once

#include <Arduino.h>
#include <SPI.h>

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


#define CURRENT_VERSION "0.2"
#define VERSION_URL "https://raw.githubusercontent.com/youruser/yourrepo/main/version.txt"
#define UPDATE_LINK "https://www.example.com/update"
#define TEMP_FILE_PATH "/sdcard/version.temp"
#define SERIAL_LOGS
#define USE_EXPERIMENTAL_APPS
#define SD_CS    12  // G12
#define SD_MOSI  14  // G14
#define SD_SCK   40  // G40
#define SD_MISO  39  // G39
#define MAX_PKT_SIZE 3000
#define ROW_SIZE 40
#define PADDING 10
#define pwnagothiModeEnabled pwnagothiMode

bool initVars();
bool saveSettings();