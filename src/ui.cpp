#include "M5Cardputer.h"
#include "lgfx/v1/misc/enum.hpp"
#include "lgfx/v1/misc/DataWrapper.hpp"
#include "HWCDC.h"
#include "Arduino.h"
#include "ui.h"
#include <FS.h>
#include <SD.h>
#include <WiFi.h>
#include "settings.h"
#include "pwnagothi.h"
#include "EapolSniffer.h"
#include "mood.h"
#include "updater.h"
#include <Update.h>
#include <FS.h>
#include <SD.h>
#include "evilPortal.h"
#include "networkKit.h"
#include "src.h"
#include "logger.h"

M5Canvas canvas_top(&M5.Display);
M5Canvas canvas_main(&M5.Display);
M5Canvas canvas_bot(&M5.Display);
M5Canvas bar_right(&M5.Display);
M5Canvas bar_right2(&M5.Display);
M5Canvas bar_right3(&M5.Display);
M5Canvas bar_right4(&M5.Display);

#ifndef LITE_VERSION
String funny_ssids[] = {
  "Mom Use This One",
  "Abraham Linksys",
  "Benjamin FrankLAN",
  "Martin Router King",
  "John Wilkes Bluetooth",
  "Pretty Fly for a Wi-Fi",
  "Bill Wi the Science Fi",
  "I Believe Wi Can Fi",
  "Tell My Wi-Fi Love Her",
  "No More Mister Wi-Fi",
  "LAN Solo",
  "The LAN Before Time",
  "Silence of the LANs",
  "House LANister",
  "Winternet Is Coming",
  "Ping's Landing",
  "The Ping in the North",
  "This LAN Is My LAN",
  "Get Off My LAN",
  "The Promised LAN",
  "The LAN Down Under",
  "FBI Surveillance Van 4",
  "Area 51 Test Site",
  "Drive-By Wi-Fi",
  "Planet Express",
  "Wu Tang LAN",
  "Darude LANstorm",
  "Never Gonna Give You Up",
  "Hide Yo Kids, Hide Yo Wi-Fi",
  "Loading…",
  "Searching…",
  "VIRUS.EXE",
  "Virus-Infected Wi-Fi",
  "Starbucks Wi-Fi",
  "Text your mom for Password",
  "Yell NIGGA for Password",
  "The Password Is 1234",
  "Free Public Wi-Fi",
  "No Free Wi-Fi Here",
  "Get Your Own Damn Wi-Fi",
  "It Hurts When IP",
  "Dora the Internet Explorer",
  "404 Wi-Fi Unavailable",
  "Porque-Fi",
  "Titanic Syncing",
  "Test Wi-Fi Please Ignore",
  "Drop It Like It's Hotspot",
  "Life in the Fast LAN",
  "The Creep Next Door",
  "Ye Olde Internet"
};

String rickroll_ssids[]{
  "01 Never gona give you up",
  "02 Never gona let you down",
  "03 Never gona run around",
  "04 And desert you",
  "05 Never gona make you cry",
  "06 Never gona say goodbye",
  "07 Never gonna tell a lie ",
  "08 and hurt you",
};

String broken_ssids[]{
  "Broken_Wi-Fi",
  "Unstable_Network",
  "Corrupted_AP",
  "Glitchy_SSID",
  "???",
  "Error_404_AP",
  "NULL_NETWORK",
  "WiFi_Broken",
  "SSID_NOT_FOUND",
  "WiFi?WiFi!",
  "Unkn0wn",
  "WiFi_Failure",
  "Lost_Connection",
  "AP_Crash",
  "WiFi_Glitch",
  "SSID_#@!$%",
  "Network_Error",
  "WiFi_Bugged",
  "WiFi_???",
  "SSID_Broken",
};
#endif

menu main_menu[] = {
    {"Manual mode", 1},
    {"Auto", 4},
    #ifdef USE_EXPERIMENTAL_APPS
    {"Bluetooth", 2},
    {"IR", 3},
    {"Bad USB", 5},
    #endif
    {"Config", 6}
};

menu wifi_menu[] = {
    {"Turn on/off", 47},
    {"Select Networks", 20},
    {"Clone & Details", 21},
    {"Acces point", 22},
    {"Deauth", 23},
    {"Sniffing", 24}
};
#ifdef USE_EXPERIMENTAL_APPS
menu bluetooth_menu[] = {
    {"BLE Spam", 25},
    {"Connect to phone", 26},
    {"Emulate BT Keyboard", 27},
    {"Chat", 28}, 
    {"Scan", 29},
    {"Turn off", 30}
};

menu IR_menu[] = {
    {"Saved remotes", 31},
    {"Send IR", 32},
    {"Recerve IR", 33},
    {"Learn new Remote", 34},
    {"Import from SD", 35}
};
#endif

menu pwngotchi_menu[] = {
    {"Turn on", 36},
    {"Turn off", 37},
    {"Whitelist", 38},
    {"Handshakes", 39}
};

menu settings_menu[] = {
  {"Pwnagothi on boot", 48},
  {"Change Hostname", 40},
  {"Theme", 50},
  {"Skip EAPOL check", 49},
  {"Display brightness", 41},
  {"Sound", 42},
  {"Connect to wifi", 43},
  {"Update system", 44},
  {"Factory reset", 51},
  {"About", 45},
  {"Power off", 46}
};


bool appRunning;
bool userInputVar;
uint8_t menu_current_pages = 1;
int32_t display_w;
int32_t display_h;
int32_t canvas_h;
int32_t canvas_center_x;
int32_t canvas_top_h;
int32_t canvas_bot_h;
int32_t canvas_peers_menu_h;
int32_t canvas_peers_menu_w;
bool keyboard_changed = false;
uint8_t menu_len;
uint8_t menu_current_opt = 0;
uint8_t menu_current_page = 1;  
bool singlePage;
uint8_t menuID = 0;
uint8_t currentBrightness = 100;
String wifiChoice;
uint8_t intWifiChoice;
bool apMode;
String loginCaptured = "";
String passCaptured = "";
bool cloned;
uint16_t bg_color_rgb565;
uint16_t tx_color_rgb565;

uint16_t RGBToRGB565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint16_t hexToRGB565(String hex) {
  if (hex.startsWith("#")) {
    hex = hex.substring(1);
  }
  if (hex.length() != 8) {
    logMessage("Invalid hex color format. Expected RRGGBBAA.");
    return TFT_BLACK; // Default to black if the format is incorrect
  }
  uint32_t color = strtoul(hex.c_str(), nullptr, 16);
  uint8_t r = (color >> 24) & 0xFF;
  uint8_t g = (color >> 16) & 0xFF;
  uint8_t b = (color >> 8) & 0xFF;
  return RGBToRGB565(r, g, b);
}

void initColorSettings(){
  bg_color_rgb565 = hexToRGB565(bg_color);
  tx_color_rgb565 = hexToRGB565(tx_color);
}

void initUi() {
  M5.Display.setRotation(1);
  M5.Display.setTextSize(1);
  M5.Display.fillScreen(bg_color_rgb565);
  M5.Display.setTextColor(tx_color_rgb565);

  display_w = M5.Display.width();
  display_h = M5.Display.height();
  canvas_h = display_h * .8;
  canvas_center_x = display_w / 2;
  canvas_top_h = display_h * .1;
  canvas_bot_h = display_h * .1;
  canvas_peers_menu_h = display_h * .8;
  canvas_peers_menu_w = display_w * .8;

  canvas_top.createSprite(display_w, canvas_top_h);
  canvas_bot.createSprite(display_w, canvas_bot_h);
  canvas_main.createSprite(display_w /*- (display_w * 0.02)*/, canvas_h);
  // bar_right.createSprite((display_w * 0.02) / 2, (canvas_h - 6) / 4 );
  // bar_right2.createSprite((display_w * 0.02) / 2, (canvas_h - 6) / 4 );
  // bar_right3.createSprite((display_w * 0.02) / 2, (canvas_h - 6) / 4 );
  // bar_right4.createSprite((display_w * 0.02) / 2, (canvas_h - 6) / 4 );
  logMessage("UI initialized");
}

uint8_t returnBrightness(){return currentBrightness;}

#ifndef LITE_VERSION

bool toggleMenuBtnPressed() {
  delay(10);
  return (keyboard_changed && (M5Cardputer.Keyboard.isKeyPressed('`')));
}

bool isOkPressed() {
  delay(10);
  return (keyboard_changed && M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER));
}

bool isNextPressed() {
  delay(10);
  return keyboard_changed && (M5Cardputer.Keyboard.isKeyPressed('.') );
}
bool isPrevPressed() {
  delay(10);
  return keyboard_changed && (M5Cardputer.Keyboard.isKeyPressed(';'));
}

#endif

void updateUi(bool show_toolbars, bool triggerPwnagothi) {
  if(pwnagothiMode && triggerPwnagothi){
    pwnagothiLoop();
  }
  #ifndef LITE_VERSION
  keyboard_changed = M5Cardputer.Keyboard.isChange();
  if(keyboard_changed){Sound(10000, 100, sound);}               
  if (toggleMenuBtnPressed()) {
    if(pwnagothiMode){
      return;
    }
    // If menu is open, return to main menu
    // If not, toggle menu
    if (menuID == true) {
      menu_current_opt = 0;
      menu_current_page = 1;
      menuID = 0;
    } else {
      menuID = 1;
      menu_current_opt = 0;
      menu_current_page = 1;
    }
  }
  #endif

  String mood_face = getCurrentMoodFace();
  String mood_phrase = getCurrentMoodPhrase();

  drawTopCanvas();
  drawBottomCanvas();

  #ifndef LITE_VERSION
  if (menuID == 1) {
    menu_current_pages = 2;
    menu_len = 6;
    #ifdef USE_EXPERIMENTAL_APPS
    drawMenuList(main_menu, 1, 6);
    #else
    drawMenuList(main_menu, 1, 3);
    #endif
    //drawMenu();
  } 
  else if (menuID == 2){
    if(!pwnagothiMode)
    {
      drawMenuList( wifi_menu , 2, 6);
      //drawMenu();
    }
    else{
      drawInfoBox("INFO", "Pwnagothi auto mode enabled", "Turn it off to operate.", false, false);
      delay(5000);
      menuID = 0;
    }
  }
  #ifdef USE_EXPERIMENTAL_APPS
  else if (menuID == 3){
    if(!pwnagothiMode)
    {
      drawMenuList( bluetooth_menu , 3, 6);
      drawMenu();
    }
    else{
      drawInfoBox("INFO", "Pwnagothi auto mode enabled", "Turn it off to operate.", false, false);
      delay(5000);
      menuID = 0;
    }
  }
  else if (menuID == 4){
    if(!pwnagothiMode)
    {
      drawMenuList( IR_menu , 4, 5);
      drawMenu();
    }
    else{
      drawInfoBox("INFO", "Pwnagothi auto mode enabled", "Turn it off to operate.", false, false);
      delay(5000);
      menuID = 0;
    }
  }
  #endif
  else if (menuID == 5){
    drawMenuList( pwngotchi_menu , 5, 4);
    //drawMenu();
    
  }
  else if (menuID == 6){
    if(!pwnagothiMode)
    {
      drawMenuList( settings_menu , 6, 11);
      //drawMenu();
    }
    else{
      drawInfoBox("INFO", "Pwnagothi auto mode enabled", "Turn it off to operate.", false, false);
      delay(5000);
      menuID = 0;
    }
  }  
  else if (menuID == 0)
  {
    drawMood(mood_face, mood_phrase);
  }
  else if(appRunning){}
  #endif
  #ifdef LITE_VERSION
    drawMood(mood_face, mood_phrase);
  #endif 
  //drawRightBar();

  M5.Display.startWrite();
  if (show_toolbars) {
    canvas_top.pushSprite(0, 0);
    canvas_bot.pushSprite(0, canvas_top_h + canvas_h);
    //bar_right1.pushSprite(display_w * 0.98, 2*(canvas_top_h + 3));
  }
  bar_right.pushSprite(display_w * 0.98, canvas_top_h + 5);
  bar_right2.pushSprite(display_w * 0.98, ( canvas_top_h + 5 ) + (1 * ((canvas_h - 6)/4)) - 1 );
  bar_right3.pushSprite(display_w * 0.98, ( canvas_top_h + 5 ) + (2 * ((canvas_h - 6)/4)) - 1 );
  bar_right4.pushSprite(display_w * 0.98, ( canvas_top_h + 5 ) + (3 * ((canvas_h - 6)/4)) - 1 );
  canvas_main.pushSprite(0, canvas_top_h);
  M5.Display.endWrite();
}

void drawRightBar() {
  //TODO
}

inline void resetSprite(){bar_right.fillSprite(bg_color_rgb565);}

void drawTopCanvas() {
  canvas_top.fillSprite(bg_color_rgb565);
  canvas_top.setTextSize(1);
  canvas_top.setTextColor(tx_color_rgb565);
  canvas_top.setColor(tx_color_rgb565);
  canvas_top.setTextDatum(top_left);
  canvas_top.drawString("CH:" + String(WiFi.channel()) + " AP: " + String(WiFi.scanComplete()), 0, 3);
  canvas_top.setTextDatum(top_right);
  unsigned long ms = millis();

  unsigned long seconds = ms / 1000;
  unsigned int minutes = seconds / 60;
  unsigned int hours = minutes / 60;

  seconds = seconds % 60;
  minutes = minutes % 60;

  // Pad with zero if needed
  char buffer[9];
  sprintf(buffer, "%02u:%02u:%02lu", hours, minutes, seconds);
  canvas_top.drawString("UPS " + String(M5.Power.getBatteryLevel()) + "%  UP:" + buffer , display_w, 3);
  canvas_top.drawLine(0, canvas_top_h - 1, display_w, canvas_top_h - 1);
}

void drawBottomCanvas() {
  canvas_bot.fillSprite(bg_color_rgb565);
  canvas_bot.setTextSize(1);
  canvas_bot.setTextColor(tx_color_rgb565);
  canvas_bot.setColor(tx_color_rgb565);
  canvas_bot.setTextDatum(top_left);
  uint16_t captures = sessionCaptures;
  uint16_t allTimeCaptures = pwned_ap;
  canvas_bot.drawString("PWND: " + String(captures)+ "/" + String(allTimeCaptures), 3, 5);
  String wifiStatus;
  if(WiFi.status() == WL_NO_SHIELD){
    wifiStatus = "off";
    if(apMode){canvas_bot.drawString("Wifi: AP  " + wifiChoice, 0, 5);}
  }
  else if(WiFi.status() == WL_CONNECTED){
    wifiStatus = "connected";
  }
  else if(WiFi.status() ==  WL_IDLE_STATUS){
    wifiStatus = "IDLE";
  }
  else if(WiFi.status() == WL_CONNECT_FAILED){
    wifiStatus = "error";
  }
  else if(WiFi.status() ==  WL_CONNECTION_LOST){
    wifiStatus = "lost";
  }
  else if(WiFi.status() ==  WL_DISCONNECTED){
    wifiStatus = "disconnected";
  }
  canvas_bot.setTextDatum(top_right);
  canvas_bot.drawString(String((pwnagothiMode) ? "AUTO" : "MANU") + " " + wifiStatus, display_w, 5);
  canvas_bot.drawLine(0, 0, display_w, 0);
}

void drawMood(String face, String phrase) {
  canvas_main.fillSprite(bg_color_rgb565);
  canvas_main.setTextSize(1.5);
  canvas_main.setTextDatum(top_left);
  canvas_main.setCursor(3, 10);
  canvas_main.println(hostname + ">");
  canvas_main.setTextSize(4);
  canvas_main.setCursor(5, 10);
  canvas_main.setTextColor(tx_color_rgb565);
  canvas_main.setColor(tx_color_rgb565);
  //canvas_main.setTextStyle(BOLD);
  canvas_main.drawString(face, 5 , 30);
  canvas_main.setTextSize(1.5);
  canvas_main.setCursor(3, canvas_h - 40);
  canvas_main.println("> " + phrase);
}

void drawInfoBox(String tittle, String info, String info2, bool canBeQuit, bool isCritical) {
  appRunning = true;
  while(true){
    drawTopCanvas();
    drawBottomCanvas();
    if(canBeQuit){delay(100);}
    canvas_main.fillScreen(bg_color_rgb565);
    canvas_main.setTextColor(tx_color_rgb565);
    canvas_main.clear(bg_color_rgb565);
    canvas_main.setTextSize(3);
    if(isCritical){canvas_main.setColor(RED);}
    else {canvas_main.setColor(tx_color_rgb565);}
    canvas_main.setTextDatum(middle_center);
    canvas_main.drawString(tittle, canvas_center_x, canvas_h / 4);
    canvas_main.setTextSize(1.5);
    canvas_main.setTextDatum(middle_center);
    canvas_main.drawString(info, canvas_center_x, canvas_h / 2);
    canvas_main.drawString(info2, canvas_center_x, (canvas_h / 2) + 20);
    drawRightBar();
    if(canBeQuit){
      canvas_main.setTextSize(1);
      canvas_main.drawString("To exit press OK", canvas_center_x, canvas_h * 0.9);

        sleepFunction();
        drawBottomCanvas();
        pushAll();
        M5.update();
        M5Cardputer.update();
        if(M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)){
          Sound(10000, 100, sound);
          return ;
        }

    }
    else{
      drawBottomCanvas();
      pushAll();
      return;
    }
  }
  appRunning = false;
}

#ifndef LITE_VERSION

inline void trigger(uint8_t trigID){logMessage("Trigger" + String(trigID));}

void runApp(uint8_t appID){
  logMessage("App started running, ID:"+ String(appID));
  menu_current_opt = 0;
  menu_current_page = 1;
  menuID = 0; 
  if(appID){
    if(appID == 1){drawMenuList( wifi_menu , 2, 6);}
    #ifdef USE_EXPERIMENTAL_APPS
    if(appID == 2){drawMenuList(bluetooth_menu, 3, 6);}
    if(appID == 3){drawMenuList(IR_menu, 4, 5 );}
    #endif
    if(appID == 4){drawMenuList(pwngotchi_menu, 5 , 3 );}
    if(appID == 5){drawInfoBox("ERROR", "not implemented", "" ,  true, true);}
    if(appID == 6){drawMenuList(settings_menu ,6  ,11);}
    if(appID == 7){}
    if(appID == 8){}
    if(appID == 9){}
    if(appID == 10){}
    if(appID == 11){}
    if(appID == 12){}
    if(appID == 13){}
    if(appID == 14){}
    if(appID == 15){}
    if(appID == 16){}
    if(appID == 17){}
    if(appID == 18){}
    if(appID == 19){}
    if(appID == 20){
      WiFi.mode(WIFI_STA);
      int numNetworks = WiFi.scanNetworks();
      String wifinets[20];
      if (numNetworks == 0) {
        drawInfoBox("Info", "No wifi nearby", "Abort.", true, false);
        return;
      } else {
        // Przechodzimy przez wszystkie znalezione sieci i zapisujemy ich nazwy w liście
        for (int i = 0; i < numNetworks; i++) {
        String ssid = WiFi.SSID(i);
        
        wifinets[i] = String(ssid);
        logMessage(wifinets[i]);
        }
      }
      uint8_t wifisel = drawMultiChoice("Select WIFI network:", wifinets, numNetworks, 2, 0);
      wifiChoice = WiFi.SSID(wifisel);
      intWifiChoice = wifisel;
      logMessage("Selected wifi: "+ wifiChoice);
      drawInfoBox("Succes", wifiChoice, "Was selected", true, false);
      updateActivity(true);
    }
    if(appID == 21){
      if(wifiChoice.equals("")){
        drawInfoBox("Error", "No wifi selected", "Do it first", true, false);
      }
      else{
        drawWifiInfoScreen(WiFi.SSID(intWifiChoice), WiFi.BSSIDstr(intWifiChoice), String(WiFi.RSSI(intWifiChoice)), String(WiFi.channel(intWifiChoice)));
      }
      updateActivity(true);
    }
    if(appID == 22){
      String appList[] = {"Phishing form", "Beacon spam", "AP mode", "Turn OFF"};
      uint8_t tempChoice = drawMultiChoice("What to do?", appList , 4 , 2 , 2);
      if(tempChoice==0){
        if(cloned){
          startPortal(wifiChoice);
        }
        else{
          String uinput = userInput("SSID?", "Enter wifi name for ap.", 30);
          startPortal(uinput);
        }
        delay(100);
        apMode = true;
        while(true){
          updatePortal();
          M5.update();
          M5Cardputer.update();
          sleepFunction();
          Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
          if(!loginCaptured.equals("") && !passCaptured.equals("")){
            drawInfoBox("New victim!", loginCaptured, passCaptured, false, false);
          }
          else{
            drawInfoBox("Evil portal", "Evli portal active...", "Enter to exit", false, false);
          }
          keyboard_changed = M5Cardputer.Keyboard.isChange();
          if(keyboard_changed){Sound(10000, 100, sound);}    
          if (status.enter) {
            WiFi.eraseAP();
            WiFi.mode(WIFI_MODE_NULL);
            apMode = false;
            wifiChoice = "";
            break;
          }
        }
      }
      if(tempChoice == 1){
        String ssidMenu[] = {"Funny SSID", "Broken SSID", "Rick Roll", "Make your own :)"};
        M5.update();
        M5Cardputer.update();
        delay(10);
        uint8_t ssidChoice = drawMultiChoice("Select list", ssidMenu, 4 , 2 , 2);
        if(ssidChoice==0){
          broadcastFakeSSIDs( funny_ssids, 48, sound);
        }
        else if (ssidChoice==1){
          broadcastFakeSSIDs( broken_ssids, 20, sound);
        }
        else if (ssidChoice==2){
          broadcastFakeSSIDs( rickroll_ssids, 8, sound);
          menu_current_opt = 0;
          menu_current_page = 1;
          menuID = 0;
          }
        else if (ssidChoice==3){
          String* BeaconList = makeList("Create spam list", 48, false, 30);
          broadcastFakeSSIDs( BeaconList , sizeof(BeaconList), sound);
        }
        else{
          return;
        }
      }
      else if(tempChoice == 2){
        if(apMode){
          bool answear = drawQuestionBox("AP of?", "AP arleady running!", "Power AP off?");
          if (answear){
            WiFi.mode(WIFI_MODE_NULL);
            apMode = false;
            wifiChoice = "";
            return;
            }
        }
        WiFi.disconnect(true);
        WiFi.mode(WIFI_MODE_AP);
        if(cloned){
          String pass = userInput("Password", "Create password for AP", 30);
          bool result = WiFi.softAP(wifiChoice, pass);
          if(result){
            drawInfoBox("Succes", "AP started", "succesfully", true, false);
            apMode = true;
          }
          else {
            drawInfoBox("Error", "Something happend!", "Something happend!", true, false);
          }
          cloned = false;
          wifiChoice = "";
        }
        else{
          String apssid = userInput("AP name:", "Enter wifi name", 30);
          wifiChoice = apssid;
          String pass = userInput("Password", "Create password for AP", 30);
          bool result = WiFi.softAP(apssid, pass);
          if(result){
            drawInfoBox("Succes", "AP started", "succesfully", true, false);
            apMode = true;
          }
          else {
            drawInfoBox("Error", "Something happend!", "Something happend!", true, false);
          }
          cloned = false;
          wifiChoice = "";
        }
      }
      else if (tempChoice ==3) {
        bool answear = drawQuestionBox("Power AP off?", "Are you sure?", "");
        if (answear){
          WiFi.mode(WIFI_MODE_NULL);
          apMode = false;
          wifiChoice = "";
          }
      }
      menu_current_opt = 0;
      menu_current_page = 1;
      menuID = 0;
      updateActivity(true);
    }
    if(appID == 23){
      bool answwear = drawQuestionBox("WARNING!", "This is illegal to use not", "on your network! Continue?");
      if (answwear){
        if(!wifiChoice.equals("")){
          set_target_channel(WiFi.SSID(intWifiChoice).c_str());
          setMac(WiFi.BSSID(intWifiChoice));
          logMessage("User inited deauth");
          initClientSniffing();
          String clients[50];
          int clientLen;
          while(true){
            get_clients_list(clients, clientLen);
            drawInfoBox("Searching...", "Found "+ String(clientLen)+ " clients", "ENTER for next step", false, false);
            updateM5();
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            sleepFunction();
            if(status.enter){
              delay(150);
              esp_wifi_set_promiscuous(false);
              break;
            }
          }
          uint8_t target = drawMultiChoice("Select target.", clients, clientLen, 0, 0);
          if(target==100){
            return;
          }
          logMessage("Selected target: " + clients[target]);
          int previousMillis;
          uint16_t interval = 1000;
          int PPS;
          drawInfoBox("Deauth!", "ENTER to end. Target:", String(clients[target]) + " PPS: " + String(PPS), false, false);
          while(true){
            int currentMillis = millis();  
            if (currentMillis - previousMillis >= interval) {
              drawInfoBox("Deauth!", "Deauth active on target:", String(clients[target]) + " PPS: " + String(PPS), false, false);
              previousMillis = currentMillis;
              PPS = 0;
            }
            updateM5();
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            sleepFunction();
            if(status.enter){
              break;
            }

            if(send_deauth_packets(clients[target], 1)){
              PPS++;
            }
            
          }
          
          clearClients();
        }
        else{
          drawInfoBox("Error!", "No wifi selected!", "Select one first!", true, false);
        }
      }
      updateActivity(true);
    }
    if(appID == 24){
      String mmenu[] = {"Mac sniffing", "EAPOL sniffing"};
      singlePage = false;
      menu_current_pages = 2;
      uint8_t answerrr = drawMultiChoice("Sniffing", mmenu, 2, 1, 0);
      if(answerrr == 0){
        String mmenuu[] = {"Auto switch" ,"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
        answerrr = drawMultiChoice("Select chanel", mmenuu, 13, 1, 0);
        if(true){
          uint8_t chanelSwitch = 1;
          static unsigned long lastSwitchTime = millis();
          const unsigned long channelSwitchInterval = 500;  
          esp_wifi_set_channel(answerrr, WIFI_SECOND_CHAN_NONE);
          WiFi.mode(WIFI_STA);  // Ustawienie trybu WiFi na stację
          esp_wifi_set_promiscuous(true);  // Włączenie trybu promiskuitywnego
          esp_wifi_set_promiscuous_rx_cb(client_sniff_promiscuous_rx_cb);
          logMessage("Started mac sniffing!");
          canvas_main.clear();
          uint8_t line;
          while(true){
            M5.update();
            M5Cardputer.update();  
            keyboard_changed = M5Cardputer.Keyboard.isChange();
            if(keyboard_changed){Sound(10000, 100, sound);}
            sleepFunction();
            drawTopCanvas();
            drawBottomCanvas();
            canvas_main.clear(bg_color_rgb565);
            canvas_main.fillSprite(bg_color_rgb565); //Clears main display
            canvas_main.setTextSize(1);
            canvas_main.setTextColor(tx_color_rgb565);
            canvas_main.setColor(tx_color_rgb565);
            canvas_main.setTextDatum(top_left);
            canvas_main.setCursor(1, (((PADDING + 1) * line) + 5) + 1);
            canvas_main.println("From:             To:               Ch:");
            line++;
            canvas_main.setCursor(1, (((PADDING + 1) * line) + 5) + 1);
            canvas_main.println("---------------------------------------");
            line++;
            int macCount;  // Non-const integer to hold the count of MAC entries
            const MacEntry* tableOfMac = get_mac_table(macCount);  // Get the MAC table
            if(macCount){
              for (int i = macCount ; i > 0; i--) {
                // Convert MAC addresses to strings and print them
                canvas_main.setCursor(1, (((PADDING + 1) * line) + 5) + 1);
                String sourceMac = macToString(tableOfMac[i-1].source);
                String destinationMac = macToString(tableOfMac[i-1].destination);
                String chanelMac = String(tableOfMac[i-1].channel);
                // Example usage with canvas_main
                canvas_main.println(sourceMac + " " + destinationMac + " " + chanelMac);
                line++;
              }
            }
            pushAll();
            line = 0;
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            for(auto i : status.word){
              if(i=='`' && status.fn){
              esp_wifi_set_promiscuous(false);
              WiFi.mode(WIFI_MODE_NULL);
              return;
              }
            }

            if (millis() - lastSwitchTime > channelSwitchInterval && !answerrr) {
              chanelSwitch++;
              if (chanelSwitch > 12) {
                chanelSwitch = 1;  // Loop back to channel 1
              }
              lastSwitchTime = millis();
              esp_wifi_set_channel(chanelSwitch , WIFI_SECOND_CHAN_NONE);
            }

          }
        }
      }
      else if(answerrr==1){
        String mmenuu[] = {"Auto switch" ,"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"};
        answerrr = drawMultiChoice("Select chanel", mmenuu, 13, 1, 0);
        if(SnifferBegin(answerrr)){
          canvas_main.clear();
          pushAll();
          uint8_t line;
          while(true){
            M5.update();
            M5Cardputer.update();  
            keyboard_changed = M5Cardputer.Keyboard.isChange();
            if(keyboard_changed){Sound(10000, 100, sound);}
            keyboard_changed = M5Cardputer.Keyboard.isChange();
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            for(auto i : status.word){
              if(i=='`' && status.fn){
                return;
              }
              delay(250);
            }
            sleepFunction();
            drawTopCanvas();
            drawBottomCanvas();
            canvas_main.clear(bg_color_rgb565);
            canvas_main.setTextSize(1);
            canvas_main.setTextColor(tx_color_rgb565);
            //canvas_main.setColor(tx_color_rgb565);
            canvas_main.setTextDatum(top_left);
            canvas_main.setCursor(1, (((PADDING + 1) * line) + 5) + 1);
            canvas_main.println("EAPOL sniffer ver.1.0 by Devsur.");
            line++;
            canvas_main.setCursor(1, (((PADDING + 1) * line) + 5) + 1);
            canvas_main.println("From:             To SSID:");
            line++; // ID is what is added to file to identify thic=s copture of others
            canvas_main.setCursor(1, (((PADDING + 1) * line) + 5) + 1);
            canvas_main.println("---------------------------------------");
            line++;
            canvas_main.setCursor(1, (((PADDING + 1) * line) + 5) + 1);
            int packetCount = SnifferGetClientCount();
            if(packetCount){
              const PacketInfo* packets = SnifferGetPacketInfoTable();
              for (int i = packetCount  ; i > 0; i--){
                //String strMacSrc = macToString(packets[i-1].srcMac);
                String strMacDest = macToString(packets[i-1].destMac);
                String fileID = String(packets[i-1].fileName);
                canvas_main.setCursor(1, (((PADDING + 1) * line) + 5) + 1);
                canvas_main.println(strMacDest + "   " + fileID);
                line++;
              }
            
            }
            pushAll();
            SnifferLoop();
            line = 0;
          }
        }
        else{
          drawInfoBox("Error!", "Can't init EAPOL sniffer.", "Check SD card!", true, false);
          return;
        }
      }
      updateActivity(true);
    }
    if(appID == 25){}
    if(appID == 26){}
    if(appID == 27){}
    if(appID == 28){}
    if(appID == 29){}
    if(appID == 30){}
    if(appID == 31){}
    if(appID == 32){}
    if(appID == 33){}
    if(appID == 34){}
    if(appID == 35){}
    if(appID == 36){
      if(!pwnagothiMode){
        bool answear = drawQuestionBox("CONFIRMATION", "Operate only if you ", "have premision!");
        if(answear){
          drawInfoBox("INITIALIZING", "Pwnagothi mode initialization", "please wait...", false, true);
          if(pwnagothiBegin()){
            drawInfoBox("SUCCESS", "Press ENTER to begin", "operation", true, false);
            pwnagothiMode = true;
          }
          else{
            drawInfoBox("ERROR", "Pwnagothi init failed!", "", true, true);
            pwnagothiMode = false;
          }
          return;
        }
      }
      else{
        drawInfoBox("WTF?!", "Pwnagothi mode is on", "Can't you just look at UI!??", true, true);
      }
      return;
    }
    if(appID == 37){
      pwnagothiMode = false;
      WiFi.mode(WIFI_MODE_NULL);
      drawInfoBox("INFO", "Auto mode turned off", "Enabled manual mode", true, false);
    }
    if(appID == 38){
      editWhitelist();
    }
    if(appID == 39){
      if(!SD.begin(SD_CS, sdSPI, 1000000)) {
        drawInfoBox("Error", "Cannot open SD card", "Check SD card!", true, true);
        return;
      }
      File root = SD.open("/handshake");
      if (!root || !root.isDirectory()) {
        drawInfoBox("Error", "Cannot open /handshakes", "Check SD card!", true, true);
        return;
      }
      String fileList[50];
      uint8_t fileCount = 0;
      File file = root.openNextFile();
      while (file && fileCount < 50) {
        if (!file.isDirectory()) {
          fileList[fileCount++] = String(file.name());
        }
        file = root.openNextFile();
      }
      if (fileCount == 0) {
        drawInfoBox("Info", "No handshakes found", "", true, false);
        return;
      }
      drawMultiChoice("Handshakes:", fileList, fileCount, 5, 3);
      updateActivity(true);
    }
    if(appID == 40){
        String name = userInput("New value", "Change Hostname to:", 18);
        if(name != ""){
          hostname = name;
          if(saveSettings()){
            return;
          }
          else{drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);}
          return;
        }
        drawInfoBox("Name invalid", "Null inputed,", "operation abort", true, false);
    }
    if(appID == 41){
      String value = userInput("Brightness", "Change Brightness to (max 255):", 3);
      if(value == ""){
        drawInfoBox("Error", "Invalid Value", value , true, false);
        return;
      }
      //char setTo = value.toInt();
      uint8_t digit2;
      uint8_t digit3;
      uint16_t name;
      uint8_t digit1 = value.charAt(0) - 48;
      name =  digit1;
      if(value.charAt(1) >= 48){
        uint8_t digit2 = value.charAt(1) - 48;
        name = (digit1 * 10) + digit2;
      }
      if(value.charAt(2) >= 48){
        uint8_t digit3 = value.charAt(2) - 48;
        name = (digit1 * 100) + (digit2 * 10) + digit3;
      }
      
      if(name<=255 && name!=0){
        logMessage(String(name));
        M5.Lcd.setBrightness(int(name));
        brightness = int(name);
        if(saveSettings()){
          return;
        }
        else{drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);}
      }
      else{drawInfoBox("Error", "Invalid Value", String(name) , true, false);}
    }
    if(appID == 42){
      String selection[] = {"Off", "On"};
      delay(50);
      sound = drawMultiChoice("Sound", selection, 2, 6, 2);
      if(saveSettings()){
        return;
      }
      else{drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);}
    }
    if(appID == 43){
      WiFi.mode(WIFI_STA);
      if(savedApSSID && savedAPPass){
        WiFi.begin(savedApSSID, savedAPPass);
        uint8_t counter;
        while (counter<=10 && !WiFi.isConnected()) {
          delay(1000);
          drawInfoBox("Connecting", "Please wait...", "You will be soon redirected ", false, false);
          counter++;
        }
        counter = 0;
        }
        if(WiFi.isConnected()){
          drawInfoBox("Connected", "Connected succesfully to", String(WiFi.SSID()) , true, false);
          return;
        }
      
      int numNetworks = WiFi.scanNetworks();
      String wifinets[20];
      if (numNetworks == 0) {
        drawInfoBox("Info", "No wifi nearby", "Abort.", true, false);
        return;
      } else {
        // Przechodzimy przez wszystkie znalezione sieci i zapisujemy ich nazwy w liście
        for (int i = 0; i < numNetworks; i++) {
        String ssid = WiFi.SSID(i);
        
        wifinets[i] = String(ssid);
        logMessage(wifinets[i]);
        }
      }
      uint8_t wifisel = drawMultiChoice("Select WIFI network:", wifinets, numNetworks, 6, 3);
      //String ssid = userInput("Wifi SSID", "Enter wifi name to connect.", 30);
      String password = userInput("Password", "Enter wifi password" , 30);
      WiFi.begin(WiFi.SSID(wifisel), password);
      savedApSSID = WiFi.SSID(wifisel);
      savedAPPass = password;
      uint8_t counter;
      while (counter<=10 && !WiFi.isConnected()) {
        delay(1000);
        drawInfoBox("Connecting", "Please wait...", "You will be soon redirected ", false, false);
        counter++;
      }
      counter = 0;
      if(WiFi.isConnected()){
        drawInfoBox("Connected", "Connected succesfully to", String(WiFi.SSID()) , true, false);
        if(saveSettings()){
          return;
        }
        else{drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);}
      }
      else{
        drawInfoBox("Error", "Connection failed", "Maybe wrong password...", true, false);
      }
    }
    if(appID == 44){
      String tempMenu[] = {"From SD", "From WIFI", "From Github"};
      uint8_t choice = drawMultiChoice("Update type", tempMenu, 3, 6, 4);
      if(choice == 0){updateFromSd();}
      else if(choice == 1){
        if(!(WiFi.status() == WL_CONNECTED)){
          runApp(43);
        }
        updateFromHTML();
      }
      else if(choice == 2){
        if(!(WiFi.status() == WL_CONNECTED)){
          runApp(43);
        }
        drawInfoBox("Updating...", "Updating from github...", "This may take a while...", false,false);
        updateFromGithub();
        drawInfoBox("ERROR!", "Update failed!", "Try again or contact dev", true, false);
      }
      }
    if(appID == 45){
      drawInfoBox("M5Gothi", "v" + String(CURRENT_VERSION) + " by Devsur11  ", "www.github.com/Devsur11 ", true, false);
    }
    if(appID == 46){
      M5.Display.fillScreen(tx_color_rgb565);
      esp_deep_sleep_start(); 
      }
    if(appID == 47){
      String options[] = {"Turn ON", "Turn OFF", "Back"};
      int choice = drawMultiChoice("WiFi Power", options, 3, 2, 0);
      if (choice == 0) {
        WiFi.mode(WIFI_STA);
        drawInfoBox("WiFi", "WiFi turned ON", "", true, false);
      } else if (choice == 1) {
        WiFi.mode(WIFI_MODE_NULL);
        drawInfoBox("WiFi", "WiFi turned OFF", "", true, false);
      } else {
        return;
      }
    }
    if(appID == 48){
      String options[] = {"Enable", "Disable", "Back"};
      int choice = drawMultiChoice("Pwnagothi on boot", options, 3, 6, 0);
      if (choice == 0) {
        pwnagothiModeEnabled = true;
        if (saveSettings()) {
          drawInfoBox("Success", "Pwnagothi will run", "on boot", true, false);
          return;
        } else {
          drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);
        }
      } else if (choice == 1) {
        pwnagothiModeEnabled = false;
        if (saveSettings()) {
          drawInfoBox("Success", "Pwnagothi will NOT run", "on boot", true, false);
          return;
        } else {
          drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);
        }
      } else {
        return;
      }
    }
    if(appID == 49){
      String options[] = {"Enable", "Disable", "Back"};
      int choice = drawMultiChoice("Skip EAPOL check", options, 3, 6, 0);
      if (choice == 0) {
        skip_eapol_check = false;
        if (saveSettings()) {
          drawInfoBox("Success", "EAPOL check enabled", "", true, false);
          return;
        } else {
          drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);
        }
      } else if (choice == 1) {
        skip_eapol_check = true;
        if (saveSettings()) {
          drawInfoBox("Success", "EAPOL check disabled", "", true, false);
          return;
        } else {
          drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);
        }
      } else {
        return;
      }
    }
    if(appID == 50){
      String themeOptions[] = {"White mode", "Dark mode", "Custom", "Back"};
      int themeChoice = drawMultiChoice("Theme", themeOptions, 4, 6, 0);

      if (themeChoice == 0) {
        bg_color = "#FFFFFFFF";
        tx_color = "#000000";
        if (saveSettings()) {
          drawInfoBox("Theme", "White mode applied", "Restarting...", false, false);
          delay(1000);
          ESP.restart();
        } else {
          drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);
        }
      } else if (themeChoice == 1) {
        bg_color = "#000000";
        tx_color = "#FFFFFFFF";
        if (saveSettings()) {
          drawInfoBox("Theme", "Dark mode applied", "Restarting...", false, false);
          delay(1000);
          ESP.restart();
        } else {
          drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);
        }
      } else if (themeChoice == 2) {
        delay(150);
        drawInfoBox("Custom Theme", "Set background color with picker", "Make sure to see text!", false, false);
        delay(5000);
        String customBg = colorPickerUI(false, "#000000ff");
        if (customBg == "exited") return;
        delay(150);
        drawInfoBox("Custom Theme", "Set text color with picker", "Make sure to see text!", false, false);
        delay(5000);
        String customTx = colorPickerUI(true, customBg);
        if (customTx == "exited") return;
        bg_color = customBg;
        tx_color = customTx;
        if (saveSettings()) {
          drawInfoBox("Theme", "Custom theme applied", "Restarting...", false, false);
          delay(1000);
          ESP.restart();
        } else {
          drawInfoBox("ERROR", "Save setting failed!", "Check SD Card", true, false);
        }
      } else {
        return;
      }
    }
    if(appID == 51){
      bool confirm = drawQuestionBox("Factory Reset", "Delete config and restart?", "", "Press 'y' to confirm, 'n' to cancel");
      if (!confirm) {
        drawInfoBox("Aborted", "Factory reset cancelled", "", true, false);
        return;
      }
      drawInfoBox("Factory Reset", "Deleting config...", "", false, false);
      if (SD.exists(NEW_CONFIG_FILE)) {
        SD.remove(NEW_CONFIG_FILE);
        drawInfoBox("Success", "Config deleted", "Restarting...", false, false);
        delay(1000);
        ESP.restart();
      } else {
        drawInfoBox("Error", "Config file not found", "Nothing to delete", true, false);
      }
    }
    return;
  }
}


void drawMenu() {
  if (isNextPressed()) {
    if (menu_current_opt < menu_len - 1 ) {
      menu_current_opt++;
    } else {
      menu_current_opt = 0;
    }
  }

  if (isPrevPressed()) {
    if (menu_current_opt > 0) {
      menu_current_opt--;
    }
    else {
      menu_current_opt = (menu_len - 1);
    }
  }

  if(isOkPressed()){
    return;
  }
  if(!singlePage){
    if(menu_current_opt < 5 && menu_current_page != 1){
        menu_current_page= 1;
      
    } else if(menu_current_opt >= 5 && menu_current_page != 2){
        menu_current_page = 2;
      
  }
  }
  //uint8_t test = main_menu[1].command; - how to acces 2`nd column - for me
}

String userInput(String tittle, String desc, uint8_t maxLenght){
  uint8_t temp = 0;
  String textTyped;
  appRunning = true;
  delay(500);
  //bool loop = 1;
  while (true){
    drawTopCanvas();
    drawBottomCanvas();
    canvas_main.clear(bg_color_rgb565);
    canvas_main.setTextSize(3);
    canvas_main.setTextColor(tx_color_rgb565);
    canvas_main.setTextDatum(middle_center);
    canvas_main.drawString(tittle, canvas_center_x, canvas_h / 4);
    canvas_main.setTextSize(1);
    canvas_main.drawString(desc, canvas_center_x, canvas_h * 0.9);
    M5.update();
    M5Cardputer.update();
    sleepFunction();
    //auto i;
    keyboard_changed = M5Cardputer.Keyboard.isChange();
    if(keyboard_changed){Sound(10000, 100, sound);}    
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    for(auto i : status.word){
      if(i=='`' && status.fn){
        return  "";
      }
      textTyped = textTyped + i;
      temp ++;
      delay(250);
    }
    if (status.del && temp >=1) {
      textTyped.remove(textTyped.length() - 1);
      temp --;
      delay(250);
    }
    if (status.enter) {
      break;
    }
    
    if(temp > maxLenght){
      drawInfoBox("Error", "Can't type more than " + String(maxLenght), " characters" , true, false);
      textTyped.remove(textTyped.length() - 1);
      temp --;
      delay(100);
    }
    canvas_main.clear(bg_color_rgb565);
    canvas_main.setTextSize(3);
    canvas_main.setTextColor(tx_color_rgb565);
    canvas_main.setTextDatum(middle_center);
    canvas_main.drawString(tittle, canvas_center_x, canvas_h / 4);
    canvas_main.setTextSize(1);
    canvas_main.drawString(desc, canvas_center_x, canvas_h * 0.9);
    canvas_main.setTextSize(1.5);
    canvas_main.setCursor(0 , canvas_h /2);
    canvas_main.println(textTyped);
    pushAll();
  }
  //drawInfoBox("Confirm value:", textTyped, true, false);
  appRunning = false;
  logMessage("Userinput returning: " + textTyped);
  return textTyped;
}

String multiplyChar(char toMultiply, uint8_t literations){
  String toReturn;
  char temp = toMultiply;
  for(uint8_t i = 1; i>=literations; i++){
    toReturn = toReturn + temp;
  }
  return toReturn;
}

bool drawQuestionBox(String tittle, String info, String info2, String label) {
  appRunning = true;
  delay(100);
  while(true){
    drawTopCanvas();
    drawBottomCanvas();
    canvas_main.clear(bg_color_rgb565);
    canvas_main.setTextSize(3);
    canvas_main.setColor(tx_color_rgb565);
    canvas_main.setTextDatum(middle_center);
    canvas_main.drawString(tittle, canvas_center_x, canvas_h / 4);
    canvas_main.setTextSize(1.5);
    canvas_main.setTextDatum(middle_center);
    canvas_main.drawString(info, canvas_center_x, canvas_h / 2);
    canvas_main.drawString(info2, canvas_center_x, (canvas_h / 2) + 20);
    canvas_main.setTextSize(1);
    canvas_main.drawString( label, canvas_center_x, canvas_h * 0.9);
    pushAll();
    M5.update();
    M5Cardputer.update();
    keyboard_changed = M5Cardputer.Keyboard.isChange();
    if(keyboard_changed){Sound(10000, 100, sound);}    
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    sleepFunction();

    
    for(auto i : status.word){
      if(i=='`' && status.fn){
        appRunning = false;
        return false;
      }
      else if(i=='y'){
        logMessage("yes");
        return true;
      }
      else if(i=='n'){
        logMessage("No");
        return false;
      }
    }
  }
  appRunning = false;
}

int drawMultiChoice(String tittle, String toDraw[], uint8_t menuSize , uint8_t prevMenuID, uint8_t prevOpt) {
  delay(100);
  uint8_t tempOpt = 0;
  menu_current_opt = 0;
  menu_current_page = 1;
  menu_len = menuSize;
  singlePage = false;
  while(true){
    drawTopCanvas();
    drawBottomCanvas();
    M5.update();
    M5Cardputer.update();  
    keyboard_changed = M5Cardputer.Keyboard.isChange();
    if(keyboard_changed){Sound(10000, 100, sound);}
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    sleepFunction();

    canvas_main.clear(bg_color_rgb565);
    canvas_main.fillSprite(bg_color_rgb565); //Clears main display
    canvas_main.setTextSize(1.5);
    canvas_main.setTextColor(tx_color_rgb565);
    canvas_main.setColor(tx_color_rgb565);
    canvas_main.setTextDatum(top_left);
    canvas_main.setCursor(1, PADDING + 1);
    canvas_main.println(tittle);
    canvas_main.setTextSize(2);
    char display_str[100] = "";
    for (uint8_t j = 0; j < (menu_len - ((menu_current_page - 1) * 4)) ; j++) {
      sprintf(display_str, "%s %s", (tempOpt == j+( (menu_current_page - 1) * 4 ) ) ? ">" : " ",
             toDraw[j+ ( (menu_current_page - 1) * 4)].c_str());
      int y = PADDING + (j * ROW_SIZE / 2) + 20;
      canvas_main.drawString(display_str, 0, y);
    }
    pushAll();

    
    for(auto i : status.word){
      if(i=='`'){
        Sound(10000, 100, sound);
        menuID = prevMenuID;
        menu_current_opt = prevOpt;
        return -1;
      }
    }

    if (isNextPressed()) {
      if (menu_current_opt < menu_len - 1 ) {
        menu_current_opt++;
        tempOpt++;
      } else {
        menu_current_opt = 0;
        tempOpt = 0;
      }
    }
    if (isPrevPressed()) {
      if (menu_current_opt > 0) {
        menu_current_opt--;
        tempOpt--;
      }
      else {
        menu_current_opt = (menu_len - 1);
        tempOpt = (menu_len - 1);
      }
    }
    if(!singlePage){
      if(menu_current_opt < 4 && menu_current_page != 1){
        menu_current_page = 1;
      } 
      else if(menu_current_opt >= 4 && menu_current_page != 2 && menu_current_opt <8){
        menu_current_page = 2;
      }
      else if(menu_current_opt >= 8 && menu_current_page != 3 && menu_current_opt <12){
        menu_current_page = 3;
      }
      else if(menu_current_opt >= 12 && menu_current_page != 4 && menu_current_opt <=16){
        menu_current_page = 4;
      }
    }
    if(isOkPressed()){
      Sound(10000, 100, sound);
      menuID = prevMenuID;
      menu_current_opt = prevOpt;
      return tempOpt;
    }
    
  }
}

String* makeList(String windowName, uint8_t appid, bool addln, uint8_t maxEntryLen){
  uint8_t writeID = 0;
  String list[] = {"Add element", "Remove element" , "Done", "Preview"};
  String* listToReturn;
  if(maxEntryLen > 12){maxEntryLen = 12;}
  delay(100);
  while(true){
    sleepFunction();
    uint8_t choice = drawMultiChoice(windowName, list, 4 , 0, 0);
    if (choice==0){
      String tempText = userInput("Add value:", "", maxEntryLen);
      if(addln){
        listToReturn[writeID] = tempText + "\n";
        writeID++;
      }
      else{
        listToReturn[writeID] = tempText;
        writeID++;
      }
    }
    else if (choice==2){
      delay(100);
      return listToReturn;
    }
    else if (choice==1){
      s16_t idOfItemToRemove = drawMultiChoice("Remove element", list, writeID, 0, 0);
      if (idOfItemToRemove == -1) {
        continue;
      }
      else
      {// Shift items up to remove the selected one
      for (uint8_t i = idOfItemToRemove; i < writeID - 1; i++) {
        list[i] = list[i + 1];
      }
      list[writeID - 1] = "";
      writeID--;}
    }
    else if (choice==3){
      delay(100);
      while(true){
        if (writeID == 0) {
          drawInfoBox("Info", "Whitelist is empty", "Nothing to preview.", true, false);
          break;
        }
        drawTopCanvas();
        drawBottomCanvas();
        sleepFunction();
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        keyboard_changed = M5Cardputer.Keyboard.isChange();
        if(keyboard_changed){Sound(10000, 100, sound);}  
        M5.update();
        M5Cardputer.update();
        if(isOkPressed()){break;}
        drawList(listToReturn, writeID);
        pushAll();
        drawMenu();
      }
    }
  }
}

void drawList(String toDraw[], uint8_t manu_size){
  logMessage("Drawing list with size: " + String(manu_size));
  menu_len = manu_size;

  singlePage = false;
  canvas_main.fillSprite(bg_color_rgb565); // Clears main display
  canvas_main.setTextSize(2);
  canvas_main.setTextColor(tx_color_rgb565);
  canvas_main.setColor(tx_color_rgb565);
  canvas_main.setTextDatum(top_left);
  char display_str[200] = "";

  int maxHeight = canvas_main.height() - 10; // leave some margin
  int maxWidth = canvas_main.width() - 10;   // leave some margin
  int lineHeight = 18; // Adjust as needed
  int y = PADDING;
  int itemsShown = 0;
  int itemIndex = 0;
  int startIndex = 0;

  // Calculate which item to start from, so that the selected item is visible
  int totalLines = 0;
  int selectedLine = 0;
  for (uint8_t i = 0; i < menu_len; i++) {
    String prefix = (menu_current_opt == i) ? ">" : " ";
    String item = prefix + toDraw[i];
    int itemLen = item.length();
    int usedLines = 0;
    int start = 0;
    while (start < itemLen) {
      int len = 1;
      while (start + len <= itemLen && canvas_main.textWidth(item.substring(start, start + len)) < maxWidth) {
        len++;
      }
      len--;
      start += len;
      usedLines++;
    }
    if (i < menu_current_opt) {
      selectedLine += usedLines;
    }
    totalLines += usedLines;
  }

  // If the selected item is out of view, scroll so it's visible
  int linesPerPage = maxHeight / lineHeight;
  int firstVisibleLine = 0;
  if (selectedLine + 1 > linesPerPage) {
    firstVisibleLine = selectedLine + 1 - linesPerPage;
  }

  int currentLine = 0;
  for (uint8_t i = 0; i < menu_len; i++) {
    String prefix = (menu_current_opt == i) ? ">" : " ";
    String item = prefix + toDraw[i];
    int itemLen = item.length();
    int start = 0;
    while (start < itemLen) {
      int len = 1;
      while (start + len <= itemLen && canvas_main.textWidth(item.substring(start, start + len)) < maxWidth) {
        len++;
      }
      len--;
      if (currentLine >= firstVisibleLine && (currentLine - firstVisibleLine) * lineHeight + y < maxHeight) {
        String lineStr = item.substring(start, start + len);
        canvas_main.drawString(lineStr, 0, y + (currentLine - firstVisibleLine) * lineHeight);
      }
      start += len;
      currentLine++;
      if ((currentLine - firstVisibleLine) * lineHeight + y >= maxHeight) {
        break;
      }
    }
    if ((currentLine - firstVisibleLine) * lineHeight + y >= maxHeight) {
      break;
    }
  }
}

void drawMenuList(menu toDraw[], uint8_t menuIDPriv, uint8_t menu_size) {
  menuID = menuIDPriv;
  menu_len = menu_size;
  M5.update();
  M5Cardputer.update();

  singlePage = false;
  canvas_main.fillSprite(bg_color_rgb565); // Clears main display
  canvas_main.setTextSize(2);
  canvas_main.setTextColor(tx_color_rgb565);
  canvas_main.setColor(tx_color_rgb565);
  canvas_main.setTextDatum(top_left);
  char display_str[200] = "";

  int maxHeight = canvas_main.height() - 10; // leave some margin
  int maxWidth = canvas_main.width();   // leave some margin
  int lineHeight = 18; // Adjust as needed
  int y = PADDING;

  // Calculate which item to start from, so that the selected item is visible
  int totalLines = 0;
  int selectedLine = 0;
  for (uint8_t i = 0; i < menu_len; i++) {
    String prefix = (menu_current_opt == i) ? ">" : " ";
    String item = prefix + toDraw[i].name;
    int itemLen = item.length();
    int usedLines = 0;
    int start = 0;
    while (start < itemLen) {
      int len = 1;
      while (start + len <= itemLen && canvas_main.textWidth(item.substring(start, start + len)) < maxWidth) {
        len++;
      }
      len--;
      start += len;
      usedLines++;
    }
    if (i < menu_current_opt) {
      selectedLine += usedLines;
    }
    totalLines += usedLines;
  }

  // If the selected item is out of view, scroll so it's visible
  int linesPerPage = maxHeight / lineHeight;
  int firstVisibleLine = 0;
  if (selectedLine + 1 > linesPerPage) {
    firstVisibleLine = selectedLine + 1 - linesPerPage;
  }

  int currentLine = 0;
  for (uint8_t i = 0; i < menu_len; i++) {
    String prefix = (menu_current_opt == i) ? ">" : " ";
    String item = prefix + toDraw[i].name;
    int itemLen = item.length();
    int start = 0;
    while (start < itemLen) {
      int len = 1;
      while (start + len <= itemLen && canvas_main.textWidth(item.substring(start, start + len)) < maxWidth) {
        len++;
      }
      len--;
      if (currentLine >= firstVisibleLine && (currentLine - firstVisibleLine) * lineHeight + y < maxHeight) {
        String lineStr = item.substring(start, start + len);
        canvas_main.drawString(lineStr, 0, y + (currentLine - firstVisibleLine) * lineHeight);
      }
      start += len;
      currentLine++;
      if ((currentLine - firstVisibleLine) * lineHeight + y >= maxHeight) {
        break;
      }
    }
    if ((currentLine - firstVisibleLine) * lineHeight + y >= maxHeight) {
      break;
    }
  }
  keyboard_changed = M5Cardputer.Keyboard.isChange();
  if(keyboard_changed){Sound(10000, 100, sound);}
  sleepFunction();

  // Handle input and app execution
  M5.update();
  M5Cardputer.update();
  auto status = M5Cardputer.Keyboard.keysState();
  
  int nextCount = 0, prevCount = 0;
  if (M5Cardputer.Keyboard.isKeyPressed(KEY_ENTER)) {
    delay(150); // Debounce delay
    runApp(toDraw[menu_current_opt].command);
  }
  if (M5Cardputer.Keyboard.isKeyPressed('`')){
    delay(150); // Debounce delay
    return;
  }
  if (M5Cardputer.Keyboard.isKeyPressed('.')){
    delay(100); // Small delay to avoid too fast increments
    nextCount++;
  }
  if (M5Cardputer.Keyboard.isKeyPressed(';')){
    delay(100); // Small delay to avoid too fast increments
    prevCount++;
  }

  // Move selection by number of keypresses detected
  if (nextCount > 0) {
    menu_current_opt = (menu_current_opt + nextCount) % menu_len;
  }
  if (prevCount > 0) {
    menu_current_opt = (menu_current_opt + menu_len - prevCount) % menu_len;
  }
}

void logVictim(String login, String pass){
  loginCaptured = login;
  passCaptured = pass;
  return;
}

void drawWifiInfoScreen(String wifiName, String wifiMac, String wifiRRSI, String wifiChanel){
  delay(100);
  while(true){
    drawTopCanvas();
    drawBottomCanvas();
    canvas_main.fillSprite(bg_color_rgb565);
    canvas_main.setTextSize(2);
    canvas_main.setTextColor(tx_color_rgb565);
    canvas_main.setColor(tx_color_rgb565);
    canvas_main.setTextDatum(middle_center);
    canvas_main.drawString(wifiChoice, display_w/2, 25);
    canvas_main.setTextSize(1.5);
    canvas_main.drawString("Mac: " + wifiMac, display_w/2 , 50);
    canvas_main.drawString(wifiRRSI + " RRSI, Chanel: " + wifiChanel, display_w/2, 70);
    canvas_main.setTextSize(1);
    canvas_main.drawString("<To clone press C, ENTER to exit>", display_w/2, 100);
    pushAll();
    updateM5();
    keyboard_changed = M5Cardputer.Keyboard.isChange();
    sleepFunction();
    if(keyboard_changed){Sound(10000, 100, sound);} 
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    for(auto i : status.word){
      if(i == 'c'){
        cloned = true;
        return;
      }
    }
    if(status.enter){
      return;
    }
  }
}

#endif

inline void pushAll(){
  M5.Display.startWrite();
  canvas_top.pushSprite(0, 0);
  canvas_bot.pushSprite(0, canvas_top_h + canvas_h);
  canvas_main.pushSprite(0, canvas_top_h);
  M5.Display.endWrite();
}

inline void updateM5(){
  M5.update();
  M5Cardputer.update();
  keyboard_changed = M5Cardputer.Keyboard.isChange();
  if(keyboard_changed){Sound(10000, 100, sound);}   
}

bool sleep_mode = false;

void sleepFunction(){
  if(M5.BtnA.isPressed()){
    if(sleep_mode == false){
      delay(250);
      M5.Lcd.setBrightness(0);
      M5.Display.fillScreen(tx_color_rgb565);
      sleep_mode = true;
      return;
    }
    if(sleep_mode == true){
      delay(250);
      M5.Lcd.setBrightness(brightness);
      initUi();
      sleep_mode = false;
    }
  }
}

#ifndef LITE_VERSION

void editWhitelist(){
  uint16_t writeID = 0;
  String* whitelist = parseWhitelist(writeID);
  String list[] = {"Add element", "Remove element" , "Done", "Preview"};
  delay(100);
  while(true){
    initVars();
    logMessage("WRITE ID: " + String(writeID));
    String* listToReturn = parseWhitelist(writeID);
    sleepFunction();
    s8_t choice = drawMultiChoice("Whitelist editor", list, 4 , 0, 0);
    if (choice==0){
      String tempText = userInput("Add value:", "", 12);
      addToWhitelist(tempText);
      writeID++;
    }
    else if (choice==2 || choice == -1){
      delay(100);
      setWhitelistFromArray(listToReturn);
      return;
    }
    else if (choice==1){
      s16_t idOfItemToRemove = drawMultiChoice("Remove element", listToReturn, writeID, 0, 0);
      if(idOfItemToRemove == -1){
        continue;
      }
      else
      {removeItemFromWhitelist(listToReturn[idOfItemToRemove]);
      writeID = writeID - 2;
      if(writeID < 0){
        writeID = 0;
      }}
    }
    else if (choice==3){
      delay(100);
      while(true){
        // Exception handler: if list is empty, show info and break
        if (writeID == 0) {
          drawInfoBox("Info", "Whitelist is empty", "Nothing to preview.", true, false);
          break;
        }
        drawTopCanvas();
        drawBottomCanvas();
        sleepFunction();
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        keyboard_changed = M5Cardputer.Keyboard.isChange();
        if(keyboard_changed){Sound(10000, 100, sound);}  
        for(auto i : status.word){
          if(i=='`'){
            break;
          }
        }
        M5.update();
        M5Cardputer.update();
        if(isOkPressed()){break;}
        drawList(listToReturn, writeID);
        pushAll();
        drawMenu();
      }
    }
  }
}

String colorPickerUI(bool pickingText, String bg_color_toset) {
  int r = 0, g = 0, b = 0;
  int selected = 0; // 0=R, 1=G, 2=B
  bool done = false;
  String result = "";

  // Adjusted sizes for better fit
  int box_w = 40, box_h = 30;
  int box_y = canvas_h / 2 - box_h / 2 - 10;
  int box_x[3] = {canvas_center_x - box_w - 25, canvas_center_x, canvas_center_x + box_w + 25};

  int preview_w = 70, preview_h = 25;
  int preview_x = canvas_center_x;
  int preview_y = box_y + box_h + 20;

  while (!done) {
    canvas_main.fillSprite(bg_color_rgb565);
    canvas_main.setTextSize(2);
    canvas_main.setTextDatum(middle_center);
    canvas_main.setTextColor(tx_color_rgb565);
    canvas_main.drawString("Select color", canvas_center_x, canvas_h / 6);

    // Draw color boxes
    for (int i = 0; i < 3; i++) {
      uint16_t border_color = (selected == i) ? tx_color_rgb565 : bg_color_rgb565;
      canvas_main.drawRect(box_x[i] - box_w/2, box_y, box_w, box_h, border_color);
      canvas_main.setTextSize(2);
      canvas_main.setTextColor(tx_color_rgb565);
      int val = (i == 0) ? r : (i == 1) ? g : b;
      canvas_main.drawString(String(val), box_x[i], box_y + box_h/2 - 8 + 10);
      canvas_main.setTextSize(1);
      String label = (i == 0) ? "red" : (i == 1) ? "green" : "blue";
      canvas_main.drawString(label, box_x[i], box_y + box_h + 10);
    }

    // Draw preview/confirm box
    uint16_t preview_color = RGBToRGB565(r, g, b);
    if(pickingText){
      canvas_main.drawRect(preview_x - preview_w/2, preview_y, preview_w, preview_h * 2/3, hexToRGB565(bg_color_toset));
      canvas_main.fillRect(preview_x - preview_w/2, preview_y, preview_w, preview_h * 2/3, hexToRGB565(bg_color_toset));
    }
    else{
      canvas_main.drawRect(preview_x - preview_w/2, preview_y, preview_w, preview_h * 2/3, preview_color);
      canvas_main.fillRect(preview_x - preview_w/2, preview_y, preview_w, preview_h * 2/3, preview_color);
    }
    
    canvas_main.setTextSize(1);
    canvas_main.setTextColor(tx_color_rgb565);
    if(pickingText) canvas_main.setTextColor(preview_color);
    canvas_main.drawString("Confirm", preview_x, preview_y + preview_h/2 - 6);
    canvas_main.drawString("Up/Down: value Left/Right: color OK set", preview_x, preview_y + preview_h/2 + 12);

    pushAll();

    // Handle input
    M5.update();
    M5Cardputer.update();
    Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
    keyboard_changed = M5Cardputer.Keyboard.isChange();
    if(keyboard_changed){Sound(10000, 100, sound);}  

    for (auto k : status.word) {
      if (k == '/') { // right
        selected = (selected + 1) % 3;
      }
      if (k == ',') { // left
        selected = (selected + 2) % 3;
      }
      if (k == ';') { // up
        if (selected == 0 && r < 255) r++;
        if (selected == 1 && g < 255) g++;
        if (selected == 2 && b < 255) b++;
      }
      if (k == '.') { // down
        if (selected == 0 && r > 0) r--;
        if (selected == 1 && g > 0) g--;
        if (selected == 2 && b > 0) b--;
      }
    }
    // Exit if fn+` pressed
    if (status.fn) {
      for (auto k : status.word) {
        if (k == '`') {
          return "exited";
        }
      }
    }
    if (status.enter) {
      char hexStr[9];
      sprintf(hexStr, "#%02X%02X%02XFF", r, g, b);
      result = String(hexStr);
      done = true;
      break;
    }
    delay(80);
  }
  return result;
}

#endif