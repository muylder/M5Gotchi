#include "M5Cardputer.h"
#include "M5Unified.h"
#include "ui.h"
#include "settings.h"
#include "mood.h"
#include "pwnagothi.h"
#include "moodLoader.h"
#include "Arduino.h"
// crash_rtc_log.h  (include in your project)
#include <Arduino.h>
#include "SD.h"
#include "esp_system.h"
#include "esp_log.h"
#include <stdarg.h>

#define RTC_CRASH_BUF_SIZE 4096
// Keep buffer in RTC slow memory so it survives resets
RTC_DATA_ATTR static char rtc_crash_buf[RTC_CRASH_BUF_SIZE];
RTC_DATA_ATTR static size_t rtc_crash_len = 0;
static File sharedLog = File();

static int crash_capture_vprintf(const char *fmt, va_list args) {
    // Format into a temporary stack buffer then append into the RTC buffer.
    char tmp[512];
    int len = vsnprintf(tmp, sizeof(tmp), fmt, args);
    if (len > 0) {
        // write to Serial as usual
        Serial.write((uint8_t*)tmp, len);
        // append to RTC buffer if space
        size_t avail = (rtc_crash_len < RTC_CRASH_BUF_SIZE) ? (RTC_CRASH_BUF_SIZE - rtc_crash_len - 1) : 0;
        if (avail > 0) {
            size_t tocopy = (size_t)len;
            if (tocopy > avail) tocopy = avail;
            memcpy(&rtc_crash_buf[rtc_crash_len], tmp, tocopy);
            rtc_crash_len += tocopy;
            rtc_crash_buf[rtc_crash_len] = 0;
        }
    }
    return len;
}

// Call early in setup(), after Serial.begin() so Serial works:
void enable_rtc_crash_capture() {
    // Make sure we don't reinitialize buffer on normal boot.
    if (rtc_crash_len == 0 && rtc_crash_buf[0] == 0) {
        // initialize
        rtc_crash_len = 0;
        rtc_crash_buf[0] = 0;
    }
    esp_log_set_vprintf(&crash_capture_vprintf);
}

// Call at very start of setup() to check if there's a saved crash and dump it to SD
void flush_rtc_crash_to_sd() {
    if (rtc_crash_len == 0) return; // nothing to do

    // Try to mount the SD card (use your board's CS pin if needed)
    if (!SD.begin()) {
        Serial.println("flush_rtc_crash_to_sd: SD.begin() failed");
        return;
    }

    File f = SD.open("/crash_from_rtc.txt", FILE_APPEND);
    if (!f) {
        Serial.println("flush_rtc_crash_to_sd: open failed");
        return;
    }
    f.write((const uint8_t*)rtc_crash_buf, rtc_crash_len);
    f.flush();
    f.close();

    // Clear buffer so we don't re-dump on next boot
    rtc_crash_len = 0;
    rtc_crash_buf[0] = 0;
}

#ifdef LITE_VERSION
#include "githubUpdater.h"
#include "wpa_sec.h"
#endif

uint8_t state;
uint8_t activity = 14;
unsigned long previousMillis = 0;  // Zmienna do przechowywania ostatniego czasu wykonania funkcjami
unsigned long interval = 120000;  // 2 minuty w milisekundach (2 * 60 * 1000)
bool firstSoundEnable;
bool isSoundPlayed = false;
uint32_t last_mood_switch = 10001;
uint8_t wakeUpList[] = {0, 1, 2};

void initM5() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Keyboard.begin();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);  
  flush_rtc_crash_to_sd();
  enable_rtc_crash_capture();
  logMessage("System booting...");
  initM5();
  initVars();
  M5.Display.setBrightness(brightness);
  initColorSettings();
  initUi();
  preloadMoods();
  if(initVars()){}
  else{
    #ifndef BYPASS_SD_CHECK
    drawInfoBox("ERROR!", "SD card is needed to work.", "Insert it and restart", false, true);
    while(true){delay(10);}
    #endif
  }
  wakeUp();
  #ifdef LITE_VERSION
  #ifndef SKIP_AUTO_UPDATE
  drawInfoBox("Update", "Checking for updates", "Please wait...", false, false);
  WiFi.begin(savedApSSID.c_str(), savedAPPass.c_str());
  delay(5000);
  if(check_for_new_firmware_version(true)) {
    drawInfoBox("Update", "New firmware version available", "Downloading...", false, false);
    delay(1000);
    logMessage("New firmware version available, downloading...");
    if(ota_update_from_url(true)) {
      drawInfoBox("Update", "Update successful", "Restarting...", false, false);
      logMessage("Update successful, restarting...");
      delay(1000);
      ESP.restart();
    } else {
      logMessage("Update failed");
    }
  } else {
    drawInfoBox("Update", "No new firmware version found", "Booting...", false, false);
    logMessage("No new firmware version found, or wifi not connected");
    delay(1000);
  }
  if(WiFi.status() == WL_CONNECTED) {
    if(lite_mode_wpa_sec_sync_on_startup){
      logMessage("Syncing known networks with WPA_SEC");
      processWpaSec(wpa_sec_api_key.c_str());
    }
  }
  #endif
  #endif

  if(pwnagothiModeEnabled) {
    logMessage("Pwnagothi mode enabled");
    pwnagothiBegin();
  } else {
    logMessage("Pwnagothi mode disabled");
  }
}

void wakeUp() {
  for (uint8_t i = 0; i <= 2; i++) {
    setMood(wakeUpList[i]);
    updateUi();
    delay(1250);
  }
}


void loop() {
  unsigned long currentMillis = millis();
  M5.update();
  M5Cardputer.update();
  if(M5Cardputer.Keyboard.isKeyPressed(KEY_OPT) && M5Cardputer.Keyboard.isKeyPressed(KEY_LEFT_CTRL) && M5Cardputer.Keyboard.isKeyPressed(KEY_FN)){
    esp_will_beg_for_its_life();
  }
  updateUi(true);
  if(!pwnagothiMode)  {
    if (currentMillis >= interval) {
      interval = interval + 120000;  // Zaktualizowanie czasu ostatniego wykonania funkcji
      updateActivity(false);  // Wykonanie funkcji co 2 minuty
      setMood(activity);
      logMessage("Mood changed");
    }
    setMood(activity);
  }
}

void updateActivity(bool reward = false) {
  if(reward){
    if(activity == 2 || activity == 26){
      activity = 10;
    }
    else{
      activity++;
    }
  }
  else{
    if(activity==0){  
    }
    else{
      activity--;
    }
  } 
}

void Sound(int frequency, int duration, bool sound){
  if(sound && isSoundPlayed==false){
    isSoundPlayed = true;
    M5Cardputer.Speaker.tone(frequency, duration);
  }
  else if (isSoundPlayed == true){
    isSoundPlayed = false;
  }
}