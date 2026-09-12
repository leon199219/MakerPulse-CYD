# Sharing MakerPulse (GitHub + MakerWorld)

Public repo: https://github.com/leon199219/MakerPulse-CYD

Keep your home Docker and DuckDNS private. Share publicly: source on GitHub, configurator link (`*.grok.me`), and optionally a model on MakerWorld.

## Never upload

- A `config.h` with your Wi-Fi, Telegram token or MQTT password
- A Home Assistant IP, DuckDNS host, or internal compose files

Others generate their own zip via the configurator, or copy `firmware/config.h.example`.

## GitHub

At the top of the README: your Grok configurator (`*.grok.me`), not `http://…duckdns…`.

After a firmware fix, **Publish** again on Grok, otherwise the old zip stays online.

## MakerWorld model (optional)

Only if you ship a printable holder / bezel.

**Description (copy)**

```md
MakerPulse — MakerWorld stats on a Cheap Yellow Display (ESP32-2432S028).

The screen shows downloads, likes, prints, boosts, collections and comments.
When a value changes, the module sends a Telegram message itself.
Home Assistant can control the backlight (on/off + brightness) over MQTT.

## What you need
- ESP32-2432S028 (CYD 2.8")
- USB data cable
- Wi-Fi
- (optional) Telegram bot, MQTT / Home Assistant

## Getting started
1. Open the configurator: <YOUR-GROK.ME-URL>
2. Link your MakerWorld profile, fill in Wi-Fi (and optionally Telegram/MQTT).
3. Download MakerPulse_CYD.zip and flash with Arduino IDE.
4. Put the CYD in this printed holder.

Source and updates: https://github.com/leon199219/MakerPulse-CYD

The module then talks to MakerWorld on its own. Nothing runs in someone else's cloud.
```

**Tags:** ESP32, CYD, MakerWorld, dashboard, Bambu, Home Assistant
