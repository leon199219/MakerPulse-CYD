# MakerPulse in Docker

De configurator (profiel koppelen, firmware-zip, Telegram-test) draait als container. De CYD zelf heeft deze pagina niet nodig — die praat rechtstreeks met MakerWorld.

## Welk pakket?

| Je hebt | Wat je doet |
| --- | --- |
| `makerpulse-docker.zip` (uit de configurator) | Uitpakken, daarna het blok hieronder |
| Alleen deze GitHub-repo | Bevat de firmware, **niet** de Node-app. Download de docker-zip via de configurator |

Een clone van [MakerPulse-CYD](https://github.com/leon199219/MakerPulse-CYD) bouwt deze image niet: `src/` staat bewust niet op GitHub.

## Snel (in de zip-map)

```bash
docker compose up -d --build
```

Daarna: `http://SERVER-IP:3080`

## In je bestaande compose

`makerpulse` moet een **eigen service** zijn, op hetzelfde niveau als je andere namen (`ha-mcp`, `homeassistant`, …). Niet erin plakken.

Fout (dit geeft `additional properties 'makerpulse' not allowed`):

```yaml
services:
  ha-mcp:
    image: ...
    makerpulse:          # verkeerd: genest
      build: ./makerpulse
```

Goed:

```yaml
services:
  ha-mcp:
    image: ...

  makerpulse:
    build: ./makerpulse
    image: makerpulse:local
    container_name: makerpulse
    restart: unless-stopped
    ports:
      - "3080:3000"
    environment:
      PORT: "3000"
      HOST: "0.0.0.0"
      NITRO_HOST: "0.0.0.0"
```

De map `makerpulse` (met Dockerfile) zet je naast de compose-file, bv. `/opt/makerpulse` als je file `/opt/docker-compose.yaml` is.

Start vanuit die map, niet vanuit `/root` na `sudo -i`:

```bash
cd /opt
docker compose up -d --build makerpulse
```

Zet `3080` om als die poort al bezet is. Achter een reverse proxy (nginx, Caddy, Traefik) kun je de poort intern laten en alleen `/` doorzetten.

## Firmware-update

De sketch zit in de image (gebouwd bij `--build`). Een nieuwe `MakerPulse_CYD.zip` van een oude container is dus ook oud.

Na een MakerPulse-update:

1. Download `makerpulse-docker.zip` opnieuw.
2. Vervang de map `makerpulse` (bijv. `/opt/makerpulse`).
3. `docker compose up -d --build makerpulse`

In de .ino moet de eerste regel `MakerPulse CYD 2026.09.12a` (of nieuwer) zijn.

## Wat erin zit

- MakerWorld ophalen (server-side, geen CORS-probleem)
- Telegram chat zoeken en testbericht
- Firmware-zip genereren

Geen database, geen login. MQTT voor de CYD-backlight blijft op de ESP32 zelf — dat gaat niet via deze container.
