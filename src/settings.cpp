#include "settings.h"
#include "ArduinoJson.h"
#include "SD.h"
#include "logger.h"
#include "pwnagothi.h"

String hostname = "M5";
bool sound = true;
int brightness = 150;
uint16_t pwned_ap;
SPIClass sdSPI;
String savedApSSID;
String savedAPPass;
String whitelist;
File FConf;
bool pwnagothiMode = false;
uint8_t sessionCaptures;
bool pwnagothiModeEnabled = false;
String bg_color = "#ffffffff";
String tx_color = "#000000ff";
bool skip_eapol_check = false;

bool migrateOldConfig() {
    if (!SD.exists(OLD_CONFIG_FILE)) return false;

    File oldFile = SD.open(OLD_CONFIG_FILE, FILE_READ);
    if (!oldFile) {
        logMessage("Failed to open old config file");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, oldFile);
    oldFile.close();

    if (error) {
        logMessage("Failed to parse old config file");
        return false;
    }

    // Check for all required variables
    if (!doc["Hostname"].is<const char*>() ||
        !doc["sound"].is<bool>() ||
        !doc["brightness"].is<int>() ||
        !doc["pwned_ap"].is<uint16_t>() ||
        !doc["savedApSSID"].is<const char*>() ||
        !doc["savedAPPass"].is<const char*>() ||
        !doc["whitelist"].is<const char*>() ||
        !doc["auto_mode_on_startup"].is<bool>()) {
        logMessage("Old config missing required keys, not migrating");
        return false;
    }

    // Copy values to variables
    hostname = String(doc["Hostname"].as<const char*>());
    sound = doc["sound"];
    brightness = doc["brightness"];
    pwned_ap = doc["pwned_ap"];
    savedApSSID = String(doc["savedApSSID"].as<const char*>());
    savedAPPass = String(doc["savedAPPass"].as<const char*>());
    whitelist = String(doc["whitelist"].as<const char*>());
    pwnagothiMode = doc["auto_mode_on_startup"];
    pwnagothiModeEnabled = doc["auto_mode_on_startup"];

    // New options: bg_color, tx_color, skip_eapol_check
    String bg_color = doc.containsKey("bg_color") ? String(doc["bg_color"].as<const char*>()) : "#000000";
    String tx_color = doc.containsKey("tx_color") ? String(doc["tx_color"].as<const char*>()) : "#FFFFFF";
    bool skip_eapol_check = doc.containsKey("skip_eapol_check") ? doc["skip_eapol_check"].as<bool>() : false;

    // Save to new config file
    JsonDocument newConfig;
    newConfig["Hostname"] = hostname;
    newConfig["sound"] = sound;
    newConfig["brightness"] = brightness;
    newConfig["pwned_ap"] = pwned_ap;
    newConfig["savedApSSID"] = savedApSSID;
    newConfig["savedAPPass"] = savedAPPass;
    newConfig["whitelist"] = whitelist;
    newConfig["auto_mode_on_startup"] = pwnagothiModeEnabled;
    newConfig["bg_color"] = bg_color;
    newConfig["tx_color"] = tx_color;
    newConfig["skip_eapol_check"] = skip_eapol_check;

    FConf = SD.open(NEW_CONFIG_FILE, FILE_WRITE, true);
    if (FConf) {
        String output;
        serializeJsonPretty(newConfig, output);
        FConf.print(output);
        FConf.close();
        logMessage("Migrated old config to new config file");
        SD.remove(OLD_CONFIG_FILE);
        return true;
    } else {
        logMessage("Failed to write new config file during migration");
        return false;
    }
}

bool initVars(){
    if(!SD.begin(SD_CS, sdSPI, 1000000)){
        logMessage("JSON parser failed, sd card init failed");
        return false;
    }

    // Migrate old config if present
    migrateOldConfig();

    if(SD.open(NEW_CONFIG_FILE, "r", false)){
        logMessage("Conf file found, loading data");

        File file = SD.open(NEW_CONFIG_FILE, FILE_READ);
        if (!file) {
            logMessage("Failed to open config file");
            return false;
        }

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, file);

        file.close();

        if (error) {
            logMessage("deserializeJson() failed: ");
            logMessage(error.c_str());
            return false;
        }
        logMessage("Setting values up");
        hostname = String(doc["Hostname"].as<const char*>());
        sound = doc["sound"];
        brightness = doc["brightness"];
        pwned_ap = doc["pwned_ap"];
        savedApSSID = String(doc["savedApSSID"].as<const char*>());
        savedAPPass = String(doc["savedAPPass"].as<const char*>());
        whitelist = String(doc["whitelist"].as<const char*>());
        pwnagothiMode = doc["auto_mode_on_startup"];
        pwnagothiModeEnabled = doc["auto_mode_on_startup"];

        // Load new options if present, else keep defaults
        if (doc.containsKey("bg_color")) bg_color = String(doc["bg_color"].as<const char*>());
        if (doc.containsKey("tx_color")) tx_color = String(doc["tx_color"].as<const char*>());
        if (doc.containsKey("skip_eapol_check")) skip_eapol_check = doc["skip_eapol_check"].as<bool>();
    }
    else{
        logMessage("Conf file not found, creating one");
        JsonDocument config;
        config["Hostname"] = hostname;
        config["sound"] = sound;
        config["brightness"] = brightness;
        config["pwned_ap"] = pwned_ap;
        config["savedApSSID"] = savedApSSID;
        config["savedAPPass"] = savedAPPass;
        config["whitelist"] = whitelist;
        config["auto_mode_on_startup"] = pwnagothiModeEnabled;
        config["bg_color"] = bg_color;
        config["tx_color"] = tx_color;
        config["skip_eapol_check"] = skip_eapol_check;

        logMessage("JSON data creation successful, proceeding to save");

        FConf = SD.open(NEW_CONFIG_FILE, FILE_WRITE, true);
        if (FConf) {
            String output;
            serializeJsonPretty(config, output);
            FConf.print(output);
            FConf.close();
            logMessage("Config saved successfully");
        } else {
            logMessage("Failed to open config file for writing");
        }
    }
    return true;
}

bool saveSettings(){
    // Load current values or defaults for new options
    String bg_color = "#000000";
    String tx_color = "#FFFFFF";
    bool skip_eapol_check = false;

    // Try to load from existing config file if present
    if (SD.exists(NEW_CONFIG_FILE)) {
        File file = SD.open(NEW_CONFIG_FILE, FILE_READ);
        if (file) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, file);
            file.close();
            if (!error) {
                if (doc.containsKey("bg_color")) bg_color = String(doc["bg_color"].as<const char*>());
                if (doc.containsKey("tx_color")) tx_color = String(doc["tx_color"].as<const char*>());
                if (doc.containsKey("skip_eapol_check")) skip_eapol_check = doc["skip_eapol_check"].as<bool>();
            }
        }
    }

    JsonDocument config;
    config["Hostname"] = hostname;
    config["sound"] = sound;
    config["brightness"] = brightness;
    config["pwned_ap"] = pwned_ap;
    config["savedApSSID"] = savedApSSID;
    config["savedAPPass"] = savedAPPass;
    config["whitelist"] = whitelist;
    config["auto_mode_on_startup"] = pwnagothiModeEnabled;
    config["bg_color"] = bg_color;
    config["tx_color"] = tx_color;
    config["skip_eapol_check"] = skip_eapol_check;
    
    logMessage("JSON data creation successful, proceeding to save");
    FConf = SD.open(NEW_CONFIG_FILE, FILE_WRITE, false);
    if (FConf) {
        String output;
        serializeJsonPretty(config, output);
        FConf.print(output);
        FConf.close();
        logMessage("Config saved successfully");
        return true;
    } else {
        logMessage("Failed to open config file for writing");
        return false;
    }
}