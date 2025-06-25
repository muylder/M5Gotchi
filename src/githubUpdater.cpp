#include "settings.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "githubUpdater.h"

extern const char github_root_cert_pem_start[] asm("_binary_certs_github_root_cert_pem_start");
extern const char github_root_cert_pem_end[] asm("_binary_certs_github_root_cert_pem_end");


void ota_update_from_url() {

  //drawInfoBox("Updating...", "Updating from github...", "This may take a while...", false,false);
  esp_http_client_config_t config = {
    .url = UPDATE_LINK,
    .cert_pem = github_root_cert_pem_start,
  };

  esp_err_t ret = esp_https_ota(&config);
  if (ret == ESP_OK) {
    esp_restart();
  } else {
    //drawInfoBox("ERROR!", "Update failed, reason:", esp_err_to_name(ret), true, false);
    printf("OTA failed: %s\n", esp_err_to_name(ret));
  }
}