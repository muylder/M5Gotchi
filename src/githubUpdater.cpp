#include "settings.h"
#include "logger.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "githubUpdater.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <string>
#include <cstring>


extern const char github_root_cert_pem_start[] asm("_binary_certs_github_root_cert_pem_start");
extern const char github_root_cert_pem_end[] asm("_binary_certs_github_root_cert_pem_end");


void ota_update_from_url() {

  //drawInfoBox("Updating...", "Updating from github...", "This may take a while...", false,false);
  esp_http_client_config_t config = {
    .url = UPDATE_LINK,
    .cert_pem = github_root_cert_pem_start,
  };
  // Initialize the HTTP client
  esp_err_t ret = esp_https_ota(&config);
  if (ret == ESP_OK) {
    esp_restart();
  } else {
    //drawInfoBox("ERROR!", "Update failed, reason:", esp_err_to_name(ret), true, false);
    printf("OTA failed: %s\n", esp_err_to_name(ret));
  }
}

bool check_for_new_firmware_version() {
  // const char* CURRENT_VERSION = "0.2";
  // const char* VERSION_URL = 
  // const char* TEMP_FILE_PATH = 

  // Configure HTTP client
  esp_http_client_config_t config = {
    .url = VERSION_URL,
    .cert_pem = github_root_cert_pem_start,
  };

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (client == NULL) {
    logMessage("Failed to initialise HTTP connection");
    return false;
  }

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    logMessage((std::string("Failed to open HTTP connection: ") + esp_err_to_name(err)).c_str());
    esp_http_client_cleanup(client);
    return false;
  }

  FILE* f = fopen(TEMP_FILE_PATH, "w");
  if (!f) {
    logMessage("Failed to open file for writing");
    esp_http_client_cleanup(client);
    return false;
  }

  int content_length = esp_http_client_fetch_headers(client);
  if (content_length <= 0) {
    logMessage("Content length error");
    fclose(f);
    esp_http_client_cleanup(client);
    return false;
  }

  char buffer[128];
  int read_len;
  while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer)-1)) > 0) {
    fwrite(buffer, 1, read_len, f);
  }
  fclose(f);
  esp_http_client_cleanup(client);

  // Read version from file
  f = fopen(TEMP_FILE_PATH, "r");
  if (!f) {
    logMessage("Failed to open temp file for reading");
    remove(TEMP_FILE_PATH);
    return false;
  }
  char remote_version[32] = {0};
  fgets(remote_version, sizeof(remote_version), f);
  fclose(f);

  // Remove trailing newline
  remote_version[strcspn(remote_version, "\r\n")] = 0;

  // Clean up temp file
  remove(TEMP_FILE_PATH);

  // Compare versions
  if (strcmp(CURRENT_VERSION, remote_version) != 0) {
    return true;
  }
  return false;
}