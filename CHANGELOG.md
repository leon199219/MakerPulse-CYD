# Changelog

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
