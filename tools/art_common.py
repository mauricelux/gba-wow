"""Shared helpers for the placeholder art generators.

Everything here writes indexed BMP files in the format Butano's asset tool expects:
uncompressed, 8 bits per pixel, palette index 0 is transparent.
"""

from pathlib import Path

import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
GRAPHICS = ROOT / 'graphics'
INCLUDE = ROOT / 'include'
PREVIEW = ROOT / 'tools' / 'preview'


def gba_color(r, g, b):
    """Snap an RGB color to the GBA's 15-bit color space (5 bits per channel)."""
    return (r & 0xF8, g & 0xF8, b & 0xF8)


def save_indexed_bmp(path, pixels, palette, pad_to_256=False):
    """Save a 2D array of palette indices as an indexed BMP.

    palette is a list of (r, g, b) tuples. With pad_to_256 the palette is padded with unique
    filler colors, which Butano needs for images using more than 16 colors.
    """
    palette = [gba_color(*c) for c in palette]
    if len(set(palette)) != len(palette):
        raise ValueError(f'{path}: palette has duplicate colors')
    if pad_to_256:
        filler = 0
        while len(palette) < 256:
            color = gba_color(8 + (filler % 31) * 8, 8 + ((filler // 31) % 31) * 8, 248)
            filler += 1
            if color not in palette:
                palette.append(color)
    used = int(pixels.max()) + 1
    if used > len(palette):
        raise ValueError(f'{path}: pixel index {used - 1} outside the palette')
    image = Image.fromarray(pixels.astype(np.uint8), 'P')
    flat = [v for c in palette for v in c]
    image.putpalette(flat, rawmode='RGB')
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path, bits=8)


def save_preview_png(path, pixels, palette, scale=1):
    """Save an RGB preview, with index 0 shown as magenta so transparency is visible."""
    lut = np.array([gba_color(*c) for c in palette] + [(0, 0, 0)] * (256 - len(palette)), dtype=np.uint8)
    rgb = lut[pixels]
    image = Image.fromarray(rgb, 'RGB')
    if scale != 1:
        image = image.resize((image.width * scale, image.height * scale), Image.NEAREST)
    path.parent.mkdir(parents=True, exist_ok=True)
    image.save(path)


def unique_tile_count(pixels):
    """Count unique 8x8 tiles the way Butano reduces them (repeats and flips merged)."""
    h, w = pixels.shape
    seen = set()
    for ty in range(0, h, 8):
        for tx in range(0, w, 8):
            tile = pixels[ty:ty + 8, tx:tx + 8]
            variants = (tile, tile[:, ::-1], tile[::-1, :], tile[::-1, ::-1])
            key = min(v.tobytes() for v in variants)
            seen.add(key)
    return len(seen)


def check_tile_banks(pixels, name):
    """Every 8x8 tile of a 4bpp background must use colors from a single 16-color bank."""
    h, w = pixels.shape
    for ty in range(0, h, 8):
        for tx in range(0, w, 8):
            tile = pixels[ty:ty + 8, tx:tx + 8]
            banks = {int(v) // 16 for v in np.unique(tile) if v % 16 != 0}
            if len(banks) > 1:
                raise ValueError(f'{name}: tile at ({tx}, {ty}) mixes palette banks {sorted(banks)}')
