//This file is copied from https://github.com/viniciusbo/m5-palnagotchi repo 
//- all credits to the original author
//I just added some of my code...

#include "ArduinoJson.h"
#include "esp_wifi.h"

typedef struct {
  int epoch;
  String face;
  String grid_version;
  String identity;
  String name;
  int pwnd_run;
  int pwnd_tot;
  String session_id;
  int timestamp;
  int uptime;
  String version;
  signed int rssi;
  int last_ping;
  bool gone;
} pwngrid_peer;

void pwngridAdvertise();