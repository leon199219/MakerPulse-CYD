# MakerPulse

MakerWorld-stats op een Cheap Yellow Display (ESP32-2432S028): downloads, likes, prints, boosts, collecties en comments. Bij een wijziging stuurt de CYD zelf een Telegram, met de titel van het model. Comments is het totaal van **al je gepubliceerde modellen**.

De module praat daarna zelf met MakerWorld. Je browser mag dicht.

**Firmware:** `2026.09.12a`

## Snel starten

**Aanbevolen:** open de [configurator](#configurator) (je gepubliceerde Grok-link, `*.grok.me`).

1. Plak een modellink of je MakerWorld-gebruikers-ID.
2. Vul WiFi in (2,4 GHz), optioneel Telegram en MQTT.
3. Download `MakerPulse_CYD.zip`.
4. Flash met Arduino IDE — zie [firmware/README.md](firmware/README.md).

Niets in de sketch aanpassen: `config.h` is al ingevuld.

## Hardware

- ESP32-2432S028 (CYD 2,8", 240×320)
- USB-kabel die data doorgeeft (geen charge-only)
- 2,4 GHz WiFi

Optioneel: Telegram-bot, MQTT / Home Assistant (scherm aan/uit + helderheid).

## Flashen vanaf GitHub

Zonder configurator:

1. Kopieer [`firmware/config.h.example`](firmware/config.h.example) naar `firmware/config.h`.
2. Vul WiFi-naam, wachtwoord en `MAKERWORLD_UID` in (elke regel heeft uitleg).
3. Open `firmware/MakerPulse_CYD.ino` in Arduino IDE.
4. Bord: **ESP32 Dev Module**, flash 4MB, partition *Default 4MB with spiffs*.
5. Libraries: **LovyanGFX**, **ArduinoJson 7**, **PubSubClient**.
6. Zet de map op een pad zonder spaties, bijv. `C:\MakerPulse_CYD\`.

Uitgebreide stappen: [firmware/README.md](firmware/README.md).

## Configurator

De web-GUI maakt de zip met jouw instellingen. Die GUI is geen GitHub Pages-site (er is een server nodig voor MakerWorld).

| Manier | Wanneer |
| --- | --- |
| Gepubliceerde Grok-link (`*.grok.me`) | Delen met anderen — geen account nodig als toegang “iedereen met de link” is |
| Docker op je eigen server | Zelf hosten, zie [DOCKER.md](DOCKER.md) |
| Deze repo | Firmware (Arduino) + handleiding |

Na een firmware-fix: **opnieuw publiceren** op Grok, anders blijft de oude zip online.

## Zelf hosten (Docker)

Zie [DOCKER.md](DOCKER.md). Geen database, geen login. MQTT voor de backlight zit op de ESP32 zelf.

## Wat zit waar

| Pad | Inhoud |
| --- | --- |
| [`firmware/`](firmware/) | Arduino-sketch, `mp_types.h`, `config.h.example` |
| [`DOCKER.md`](DOCKER.md) | Configurator zelf hosten |
| [`SECURITY.md`](SECURITY.md) | Wat je nooit publiceert |

## Niet uploaden

Zie [SECURITY.md](SECURITY.md). Kort: geen `config.h` met wachtwoorden, geen persoonlijke zip, geen DuckDNS.

## Licentie

[MIT](LICENSE)
