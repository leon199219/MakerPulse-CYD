# Changelog

## 2026.11.07j

### Fixed

- The name on the screen is the MakerWorld username (handle), not the profile display name.
- A username is only saved once the closing quote is in, so it is no longer cut off.

### Removed

- The “beeld naar rechts / naar beneden” fields. The picture lines up without them.
- USB left and USB right stay as the two landscape choices.

## 2026.11.07i

### Fixed

- Landscape is now drawn along the long side of the glass. The previous landscape mode was still 90 degrees off.
- Choices are Liggend and Liggend, ondersteboven. Spiegelen flips the picture without turning it upright.

## 2026.11.07h

### Fixed

- The CYD is landscape. A portrait rotation was squashing the picture off the glass. Boot now forces a landscape rotation (USB left or USB right).
- The earlier 12 px / -16 px shift is no longer applied.

## 2026.11.07g

### Fixed

- The picture sat a little too far left and too far down after the rotation change. It now shifts 12 px right and 16 px up.
- On the setup page, “Beeld naar rechts” and “Beeld naar beneden” adjust that if it is still off. Negative numbers go the other way.

## 2026.11.07f

### Fixed

- Save on the phone no longer waits on MakerWorld, so Opslaan does not sit there doing nothing. The display restarts at once and reads the model itself.
- An empty name becomes your MakerWorld username.

### Changed

- The setup screen on the CYD uses a green header and two numbered steps.

## 2026.11.07e

### Added

- The setup page can rotate the screen (0, 90, 180, 270) and mirror it. The choice is kept on the device.
- Existing displays that never saved a rotation turn 90 degrees and mirror once, so a sideways mirrored panel comes upright.
- The page at the device IP uses a dark layout with a green accent, in the style of Bambu Lab.

## 2026.11.07d

### Fixed

- The setup page no longer fails to compile. The quote in `htmlEsc` was an invalid string.

## 2026.11.07c

### Added

- The on-device setup page accepts a link to one of your models instead of a numeric UID. The display joins your Wi-Fi, reads the maker id from that model, and stores it.
- A profile URL with `/u/…` or an avatar URL still works, and a numeric UID still works.

## 2026.11.07b

### Added

- Flash the sketch on its own. With no Wi-Fi in `config.h`, the CYD opens an access point. The screen shows the network name, the password `makerpulse`, and `http://192.168.4.1`.
- That page sets Wi-Fi, MakerWorld UID, Telegram, MQTT, the check interval and the backlight mode. Saving restarts the display.
- Later changes: open `http://<ip>/setup` (the IP is on the screen). If Wi-Fi fails, tap the screen to open the setup network again.

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
