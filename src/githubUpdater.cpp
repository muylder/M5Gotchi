#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "githubUpdater.h"

void ota_update_from_url() {
  //drawInfoBox("Updating...", "Updating from github...", "This may take a while...", false,false);
  esp_http_client_config_t config = {
    .url = "https://devsur11.github.io/M5gotchi/docs/latest.bin",
    .cert_pem = NULL,  // Use `NULL` to skip cert verification, or add GitHub cert
  };

  esp_err_t ret = esp_https_ota(&config);
  if (ret == ESP_OK) {
    esp_restart();
  } else {
    //drawInfoBox("ERROR!", "Update failed, reason:", esp_err_to_name(ret), true, false);
    printf("OTA failed: %s\n", esp_err_to_name(ret));
  }
}