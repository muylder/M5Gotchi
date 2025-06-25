# M5Gothi

M5Gothi brings the powerful Pwnagothi functions and interface to the M5Cardputer platform, providing both automatic and manual Wi-Fi control through an integrated keyboard interface.

> [!CAUTION]
> THIS FIRMWARE CAN BE USED IN WAYS THAT MAY VIOLATE LOCAL LAWS. YOU ARE RESPONSIBLE FOR YOUR OWN ACTIONS. DO NOT USE THIS FOR MALICIOUS PURPOSES.
> **THIS IS A RESEARCH TOOL DESIGNED FOR EDUCATIONAL AND LEGAL SECURITY TESTING PURPOSES ONLY.**
> This tool is intended **only for legal research and educational purposes**.
> Use of this firmware on unauthorized networks, or against devices without explicit permission, is **strictly prohibited** and **illegal in many countries**.
> The author takes **no responsibility** for any misuse, damage, or legal consequences resulting from use of this software. Always comply with local laws and regulations.

---

##  Features

- Full Pwnagothi functionality adapted for M5Cardputer
- Manual Wi-Fi control via integrated keyboard UI making it first fully UI-controlled pwnagotchi
- Automatic handshake capture using “Pwnagothi Auto Mode”
- **Update firmware directly from GitHub, SD card, or built-in Web UI**
- Planned support for advanced attack modes:
  - BadUSB
  - BLE attacks
  - IR Remote emulation
  - BadBLE device impersonation

---
> [!IMPORTANT]
> An SD card is **required** for the firmware to function properly.

### SD Card File Structure

- Configuration file will be created at first boot and then used to store informations. **Don't mess with it at your own - use device UI to change these values!**
- Captured handshakes will be stored inside a folder called:
  ```
  /handshake
  ```

---

##  Supported Devices

| Device         | Status         | Notes                          |
|----------------|----------------|--------------------------------|
| M5Cardputer    | ✅ Supported   | Main target device             |
| M5Stack Core2  | ⏳ Planned     | Requires UI adaptation         |
| M5StickC       | ⏳ Planned     | Limited screen/input capability|
| M5Paper        | ⏳ Planned     | E-ink rendering testing needed |

---

##  TODO / Planned Features

| Feature                 | Status     |
|-------------------------|------------|
| Pwnagothi Auto Mode     | ✅ Done    |
| GitHub Update Support   | ✅ Done    |
| SD Card Update          | ✅ Done    |
| Web UI Update           | ✅ Done    |
| BLE Attacks             | ⏳ Planned |
| BadUSB Mode             | ⏳ Planned |
| BadBLE Emulation        | ⏳ Planned |
| IR Remote Support       | ⏳ Planned |

---

## Requirements

- [PlatformIO](https://platformio.org/) — for building and flashing the firmware
- Git — for cloning the repository
- All other dependencies are automatically handled by PlatformIO

### Installing PlatformIO

You can install PlatformIO using either:

- **Visual Studio Code extension**
  - Install VS Code
  - Open Extensions → Search for "PlatformIO IDE"
  - Install and reload

- **Command-line (CLI)**
  Follow instructions here: [https://platformio.org/install/cli](https://platformio.org/install/cli)

---

## Build and Flash Instructions

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/ESPBlaster.git
   cd ESPBlaster
   ```

2. Build and upload via PlatformIO:
   ```bash
   pio run
   pio run --target upload
   ```

---

## Usage Instructions

- **UI** is fully controlled via the **built-in keyboard**
- Press `ESC` to open the main menu
- Use **arrow keys** to navigate
- Exit apps using `Fn + ESC`
- Press `G0` button to toggle the screen on/off

---

##  Update Methods

| Method        | Status    | Description                                     |
|---------------|-----------|-------------------------------------------------|
| GitHub        | ✅ Done    | Auto update via GitHub Pages through UI        |
| SD Card       | 🟡 In Progress | Place new firmware file and trigger update   |
| Web UI        | ✅ Done   | Upload update through browser interface     |

---

## Examples
[] Fill this section before relase!

---

## License

This project is licensed under the **MIT License** — see [LICENSE](LICENSE) for details.

---

## Contributing

Contributions, issue reports, and pull requests are welcome!
To help out, fork this repo and open a PR.

---

## Contact

Join our Discord community for support, discussion, and sneak peeks at upcoming features.

---

# TODO
- add pwnagothi auto mode - done!!!!!!!!!!!!!!!
- add update from github - done but needs http address that will be created after relase
- relase to public
