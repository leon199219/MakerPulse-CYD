# Security

## Never publish

- A `config.h` with a Wi-Fi password, Telegram bot token or MQTT password
- A `MakerPulse_CYD.zip` you already generated with your own credentials
- A DuckDNS host, an internal Home Assistant IP, or a compose file with secrets

Other people make their own zip via the configurator, or copy [`firmware/config.h.example`](firmware/config.h.example) to `config.h`.

`config.h` is listed in `.gitignore`. Before you push, check it is not in the commit.

## Telegram

A bot token is a password. Each person should create their own bot with [@BotFather](https://t.me/BotFather). Do not share a token in issues or screenshots.

## MakerWorld

MakerPulse uses public MakerWorld data only (profile + published models). No MakerWorld login is required.

## MQTT

Mosquitto user/password belong in `config.h` on the device, not in this repo.
