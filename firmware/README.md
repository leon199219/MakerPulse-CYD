# MakerPulse for ESP32-2432S028 (CYD)

This folder is the Arduino sketch. Clone the repo and open `MakerPulse_CYD.ino` here, or download a zip from the configurator (then `config.h` is already filled in).

The sketch shows your MakerWorld stats on the 2.8" screen (downloads, likes, prints, boosts, collections, comments) and sends a Telegram message on changes, including the model title. Comments is the total of all your published models, not just the featured ones. The module keeps checking on its own — you can close the browser.

## What you need

- ESP32-2432S028 (Cheap Yellow Display, 240×320)
- USB cable that carries data (not charge-only)
- Arduino IDE 2
- 2.4 GHz Wi-Fi
- A MakerWorld profile (numeric user ID — see below)
- Optional: Telegram bot, MQTT / Home Assistant

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

1. Unzip the archive (or clone this repo).
2. Move the folder to **`C:\MakerPulse_CYD\`** (not Downloads, not Documents).
3. Keep these files in that same folder:
   - `MakerPulse_CYD.ino`
   - `mp_types.h`
   - `config.h.example`
   - `config.h` (you create this in the next step)

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

## 4. Create and edit `config.h`

The sketch does **not** contain your Wi-Fi or MakerWorld id. Those go in `config.h` in the same folder as the `.ino`.

### From GitHub (manual)

1. Copy [`config.h.example`](config.h.example) to **`config.h`** (same folder).
   - Windows Explorer: copy the file, rename the copy to `config.h`.
   - Or in a terminal: `copy config.h.example config.h`
2. Open `config.h` in Arduino IDE or any text editor.
3. Replace the placeholders. Every `#define` has a comment on the same line.
4. Save. Do **not** commit `config.h` — it is gitignored because it holds secrets.

Minimum you must change:

```c
#define WIFI_SSID "YourNetwork"           // 2.4 GHz only, not 5 GHz
#define WIFI_PASSWORD "YourPassword"
#define MAKERWORLD_UID 1298228011UL       // digits only, keep the UL
#define MAKER_NAME "Maker"                // short label on the screen
```

Leave `JOUW_WIFI_NAAM` / `JOUW_WIFI_WACHTWOORD` / `MAKERWORLD_UID 0` and the CYD will stop at **Fill in config.h**.

### From the configurator zip

`config.h` is already filled in. Do not edit it unless you want to change Wi-Fi, UID or options. Then skip to flashing.

### Optional settings (same file)

| Define | Meaning |
| --- | --- |
| `TELEGRAM_BOT_TOKEN` | From [@BotFather](https://t.me/BotFather) after `/newbot`. Empty = no messages. |
| `TELEGRAM_CHAT_ID` | Your chat id (a number, sometimes negative). Empty = no messages. |
| `POLL_INTERVAL_SEC` | How often to check MakerWorld: `120`, `300`, `600` or `900`. |
| `CYD_ROTATION` | `0` USB bottom, `1` USB left, `2` USB top, `3` USB right. |
| `CYD_INVERT` | `1` if colours look negative / washed out. |
| `CYD_PANEL_ST7789` | `1` on a newer USB-C CYD. Then also set `CYD_OFFSET_ROTATION 0` and `CYD_RGB_ORDER 1`. |
| `MQTT_HOST` | Home Assistant IP. Empty = MQTT off. |
| `MQTT_PORT` | Default `1883`. |
| `MQTT_USER` / `MQTT_PASS` | Mosquitto login, or empty. |
| `NOTIFY_DOWNLOADS` … `NOTIFY_COMMENTS` | `1` = Telegram for that metric, `0` = off. |

### No `config.h` at all

Only if the example file is missing: edit `MakerPulse_CYD.ino` around line 45 (`// --- Manual settings`). Fill the same `#define`s there. Prefer `config.h` so you do not touch the sketch.

### Find `MAKERWORLD_UID`

MakerPulse does not create this number. Bambu/MakerWorld assigns it. Use **only the digits**, without `@` and without `user_`. A handle such as `@YourName` is not a UID. A model URL (`/models/1841486-…#profileId-1234567`) is a model / print-profile ID, not your account number.

**Fastest (recommended)**

1. Go to [makerworld.com](https://makerworld.com) and sign in.
2. Press **F12** → **Console**.
3. Paste this and press Enter:

```javascript
fetch("/api/v1/design-user-service/my/preference")
  .then(r => r.json())
  .then(d => { console.log("UID:", d.uid); copy(String(d.uid)); });
```

The UID appears in the console and is copied. Paste it into `MAKERWORLD_UID` and keep the `UL` suffix.

That `uid` field is the numeric account number that tracking projects expect (about 10 digits).

**Without the API**

- Handle looks like `@user_1298228011`? Then `1298228011` is usually already your UID.
- Avatar: open your profile, right-click the photo → **Copy image address**. The URL often contains `/avatar/4044076662/` — that number is your UID.
- Open one of **your own** models. In the page source or the Network tab you will find `designCreator.uid`.
- If the address bar shows `/u/3535571310`, that number is the UID.

`MAKER_NAME` is only the text on the screen. It can be a handle. `MAKERWORLD_UID` must stay numeric.

## 5. Flashing

1. Confirm `config.h`, `config.h.example` and `mp_types.h` sit next to `MakerPulse_CYD.ino`.
2. Plug in the CYD.
3. Sketch → Upload.
4. Stuck on "Connecting..."? Hold **BOOT**, tap **RST**, release BOOT, and upload again.

After a successful upload the screen goes Wi-Fi → MakerWorld → your six figures.

## 6. The screen looks wrong ## TROUBLESHOOTING

Change these in **`config.h`**, save, and flash again. Do not rotate the sketch inside Arduino.

| What you see | Change in `config.h` |
| --- | --- |
| Image rotated 90°, text scrambled | `CYD_ROTATION` (`0`–`3`, USB bottom/left/top/right) |
| Still 90° on a USB-C CYD | `CYD_PANEL_ST7789 1`, `CYD_OFFSET_ROTATION 0`, `CYD_RGB_ORDER 1` |
| Colours inverted or washed out | `CYD_INVERT 1` |
| Completely white/black | USB data cable and power |

## 7. Telegram

1. In Telegram: search **@BotFather** → `/newbot` → copy the token.
2. Open your new bot and tap **Start**.
3. Put the token and chat ID in `config.h` (`TELEGRAM_BOT_TOKEN`, `TELEGRAM_CHAT_ID`) — or in the configurator before you download the zip.
4. Flash again. Send a test from the configurator if you use it.

The CYD stores the last figures in flash. The first boot does not send a notification; after that only a real change does. The message names the title of **every** model whose downloads, likes, prints, boosts, collections or comments changed — not only the pinned one. Comments is the sum of **all** published models. The bottom of the screen shows `gevonden/totaal` (e.g. `48/48`); red means that round's count was incomplete and the previous total is kept.

## 8. Home Assistant — screen on/off and brightness

After flashing, the CYD shows its IP at the bottom-right of the screen (`makerpulse-cyd.local`).

### Option A — MQTT (recommended)

1. Install the **Mosquitto broker** add-on in Home Assistant.
2. In `config.h` set `MQTT_HOST` to the Home Assistant IP (and `MQTT_USER` / `MQTT_PASS` if Mosquitto requires them).
3. Flash again.
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
| Screen says fill in `config.h` | File is named `config.h` (not `.example`). Wi-Fi placeholders and `MAKERWORLD_UID 0` replaced. |
| `bootloader.bin was unexpected at this time` | Move the folder to `C:\MakerPulse_CYD\`. No spaces, no parentheses, no OneDrive/Downloads. |
| Wi-Fi failed | SSID/password in `config.h`, 2.4 GHz (not 5 GHz) |
| MakerWorld offline | Numeric `MAKERWORLD_UID` (console snippet / avatar URL), internet |
| Telegram failed | Token and chat ID in `config.h`, `/start` tapped |
| Upload fails | Data cable, CH340 driver, BOOT button |
