# Rebble Appstore listing — Old School RuneScape

Copy these fields into [dev-portal.rebble.io](https://dev-portal.rebble.io/submit) or CloudPebble store metadata.

| Field | Value |
| --- | --- |
| App name | Old School RuneScape |
| Type | Watchface |
| Category | Faces *(automatic)* |
| Website | *(optional)* |
| Source code | *(set after push)* |
| `.pbw` | `build/OSRSWatchface.pbw` |
| CloudPebble zip | `dist/OSRS-Watchface-cloudpebble.zip` |

## Description

Paste into each platform asset collection (max 1600 characters):

```
Welcome to Gielinor — the scene is the watchface.

Full-bleed PixelLab landscapes change with the weather. Yellow OSRS-style time and stats float over the art (no panel grid). Steps, heart, battery, a 4-hour forecast, and tomorrow’s high/low stay glanceable at the edges.

• Full-screen scenes: clear, cloud, rain, snow, storm, fog
• Open-Meteo weather (no API key)
• Overlay time, date, condition, steps/HR/battery, forecast
• Optional classic HP / prayer / run orbs
• Themes: Classic, Dark, Wilderness
• Clay settings for time, weather, and chrome

Color watches get saturated Gielinor art; black & white watches keep the silhouette with optional invert.
```

*(~620 characters)*

## Release notes (v1.0.0)

```
Initial release: Comic Weather–style information density with OSRS PixelLab art, Open-Meteo forecast, Health chips, Clay settings, Classic/Dark/Wilderness themes.
```

## Screenshots

Exact platform sizes (no watch bezel). Regenerate with:

```powershell
python tools/generate_store_screenshots.py
```

| Platform | Size | Folder |
| --- | --- | --- |
| basalt (Pebble Time) | 144×168 | `store/screenshots/basalt/` |
| diorite (Pebble 2) | 144×168 | `store/screenshots/diorite/` |
| flint (Pebble 2 Duo) | 144×168 | `store/screenshots/flint/` |
| emery (Pebble Time 2) | 200×228 | `store/screenshots/emery/` |

Five shots per platform:

1. Clear day  
2. Rain  
3. Storm (Wilderness theme)  
4. Snow (Dark theme)  
5. Fog

## Listing checklist

- [ ] Screenshots uploaded per platform (up to 5)
- [ ] Description above on each asset collection
- [ ] Source URL set once the repo is published
- [ ] Publish `build/OSRSWatchface.pbw` as release 1.0.0
- [ ] Or import `dist/OSRS-Watchface-cloudpebble.zip` into CloudPebble
