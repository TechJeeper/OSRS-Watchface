# Old School RuneScape (Pebble)

An OSRS-themed watchface: full-bleed PixelLab weather scenes with Comic Weather–style glanceable overlays (not a panel grid). Weather from Open-Meteo.

## At a glance

- **Scene** — full-screen clear / cloud / rain / snow / storm / fog
- **Time** — big yellow OSRS-style numerals floating over the art
- **Overlays** — date, condition, steps · heart · battery, 4-hour forecast, tomorrow
- **Optional orbs** — HP≈battery · prayer≈Bluetooth · run≈minute
- **Themes** — Classic · Dark · Wilderness

## Settings (Clay)

Open **Old School RuneScape → Settings** in the Pebble/Rebble app:

- Hour format / leading zero / date
- °C / °F and weather refresh interval
- Toggle chips, forecast, tomorrow, orbs, heart rate
- Theme + B&W invert

## Build

On Windows (Podman or Docker):

```powershell
./build.ps1
```

Output: `build/OSRSWatchface.pbw`

```powershell
./build.ps1 -Phone 192.168.1.23
./build.ps1 -Zip
```

CloudPebble zip: `dist/OSRS-Watchface-cloudpebble.zip`

## Assets

Raw PixelLab downloads live under `docs/raw/`. Rebuild watch resources with:

```powershell
python tools/prepare_assets.py
python tools/generate_store_screenshots.py
```

## Layout

```
src/c/                 face, weather, theme, settings
src/pkjs/              Clay + Open-Meteo
resources/images/      orbs + weather scenes
```
