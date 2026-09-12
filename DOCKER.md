# MakerPulse in Docker

De configurator (profiel koppelen, firmware-zip, Telegram-test) draait als container op je server. De CYD zelf heeft deze pagina niet nodig — die praat rechtstreeks met MakerWorld.

## Snel

```bash
docker compose up -d --build
```

Daarna: `http://SERVER-IP:3080`

`makerpulse` moet een **eigen service** zijn, niet genest onder een andere service. Zie de voorbeelden in deze file in de bron-repo als je hem in een bestaande compose plakt.

Zet `3080` om als die poort al bezet is.

## Firmware-update

De sketch zit in de image (gebouwd bij `--build`). Een nieuwe zip van een oude container is dus ook oud. Rebuild na een update.

In de .ino moet de eerste regel `MakerPulse CYD 2026.09.12a` (of nieuwer) zijn.

Geen database, geen login. MQTT voor de CYD-backlight blijft op de ESP32 zelf.
