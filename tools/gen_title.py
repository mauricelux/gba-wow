"""Generate the title screen logo: graphics/title_logo.bmp.

The logo is 192x64 pixels, cut into six 64x32 sprite frames (left to right, top to bottom). The
letters come from a small block font defined below, scaled up and shaded like gold.
"""

import numpy as np

from art_common import GRAPHICS, PREVIEW, save_indexed_bmp, save_preview_png

WIDTH = 192
HEIGHT = 64
FRAME_WIDTH = 64
FRAME_HEIGHT = 32

FONT = {
    'W': ['X...X', 'X...X', 'X...X', 'X.X.X', 'X.X.X', 'XX.XX', 'X...X'],
    'O': ['.XXX.', 'X...X', 'X...X', 'X...X', 'X...X', 'X...X', '.XXX.'],
    'R': ['XXXX.', 'X...X', 'X...X', 'XXXX.', 'X.X..', 'X..X.', 'X...X'],
    'L': ['X....', 'X....', 'X....', 'X....', 'X....', 'X....', 'XXXXX'],
    'D': ['XXXX.', 'X...X', 'X...X', 'X...X', 'X...X', 'X...X', 'XXXX.'],
    'F': ['XXXXX', 'X....', 'X....', 'XXXX.', 'X....', 'X....', 'X....'],
    'A': ['.XXX.', 'X...X', 'X...X', 'XXXXX', 'X...X', 'X...X', 'X...X'],
    'C': ['.XXX.', 'X...X', 'X....', 'X....', 'X....', 'X...X', '.XXX.'],
    'T': ['XXXXX', '..X..', '..X..', '..X..', '..X..', '..X..', '..X..'],
}

PALETTE = [
    (255, 0, 255),      # 0 transparent
    (48, 24, 8),        # 1 outline
    (8, 8, 16),         # 2 shadow
    (255, 248, 200),    # 3 gold, lightest
    (255, 232, 136),
    (248, 200, 72),
    (232, 168, 40),
    (200, 128, 24),
    (160, 88, 16),
    (112, 56, 16),      # 9 gold, darkest
    (96, 104, 120),     # 10 steel dark
    (152, 160, 176),    # 11 steel
    (216, 224, 232),    # 12 steel light
]
OUTLINE, SHADOW = 1, 2
GOLD = list(range(3, 10))
STEEL_DARK, STEEL, STEEL_LIGHT = 10, 11, 12


def text_mask(text, scale, spacing):
    """A boolean mask of the text in the block font, every stroke one pixel bolder."""
    glyphs = []
    for ch in text:
        if ch == ' ':
            glyphs.append(np.zeros((7 * scale, 3 * scale), dtype=bool))
            continue
        rows = FONT[ch]
        small = np.array([[c == 'X' for c in row] for row in rows], dtype=bool)
        big = small.repeat(scale, axis=0).repeat(scale, axis=1)
        bold = np.zeros((big.shape[0], big.shape[1] + 1), dtype=bool)
        bold[:, :-1] |= big
        bold[:, 1:] |= big
        glyphs.append(bold)
    height = 7 * scale
    gap = np.zeros((height, spacing), dtype=bool)
    parts = []
    for i, glyph in enumerate(glyphs):
        if i:
            parts.append(gap)
        parts.append(glyph)
    return np.concatenate(parts, axis=1)


def grow(mask):
    out = mask.copy()
    out[1:, :] |= mask[:-1, :]
    out[:-1, :] |= mask[1:, :]
    out[:, 1:] |= mask[:, :-1]
    out[:, :-1] |= mask[:, 1:]
    return out


def stamp_text(img, mask, top):
    """Draws gold text centered horizontally with its top row at top."""
    h, w = mask.shape
    left = (WIDTH - w) // 2
    full = np.zeros((HEIGHT, WIDTH), dtype=bool)
    full[top:top + h, left:left + w] = mask

    shadow = np.zeros_like(full)
    shadow[2:, 2:] = grow(full)[:-2, :-2]
    img[shadow & (img == 0)] = SHADOW
    img[grow(full)] = OUTLINE

    # A vertical gold gradient, with a lighter top edge and a darker bottom edge on every stroke.
    for y in range(top, top + h):
        level = (y - top) * (len(GOLD) - 1) / max(1, h - 1)
        for x in range(left, left + w):
            if not full[y, x]:
                continue
            index = int(round(level))
            if not full[y - 1, x]:
                index -= 2
            elif not full[y + 1, x]:
                index += 1
            img[y, x] = GOLD[max(0, min(len(GOLD) - 1, index))]


def stamp_sword(img, y):
    """A sword lying under the title, point to the right."""
    left, right = 28, WIDTH - 28
    guard = left + 34
    blade = np.zeros((HEIGHT, WIDTH), dtype=bool)
    blade[y - 1:y + 2, guard + 3:right - 4] = True
    for i in range(4):
        blade[y - 1 + (i + 1) // 2:y + 2 - (i + 1) // 2, right - 4 + i] = True
    grip = np.zeros_like(blade)
    grip[y - 1:y + 2, left + 6:guard - 2] = True
    pommel = np.zeros_like(blade)
    pommel[y - 2:y + 3, left + 2:left + 7] = True
    cross = np.zeros_like(blade)
    cross[y - 6:y + 7, guard - 2:guard + 3] = True

    every = blade | grip | pommel | cross
    img[grow(every)] = OUTLINE
    img[blade] = STEEL
    img[y - 1, guard + 3:right - 4] = STEEL_LIGHT
    img[y + 1, guard + 3:right - 4] = STEEL_DARK
    img[grip] = GOLD[6]
    img[y - 1, left + 6:guard - 2] = GOLD[4]
    for x in range(left + 8, guard - 2, 3):
        img[y - 1:y + 2, x] = GOLD[5]
    img[pommel] = GOLD[2]
    img[cross] = GOLD[3]
    img[y - 6:y + 7, guard - 2] = GOLD[1]
    img[y + 6, guard - 2:guard + 3] = GOLD[5]


def main():
    img = np.zeros((HEIGHT, WIDTH), dtype=np.uint8)
    stamp_sword(img, 52)
    stamp_text(img, text_mask('WORLD OF', 2, 3), 3)
    stamp_text(img, text_mask('WARCRAFT', 3, 3), 21)

    frames = []
    for fy in range(0, HEIGHT, FRAME_HEIGHT):
        for fx in range(0, WIDTH, FRAME_WIDTH):
            frames.append(img[fy:fy + FRAME_HEIGHT, fx:fx + FRAME_WIDTH])
    sheet = np.concatenate(frames, axis=0)
    save_indexed_bmp(GRAPHICS / 'title_logo.bmp', sheet, PALETTE)
    (GRAPHICS / 'title_logo.json').write_text('{\n    "type": "sprite",\n    "height": 32\n}\n')
    save_preview_png(PREVIEW / 'title_logo.png', img, PALETTE, scale=3)
    print('title logo written')


if __name__ == '__main__':
    main()
