"""Pack PixelLab sprites into Pebble resources for the OSRS watchface."""
from __future__ import annotations

from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
RAW = ROOT / "docs" / "raw"
OUT_UI = ROOT / "resources" / "images" / "ui"
OUT_WX = ROOT / "resources" / "images" / "weather"
OUT_IMG = ROOT / "resources" / "images"

BLACK_KEY = 18

ORBS = {
    "orb_hp.png": 28,
    "orb_prayer.png": 28,
    "orb_run.png": 28,
}

SCENES = (
    "scene_clear.png",
    "scene_cloud.png",
    "scene_rain.png",
    "scene_snow.png",
    "scene_storm.png",
    "scene_fog.png",
)

SCENE_SIZE = (144, 168)
WHITE_BORDER = 230


def crop_white_border(im: Image.Image, threshold: int = WHITE_BORDER) -> Image.Image:
    """Remove PixelLab letterbox / window frames so scenes are full-bleed.

    PixelLab often leaves a 1px dirty edge outside a thick white matte. Treat
    near-white as empty and crop to the real painted content bbox.
    """
    im = im.convert("RGBA")
    w, h = im.size
    mask = Image.new("L", (w, h), 0)
    src = im.load()
    dst = mask.load()
    for y in range(h):
        for x in range(w):
            r, g, b, a = src[x, y]
            if a < 8:
                continue
            if r >= threshold and g >= threshold and b >= threshold:
                continue
            dst[x, y] = 255

    bbox = mask.getbbox()
    if not bbox:
        return im

    left, top, right, bottom = bbox
    # Trim residual mostly-empty gutters (white columns with a few content pixels).
    px = mask.load()

    def mostly_empty_col(x: int) -> bool:
        painted = sum(1 for y in range(top, bottom) if px[x, y])
        return painted < 0.15 * (bottom - top)

    def mostly_empty_row(y: int) -> bool:
        painted = sum(1 for x in range(left, right) if px[x, y])
        return painted < 0.15 * (right - left)

    while left < right - 8 and mostly_empty_col(left):
        left += 1
    while right > left + 8 and mostly_empty_col(right - 1):
        right -= 1
    while top < bottom - 8 and mostly_empty_row(top):
        top += 1
    while bottom > top + 8 and mostly_empty_row(bottom - 1):
        bottom -= 1

    return im.crop((left, top, right, bottom))


def key_black(im: Image.Image, threshold: int = BLACK_KEY) -> Image.Image:
    im = im.convert("RGBA")
    px = im.load()
    w, h = im.size
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if a == 0:
                continue
            if r <= threshold and g <= threshold and b <= threshold:
                px[x, y] = (0, 0, 0, 0)
    return im


def crop_content(im: Image.Image) -> Image.Image:
    bbox = im.getbbox()
    if not bbox:
        return im
    left, top, right, bottom = bbox
    return im.crop(
        (max(0, left - 1), max(0, top - 1), min(im.size[0], right + 1), min(im.size[1], bottom + 1))
    )


def fit_square(im: Image.Image, size: int) -> Image.Image:
    im = crop_content(im)
    ratio = min(size / im.size[0], size / im.size[1])
    nw = max(1, int(round(im.size[0] * ratio)))
    nh = max(1, int(round(im.size[1] * ratio)))
    scaled = im.resize((nw, nh), Image.Resampling.NEAREST)
    out = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    out.paste(scaled, ((size - nw) // 2, (size - nh) // 2), scaled)
    return out


def main() -> None:
    OUT_UI.mkdir(parents=True, exist_ok=True)
    OUT_WX.mkdir(parents=True, exist_ok=True)

    for name, size in ORBS.items():
        src = RAW / name
        if not src.exists():
            raise SystemExit(f"missing {src}")
        im = fit_square(key_black(Image.open(src)), size)
        dest = OUT_UI / name
        im.save(dest)
        print(f"ui {dest.name} {im.size[0]}x{im.size[1]}")

    for name in SCENES:
        src = RAW / name
        if not src.exists():
            raise SystemExit(f"missing {src}")
        im = crop_white_border(Image.open(src))
        # Stretch cropped art edge-to-edge — no letterbox window frame.
        im = im.resize(SCENE_SIZE, Image.Resampling.NEAREST)
        dest = OUT_WX / name
        im.save(dest)
        print(f"wx {dest.name} {im.size[0]}x{im.size[1]} (from crop)")

    menu_src = RAW / "menu_icon.png"
    im = fit_square(key_black(Image.open(menu_src)), 25)
    im.save(OUT_IMG / "menu_icon.png")
    print("menu menu_icon.png 25x25")
    print("done")


if __name__ == "__main__":
    main()
