# Copilot instructions — M5Gothi

Quick context
- Firmware for ESP32 (PlatformIO, Arduino framework) targeting the M5Cardputer (board: `m5stack-stamps3`).
- Main entry: `src/src.ino`. UI layer: `src/ui.*`. Pwnagothi logic: `src/pwnagothi.*`. Moods: `src/mood.*` and `src/moodLoader.*`.
- SD card is required at runtime; configuration and runtime data live on the SD card (see `m5gotchi.conf`, `personality.conf`, `/handshake`, `uploaded.json`, `cracked.json`).

What to read first (high signal files)
- `src/src.ino` — boot, init sequence, main loop, example of keyboard handling and update flow.
- `src/ui.*` — all rendering helpers and `drawInfoBox` usage.
- `src/settings.h` — compile-time flags, pin mappings and defaults (e.g. SD pins, feature flags).
- `platformio.ini` — environments, build flags, embedded certs (`board_build.embed_files`) and dependency versions.
- `src/githubUpdater.*` and `src/wpa_sec.*` — OTA/update and WPA-Sec integration (only compiled for `LITE_VERSION`).
- `pre_commit.sh` and `README.md` — local workflow and legal/safety notes.

Architecture & patterns (practical notes)
- The project keeps device-specific behavior behind PlatformIO envs: `Cardputer-dev`, `Cardputer-full`, `cardputer-lite`, `m5stick-c`. Use `-e <env>` to build for a target.
- Feature flags are set at compile time (preprocessor): `LITE_VERSION`, `BYPASS_SD_CHECK`, `SKIP_AUTO_UPDATE`. Code branches (e.g. auto-update, wpa-sec sync) depend on those defines.
- Hardware + UI: uses `M5Unified` and `M5Cardputer` libraries; keyboard input via `M5Cardputer.Keyboard` and key constants (e.g. `KEY_OPT`, `KEY_LEFT_CTRL`, `KEY_FN`). Search usages in `src/src.ino` for examples.
- Logging & UI: prefer `logMessage(...)` for textual logs and `drawInfoBox(...)` for small interactive messages — both are used widely; see `src/logger.*` and `src/ui.*`.
- SD-first runtime: the firmware expects files to exist on the SD card. The code will block (in normal builds) if the SD card isn't present (guarded by `BYPASS_SD_CHECK`). Don't remove SD flows without a device test.

Build / flash / serial monitor (examples)
- Build default env (choose env from `platformio.ini`):

```bash
pio run -e cardputer-lite
``` 

- Build + upload to device:

```bash
pio run -e cardputer-lite --target upload
```

- Open serial monitor (115200, with exception decoder):

```bash
pio device monitor -e cardputer-lite --baud 115200
```

Key PlatformIO details to respect
- `board_build.embed_files` in each env includes `certs/*.pem` — these are embedded in the binary and expected by TLS code paths.
- `monitor_filters = esp32_exception_decoder` is enabled; keep it during debug to get decoded backtraces.
- Don't change lib versions in `platformio.ini` lightly; try builds locally and on-device.

Automation & update flow
- Lite builds may auto-check and download updates at boot (`githubUpdater` functions called from `src/src.ino`). The checks run only if `LITE_VERSION` is defined and `SKIP_AUTO_UPDATE` is not defined.
- WPA-Sec sync runs on startup when `lite_mode_wpa_sec_sync_on_startup` is true and credentials are present in `m5gotchi.conf`.

Contribution & safety
- Run `./pre_commit.sh` before opening a PR (repository recommends it in `README.md`).
- This firmware can be used for activities that may be illegal. See `README.md` CAUTION block — do not change or remove that warning.

Small examples to copy/paste
- Check for new firmware (see `src/src.ino`):

```cpp
// example flow in setup()
if(check_for_new_firmware_version(true)) {
  ota_update_from_url(true);
}
```

- Keyboard combo to trigger special behavior (see `src/src.ino`):

```cpp
if(M5Cardputer.Keyboard.isKeyPressed(KEY_OPT) && M5Cardputer.Keyboard.isKeyPressed(KEY_LEFT_CTRL) && M5Cardputer.Keyboard.isKeyPressed(KEY_FN)) {
  esp_will_beg_for_its_life();
}
```

Where to add tests / smoke checks
- There are no unit tests in the repo. If adding tests, prefer small PlatformIO-compatible integration checks or a host-side script that validates generated SD files and JSON formats.

If something is unclear
- If you need runtime-only details (exact SD layout created on first boot, or precise content of `m5gotchi.conf`) run the firmware in-device and inspect the SD card contents. I can add more examples if you point me to exact files to document.

---
If you'd like, I can merge this into the repo now and add quick links to specific functions (e.g. `check_for_new_firmware_version`, `ota_update_from_url`) with line references.
