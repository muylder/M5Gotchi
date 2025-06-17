#include "settings.h"
#include "ArduinoJson.h"
#include "SD.h"
#include "logger.h"

String hostname = "M5";
bool sound = true;
int brightness = 255;
uint16_t pwned_ap;
SPIClass sdSPI;
String savedApSSID;
String savedAPPass;
File FConf;

bool initVars(){
    SD.begin(SD_CS, sdSPI, 1000000);
    logMessage("SD Card init!");
    if(!SD.begin(SD_CS, sdSPI, 1000000)){
        logMessage("JSON parser failed, sd card init failed");
        return false;
    }
    else{
        if(SD.open("/config.conf", "r", false)){
            logMessage("Conf file found, loading data");

            // 1. Open the config file
            File file = SD.open("/config.conf", FILE_READ);
            logMessage("conf file opened");
            if (!file) {
              logMessage("Failed to open config file");
              return false;
            }

            JsonDocument doc;

            logMessage(" 3. Parse JSON directly from the file stream");
            DeserializationError error = deserializeJson(doc, file);

            logMessage("4. Always close the file after reading");
            file.close();

            // 5. Check for deserialization errors
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
            
            logMessage("JSON data creation successful, proceeding to save");

            FConf = SD.open("/config.conf", FILE_WRITE, true);
            if (FConf) {
                String output;
                serializeJsonPretty(config, output);
                FConf.print(output);  // or FConf.write((const uint8_t*)output.c_str(), output.length());
                FConf.close();
                logMessage("Config saved successfully");
            } else {
                logMessage("Failed to open config file for writing");
            }
        }
        return true;
    }
}

bool saveSettings(){
    JsonDocument config;
    config["Hostname"] = hostname;
    config["sound"] = sound;
    config["brightness"] = brightness;
    config["pwned_ap"] = pwned_ap;
    config["savedApSSID"] = savedApSSID;
    config["savedAPPass"] = savedAPPass;
    
    logMessage("JSON data creation successful, proceeding to save");
    FConf = SD.open("/config.conf", FILE_WRITE, true);
    if (FConf) {
        String output;
        serializeJsonPretty(config, output);
        FConf.print(output);  // or FConf.write((const uint8_t*)output.c_str(), output.length());
        FConf.close();
        logMessage("Config saved successfully");
        return true;
    } else {
        logMessage("Failed to open config file for writing");
        return false;
    }
}