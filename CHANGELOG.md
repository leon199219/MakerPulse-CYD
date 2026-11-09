# Changelog

## Unreleased

### Changed

- README: new cover image and product photos.
- Docs: Telegram and Home Assistant (MQTT) are clearly marked as optional; updated cover image.

### Removed

- Removed the old hero.png and cyd.png images.

## 2026.11.09-ui4

### Changed

- New screen design in MakerWorld style: dark grey cards, MakerWorld green, an icon per statistic, a header with a logo mark and follower pill, and restyled setup, status and "Connected" screens.
- Numbers use a smooth, anti-aliased font (Nunito digits). The change since the last summary is a green badge (red for a drop).
- Footer: Wi-Fi signal bars, MQTT pill, and a line that counts down to the next check. The clock updates every minute.
- Drawing goes through small sprites, so updates no longer flicker. Settings, web server, setup network, MQTT, Telegram and data fetching are unchanged.

### Added

- Serial log at 115200 baud: boot reason, Wi-Fi, and heap/Wi-Fi/MQTT/census at every check. No secrets.
- README: screenshots of the new screen, app-only update (`0x10000`) and power supply notes.

## 2026.11.08a

### Changed

- The statistics screen uses a dark tile layout with a green bar. The six totals, the changes, the follower count and the status line stay the same.

## 2026.11.07

### Added

- Built-in web server. Flash the sketch, then set Wi-Fi, MakerWorld, Telegram, MQTT and the backlight in the browser. Change them later at `http://<ip>/setup` without flashing again.

## 2026.11.06a