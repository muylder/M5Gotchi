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
String tx_color = "#00000000";
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
    String bg_color = doc["bg_color"].is<const char*>() ? String(doc["bg_color"].as<const char*>()) : "#000000";
    String tx_color = doc["tx_color"].is<const char*>() ? String(doc["tx_color"].as<const char*>()) : "#FFFFFF";
    bool skip_eapol_check = doc["skip_eapol_check"].is<bool>() ? doc["skip_eapol_check"].as<bool>() : false;

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

bool initVars() {
    if (!SD.begin(SD_CS, sdSPI, 1000000)) {
        logMessage("JSON parser failed, sd card init failed");
        return false;
    }

    migrateOldConfig();

    bool configChanged = false;
    JsonDocument config;

    if (SD.exists(NEW_CONFIG_FILE)) {
        logMessage("Conf file found, loading data");
        File file = SD.open(NEW_CONFIG_FILE, FILE_READ);
        if (!file) {
            logMessage("Failed to open config file");
            return false;
        }

        DeserializationError error = deserializeJson(config, file);
        file.close();

        if (error) {
            logMessage("deserializeJson() failed: ");
            logMessage(error.c_str());
            return false;
        }

        // Load each option, fallback to default if missing
        if (config["Hostname"].is<const char*>()) hostname = String(config["Hostname"].as<const char*>());
        else configChanged = true;

        if (config["sound"].is<bool>()) sound = config["sound"];
        else configChanged = true;

        if (config["brightness"].is<int>()) brightness = config["brightness"];
        else configChanged = true;

        if (config["pwned_ap"].is<uint16_t>()) pwned_ap = config["pwned_ap"];
        else configChanged = true;

        if (config["savedApSSID"].is<const char*>()) savedApSSID = String(config["savedApSSID"].as<const char*>());
        else configChanged = true;

        if (config["savedAPPass"].is<const char*>()) savedAPPass = String(config["savedAPPass"].as<const char*>());
        else configChanged = true;

        if (config["whitelist"].is<const char*>()) whitelist = String(config["whitelist"].as<const char*>());
        else configChanged = true;

        if (config["auto_mode_on_startup"].is<bool>()) {
            pwnagothiMode = config["auto_mode_on_startup"];
            pwnagothiModeEnabled = config["auto_mode_on_startup"];
        } else configChanged = true;

        if (config["bg_color"].is<const char*>()) bg_color = String(config["bg_color"].as<const char*>());
        else configChanged = true;

        if (config["tx_color"].is<const char*>()) tx_color = String(config["tx_color"].as<const char*>());
        else configChanged = true;

        if (config["skip_eapol_check"].is<bool>()) skip_eapol_check = config["skip_eapol_check"].as<bool>();
        else configChanged = true;
    } else {
        logMessage("Conf file not found, creating one");
        configChanged = true;
    }

    // Always update config with all required keys
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

    if (configChanged) {
        logMessage("Config updated with missing/default values, saving...");
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