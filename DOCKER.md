# OPTIONAL — MakerPulse in Docker

The configurator (link a profile, firmware zip, Telegram test) runs as a container. The CYD itself does not need this page — it talks to MakerWorld directly.

## Which package?

| You have | What to do |
| --- | --- |
| `makerpulse-docker.zip` (from the configurator) | Unzip, then follow the block below |
| This GitHub repo only | Contains the firmware, **not** the Node app. Download the Docker zip from the configurator |

A clone of [MakerPulse-CYD](https://github.com/leon199219/MakerPulse-CYD) will not build this image: `src/` is intentionally not on GitHub.

## Quick start (inside the unzipped folder)

```bash
docker compose up -d --build
```

Then: `http://SERVER-IP:3080`

## In your compose

```yaml
services:
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

Put the `makerpulse` folder (with the Dockerfile) next to the compose file, e.g. `/opt/makerpulse` if your file is `/opt/docker-compose.yaml`.

Start from that directory, not from `/root` after `sudo -i`:

```bash
cd /opt
docker compose up -d --build makerpulse
```

Change `3080` if that port is already taken. Behind a reverse proxy (nginx, Caddy, Traefik) you can keep the port internal and forward `/` only.

## Firmware update

The sketch is baked into the image (at `--build`). A new `MakerPulse_CYD.zip` from an old container is therefore also old.

After a MakerPulse update:

1. Download `makerpulse-docker.zip` again.
2. Replace the `makerpulse` folder (e.g. `/opt/makerpulse`).
3. `docker compose up -d --build makerpulse`

The first line of the `.ino` must be `MakerPulse CYD 2026.09.12a` (or newer).

## What is included

- Fetching MakerWorld (server-side, no CORS issue)
- Finding a Telegram chat and sending a test message
- Generating the firmware zip

No database, no login. MQTT for the CYD backlight stays on the ESP32 itself — it does not go through this container.
