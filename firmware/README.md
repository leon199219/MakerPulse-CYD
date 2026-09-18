# MakerPulse for ESP32-2432S028 (CYD)

This folder is the Arduino sketch. Clone the repo and open `MakerPulse_CYD.ino` here, or download a zip from the configurator (then `config.h` is already filled in).

The sketch shows your MakerWorld stats on the 2.8" screen (downloads, likes, prints, boosts, collections, comments) and sends a Telegram message on changes, including the model title. Comments is the total of all your published models, not just the featured ones. The module keeps checking on its own — you can close the browser.

## What you need

- ESP32-2432S028 (Cheap Yellow Display, 240×320)
- USB cable that carries data (not charge-only)
- Arduino IDE 2
- Wi-Fi
- A MakerWorld profile (public user ID — see below)
- Optional: Telegram bot

## 1. Set up Arduino IDE

1. Install [Arduino IDE 2](https://www.arduino.cc/en/software).
2. File → Preferences → Additional boards manager URLs, add:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Boards Manager: install **esp32** by Espressif (not the "Arduino ESP32 Boards" package). Version 2.0.17 or 3.x is fine.
4. Library Manager, install:
   - **LovyanGFX** (lovyan03)
   - **ArduinoJson** (Benoit Blanchon, version 7)
   - **PubSubClient** (Nick O'Leary) — required for Home Assistant / MQTT

## 2. Put the sketch on a simple path (Windows)

Arduino on Windows crashes on paths with spaces, parentheses or OneDrive.

1. Unzip the archive.
2. Move the folder to **`C:\MakerPulse_CYD\`** (not Downloads, not Documents).
3. Open **`C:\MakerPulse_CYD\MakerPulse_CYD.ino`**.

Do not use special characters in the path: `-_().,` and similar.
If you leave it in Downloads (`MakerPulse_CYD (1)` or OneDrive) you get:
`bootloader.bin was unexpected at this time`.

## 3. Board settings

- Board: **ESP32 Dev Module**
- Upload Speed: **115200**
- Flash Size: **4MB (32Mb)**
- Partition Scheme: **Default 4MB with spiffs**
- PSRAM: **Disabled**
- USB CDC On Boot: **Enabled** (with Arduino-ESP32 3.x)

Select the correct port (USB).

## 4. MakerWorld account (`MAKERWORLD_UID`)

MakerPulse does not create this number. Bambu/MakerWorld assigns it to your account. The CYD uses it to fetch **public** profile and model stats — no MakerWorld login is stored on the device.

Use **only the digits**, without `@` and without `user_`. A handle such as `@Le0n._.` is not a UID. A model URL (`/models/3206534-…#profileId-3629145`) is a model / print-profile ID, not your account number.

### Fastest method (recommended)

1. Go to [makerworld.com](https://makerworld.com) and sign in.
2. Press **F12** → **Console** tab.
3. Paste this and press Enter:

```javascript
fetch("/api/v1/design-user-service/my/preference")
  .then(r => r.json())
  .then(d => { console.log("UID:", d.uid); copy(String(d.uid)); });
```

Your UID appears in the console and is copied to the clipboard. Paste that number into `MAKERWORLD_UID`.

That `uid` field is the numeric account number that tracking projects expect (about 10 digits).

### Faster still, without the API

- Handle looks like `@user_1298228011`? Then `1298228011` is usually already your UID.
- Avatar: open your profile, right-click the photo → **Copy image address**. The URL often contains `/avatar/4044076662/` — that number is your UID.
- Open one of **your own** models. In the page source or the Network tab you will find `designCreator.uid`.
- If the address bar shows `/u/3535571310`, that number is the UID.

In `config.h`:

```c
#define MAKERWORLD_UID 3384175483UL
#define MAKER_NAME "Le0n._."
```

Keep the `UL` suffix. `MAKER_NAME` is only the label on the screen (a handle is fine there). `MAKERWORLD_UID` must stay the numeric id.

## 5. Flashing

1. Open `MakerPulse_CYD.ino` (leave `config.h`, `config.h.example` and `mp_types.h` in the same folder).
   - Zip from the configurator: **change nothing**, just Upload.
   - From GitHub: copy `config.h.example` → `config.h` and fill in Wi-Fi + MakerWorld ID (every line has a comment).
        -  Or edit manually: `MakerPulse_CYD.ino` >> line 45  // --- Manual settings
2. Plug in the CYD.
3. Sketch → Upload.
4. Stuck on "Connecting..."? Hold **BOOT**, tap **RST**, release BOOT, and upload again.

After a successful upload the screen starts with Wi-Fi → MakerWorld → your six figures.

## 6. The screen looks wrong ## TROUBLESHOOTING

Do not rotate the sketch in Arduino — pick the orientation in the configurator, download again and flash again.

- Image rotated 90°, text scrambled: different **rotation** (USB left/right/bottom/top).
- Still 90°: enable **newer CYD (USB-C)**.
- Colors inverted or very washed out: **Invert colors**.
- Completely white/black: USB cable and power.

## 7. Telegram

1. In Telegram: search **@BotFather** → `/newbot` → copy the token.
2. Open your new bot and tap **Start**.
3. Enter the chat ID in the configurator.
4. Send a test message there before you flash.

The CYD stores the last figures in flash. The first boot does not send a notification; after that only a real change does. The message names the title of **every** model whose downloads, likes, prints, boosts, collections or comments changed — not only the pinned one. Comments is the sum of **all** published models. The bottom of the screen shows `gevonden/totaal` (e.g. `48/48`); red means that round's count was incomplete and the previous total is kept.

## 8. Home Assistant — screen on/off and brightness

After flashing, the CYD shows its IP at the bottom-right of the screen.

### Option A — MQTT (recommended)

1. Install the **Mosquitto broker** add-on in Home Assistant.
2. In the MakerPulse configurator, enter the Home Assistant IP (and user/password if you set those in Mosquitto).
3. Download again and flash again.
4. In HA: Settings → Devices → MQTT. Light **MakerPulse scherm** appears automatically (on/off + brightness 0–255).

Discovery prefix is `homeassistant`. MQTT discovery must be on (default for the official integration).

### Option B — HTTP, without MQTT

```yaml
rest_command:
  makerpulse_light:
    url: "http://makerpulse-cyd.local/light"
    method: POST
    headers:
      Content-Type: application/json
    payload: '{"state": "{{ state }}", "brightness": {{ brightness | default(255) }}}'
```

On: `state: ON`, brightness `0–255`. Off: `state: OFF`.

Status: `http://makerpulse-cyd.local/light` or `http://IP/light`.

## Problems

| Symptom | Check |
| --- | --- |
| `bootloader.bin was unexpected at this time` | Move the folder to `C:\MakerPulse_CYD\`. No spaces, no parentheses, no OneDrive/Downloads. |
| Wi-Fi failed | SSID/password, 2.4 GHz (not 5 GHz) |
| MakerWorld offline | Numeric `MAKERWORLD_UID` (console snippet / avatar URL), internet |
| Telegram failed | Token, chat ID, /start tapped |
| Upload fails | Data cable, CH340 driver, BOOT button |
