"""Generate the UI background tiles (graphics/ui_tiles.bmp) and palette (graphics/ui_palette.bmp).

The UI is a 32x32-cell background drawn over everything. Its map is built at runtime by gw::ui,
so the tile order here is part of the code's contract (see include/gw_ui.h):

  0-94      glyphs for ASCII 32-126 over a transparent background (0 = space)
  95-189    the same glyphs over the panel fill (95 = panel fill)
  190-197   panel border: top-left, top, top-right, left, right, bottom-left, bottom, bottom-right
  198       bar left cap, 199-207 bar segment with 0-8 pixels filled, 208 bar right cap
  209       cursor arrow on the panel, 210 cursor arrow over the map
  211       scroll arrow up (panel), 212 scroll arrow down (panel)
  213-215   coin icons on the panel: gold, silver, copper
  216       panel divider line

Every palette bank shares the panel and border colors; banks differ in text color (index 1) and
bar fill (indices 7-8), so color is chosen per cell by its palette bank.
"""

import numpy as np
from PIL import Image

from art_common import GRAPHICS, PREVIEW, ROOT, gba_color, save_indexed_bmp, save_preview_png

FONT = ROOT / 'butano' / 'common' / 'graphics' / 'common_fixed_8x8_font.bmp'

TRANSPARENT, TEXT, SHADOW, PANEL, BORDER_LIGHT, BORDER_DARK, BAR_EMPTY, BAR, BAR_LIGHT, BAR_FRAME = range(10)
GOLD, SILVER, COPPER, WHITE = 10, 11, 12, 13

# (text color, bar color, bar highlight) per bank; the order matches gw::ui::color.
BANKS = [
    ((240, 240, 240), (56, 176, 64), (120, 224, 104)),     # white text, health bar
    ((248, 216, 72), (192, 40, 40), (232, 96, 80)),        # yellow text, rage / enemy health
    ((152, 152, 160), (40, 80, 208), (104, 144, 248)),     # gray text, mana
    ((96, 224, 96), (136, 64, 200), (192, 128, 240)),      # green text, experience
    ((240, 88, 64), (224, 144, 32), (248, 200, 96)),       # red text, cast bar
    ((96, 152, 248), (168, 168, 176), (224, 224, 232)),    # blue text, gray bar
    ((200, 120, 248), (232, 200, 64), (248, 240, 152)),    # purple text, gold bar
]

SHARED = {
    TRANSPARENT: (255, 0, 255),
    SHADOW: (16, 16, 24),
    PANEL: (24, 32, 56),
    BORDER_LIGHT: (192, 168, 104),
    BORDER_DARK: (96, 80, 48),
    BAR_EMPTY: (40, 32, 32),
    BAR_FRAME: (8, 8, 8),
    GOLD: (240, 192, 56),
    SILVER: (192, 192, 208),
    COPPER: (192, 112, 56),
    WHITE: (248, 248, 248),
    14: (64, 80, 120),
    15: (112, 96, 64),
}


def palette():
    colors = []
    for text, bar, bar_light in BANKS:
        bank = [SHARED.get(i, (0, 0, 0)) for i in range(16)]
        bank[TEXT] = text
        bank[BAR] = bar
        bank[BAR_LIGHT] = bar_light
        colors += bank
    return colors


def tile(rows, keys):
    return np.array([[keys[c] for c in row] for row in rows], dtype=np.uint8)


def glyphs():
    font = np.array(Image.open(FONT))
    out = {}
    for code in range(33, 127):
        out[code] = font[(code - 33) * 8:(code - 33) * 8 + 8, :]
    return out


def glyph_tile(glyph, background):
    t = np.full((8, 8), background, dtype=np.uint8)
    if glyph is not None:
        t[glyph == 1] = TEXT
        t[glyph == 2] = SHADOW
    return t


K = {'.': PANEL, 'L': BORDER_LIGHT, 'D': BORDER_DARK, ' ': TRANSPARENT, 'o': SHADOW, '#': TEXT,
     'g': GOLD, 's': SILVER, 'c': COPPER, 'w': WHITE, 'f': BAR_FRAME, 'e': BAR_EMPTY, 'b': BAR,
     'h': BAR_LIGHT}

BORDER = {
    'tl': [' DDDDDDD', 'DLLLLLLL', 'DL......', 'DL......', 'DL......', 'DL......', 'DL......', 'DL......'],
    't': ['DDDDDDDD', 'LLLLLLLL', '........', '........', '........', '........', '........', '........'],
    'l': ['DL......'] * 8,
}


def border_tiles():
    tl = tile(BORDER['tl'], K)
    t = tile(BORDER['t'], K)
    left = tile(BORDER['l'], K)
    tr = tl[:, ::-1]
    right = left[:, ::-1]
    bl = tl[::-1, :]
    b = t[::-1, :]
    br = tl[::-1, ::-1]
    # Bottom and right edges use the dark color on the outside only.
    return [tl, t, tr, left, right, bl, b, br]


def bar_tiles():
    segments = []
    for filled in range(9):
        rows = []
        for y in range(8):
            if y in (0, 7):
                rows.append(' ' * 8)
            elif y in (1, 6):
                rows.append('f' * 8)
            else:
                fill = 'h' if y == 2 else 'b'
                rows.append(fill * filled + 'e' * (8 - filled))
        segments.append(tile(rows, K))
    # The left cap holds no fill so a bar's value maps exactly onto its segments.
    left = tile(['        ', '   fffff', '  feeeee', '  feeeee', '  feeeee', '  feeeee', '   fffff',
                 '        '], K)
    right = tile(['        ', 'ffff    ', 'eeeef   ', 'eeeef   ', 'eeeef   ', 'eeeef   ', 'ffff    ',
                  '        '], K)
    return [left] + segments + [right]


def misc_tiles():
    cursor_rows = ['........', '..o.....', '..#o....', '..##o...', '..###o..', '..##o...', '..#o....',
                   '..o.....']
    cursor = tile(cursor_rows, K)
    cursor_map = tile([r.replace('.', ' ') for r in cursor_rows], K)
    up = tile(['........', '...o....', '..o#o...', '.o###o..', 'o#####o.', 'ooooooo.', '........',
               '........'], K)
    down = up[::-1, :]
    coins = []
    for c in 'gsc':
        coins.append(tile(['........', '..oooo..', '.o' + c * 4 + 'o.', '.o' + c + 'w' + c * 2 + 'o.',
                           '.o' + c * 4 + 'o.', '.o' + c * 4 + 'o.', '..oooo..', '........'], K))
    divider = tile(['........', '........', '........', 'DDDDDDDD', 'LLLLLLLL', '........', '........',
                    '........'], K)
    return [cursor, cursor_map, up, down] + coins + [divider]


def main():
    font = glyphs()
    tiles = [glyph_tile(font.get(code), TRANSPARENT) for code in range(32, 127)]
    tiles += [glyph_tile(font.get(code), PANEL) for code in range(32, 127)]
    tiles += border_tiles()
    tiles += bar_tiles()
    tiles += misc_tiles()
    assert len(tiles) == 217, len(tiles)

    sheet = np.concatenate(tiles, axis=0)
    colors = palette()
    first_bank = colors[:16]
    save_indexed_bmp(GRAPHICS / 'ui_tiles.bmp', sheet, first_bank)
    (GRAPHICS / 'ui_tiles.json').write_text('{\n    "type": "regular_bg_tiles",\n    "bpp_mode": "bpp_4"\n}\n')

    # The palette asset is an image whose palette holds every bank.
    # Banks repeat the shared colors on purpose.
    pal_pixels = np.zeros((8, 8), dtype=np.uint8)
    save_indexed_bmp(GRAPHICS / 'ui_palette.bmp', pal_pixels, colors, pad_to_256=True, allow_duplicates=True)
    (GRAPHICS / 'ui_palette.json').write_text(
        '{\n    "type": "bg_palette",\n    "bpp_mode": "bpp_4",\n    "colors_count": ' + str(len(colors)) + '\n}\n')

    # Preview: all tiles in rows of 32, drawn with bank 0.
    rows = []
    for start in range(0, len(tiles), 32):
        chunk = tiles[start:start + 32]
        chunk += [np.zeros((8, 8), dtype=np.uint8)] * (32 - len(chunk))
        rows.append(np.concatenate(chunk, axis=1))
    save_preview_png(PREVIEW / 'ui_tiles.png', np.concatenate(rows, axis=0), first_bank, scale=4)
    print(f'ui_tiles.bmp: {len(tiles)} tiles, {len(colors) // 16} palette banks')


if __name__ == '__main__':
    main()
