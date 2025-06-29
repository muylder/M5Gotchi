#include "settings.h"
#include "logger.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "githubUpdater.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <sys/stat.h>
#include "cJSON.h"

extern const char github_root_cert_pem_start[] asm("_binary_certs_github_root_cert_pem_start");
extern const char github_root_cert_pem_end[] asm("_binary_certs_github_root_cert_pem_end");

static bool ensure_temp_dir() {
  struct stat st = {0};
  if (stat(TEMP_DIR, &st) == -1) {
    if (mkdir(TEMP_DIR, 0777) != 0) {
      logMessage("Failed to create temp directory on SD card");
      return false;
    }
  }
  return true;
}

static bool download_file(const char* url, const char* dest_path) {
  esp_http_client_config_t config = {
    .url = url,
    .cert_pem = github_root_cert_pem_start,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    logMessage("Failed to initialise HTTP client");
    return false;
  }
  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    logMessage((std::string("Failed to open HTTP connection: ") + esp_err_to_name(err)).c_str());
    esp_http_client_cleanup(client);
    return false;
  }
  FILE* f = fopen(dest_path, "wb");
  if (!f) {
    logMessage("Failed to open file for writing");
    esp_http_client_cleanup(client);
    return false;
  }
  char buffer[512];
  int read_len;
  while ((read_len = esp_http_client_read(client, buffer, sizeof(buffer))) > 0) {
    fwrite(buffer, 1, read_len, f);
  }
  fclose(f);
  esp_http_client_cleanup(client);
  return true;
}

static bool parse_json_version_and_file(const char* json_path, char* out_version, size_t version_len, char* out_file, size_t file_len) {
  FILE* f = fopen(json_path, "rb");
  if (!f) {
    logMessage("Failed to open JSON file for parsing");
    return false;
  }
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* data = (char*)malloc(size + 1);
  if (!data) {
    fclose(f);
    logMessage("Failed to allocate memory for JSON");
    return false;
  }
  fread(data, 1, size, f);
  data[size] = 0;
  fclose(f);

  cJSON* root = cJSON_Parse(data);
  free(data);
  if (!root) {
    logMessage("Failed to parse JSON");
    return false;
  }
  cJSON* version = cJSON_GetObjectItem(root, "version");
  cJSON* file = cJSON_GetObjectItem(root, "file");
  if (!cJSON_IsString(version) || !cJSON_IsString(file)) {
    cJSON_Delete(root);
    logMessage("JSON missing required fields");
    return false;
  }
  strncpy(out_version, version->valuestring, version_len - 1);
  out_version[version_len - 1] = 0;
  strncpy(out_file, file->valuestring, file_len - 1);
  out_file[file_len - 1] = 0;
  cJSON_Delete(root);
  return true;
}

bool check_for_new_firmware_version(bool lite) {
  if (!ensure_temp_dir()) return false;

  const char* json_url = lite ? LITE_JSON_URL : NORMAL_JSON_URL;
  logMessage(String("Downloading JSON: ") + json_url);
  if (!download_file(json_url, TEMP_JSON_PATH)) {
    logMessage("Failed to download JSON file");
    return false;
  }

  char remote_version[32] = {0};
  char bin_url[256] = {0};
  if (!parse_json_version_and_file(TEMP_JSON_PATH, remote_version, sizeof(remote_version), bin_url, sizeof(bin_url))) {
    remove(TEMP_JSON_PATH);
    return false;
  }

  logMessage(String("Remote version: ") + remote_version + ", Current: " + CURRENT_VERSION);

  bool update_needed = strcmp(CURRENT_VERSION, remote_version) < 0;
  remove(TEMP_JSON_PATH);
  return update_needed;
}

bool ota_update_from_url(bool lite) {
  if (!ensure_temp_dir()) return false;

  const char* json_url = lite ? LITE_JSON_URL : NORMAL_JSON_URL;
  logMessage(String("Downloading JSON: ") + json_url);
  if (!download_file(json_url, TEMP_JSON_PATH)) {
    logMessage("Failed to download JSON file");
    return false;
  }

  char remote_version[32] = {0};
  char bin_url[256] = {0};
  if (!parse_json_version_and_file(TEMP_JSON_PATH, remote_version, sizeof(remote_version), bin_url, sizeof(bin_url))) {
    remove(TEMP_JSON_PATH);
    return false;
  }

  if (strcmp(CURRENT_VERSION, remote_version) >= 0) {
    logMessage("No update needed");
    remove(TEMP_JSON_PATH);
    return false;
  }

  logMessage(String("Downloading firmware bin: ") + bin_url);
  if (!download_file(bin_url, TEMP_BIN_PATH)) {
    logMessage("Failed to download firmware bin");
    remove(TEMP_JSON_PATH);
    return false;
  }
  remove(TEMP_JSON_PATH);

  esp_http_client_config_t config = {
    .url = bin_url,
    .cert_pem = github_root_cert_pem_start,
  };

  // Use esp_https_ota with file
  esp_err_t ret = esp_https_ota(&config);
  remove(TEMP_BIN_PATH);

  if (ret == ESP_OK) {
    logMessage("OTA update successful, restarting...");
    esp_restart();
    return true;
  } else {
    logMessage(String("OTA failed: ") + esp_err_to_name(ret));
    return false;
  }
}
