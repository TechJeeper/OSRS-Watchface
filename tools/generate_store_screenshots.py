#!/usr/bin/env python3
"""Render Rebble App Store screenshots — scene-first, overlay text, no panel grid."""
from __future__ import annotations

import os
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "store" / "screenshots"
WX = ROOT / "resources" / "images" / "weather"

PLATFORMS = {
    "basalt": (144, 168, True),
    "diorite": (144, 168, False),
    "flint": (144, 168, False),
    "emery": (200, 228, True),
}

THEMES = {
    "classic": {"text": (255, 255, 0), "text_dim": (255, 204, 0)},
    "dark": {"text": (255, 255, 255), "text_dim": (200, 200, 200)},
    "wilderness": {"text": (255, 255, 0), "text_dim": (255, 170, 0)},
}

SCENES = [
    {
        "file": "01-clear-day.png",
        "theme": "classic",
        "time": "14:32",
        "date": "Mon 2 Nov",
        "wx": "clear",
        "temp": "72°  CLEAR",
        "hours": "14 72°  15 71°  16 69°  17 66°",
        "tmr": "Tmr  78° / 55°",
        "chips": "4.2k   72   87%",
    },
    {
        "file": "02-rain.png",
        "theme": "classic",
        "time": "09:05",
        "date": None,
        "wx": "rain",
        "temp": "54°  RAIN",
        "hours": "09 54°  10 53°  11 52°  12 51°",
        "tmr": "Tmr  58° / 46°",
        "chips": "1.1k   68   64%",
    },
    {
        "file": "03-storm-wild.png",
        "theme": "wilderness",
        "time": "23:17",
        "date": "Sat 31 Oct",
        "wx": "storm",
        "temp": "48°  STORM",
        "hours": "23 48°  00 47°  01 46°  02 45°",
        "tmr": "Tmr  52° / 40°",
        "chips": "8.9k   81   41%",
    },
    {
        "file": "04-snow-dark.png",
        "theme": "dark",
        "time": "18:45",
        "date": "Thu 12 Jun",
        "wx": "snow",
        "temp": "28°  SNOW",
        "hours": "18 28°  19 27°  20 26°  21 25°",
        "tmr": "Tmr  31° / 19°",
        "chips": "6.0k   100%",
    },
    {
        "file": "05-fog.png",
        "theme": "classic",
        "time": "07:00",
        "date": "Wed 5 Mar",
        "wx": "fog",
        "temp": "44°  FOG",
        "hours": "07 44°  08 46°  09 49°  10 52°",
        "tmr": "Tmr  61° / 42°",
        "chips": "312   64   93%",
    },
]


def font(size: int, bold: bool = False) -> ImageFont.ImageFont:
    names = (
        ["C:/Windows/Fonts/consolab.ttf", "C:/Windows/Fonts/arialbd.ttf"]
        if bold
        else ["C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"]
    )
    for path in names:
        if os.path.isfile(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def leco(size: int) -> ImageFont.ImageFont:
    for path in ("C:/Windows/Fonts/consolab.ttf", "C:/Windows/Fonts/lucon.ttf", "C:/Windows/Fonts/arialbd.ttf"):
        if os.path.isfile(path):
            return ImageFont.truetype(path, size)
    return font(size, True)


def shadow_text(draw, text, xy, fnt, fill, shadow=(0, 0, 0)):
    x, y = xy
    draw.text((x + 1, y + 1), text, font=fnt, fill=shadow)
    draw.text((x, y), text, font=fnt, fill=fill)


def center_shadow(draw, text, y, w, fnt, fill):
    bbox = draw.textbbox((0, 0), text, font=fnt)
    tw = bbox[2] - bbox[0]
    shadow_text(draw, text, ((w - tw) // 2, y), fnt, fill)


def to_bw(src: Image.Image) -> Image.Image:
    rgba = src.convert("RGBA")
    out = Image.new("RGBA", rgba.size, (0, 0, 0, 0))
    px, opx = rgba.load(), out.load()
    for y in range(rgba.size[1]):
        for x in range(rgba.size[0]):
            r, g, b, a = px[x, y]
            if a < 32:
                continue
            lum = 0.299 * r + 0.587 * g + 0.114 * b
            opx[x, y] = (255, 255, 255, 255) if lum >= 140 else (0, 0, 0, 255)
    return out


def render(w: int, h: int, color: bool, scene: dict) -> Image.Image:
    pal = THEMES[scene["theme"]]
    if not color:
        pal = {"text": (255, 255, 255), "text_dim": (255, 255, 255)}

    tall = h >= 200
    wx = Image.open(WX / f'scene_{scene["wx"]}.png').convert("RGBA")
    if not color:
        wx = to_bw(wx)
    bg = wx.resize((w, h), Image.Resampling.NEAREST)
    img = Image.new("RGBA", (w, h), (0, 0, 0, 255))
    img.paste(bg, (0, 0))
    draw = ImageDraw.Draw(img)

    # Full-bleed scene — text floats with ink shadows only (no bars/boxes).
    bottom = 48 if tall else 40
    time_size = 42 if tall else 34
    center_shadow(draw, scene["time"], 8 if tall else 4, w, leco(time_size), pal["text"])
    y = 50 if tall else 40
    if scene.get("date"):
        center_shadow(draw, scene["date"], y, w, font(14, True), pal["text_dim"])
        y += 18
    center_shadow(draw, scene["temp"], y, w, font(16, True), pal["text"])

    by = h - bottom + 2
    center_shadow(draw, scene["chips"], by, w, font(12, True), pal["text"])
    by += 15
    center_shadow(draw, scene["hours"], by, w, font(11), pal["text_dim"])
    by += 14
    center_shadow(draw, scene["tmr"], by, w, font(12, True), pal["text"])
    return img.convert("RGB")


def main() -> None:
    if OUT.exists():
        shutil.rmtree(OUT)
    for plat, (w, h, color) in PLATFORMS.items():
        dest = OUT / plat
        dest.mkdir(parents=True, exist_ok=True)
        for scene in SCENES:
            path = dest / scene["file"]
            render(w, h, color, scene).save(path, "PNG", optimize=True)
            print(f"wrote {path.relative_to(ROOT)} ({w}x{h})")
    print("Done:", OUT)


if __name__ == "__main__":
    main()
