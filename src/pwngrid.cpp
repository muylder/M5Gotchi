//This file is copied from https://github.com/viniciusbo/m5-palnagotchi repo 
//- all credits to the original author
//I just added some of my code...

#include "pwngrid.h"
#include "settings.h"
#include "mood.h"
#include "logger.h"
// Had to remove Radiotap headers, since its automatically added
// Also had to remove the last 4 bytes (frame check sequence)
const uint8_t pwngrid_beacon_raw[] = {
    0x80, 0x00,                          // FC
    0x00, 0x00,                          // Duration
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // DA (broadcast)
    0xde, 0xad, 0xbe, 0xef, 0xde, 0xad,  // SA
    0xa1, 0x00, 0x64, 0xe6, 0x0b, 0x8b,  // BSSID
    0x40, 0x43,  // Sequence number/fragment number/seq-ctl
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Timestamp
    0x64, 0x00,                                      // Beacon interval
    0x11, 0x04,                                      // Capability info
    // 0xde (AC = 222) + 1 byte payload len + payload (AC Header)
    // For each 255 bytes of the payload, a new AC header should be set
};

const int raw_beacon_len = sizeof(pwngrid_beacon_raw);


void pwngridAdvertise() {
  JsonDocument pal_json;
  String pal_json_str = "";

  pal_json["pal"] = true;  // Also detect other Palnagotchis
  pal_json["name"] = hostname;
  pal_json["face"] = getCurrentMoodFace();
  pal_json["epoch"] = 1;
  pal_json["grid_version"] = "1.10.3";
  pal_json["identity"] =
      "32e9f315e92d974342c93d0fd952a914bfb4e6838953536ea6f63d54db6b9610";
  pal_json["pwnd_run"] = 0;
  pal_json["pwnd_tot"] = 0;
  pal_json["session_id"] = "a2:00:64:e6:0b:8b";
  pal_json["timestamp"] = 0;
  pal_json["uptime"] = 0;
  pal_json["version"] = "1.8.4";
  pal_json["policy"]["advertise"] = true;
  pal_json["policy"]["bond_encounters_factor"] = 20000;
  pal_json["policy"]["bored_num_epochs"] = 0;
  pal_json["policy"]["sad_num_epochs"] = 0;
  pal_json["policy"]["excited_num_epochs"] = 9999;

  serializeJson(pal_json, pal_json_str);
  uint16_t pal_json_len = measureJson(pal_json);
  uint8_t header_len = 2 + ((uint8_t)(pal_json_len / 255) * 2);
  uint8_t pwngrid_beacon_frame[raw_beacon_len + pal_json_len + header_len];
  memcpy(pwngrid_beacon_frame, pwngrid_beacon_raw, raw_beacon_len);

  // Iterate through json string and copy it to beacon frame
  int frame_byte = raw_beacon_len;
  for (int i = 0; i < pal_json_len; i++) {
    // Write AC and len tags before every 255 bytes
    if (i == 0 || i % 255 == 0) {
      pwngrid_beacon_frame[frame_byte++] = 0xde;  // AC = 222
      uint8_t payload_len = 255;
      if (pal_json_len - i < 255) {
        payload_len = pal_json_len - i;
      }

      pwngrid_beacon_frame[frame_byte++] = payload_len;
    }

    // Append json byte to frame
    // If current byte is not ascii, add ? instead
    uint8_t next_byte = (uint8_t)'?';
    if (isAscii(pal_json_str[i])) {
      next_byte = (uint8_t)pal_json_str[i];
    }

    pwngrid_beacon_frame[frame_byte++] = next_byte;
  }

  delay(10);  // Delay to ensure the frame is ready
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);  // Set channel to 1
  esp_err_t err = esp_wifi_80211_tx(WIFI_IF_AP, pwngrid_beacon_frame, frame_byte, false);
  if (err != ESP_OK) {
    logMessage("802.11 TX failed: " + String(err));
  }
  else {
    logMessage("802.11 TX success: " + String(frame_byte) + " bytes sent");
  }
}
