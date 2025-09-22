#include "ArduinoJson.h"
#include <vector>

struct CrackedEntry {
    String ssid;
    String bssid;     // NEW: store BSSID
    String password;
};

bool uploadToWpaSec(const char* apiKey, const char* pcapPath, const char* fileName, uint32_t timeoutMs);
bool isAlreadyUploaded(const char* fileName, JsonDocument &doc); 
void saveUploadedList(JsonDocument &doc);
void processWpaSec(const char* apiKey);
std::vector<CrackedEntry> getCrackedEntries();
String userInputFromWebServer(String titleOfEntryToGet);
