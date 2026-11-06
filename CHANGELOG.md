# Changelog

## 2026.11.07a

### Added

- Backlight choice in `config.h`: `BACKLIGHT_MODE 0` (MQTT / Home Assistant) or `1` (automatic).
- Automatic turns the screen off after 10 minutes. A changed statistic or a touch turns it on again for 10 minutes. MQTT mode does not do this.
- In automatic mode the Home Assistant light is not used. Sensors still publish when a broker is set.

## 2026.11.06a

### Added

- Followers sit beside the maker name on the screen (not a seventh row). A gain shows as `+N` until the next Telegram summary.
- MQTT sensor **Volgers** on the same Home Assistant device.
- `NOTIFY_FOLLOWERS` includes that count in the Telegram summary. `0` hides it from Telegram only.

### Changed

- Example images show followers beside the name. They use a generic maker label, not a personal account.

## 2026.10.01b

### Removed

- Docker is no longer an install path. There is no `makerpulse-docker.zip`, `DOCKER.md` or compose service.
- Install and flash the CYD with the Arduino IDE only (repo or the configurator zip).

### Changed

- README lists the MQTT options (`MQTT_HOST`, `MQTT_PORT`, `MQTT_USER`, `MQTT_PASS`) and the entities they create: light **MakerPulse scherm** plus sensors Downloads, Likes, Prints, Boosts, Collecties and Comments.

## 2026.10.01a

### Added

- MQTT discovery for six Home Assistant sensors on the same device as the backlight: Downloads, Likes, Prints, Boosts, Collecties and Comments.
- Values are published to `makerpulse/<mac>/stats` after every successful MakerWorld check, and again when the broker reconnects.
- The light **MakerPulse scherm** is unchanged.

## 2026.09.28a

### Fixed

- **Comments** now add up the Reviews & Ratings count from each model page (`design.commentCount`).
- The published-model list reports a lower `commentCount` than the model page. That list figure is no longer used for the total or for per-model comment changes.
- If a model page cannot be read, that round is discarded and the previous comment total stays.

## 2026.09.23b

### Changed

- Telegram is a **period summary** for every interval, not an instant alert on each check.
- `TELEGRAM_INTERVAL_SEC`: `0` (same as the check interval), `3600` (1 hour), `21600` (6 hours) or `86400` (24 hours).
- At the end of that period one message lists every enabled statistic (total and change over the period) and the models that moved. Nothing is sent when the period was quiet.
- The screen still refreshes on the check interval (2–15 min).

## 2026.09.23a

### Changed

- **Downloads** on the display and in Telegram now use published-model downloads (`MWCount.myDesignDownloadCount`).
- The profile field `downloadCount` is no longer used for that total. It also counts other download activity and was higher than the model-download figure shown on MakerWorld.
- If a profile has no `MWCount` block, the firmware still falls back to `downloadCount`.

Prints were already model prints (`myDesignPrintCount`). Comments remain the sum of comments on all published models.

## 2026.09.12a

- CYD firmware for ESP32-2432S028: six stats, Telegram on change (with model titles), MQTT backlight, HTTP `/light`.
