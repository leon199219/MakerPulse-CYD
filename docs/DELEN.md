# MakerPulse delen (GitHub + MakerWorld)

Publieke repo: https://github.com/leon199219/MakerPulse-CYD

Jouw thuis-Docker en DuckDNS blijven privé. Publiek deel je: bron op GitHub, configurator-link (`*.grok.me`), en optioneel een model op MakerWorld.

## Wat je nooit uploadt

- `config.h` met jouw WiFi, Telegram-token of MQTT-wachtwoord
- IP van Home Assistant, DuckDNS-host, of interne compose-files

Anderen maken hun eigen zip via de configurator, of kopiëren `firmware/config.h.example`.

## GitHub

In de README bovenaan: je Grok-configurator (`*.grok.me`), niet `http://…duckdns…`.

Na een firmware-fix opnieuw **Publish** op Grok, anders blijft de oude zip online.

## MakerWorld-model (optioneel)

Alleen als je een printbare houder/bezel meelevert.

**Beschrijving (kopiëren)**

```md
MakerPulse — MakerWorld-stats op een Cheap Yellow Display (ESP32-2432S028).

Het scherm toont downloads, likes, prints, boosts, collecties en comments.
Bij een verandering stuurt de module zelf een Telegram-bericht.
Home Assistant kan de backlight (aan/uit + helderheid) via MQTT.

## Wat je nodig hebt
- ESP32-2432S028 (CYD 2,8")
- USB-datakabel
- WiFi
- (optioneel) Telegram-bot, MQTT/Home Assistant

## Aan de slag
1. Open de configurator: <JOUW-GROK.ME-URL>
2. Koppel je MakerWorld-profiel, vul WiFi (en optioneel Telegram/MQTT) in.
3. Download MakerPulse_CYD.zip en flash met Arduino IDE.
4. Zet de CYD in deze geprinte houder.

Bron en updates: https://github.com/leon199219/MakerPulse-CYD

De module praat daarna zelf met MakerWorld. Er draait niets in de cloud van iemand anders.
```

**Tags:** ESP32, CYD, MakerWorld, dashboard, Bambu, Home Assistant
