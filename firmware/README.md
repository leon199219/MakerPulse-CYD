# MakerPulse voor ESP32-2432S028 (CYD)

Deze map is de Arduino-sketch. Clone de repo en open `MakerPulse_CYD.ino` hier, of download een zip via de configurator (dan is `config.h` al ingevuld).

Deze sketch toont je MakerWorld-stats op het 2,8"-scherm (downloads, likes, prints, boosts, collecties, comments) en stuurt een Telegram-bericht bij veranderingen, inclusief de titel van het model. Comments is het totaal van al je gepubliceerde modellen, niet alleen de uitgelichte. De module blijft zelf checken — de browser mag dicht.

## Wat je nodig hebt

- ESP32-2432S028 (Cheap Yellow Display, 240×320)
- USB-kabel die data doorgeeft (geen charge-only)
- Arduino IDE 2
- WiFi en een Telegram-bot

## 1. Arduino IDE klaarzetten

1. Installeer [Arduino IDE 2](https://www.arduino.cc/en/software).
2. Extra → Voorkeuren → Extra Board Manager-URL's, voeg toe:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Board Manager: installeer **esp32** van Espressif (niet het pakket "Arduino ESP32 Boards"). Versie 2.0.17 of 3.x is prima.
4. Library Manager, installeer:
   - **LovyanGFX** (lovyan03)
   - **ArduinoJson** (Benoit Blanchon, versie 7)
   - **PubSubClient** (Nick O'Leary) — nodig voor Home Assistant / MQTT

## 2. Zet de sketch op een simpel pad (Windows)

Arduino op Windows crasht bij paden met spaties, haakjes of OneDrive.

1. Pak de zip uit.
2. Verplaats de map naar **`C:\MakerPulse_CYD\`** (niet in Downloads, niet in Documenten).
3. Open **`C:\MakerPulse_CYD\MakerPulse_CYD.ino`**.

Gebruik geen speciale tekens in je pad: -_()., e.a.
Als je hem in Downloads laat staan (`MakerPulse_CYD (1)` of OneDrive) krijg je:
`bootloader.bin was unexpected at this time`.

## 3. Boardinstellingen

- Board: **ESP32 Dev Module**
- Upload Speed: **115200**
- Flash Size: **4MB (32Mb)**
- Partition Scheme: **Default 4MB with spiffs**
- PSRAM: **Disabled**
- USB CDC On Boot: **Enabled** (bij Arduino-ESP32 3.x)

Kies de juiste poort (USB).

## 4. Flashen

1. Open `MakerPulse_CYD.ino` (laat `config.h`, `config.h.example` en `mp_types.h` in dezelfde map).
   - Zip uit de configurator: **niets aanpassen**, alleen Uploaden.
   - Van GitHub: kopieer `config.h.example` → `config.h` en vul WiFi + MakerWorld-ID in (elke regel heeft uitleg).
        -  Of pas handmatig aan: `MakerPulse_CYD.ino` >> Regel 45  // --- Handmatige instellingen
2. Sluit de CYD aan.
3. Sketch → Uploaden.
4. Blijft hij hangen op "Connecting..."? Houd **BOOT** in, tik **RST**, laat BOOT los, en upload opnieuw.

Na een geslaagde upload start het scherm met WiFi → MakerWorld → je zes cijfers.

## 5. Scherm ziet er gek uit ## TROUBLESHOOTING

Niet de sketch in Arduino draaien — kies de stand in de configurator, download opnieuw en flash opnieuw.

- Beeld 90° gedraaid, tekst door elkaar: andere **draaiing** (USB links/rechts/onder/boven).
- Nog steeds 90°: zet **nieuwere CYD (USB-C)** aan.
- Kleuren omgekeerd of heel bleek: **Kleuren omkeren**.
- Helemaal wit/zwart: USB-kabel en voeding.

## 6. Telegram

1. In Telegram: zoek **@BotFather** → `/newbot` → kopieer de token.
2. Open je nieuwe bot en tik **Start**.
3. Chat-ID vul je in de configurator in.
4. Stuur daar een testbericht vóór je flasht.

De CYD bewaart de laatste cijfers in flash. Eerste start stuurt geen melding; daarna alleen bij échte verandering. Het bericht noemt de titel van **elk** model waarvan downloads, likes, prints, boosts, collecties of comments veranderden — niet alleen de vastgepinde. Comments is de som van **alle** gepubliceerde modellen. Onderaan het scherm staat `gevonden/totaal` (bijv. `48/48`); rood betekent dat de telling die ronde onvolledig was en het vorige totaal blijft staan.

## 7. Home Assistant — scherm aan/uit en helderheid

Na het flashen toont de CYD zijn IP rechtsonder op het scherm.

### Optie A — MQTT (aanbevolen)

1. Installeer de add-on **Mosquitto broker** in Home Assistant.
2. Vul in de MakerPulse-configurator het IP van Home Assistant in (en user/wachtwoord als je die in Mosquitto hebt gezet).
3. Download opnieuw en flash opnieuw.
4. In HA: Instellingen → Apparaten → MQTT. Lamp **MakerPulse scherm** verschijnt vanzelf (aan/uit + helderheid 0–255).

Discovery-prefix is `homeassistant`. MQTT-discovery moet aan staan (standaard bij de officiële integratie).

### Optie B — HTTP, zonder MQTT

```yaml
rest_command:
  makerpulse_light:
    url: "http://makerpulse-cyd.local/light"
    method: POST
    headers:
      Content-Type: application/json
    payload: '{"state": "{{ state }}", "brightness": {{ brightness | default(255) }}}'
```

Aan: `state: ON`, brightness `0–255`. Uit: `state: OFF`.

Status: `http://makerpulse-cyd.local/light` of `http://IP/light`.

## Problemen

| Symptoom | Check |
| --- | --- |
| `bootloader.bin was unexpected at this time` | Map naar `C:\MakerPulse_CYD\` verplaatsen. Geen spaties, geen haakjes, geen OneDrive/Downloads. |
| WiFi mislukt | SSID/wachtwoord, 2,4 GHz (geen 5 GHz) |
| MakerWorld offline | UID, internet |
| Telegram mislukt | Token, chat-ID, /start getikt |
| Upload faalt | Data-kabel, CH340-driver, BOOT-knop |
