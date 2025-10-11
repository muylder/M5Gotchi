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
String wpa_sec_api_key = "";
bool lite_mode_wpa_sec_sync_on_startup = false;
bool sd_logging = false;
bool toogle_pwnagothi_with_gpio0 = false;

// struct personality{
//     uint16_t nap_time;
//     uint16_t delay_after_wifi_scan;
//     uint16_t delay_after_no_networks_found;
//     uint16_t delay_after_attack_fail;
//     uint16_t delay_after_successful_attack;
//     uint16_t deauth_packets_sent;
//     uint16_t delay_after_deauth;
//     uint16_t delay_after_picking_target;
//     uint16_t delay_before_switching_target;
//     uint16_t delay_after_client_found;
//     bool sound_on_events;
//     bool deauth_on;
//     uint16_t handshake_wait_time;
//     bool add_to_whitelist_on_success;
//     bool add_to_whitelist_on_fail;
//     bool activate_sniffer_on_deauth;
//     uint16_t client_sniffing_time;
//     uint16_t deauth_packet_delay;
//     uint16_t delay_after_no_clients_found;
// };

personality pwnagotchi = {
    3000,      // nap_time
    100,    // delay_after_wifi_scan
    5000,   // delay_after_no_networks_found
    1000,   // delay_after_attack_fail
    3000,   // delay_after_successful_attack
    80,     // deauth_packets_sent
    50,    // delay_after_deauth
    50,    // delay_after_picking_target
    3000,   // delay_before_switching_target
    100,  // delay_after_client_found
    true,   // sound_on_events
    true,   // deauth_on
    10000,  // handshake_wait_time
    true,   // add_to_whitelist_on_success
    false,  // add_to_whitelist_on_fail
    false,   // activate_sniffer_on_deauth
    0,  // client_sniffing_time  - not used in code
    150,    // deauth_packet_delay
    3000,   // delay_after_no_clients_found
    15000   // client_discovery_timeout
};

bool initPersonality(){
    if(!SD.begin(SD_CS, sdSPI, 1000000)) {
        logMessage("Personality init failed, sd card init failed");
        return false;
    }
    bool personalityChanged = false;
    JsonDocument personalityDoc;
    
    if(SD.exists(PERSONALITY_FILE)) {
        logMessage("Personality file found, loading data");
        File file = SD.open(PERSONALITY_FILE, FILE_READ);
        if (!file) {
            logMessage("Failed to open personality file");
            return false;
        }

        DeserializationError error = deserializeJson(personalityDoc, file);
        file.close();

        if (error) {
            logMessage("deserializeJson() failed: ");
            logMessage(error.c_str());
            return false;
        }

        // Load each option, fallback to default if missing
        if (personalityDoc["nap_time"].is<uint16_t>()) pwnagotchi.nap_time = personalityDoc["nap_time"];
        else personalityChanged = true;

        if (personalityDoc["delay_after_wifi_scan"].is<uint16_t>()) pwnagotchi.delay_after_wifi_scan = personalityDoc["delay_after_wifi_scan"];
        else personalityChanged = true;

        if (personalityDoc["delay_after_no_networks_found"].is<uint16_t>()) pwnagotchi.delay_after_no_networks_found = personalityDoc["delay_after_no_networks_found"];
        else personalityChanged = true;

        if (personalityDoc["delay_after_attack_fail"].is<uint16_t>()) pwnagotchi.delay_after_attack_fail = personalityDoc["delay_after_attack_fail"];
        else personalityChanged = true;

        if (personalityDoc["delay_after_successful_attack"].is<uint16_t>()) pwnagotchi.delay_after_successful_attack = personalityDoc["delay_after_successful_attack"];
        else personalityChanged = true;

        if (personalityDoc["deauth_packets_sent"].is<uint16_t>()) pwnagotchi.deauth_packets_sent = personalityDoc["deauth_packets_sent"];
        else personalityChanged = true;

        if (personalityDoc["delay_after_deauth"].is<uint16_t>()) pwnagotchi.delay_after_deauth = personalityDoc["delay_after_deauth"];
        else personalityChanged = true;

        if (personalityDoc["delay_after_picking_target"].is<uint16_t>()) pwnagotchi.delay_after_picking_target = personalityDoc["delay_after_picking_target"];
        else personalityChanged = true;

        if (personalityDoc["delay_before_switching_target"].is<uint16_t>()) pwnagotchi.delay_before_switching_target = personalityDoc["delay_before_switching_target"];
        else personalityChanged = true;

        if (personalityDoc["delay_after_client_found"].is<uint16_t>()) pwnagotchi.delay_after_client_found = personalityDoc["delay_after_client_found"];
        else personalityChanged = true;

        if (personalityDoc["sound_on_events"].is<bool>()) pwnagotchi.sound_on_events = personalityDoc["sound_on_events"];
        else personalityChanged = true;

        if (personalityDoc["deauth_on"].is<bool>()) pwnagotchi.deauth_on = personalityDoc["deauth_on"];
        else personalityChanged = true;

        if (personalityDoc["handshake_wait_time"].is<uint16_t>()) pwnagotchi.handshake_wait_time = personalityDoc["handshake_wait_time"];
        else personalityChanged = true;

        if (personalityDoc["add_to_whitelist_on_success"].is<bool>()) pwnagotchi.add_to_whitelist_on_success = personalityDoc["add_to_whitelist_on_success"];
        else personalityChanged = true;

        if (personalityDoc["add_to_whitelist_on_fail"].is<bool>()) pwnagotchi.add_to_whitelist_on_fail = personalityDoc["add_to_whitelist_on_fail"];
        else personalityChanged = true;

        if (personalityDoc["activate_sniffer_on_deauth"].is<bool>()) pwnagotchi.activate_sniffer_on_deauth = personalityDoc["activate_sniffer_on_deauth"];
        else personalityChanged = true;

        if (personalityDoc["client_sniffing_time"].is<uint16_t>()) pwnagotchi.client_sniffing_time = personalityDoc["client_sniffing_time"];
        else personalityChanged = true;

        if (personalityDoc["deauth_packet_delay"].is<uint16_t>()) pwnagotchi.deauth_packet_delay = personalityDoc["deauth_packet_delay"];
        else personalityChanged = true;

        if (personalityDoc["delay_after_no_clients_found"].is<uint16_t>()) pwnagotchi.delay_after_no_clients_found = personalityDoc["delay_after_no_clients_found"];
        else personalityChanged = true;

        if (personalityDoc["client_discovery_timeout"].is<uint16_t>()) pwnagotchi.client_discovery_timeout = personalityDoc["client_discovery_timeout"];
        else personalityChanged = true;
    }
    else {
        logMessage("No personality file found, creating with default values");
        personalityChanged = true;
    }

    // Always update config with all required keys
    personalityDoc["nap_time"] = pwnagotchi.nap_time;
    personalityDoc["delay_after_wifi_scan"] = pwnagotchi.delay_after_wifi_scan;
    personalityDoc["delay_after_no_networks_found"] = pwnagotchi.delay_after_no_networks_found;
    personalityDoc["delay_after_successful_attack"] = pwnagotchi.delay_after_successful_attack;
    personalityDoc["deauth_packets_sent"] = pwnagotchi.deauth_packets_sent;
    personalityDoc["delay_after_deauth"] = pwnagotchi.delay_after_deauth;
    personalityDoc["delay_after_picking_target"] = pwnagotchi.delay_after_picking_target;
    personalityDoc["delay_before_switching_target"] = pwnagotchi.delay_before_switching_target;
    personalityDoc["delay_after_client_found"] = pwnagotchi.delay_after_client_found;
    personalityDoc["sound_on_events"] = pwnagotchi.sound_on_events;
    personalityDoc["deauth_on"] = pwnagotchi.deauth_on;
    personalityDoc["handshake_wait_time"] = pwnagotchi.handshake_wait_time;
    personalityDoc["add_to_whitelist_on_success"] = pwnagotchi.add_to_whitelist_on_success;
    personalityDoc["add_to_whitelist_on_fail"] = pwnagotchi.add_to_whitelist_on_fail;
    personalityDoc["activate_sniffer_on_deauth"] = pwnagotchi.activate_sniffer_on_deauth;
    personalityDoc["client_sniffing_time"] = pwnagotchi.client_sniffing_time;
    personalityDoc["deauth_packet_delay"] = pwnagotchi.deauth_packet_delay;
    personalityDoc["delay_after_no_clients_found"] = pwnagotchi.delay_after_no_clients_found;
    personalityDoc["delay_after_attack_fail"] = pwnagotchi.delay_after_attack_fail;
    personalityDoc["client_discovery_timeout"] = pwnagotchi.client_discovery_timeout;

    if (personalityChanged) {
        logMessage("Personality updated with missing/default values, saving...");
        FConf = SD.open(PERSONALITY_FILE, FILE_WRITE, true);
        if (FConf) {
            String output;
            serializeJsonPretty(personalityDoc, output);
            FConf.print(output);
            FConf.close();
            logMessage("Personality saved successfully");
        } else {
            logMessage("Failed to open personality file for writing");
            return false;
        }
    }
    return true;
}

bool savePersonality(){
    JsonDocument personalityDoc;
    personalityDoc["nap_time"] = pwnagotchi.nap_time;
    personalityDoc["delay_after_wifi_scan"] = pwnagotchi.delay_after_wifi_scan;
    personalityDoc["delay_after_no_networks_found"] = pwnagotchi.delay_after_no_networks_found;
    personalityDoc["delay_after_successful_attack"] = pwnagotchi.delay_after_successful_attack;
    personalityDoc["deauth_packets_sent"] = pwnagotchi.deauth_packets_sent;
    personalityDoc["delay_after_deauth"] = pwnagotchi.delay_after_deauth;
    personalityDoc["delay_after_picking_target"] = pwnagotchi.delay_after_picking_target;
    personalityDoc["delay_before_switching_target"] = pwnagotchi.delay_before_switching_target;
    personalityDoc["delay_after_client_found"] = pwnagotchi.delay_after_client_found;
    personalityDoc["sound_on_events"] = pwnagotchi.sound_on_events;
    personalityDoc["deauth_on"] = pwnagotchi.deauth_on;
    personalityDoc["handshake_wait_time"] = pwnagotchi.handshake_wait_time;
    personalityDoc["add_to_whitelist_on_success"] = pwnagotchi.add_to_whitelist_on_success;
    personalityDoc["add_to_whitelist_on_fail"] = pwnagotchi.add_to_whitelist_on_fail;
    personalityDoc["activate_sniffer_on_deauth"] = pwnagotchi.activate_sniffer_on_deauth;
    personalityDoc["client_sniffing_time"] = pwnagotchi.client_sniffing_time;
    personalityDoc["deauth_packet_delay"] = pwnagotchi.deauth_packet_delay;
    personalityDoc["delay_after_no_clients_found"] = pwnagotchi.delay_after_no_clients_found;
    personalityDoc["delay_after_attack_fail"] = pwnagotchi.delay_after_attack_fail;
    personalityDoc["client_discovery_timeout"] = pwnagotchi.client_discovery_timeout;

    logMessage("Personality JSON data creation successful, proceeding to save");
    FConf = SD.open(PERSONALITY_FILE, FILE_WRITE, false);
    if (FConf) {
        String output;
        serializeJsonPretty(personalityDoc, output);
        FConf.print(output);
        FConf.close();
        logMessage("Personality saved successfully");
        return true;
    } else {
        logMessage("Failed to open personality file for writing");
        return false;
    }
}


bool initVars() {
    if (!SD.begin(SD_CS, sdSPI, 1000000)) {
        logMessage("JSON parser failed, sd card init failed");
        return false;
    }

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

        if (config["auto_mode_on_startup"].is<bool>()) {pwnagothiModeEnabled = config["auto_mode_on_startup"];
        } else configChanged = true;

        if (config["bg_color"].is<const char*>()) bg_color = String(config["bg_color"].as<const char*>());
        else configChanged = true;

        if (config["tx_color"].is<const char*>()) tx_color = String(config["tx_color"].as<const char*>());
        else configChanged = true;

        if (config["skip_eapol_check"].is<bool>()) skip_eapol_check = config["skip_eapol_check"].as<bool>();
        else configChanged = true;

        if (config["wpa_sec_api_key"].is<const char*>()) wpa_sec_api_key = String(config["wpa_sec_api_key"].as<const char*>());
        else configChanged = true;

        if(config["lite_mode_wpa_sec_sync_on_startup"].is<bool>()) lite_mode_wpa_sec_sync_on_startup = config["lite_mode_wpa_sec_sync_on_startup"].as<bool>();
        else configChanged = true;

        if(config["sd_logging"].is<bool>()) sd_logging = config["sd_logging"].as<bool>();
        else configChanged = true;

        if(config["toogle_pwnagothi_with_gpio0"].is<bool>()) toogle_pwnagothi_with_gpio0 = config["toogle_pwnagothi_with_gpio0"].as<bool>();
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
    config["wpa_sec_api_key"] = wpa_sec_api_key;
    config["lite_mode_wpa_sec_sync_on_startup"] = lite_mode_wpa_sec_sync_on_startup;
    config["sd_logging"] = sd_logging;
    config["toogle_pwnagothi_with_gpio0"] = toogle_pwnagothi_with_gpio0;

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
    config["wpa_sec_api_key"] = wpa_sec_api_key;
    config["lite_mode_wpa_sec_sync_on_startup"] = lite_mode_wpa_sec_sync_on_startup;
    config["sd_logging"] = sd_logging;
    config["toogle_pwnagothi_with_gpio0"] = toogle_pwnagothi_with_gpio0;
    
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