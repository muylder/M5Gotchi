#include "M5Cardputer.h"
#include <Update.h>
#include <FS.h>
#include <SD.h>
#include "evilPortal.h"
#include "networkKit.h"
#include "src.h"
#include "logger.h"
#pragma once

struct menu {
  char name[25];
  int command;
};


String multiplyChar(char toMultiply, uint8_t literations);
void trigger(uint8_t trigID);
void drawInfoBox(String tittle, String info, String info2, bool canBeQuit, bool isCritical);
bool activityRewarded();
void initUi();
void wakeUp();
void drawMood(String face, String phrase);
void drawTopCanvas();
void drawRightBar();
void drawBottomCanvas();
void drawMenu();
void updateUi(bool show_toolbars = false, bool triggerPwnagothi = true);
void runApp(uint8_t appID = 0);
inline void resetSprite();
String userInput(String tittle, String desc, uint8_t maxLenght);
bool drawQuestionBox(String tittle, String info, String info2, String label = "To confirm press y, to abort press n");
int drawMultiChoice(String tittle, String toDraw[], uint8_t menuSize, uint8_t prevMenuID, uint8_t prevOpt);
uint8_t returnBrightness();
String* makeList(String windowName, uint8_t appid, bool addln, uint8_t maxEntryLen);
void drawList(String toDraw[], uint8_t manu_size);
void logVictim(String login, String pass);
void drawWifiInfoScreen(String wifiName, String wifiMac, String wifiRRSI, String wifiChanel);
inline void pushAll();
inline void updateM5();
void sleepFunction();
void editWhitelist();
uint16_t RGBToRGB565(uint8_t r, uint8_t g, uint8_t b);
uint16_t hexToRGB565(String hex);
String colorPickerUI(bool pickingText, String bg_color_toset);
void initColorSettings();
void drawMenuList(menu toDraw[], uint8_t menuIDPriv, uint8_t menu_size);