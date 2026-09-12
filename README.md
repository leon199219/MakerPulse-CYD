# MakerPulse

MakerWorld-statistieken op een Cheap Yellow Display ([ESP32-2432S028](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)).

![MakerPulse op het CYD-scherm](docs/cyd.png)

Downloads, likes, prints, boosts, collecties en comments — bij een wijziging stuurt de module zelf een Telegram, met de titel van het model. **Comments is het totaal van al je gepubliceerde modellen**, niet alleen het uitgelichte.

Na het flashen praat de CYD zelf met MakerWorld. Je browser mag dicht.

**Firmware:** [`2026.09.12a`](firmware/MakerPulse_CYD.ino) · **Licentie:** [MIT](LICENSE)

---

## Wat het doet

- Zes entiteiten op het 2,8"-scherm (240×320)
- Telegram bij verandering 
- Per model de titel in het bericht, niet alleen de vastgepinde
- MQTT → Home Assistant-schermverlichting (aan/uit + helderheid 0–255)
- HTTP-endpoint `/light` als je geen broker wilt
- Onvolledige comment-telling wordt niet overgenomen (`gevonden/totaal` onderaan)

Geen MakerWorld-login, geen cloud van iemand anders. Alleen publieke profiel- en modeldata.


Installeer met Arduino IDE
### Rechtstreeks vanaf deze repo

1. Clone of download de repo.
2. Kopieer [`firmware/config.h.example`](firmware/config.h.example) → `firmware/config.h`.
3. Vul WiFi-naam, wachtwoord en `MAKERWORLD_UID` in (elke regel heeft uitleg).
4. Open `firmware/MakerPulse_CYD.ino` in Arduino IDE 2.
5. Bord **ESP32 Dev Module**, flash **4MB**, partition **Default 4MB with spiffs**.
6. Libraries: **LovyanGFX**, **ArduinoJson 7**, **PubSubClient**.
7. Zet de map op een pad zonder spaties, bijv. `C:\MakerPulse_CYD\`.

Uitgebreide stappen, Telegram, MQTT en storingstabel: [firmware/README.md](firmware/README.md).

## Hardware

| | |
| --- | --- |
| Bord | ESP32-2432S028 (CYD 2,8", 240×320) |
| Kabel | USB die data doorgeeft (geen charge-only) |
| Netwerk | 2,4 GHz WiFi (geen 5 GHz) |
| Optioneel | Telegram-bot, MQTT / Home Assistant |

Nieuwere USB-C-CYD: in de configurator **nieuwere CYD (USB-C)** aanzetten, of in `config.h` `CYD_PANEL_ST7789 1`.

## Telegram

1. [@BotFather](https://t.me/BotFather) → `/newbot` → token.
2. Open je bot en tik **Start**.
3. Chat-ID in de configurator of `config.h`.
4. Stuur een testbericht vóór je flasht.

De CYD bewaart de laatste cijfers in flash. Alleen een echte delta triggert een bericht.

## Home Assistant

Na het flashen staat het IP rechtsonder op het scherm (`makerpulse-cyd.local`).

**MQTT (aanbevolen):** Mosquitto in HA, host/user/wachtwoord in MakerPulse, opnieuw flashen. Daarna verschijnt lamp **MakerPulse scherm** onder MQTT-apparaten.

**HTTP:** `POST http://makerpulse-cyd.local/light` met `{"state":"ON","brightness":255}`. Details in [firmware/README.md](firmware/README.md).

## OPTIONEEL ##
## WEB-GUI Configurator zelf hosten via Docker (Incl. docker-compose.yml) ##

De GUI is **geen** GitHub Pages-site: er is een server nodig (MakerWorld + zip).

Deze publieke repo is de **Arduino-firmware**. De Node-app zit in `makerpulse-docker.zip`, te downloaden via de configurator. Uitpakken en:

```bash
docker compose up -d --build
```

Poort 3080, geen database, geen login. Handleiding: [DOCKER.md](DOCKER.md).


## Map

```
firmware/MakerPulse_CYD.ino   sketch (niet splitsen)
firmware/mp_types.h           types voor Arduino-prototypes
firmware/config.h.example     kopieer naar config.h — nooit committen
firmware/README.md            flashen, Telegram, MQTT, storingen
docs/                         schermafbeeldingen
LICENSE                       MIT
SECURITY.md                   wat je nooit publiceert
```

`config.h` staat in [`.gitignore`](.gitignore). `mp_types.h` moet naast de `.ino` blijven staan.

## Niet publiceren

Zie [SECURITY.md](SECURITY.md).

- geen `config.h` met WiFi / Telegram-token / MQTT-wachtwoord
- geen persoonlijke `MakerPulse_CYD.zip`
- geen intern HA-IP of compose met geheimen

## Licentie

[MIT](LICENSE) © 2026 [leon199219](https://github.com/leon199219)
