# Changelog

## 2026.09.23a

### Changed

- **Downloads** on the display and in Telegram now use published-model downloads (`MWCount.myDesignDownloadCount`).
- The profile field `downloadCount` is no longer used for that total. It also counts other download activity and was higher than the model-download figure shown on MakerWorld.
- If a profile has no `MWCount` block, the firmware still falls back to `downloadCount`.

Prints were already model prints (`myDesignPrintCount`). Comments remain the sum of comments on all published models.

## 2026.09.12a

- CYD firmware for ESP32-2432S028: six stats, Telegram on change (with model titles), MQTT backlight, HTTP `/light`.
