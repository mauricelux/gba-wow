"""Generate effect sprites: target ring, projectiles, quest markers, area circle and ability icons.

Outputs graphics/fx_*.bmp and include/gw_icons.h (icon frame indices). Like the other generators
this is placeholder art drawn from simple shapes.
"""

import numpy as np

from art_common import GRAPHICS, INCLUDE, PREVIEW, gba_color, save_indexed_bmp, save_preview_png


def neighbors(mask):
    out = np.zeros_like(mask)
    out[1:, :] |= mask[:-1, :]
    out[:-1, :] |= mask[1:, :]
    out[:, 1:] |= mask[:, :-1]
    out[:, :-1] |= mask[:, 1:]
    return out


class Canvas:
    def __init__(self, size):
        self.size = size
        self.img = np.zeros((size, size), dtype=np.uint8)
        ys, xs = np.mgrid[0:size, 0:size]
        self.xs = xs + 0.5
        self.ys = ys + 0.5

    def fill(self, mask, color, outline=None):
        if outline is not None:
            self.img[neighbors(mask) & ~mask] = outline
        self.img[mask] = color

    def ellipse(self, cx, cy, rx, ry):
        return ((self.xs - cx) / rx) ** 2 + ((self.ys - cy) / ry) ** 2 <= 1.0

    def rect(self, x0, y0, x1, y1):
        return (self.xs >= x0) & (self.xs < x1) & (self.ys >= y0) & (self.ys < y1)

    def line(self, x0, y0, x1, y1, width=1.0):
        mask = np.zeros((self.size, self.size), dtype=bool)
        steps = int(max(abs(x1 - x0), abs(y1 - y0)) * 2) + 1
        for i in range(steps + 1):
            x = x0 + (x1 - x0) * i / steps
            y = y0 + (y1 - y0) * i / steps
            mask |= ((self.xs - x) ** 2 + (self.ys - y) ** 2) <= (width / 2) ** 2 + 0.25
        return mask

    def poly(self, points):
        mask = np.zeros((self.size, self.size), dtype=bool)
        n = len(points)
        for i in range(n):
            x0, y0 = points[i]
            x1, y1 = points[(i + 1) % n]
            cond = (y0 > self.ys) != (y1 > self.ys)
            with np.errstate(divide='ignore', invalid='ignore'):
                xint = (x1 - x0) * (self.ys - y0) / (y1 - y0) + x0
            mask ^= cond & (self.xs < xint)
        return mask

    def px(self, x, y, color):
        if 0 <= x < self.size and 0 <= y < self.size:
            self.img[y, x] = color


def write(name, frames, palette, size):
    sheet = np.concatenate(frames, axis=0)
    save_indexed_bmp(GRAPHICS / f'fx_{name}.bmp', sheet, palette, allow_duplicates=True)
    (GRAPHICS / f'fx_{name}.json').write_text('{\n    "type": "sprite",\n    "height": ' + str(size) + '\n}\n')
    row = np.concatenate(frames, axis=1)
    save_preview_png(PREVIEW / f'fx_{name}.png', row, palette, scale=4)


# --- target ring (32x16): drawn under the target's feet ---------------------------------------------

def target_ring():
    c = Canvas(32)
    outer = c.ellipse(16, 8, 13, 5.5)
    inner = c.ellipse(16, 8, 10.5, 3.5)
    ring = outer & ~inner
    c.img[ring] = 2
    c.img[ring & (c.ys > 8)] = 1
    return [c.img[:16, :]]


TARGET_PALETTE = [(255, 0, 255), (176, 32, 32), (248, 96, 72)] + [(0, 0, 0)] * 13


# --- projectiles and hits (16x16) ---------------------------------------------------------------------

P = {'out': 1, 'fire_d': 2, 'fire': 3, 'fire_l': 4, 'white': 5, 'frost_d': 6, 'frost': 7, 'frost_l': 8,
     'arc_d': 9, 'arc': 10, 'arc_l': 11, 'wood': 12, 'steel': 13, 'feather': 14, 'gray': 15}
PROJECTILE_PALETTE = [(255, 0, 255), (32, 16, 24), (200, 48, 24), (240, 136, 32), (248, 216, 72),
                      (248, 248, 232), (40, 80, 184), (96, 160, 240), (192, 232, 248), (96, 32, 144),
                      (176, 88, 224), (240, 176, 248), (136, 88, 48), (184, 184, 200), (200, 48, 48),
                      (152, 152, 160)]


def orb(dark, mid, light, flicker):
    c = Canvas(16)
    r = 4.5 if flicker else 4.0
    c.fill(c.ellipse(8, 8, r + 1.5, r + 1.5), P[dark])
    c.fill(c.ellipse(8, 8, r, r), P[mid])
    c.fill(c.ellipse(7, 7, r * 0.55, r * 0.55), P[light])
    c.px(6, 6, P['white'])
    if flicker:
        for x, y in ((2, 8), (13, 7), (8, 2), (8, 13)):
            c.px(x, y, P[mid])
    return c.img


def shard(flicker):
    c = Canvas(16)
    c.fill(c.poly([(8, 1.5), (12.5, 8), (8, 14.5), (3.5, 8)]), P['frost'], P['frost_d'])
    c.fill(c.poly([(8, 3.5), (10, 8), (8, 9), (6, 8)]), P['frost_l'])
    if flicker:
        c.px(3, 3, P['white'])
        c.px(12, 12, P['frost_l'])
    return c.img


def arrow(direction):
    """direction: 0 = right, 1 = down-right, 2 = down."""
    c = Canvas(16)
    if direction == 0:
        c.fill(c.line(2, 8, 12, 8, 1), P['wood'])
        c.fill(c.poly([(11, 5.5), (15, 8), (11, 10.5)]), P['steel'], P['out'])
        c.fill(c.poly([(1, 6), (4, 8), (1, 10)]), P['feather'])
    elif direction == 1:
        c.fill(c.line(3, 3, 11, 11, 1.2), P['wood'])
        c.fill(c.poly([(10, 13), (14, 14), (13, 10)]), P['steel'], P['out'])
        c.fill(c.poly([(1, 3), (3, 1), (4, 4)]), P['feather'])
    else:
        c.fill(c.line(8, 2, 8, 12, 1), P['wood'])
        c.fill(c.poly([(5.5, 11), (8, 15), (10.5, 11)]), P['steel'], P['out'])
        c.fill(c.poly([(6, 1), (8, 4), (10, 1)]), P['feather'])
    return c.img


def slash(frame):
    c = Canvas(16)
    if frame == 0:
        c.fill(c.line(4, 12, 12, 4, 2), P['white'])
    elif frame == 1:
        c.fill(c.line(3, 13, 13, 3, 3), P['fire_l'])
        c.fill(c.line(4, 12, 12, 4, 1), P['white'])
    else:
        for x, y in ((3, 12), (12, 3), (8, 8), (5, 5), (11, 11)):
            c.px(x, y, P['fire_l'])
    return c.img


def burst(frame, dark, mid, light):
    c = Canvas(16)
    r = 3 + frame * 2
    pts = []
    for i in range(16):
        a = i * np.pi / 8
        rr = r if i % 2 == 0 else r * 0.5
        pts.append((8 + rr * np.cos(a), 8 + rr * np.sin(a)))
    c.fill(c.poly(pts), P[mid], P[dark])
    c.fill(c.ellipse(8, 8, r * 0.35 + 0.5, r * 0.35 + 0.5), P[light])
    return c.img


def projectiles():
    return [orb('fire_d', 'fire', 'fire_l', False), orb('fire_d', 'fire', 'fire_l', True),
            shard(False), shard(True),
            orb('arc_d', 'arc', 'arc_l', False), orb('arc_d', 'arc', 'arc_l', True),
            arrow(0), arrow(1), arrow(2),
            slash(0), slash(1), slash(2),
            burst(0, 'fire_d', 'fire', 'fire_l'), burst(1, 'fire_d', 'fire', 'fire_l'),
            burst(0, 'frost_d', 'frost', 'frost_l'), burst(1, 'frost_d', 'frost', 'frost_l'),
            burst(0, 'arc_d', 'arc', 'arc_l'), burst(1, 'arc_d', 'arc', 'arc_l')]


# --- markers (16x16): quest marks above heads, loot sparkle -----------------------------------------

M = {'out': 1, 'y_d': 2, 'y': 3, 'y_l': 4, 'g_d': 5, 'g': 6, 'g_l': 7, 'white': 8, 'cyan': 9, 'b_d': 10, 'b': 11,
     'b_l': 12}
MARKER_PALETTE = [(255, 0, 255), (24, 16, 8), (184, 120, 16), (248, 208, 40), (248, 240, 152),
                  (88, 88, 96), (152, 152, 160), (208, 208, 216), (248, 248, 248), (152, 232, 248),
                  (32, 80, 184), (72, 144, 240), (160, 208, 255)] + \
                 [(0, 0, 0)] * 3


def mark(question, gray, blue=False):
    c = Canvas(16)
    d, m, l = ('b_d', 'b', 'b_l') if blue else ('g_d', 'g', 'g_l') if gray else ('y_d', 'y', 'y_l')
    if question:
        shape = c.ellipse(8, 5, 4.5, 4) & ~c.ellipse(8, 5.5, 2, 1.8) & ~c.rect(3, 5, 7, 10)
        shape |= c.rect(7, 7, 10, 10)
        dot = c.rect(7, 11.5, 10, 14.5)
    else:
        shape = c.poly([(5.5, 1), (10.5, 1), (9.5, 10), (6.5, 10)])
        dot = c.rect(6.5, 11.5, 9.5, 14.5)
    c.fill(shape | dot, M[m], M['out'])
    c.img[(shape | dot) & (c.xs < 7.5)] = M[l]
    c.img[(shape | dot) & (c.xs > 9)] = M[d]
    return c.img


def sparkle(frame):
    c = Canvas(16)
    size = 5 if frame == 0 else 3
    c.fill(c.line(8, 8 - size, 8, 8 + size, 1) | c.line(8 - size, 8, 8 + size, 8, 1), M['y_l'])
    c.px(8, 8, M['white'])
    if frame == 1:
        for x, y in ((3, 3), (13, 4), (4, 13), (12, 12)):
            c.px(x, y, M['cyan'])
    return c.img


def markers():
    # Frames by use: quest available, quest complete, quest in progress, two sparkles, new ranks at a
    # trainer.
    return [mark(False, False), mark(True, False), mark(True, True), sparkle(0), sparkle(1),
            mark(False, False, True)]


# --- treasure chest (16x16): closed and open ---------------------------------------------------------

CH = {'out': 1, 'wood_d': 2, 'wood': 3, 'wood_l': 4, 'iron_d': 5, 'iron': 6, 'gold_d': 7, 'gold': 8,
      'gold_l': 9, 'inside': 10, 'shadow': 11}
CHEST_PALETTE = [(255, 0, 255), (24, 16, 16), (88, 48, 24), (136, 80, 40), (176, 112, 56), (72, 72, 88),
                 (136, 136, 152), (176, 120, 24), (232, 184, 48), (248, 232, 136), (40, 24, 16),
                 (40, 48, 32)] + [(0, 0, 0)] * 4


def chest(opened):
    c = Canvas(16)
    img = c.img
    img[14:16, 2:14] = CH['shadow']
    # Body: planks with iron bands.
    img[8:15, 2:14] = CH['wood']
    img[8:15, 2] = CH['out']
    img[8:15, 13] = CH['out']
    img[14, 2:14] = CH['out']
    img[10, 3:13] = CH['wood_d']
    img[12, 3:13] = CH['wood_d']
    img[8:14, 4] = CH['iron']
    img[8:14, 11] = CH['iron']
    if opened:
        # The lid stands up behind the body and the inside is dark and empty.
        img[2:8, 2:14] = CH['wood_d']
        img[2, 2:14] = CH['out']
        img[2:8, 2] = CH['out']
        img[2:8, 13] = CH['out']
        img[3:7, 4] = CH['iron_d']
        img[3:7, 11] = CH['iron_d']
        img[7:9, 3:13] = CH['inside']
        img[8, 2:14] = CH['out']
        img[9:11, 7:9] = CH['gold_d']
    else:
        # A rounded lid with a gold lock.
        img[4:8, 3:13] = CH['wood_l']
        img[3, 4:12] = CH['out']
        img[4:8, 2] = CH['out']
        img[4:8, 13] = CH['out']
        img[4, 3] = CH['out']
        img[4, 12] = CH['out']
        img[5, 4:12] = CH['wood']
        img[4:8, 4] = CH['iron']
        img[4:8, 11] = CH['iron']
        img[8, 2:14] = CH['out']
        img[7:11, 7:9] = CH['gold']
        img[7, 7] = CH['gold_l']
        img[10, 7:9] = CH['gold_d']
    return img


# --- world map marks (8x8): the player, quest givers and found chests ---------------------------------

def map_marks():
    frames = []
    for color in ('white', 'y_l'):
        c = Canvas(8)
        c.fill(c.ellipse(4, 4, 2.6, 2.6), M[color], M['out'])
        c.px(3, 3, M['cyan'] if color == 'white' else M['white'])
        frames.append(c.img)
    c = Canvas(8)
    c.img[0:4, 3:5] = M['y']
    c.img[5:7, 3:5] = M['y']
    frames.append(c.img)
    c = Canvas(8)
    c.img[1, 2:6] = M['y']
    c.img[2:4, 5] = M['y']
    c.img[2, 2] = M['y']
    c.img[4, 3:5] = M['y']
    c.img[6, 3:5] = M['y']
    frames.append(c.img)
    for img in frames[2:]:
        mask = img > 0
        img[neighbors(mask) & ~mask] = M['out']
    c = Canvas(8)
    c.img[2:7, 1:7] = M['y_d']
    c.img[2, 1:7] = M['y_l']
    c.img[2:7, 1] = M['out']
    c.img[2:7, 6] = M['out']
    c.img[6, 1:7] = M['out']
    c.img[3:5, 3:5] = M['white']
    frames.append(c.img)
    return frames


# --- area circle (64x64): telegraphed boss attacks and frost nova -------------------------------------

def circle():
    c = Canvas(64)
    outer = c.ellipse(32, 32, 31, 31)
    inner = c.ellipse(32, 32, 28, 28)
    c.img[outer & ~inner] = 2
    c.img[c.ellipse(32, 32, 29.5, 29.5) & ~inner] = 3
    dots = inner & (((c.xs.astype(int) + c.ys.astype(int)) % 4) == 0) & ((c.ys.astype(int) % 2) == 0)
    c.img[dots] = 1
    return [c.img]


CIRCLE_PALETTE = [(255, 0, 255), (152, 32, 24), (200, 40, 32), (248, 120, 88)] + [(0, 0, 0)] * 12

# --- ability and item icons (16x16) ---------------------------------------------------------------

I = {'out': 1, 'white': 2, 'lgray': 3, 'dgray': 4, 'red_d': 5, 'red': 6, 'orange': 7, 'yellow': 8,
     'blue_d': 9, 'blue_l': 10, 'purple_d': 11, 'purple_l': 12, 'green_d': 13, 'green_l': 14,
     'brown': 15}
ICON_PALETTE = [(255, 0, 255), (16, 12, 16), (248, 248, 240), (184, 184, 192), (88, 88, 104),
                (112, 24, 24), (208, 56, 40), (240, 144, 40), (248, 224, 80), (32, 56, 136),
                (120, 184, 248), (72, 32, 112), (200, 136, 240), (40, 96, 40), (128, 208, 96),
                (136, 88, 48)]


def gray_palette(palette):
    out = [palette[0]]
    for r, g, b in palette[1:]:
        v = int(0.3 * r + 0.59 * g + 0.11 * b) // 2 + 24
        out.append((v, v, v))
    return out


def icon_base(background):
    c = Canvas(16)
    c.img[:, :] = I['out']
    c.img[1:15, 1:15] = I[background]
    c.img[1, 1:15] = I['lgray']
    c.img[1:15, 1] = I['lgray']
    return c


def sword(c, color='lgray'):
    c.fill(c.line(4, 11.5, 12, 3.5, 2), I[color], I['out'])
    c.fill(c.line(3, 9, 7, 13, 1.2), I['yellow'])
    c.fill(c.line(2.5, 13.5, 4, 12, 1.5), I['brown'])


def flame(c, outer='orange', inner='yellow'):
    c.fill(c.poly([(8, 2), (12, 8), (11.5, 12), (8, 14), (4.5, 12), (4, 8)]), I[outer], I['red_d'])
    c.fill(c.poly([(8, 6), (10, 10), (8, 12.5), (6, 10)]), I[inner])


def snowflake(c):
    for a in (0, np.pi / 3, 2 * np.pi / 3):
        dx, dy = 5.5 * np.cos(a), 5.5 * np.sin(a)
        c.fill(c.line(8 - dx, 8 - dy, 8 + dx, 8 + dy, 1.2), I['white'])
    c.px(8, 8, I['blue_l'])


def arrow_icon(c, shaft='brown', head='lgray'):
    c.fill(c.line(3.5, 12.5, 11, 5, 1.2), I[shaft])
    c.fill(c.poly([(13.5, 2.5), (12.5, 8), (8, 3.5)]), I[head], I['out'])
    c.fill(c.poly([(2, 11), (5, 14), (2.5, 14)]), I['red'])


def burst_icon(c, outer, inner):
    pts = []
    for i in range(16):
        a = i * np.pi / 8
        r = 6 if i % 2 == 0 else 3
        pts.append((8 + r * np.cos(a), 8 + r * np.sin(a)))
    c.fill(c.poly(pts), I[outer], I['out'])
    c.fill(c.ellipse(8, 8, 2, 2), I[inner])


def drop(c, color='red', light='orange'):
    c.fill(c.poly([(8, 2.5), (11.5, 9), (11, 12), (8, 13.5), (5, 12), (4.5, 9)]), I[color], I['out'])
    c.px(6, 9, I[light])
    c.px(6, 10, I[light])


def shield(c, face='blue_l', rim='lgray'):
    c.fill(c.poly([(3, 3), (13, 3), (13, 8), (8, 14), (3, 8)]), I[rim], I['out'])
    c.fill(c.poly([(5, 5), (11, 5), (11, 8), (8, 11.5), (5, 8)]), I[face])


def icons():
    out = {}

    c = icon_base('red_d'); sword(c); c.fill(c.line(10, 2.5, 13.5, 6, 1), I['yellow']); out['heroic_strike'] = c
    c = icon_base('red_d')
    c.fill(c.poly([(3, 9), (9, 5), (9, 12)]), I['brown'], I['out'])
    for y in (4, 8, 12):
        c.fill(c.line(11, 8, 14, y, 1), I['yellow'])
    out['battle_shout'] = c
    c = icon_base('brown')
    c.fill(c.poly([(3, 10), (9, 10), (9, 4), (12, 8), (9, 12), (9, 14), (3, 14)]), I['lgray'], I['out'])
    for y in (5, 7, 9):
        c.fill(c.line(2, y, 6, y, 1), I['yellow'])
    out['charge'] = c
    c = icon_base('red_d'); sword(c, 'dgray'); drop(c); out['rend'] = c
    c = icon_base('brown')
    c.fill(c.rect(4, 3, 12, 7), I['lgray'], I['out'])
    c.fill(c.rect(7, 7, 9, 13), I['brown'], I['out'])
    for x0, x1 in ((2, 4), (12, 14)):
        c.fill(c.line(x0, 12, x1, 10, 1), I['yellow'])
    out['thunder_clap'] = c
    c = icon_base('red_d')
    c.fill(c.rect(6, 3, 10, 13), I['orange'], I['out'])
    c.fill(c.line(3, 11, 13, 5, 1.2), I['white'])
    out['hamstring'] = c
    c = icon_base('dgray')
    c.fill(c.ellipse(8, 7, 4.5, 4), I['white'], I['out'])
    c.fill(c.rect(6, 10, 10, 13), I['white'], I['out'])
    c.px(6, 7, I['out']); c.px(10, 7, I['out']); c.px(7, 6, I['out']); c.px(9, 6, I['out'])
    out['execute'] = c
    c = icon_base('red_d'); sword(c)
    c.fill(c.line(4, 4, 12, 12, 2), I['lgray'], I['out'])
    out['mortal_strike'] = c
    c = icon_base('red_d'); drop(c, 'red', 'yellow'); c.fill(c.line(4, 4, 12, 4, 1), I['white']); out['bloodthirst'] = c
    c = icon_base('dgray'); shield(c); out['shield_block'] = c

    c = icon_base('red_d'); flame(c); out['fireball'] = c
    c = icon_base('blue_d'); shield(c, 'blue_l', 'white'); snowflake(c); out['frost_armor'] = c
    c = icon_base('blue_d')
    c.fill(c.poly([(8, 2), (12, 8), (8, 14), (4, 8)]), I['blue_l'], I['out'])
    c.fill(c.poly([(8, 4), (10, 8), (8, 9), (6, 8)]), I['white'])
    out['frostbolt'] = c
    c = icon_base('red_d'); burst_icon(c, 'orange', 'yellow'); out['fire_blast'] = c
    c = icon_base('purple_d')
    for dx in (-3, 0, 3):
        c.fill(c.line(4 + dx, 12, 10 + dx, 4, 1), I['purple_l'])
        c.px(10 + dx, 4, I['white'])
    out['arcane_missiles'] = c
    c = icon_base('blue_d'); snowflake(c)
    c.fill(c.ellipse(8, 8, 7, 7) & ~c.ellipse(8, 8, 6, 6), I['blue_l'])
    out['frost_nova'] = c
    c = icon_base('purple_d'); burst_icon(c, 'purple_l', 'white'); out['arcane_explosion'] = c
    c = icon_base('red_d'); flame(c, 'red', 'yellow'); c.fill(c.ellipse(8, 10, 2, 2), I['white']); out['pyroblast'] = c
    c = icon_base('blue_d'); shield(c, 'white', 'blue_l'); out['ice_barrier'] = c
    c = icon_base('purple_d')
    c.fill(c.ellipse(8, 8, 5, 5), I['purple_l'], I['out'])
    c.fill(c.ellipse(8, 8, 2, 2), I['white'])
    out['arcane_intellect'] = c

    c = icon_base('green_d')
    for dx in (-3, 0, 3):
        c.fill(c.line(5 + dx, 3, 9 + dx, 13, 1.2), I['white'])
    out['raptor_strike'] = c
    c = icon_base('green_d')
    pts = [(3 + i, 8 + 3 * np.sin(i * 0.9)) for i in range(11)]
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        c.fill(c.line(x0, y0, x1, y1, 2), I['green_l'])
    c.px(13, 8, I['red'])
    out['serpent_sting'] = c
    c = icon_base('purple_d'); arrow_icon(c, 'purple_l', 'white'); out['arcane_shot'] = c
    c = icon_base('red_d')
    for r, col in ((6, 'red'), (4, 'white'), (2, 'red')):
        c.fill(c.ellipse(8, 8, r, r), I[col])
    out['hunters_mark'] = c
    c = icon_base('blue_d'); arrow_icon(c)
    c.fill(c.ellipse(4, 4, 2, 1.2), I['yellow'])
    out['concussive_shot'] = c
    c = icon_base('brown')
    c.fill(c.poly([(3, 6), (9, 3), (13, 6), (10, 9), (13, 13), (6, 11)]), I['orange'], I['out'])
    c.px(9, 5, I['out'])
    out['aspect_hawk'] = c
    c = icon_base('green_d')
    for dy in (-3, 0, 3):
        c.fill(c.line(3, 8 + dy, 12, 8 + dy * 0.3, 1), I['lgray'])
    out['multi_shot'] = c
    c = icon_base('green_d'); arrow_icon(c); out['aimed_shot'] = c
    c = icon_base('dgray')
    c.fill(c.line(4, 3, 4, 13, 1), I['lgray'])
    c.fill(c.poly([(5, 2), (11, 8), (5, 14), (7, 8)]), I['brown'], I['out'])
    out['auto_shot'] = c

    c = icon_base('dgray'); sword(c); out['attack'] = c
    c = icon_base('dgray')
    c.fill(c.poly([(6, 2), (10, 2), (10, 5), (13, 9), (12, 14), (4, 14), (3, 9), (6, 5)]), I['lgray'], I['out'])
    c.fill(c.poly([(4, 9), (12, 9), (12, 13), (4, 13)]), I['red'])
    out['potion'] = c
    c = icon_base('dgray')
    c.fill(c.ellipse(8, 9, 5.5, 4), I['brown'], I['out'])
    c.fill(c.ellipse(7, 8, 3, 1.5), I['orange'])
    out['food'] = c
    c = icon_base('dgray')
    c.fill(c.poly([(6, 2), (10, 2), (10, 5), (13, 9), (12, 14), (4, 14), (3, 9), (6, 5)]), I['lgray'], I['out'])
    c.fill(c.poly([(4, 9), (12, 9), (12, 13), (4, 13)]), I['blue_l'])
    out['drink'] = c

    subclass_icons(out)
    return out


def fist(c, color='orange'):
    c.fill(c.rect(4, 6, 12, 12), I[color], I['out'])
    for x in (6, 8, 10):
        c.px(x, 6, I['out'])
        c.px(x, 7, I['out'])
    c.fill(c.rect(3, 8, 5, 11), I[color], I['out'])


def axe(c, blade='lgray'):
    c.fill(c.line(4, 13, 11, 4, 1.4), I['brown'], I['out'])
    c.fill(c.poly([(8, 3), (13, 3), (14, 8), (10, 8)]), I[blade], I['out'])


def face(c, skin='orange', mouth='out'):
    c.fill(c.ellipse(8, 8, 5, 5.5), I[skin], I['out'])
    c.px(6, 6, I['out']); c.px(10, 6, I['out'])
    c.fill(c.ellipse(8, 10.5, 2, 1.5), I[mouth])


def trap(c, jaws='lgray'):
    c.fill(c.ellipse(8, 11, 6, 2.5) & ~c.ellipse(8, 11, 4, 1.5), I[jaws], I['out'])
    for x in (4, 6, 8, 10, 12):
        c.fill(c.line(x, 11, x, 8.5, 1), I[jaws])


def ring(c, color, radius=6, width=1.5):
    c.fill(c.ellipse(8, 8, radius, radius) & ~c.ellipse(8, 8, radius - width, radius - width), I[color])


def skull(c, color='white'):
    c.fill(c.ellipse(8, 7, 4.5, 4), I[color], I['out'])
    c.fill(c.rect(6, 10, 10, 13), I[color], I['out'])
    c.fill(c.ellipse(6.5, 7, 1.2, 1.2), I['out'])
    c.fill(c.ellipse(9.5, 7, 1.2, 1.2), I['out'])


def subclass_icons(out):
    # warrior
    c = icon_base('red_d'); shield(c, 'red', 'lgray'); c.fill(c.ellipse(8, 7.5, 1.5, 1.5), I['yellow'])
    out['last_stand'] = c
    c = icon_base('dgray'); fist(c); out['pummel'] = c
    c = icon_base('blue_d'); shield(c)
    for y in (3, 6, 9):
        c.fill(c.line(12, y, 14, y + 1, 1), I['yellow'])
    out['shield_bash'] = c
    c = icon_base('purple_d'); face(c, 'lgray')
    for y in (4, 8, 12):
        c.fill(c.line(13, 8, 14.5, y, 1), I['white'])
    out['intimidating_shout'] = c
    c = icon_base('red_d'); sword(c)
    c.fill(c.poly([(11, 9), (14, 12), (11, 14)]), I['yellow'], I['out'])
    out['overpower'] = c
    c = icon_base('red_d')
    c.fill(c.line(3, 13, 12, 4, 1.5), I['lgray'], I['out'])
    c.fill(c.line(13, 13, 4, 4, 1.5), I['lgray'], I['out'])
    out['retaliation'] = c
    c = icon_base('red_d')
    c.fill(c.ellipse(8, 8, 6, 4) & ~c.ellipse(8, 9, 5, 3), I['white'])
    c.fill(c.ellipse(8, 11, 5, 3) & ~c.ellipse(8, 12, 4, 2.2), I['lgray'])
    out['sweeping_strikes'] = c
    c = icon_base('dgray')
    for k in range(4):
        a = k * np.pi / 2
        x, y = 8 + 5 * np.cos(a), 8 + 5 * np.sin(a)
        c.fill(c.line(8, 8, x, y, 1.5), I['lgray'], I['out'])
    c.fill(c.ellipse(8, 8, 1.5, 1.5), I['yellow'])
    out['whirling_blades'] = c
    c = icon_base('red_d'); axe(c); out['cleave'] = c
    c = icon_base('brown')
    c.fill(c.rect(4, 3, 12, 7), I['dgray'], I['out'])
    c.fill(c.rect(7, 7, 9, 14), I['brown'], I['out'])
    c.fill(c.line(3, 13, 13, 13, 1), I['yellow'])
    out['slam'] = c
    c = icon_base('red_d')
    for r in (6, 4, 2):
        c.fill(c.ellipse(8, 8, r, r) & ~c.ellipse(8.5, 7.5, r - 1, r - 1), I['lgray'])
    out['whirlwind'] = c
    c = icon_base('red_d'); face(c, 'red', 'white'); out['berserker_rage'] = c
    c = icon_base('dgray')
    c.fill(c.poly([(3, 9), (9, 5), (9, 12)]), I['purple_l'], I['out'])
    for y in (4, 8, 12):
        c.fill(c.line(11, 8, 14, y, 1), I['purple_l'])
    out['demoralizing_shout'] = c
    c = icon_base('red_d'); skull(c, 'red'); out['recklessness'] = c
    c = icon_base('dgray'); skull(c); out['death_wish'] = c
    c = icon_base('dgray'); shield(c, 'dgray')
    c.fill(c.line(6, 4, 10, 10, 1), I['out']); c.fill(c.line(10, 10, 8, 12, 1), I['out'])
    out['sunder_armor'] = c
    c = icon_base('blue_d'); shield(c); sword(c); out['revenge'] = c
    c = icon_base('dgray'); sword(c)
    c.fill(c.line(3, 3, 13, 13, 1.4), I['red']); c.fill(c.line(13, 3, 3, 13, 1.4), I['red'])
    out['disarm'] = c
    c = icon_base('dgray')
    c.fill(c.poly([(2, 2), (14, 2), (14, 9), (8, 14.5), (2, 9)]), I['lgray'], I['out'])
    c.fill(c.poly([(4, 4), (12, 4), (12, 8.5), (8, 12), (4, 8.5)]), I['yellow'])
    out['shield_wall'] = c
    c = icon_base('brown'); burst_icon(c, 'yellow', 'white'); out['concussion_blow'] = c
    c = icon_base('red_d'); shield(c, 'orange'); burst_icon(c, 'yellow', 'white'); shield(c, 'orange')
    out['shield_slam'] = c

    # mage
    c = icon_base('purple_d')
    c.fill(c.poly([(6, 2), (10, 2), (10, 5), (13, 9), (12, 14), (4, 14), (3, 9), (6, 5)]), I['lgray'], I['out'])
    c.fill(c.poly([(4, 9), (12, 9), (12, 13), (4, 13)]), I['blue_l'])
    c.px(12, 3, I['white']); c.px(13, 4, I['white'])
    out['conjure_water'] = c
    c = icon_base('purple_d')
    c.fill(c.ellipse(8, 9, 5.5, 4), I['brown'], I['out'])
    c.fill(c.ellipse(7, 8, 3, 1.5), I['orange'])
    c.px(12, 3, I['white']); c.px(13, 4, I['white'])
    out['conjure_food'] = c
    c = icon_base('purple_d')
    c.fill(c.ellipse(5, 9, 2.5, 3), I['purple_l'])
    c.fill(c.ellipse(11, 7, 2.5, 3), I['white'], I['out'])
    c.fill(c.line(5, 9, 11, 7, 1), I['purple_l'])
    out['blink'] = c
    c = icon_base('purple_d'); ring(c, 'purple_l'); c.fill(c.line(4, 12, 12, 4, 1.5), I['red'])
    out['counterspell'] = c
    c = icon_base('green_d')
    c.fill(c.ellipse(8, 9, 5, 3.5), I['white'], I['out'])
    c.fill(c.ellipse(12, 7, 2, 2), I['dgray'], I['out'])
    c.fill(c.rect(5, 12, 6, 14), I['dgray']); c.fill(c.rect(10, 12, 11, 14), I['dgray'])
    out['polymorph'] = c
    c = icon_base('purple_d'); ring(c, 'purple_l', 6, 2); ring(c, 'white', 3.5, 1); out['teleport'] = c
    c = icon_base('orange'); flame(c, 'red', 'yellow'); out['scorch'] = c
    c = icon_base('red_d')
    c.fill(c.rect(6, 2, 10, 12), I['orange'], I['out'])
    c.fill(c.rect(7, 3, 9, 11), I['yellow'])
    c.fill(c.ellipse(8, 12.5, 6, 1.5), I['red'])
    out['flamestrike'] = c
    c = icon_base('red_d'); shield(c, 'orange', 'red'); out['fire_ward'] = c
    c = icon_base('red_d'); shield(c, 'red', 'orange'); c.fill(c.ellipse(8, 8, 1.8, 2.4), I['yellow'])
    out['molten_armor'] = c
    c = icon_base('red_d'); ring(c, 'orange', 6.5, 2); ring(c, 'yellow', 3.5, 1); out['blast_wave'] = c
    c = icon_base('red_d'); flame(c, 'yellow', 'white'); out['combustion'] = c
    c = icon_base('blue_d')
    c.fill(c.poly([(2, 8), (14, 2), (14, 14)]), I['blue_l'], I['out'])
    c.fill(c.poly([(5, 8), (12, 5), (12, 11)]), I['white'])
    out['cone_of_cold'] = c
    c = icon_base('blue_d')
    for x, y in ((4, 3), (9, 2), (12, 6), (6, 7), (10, 10), (3, 11), (7, 13), (12, 13)):
        c.fill(c.line(x, y, x - 1, y + 2, 1), I['white'])
    out['blizzard'] = c
    c = icon_base('blue_d')
    c.fill(c.poly([(13.5, 2.5), (9, 10), (6, 7)]), I['white'], I['out'])
    c.fill(c.line(3, 13, 8, 8, 1.4), I['blue_l'])
    out['ice_lance'] = c
    c = icon_base('lgray'); snowflake(c); c.fill(c.ellipse(8, 8, 1.5, 1.5), I['blue_d']); out['cold_snap'] = c
    c = icon_base('blue_d')
    c.fill(c.rect(3, 3, 13, 13), I['blue_l'], I['out'])
    c.fill(c.rect(4, 4, 7, 7), I['white'])
    out['ice_block'] = c
    c = icon_base('purple_d'); burst_icon(c, 'white', 'purple_l'); ring(c, 'purple_l', 7, 1)
    out['arcane_blast'] = c
    c = icon_base('purple_d'); shield(c, 'purple_l', 'lgray'); out['mana_shield'] = c
    c = icon_base('purple_d'); shield(c, 'white', 'purple_l'); c.fill(c.ellipse(8, 7.5, 1.5, 1.5), I['purple_d'])
    out['mage_armor'] = c
    c = icon_base('purple_d')
    c.fill(c.poly([(4, 2), (12, 2), (8, 8)]), I['purple_l'], I['out'])
    c.fill(c.poly([(4, 14), (12, 14), (8, 8)]), I['purple_l'], I['out'])
    out['slow'] = c
    c = icon_base('purple_d')
    for r in (6, 4):
        c.fill(c.ellipse(8, 8, r, r) & ~c.ellipse(9, 8, r - 1.2, r - 1.2), I['blue_l'])
    c.fill(c.ellipse(8, 8, 1.5, 1.5), I['white'])
    out['evocation'] = c
    c = icon_base('purple_d')
    c.fill(c.ellipse(8, 8, 6, 3.5), I['white'], I['out'])
    c.fill(c.ellipse(8, 8, 2.2, 2.2), I['purple_l'])
    c.px(8, 8, I['out'])
    out['presence_of_mind'] = c
    c = icon_base('purple_d'); burst_icon(c, 'purple_l', 'yellow'); c.fill(c.ellipse(8, 8, 1, 1), I['white'])
    out['arcane_power'] = c

    # hunter
    c = icon_base('green_d'); face(c, 'brown'); c.fill(c.ellipse(8, 9.5, 3, 2.5), I['orange']); c.px(8, 10, I['out'])
    out['aspect_monkey'] = c
    c = icon_base('brown')
    c.fill(c.ellipse(8, 8, 5.5, 4.5), I['yellow'], I['out'])
    for x, y in ((6, 6), (10, 7), (7, 10), (11, 10), (5, 9)):
        c.px(x, y, I['out'])
    out['aspect_cheetah'] = c
    c = icon_base('green_d')
    c.fill(c.poly([(3, 12), (8, 3), (13, 6), (10, 9), (13, 12)]), I['lgray'], I['out'])
    c.fill(c.line(4, 4, 12, 12, 1.2), I['red'])
    out['wing_clip'] = c
    c = icon_base('dgray'); skull(c, 'lgray'); out['feign_death'] = c
    c = icon_base('green_d')
    for dy in (-4, 0, 4):
        c.fill(c.line(3, 8 + dy, 12, 8 + dy, 1), I['white'])
        c.px(13, 8 + dy, I['yellow'])
    out['rapid_fire'] = c
    c = icon_base('green_d')
    for x in (4, 8, 12):
        c.fill(c.line(x, 2, x - 1, 10, 1), I['brown'])
        c.fill(c.poly([(x - 2.5, 10), (x + 0.5, 10), (x - 1, 13)]), I['lgray'])
    out['volley'] = c
    c = icon_base('dgray'); arrow_icon(c)
    for x, y in ((3, 4), (5, 2), (2, 7)):
        c.px(x, y, I['yellow'])
    out['scatter_shot'] = c
    c = icon_base('green_d'); arrow_icon(c, 'yellow', 'white'); ring(c, 'yellow', 7, 1); out['trueshot_aura'] = c
    c = icon_base('brown')
    c.fill(c.poly([(3, 4), (13, 4), (11, 8), (5, 8)]), I['white'], I['out'])
    c.fill(c.poly([(3, 12), (13, 12), (11, 9), (5, 9)]), I['white'], I['out'])
    out['mongoose_bite'] = c
    c = icon_base('dgray'); trap(c); flame(c); trap(c); out['immolation_trap'] = c
    c = icon_base('dgray'); trap(c); c.fill(c.rect(5, 2, 11, 8), I['blue_l'], I['out']); out['freezing_trap'] = c
    c = icon_base('blue_d'); trap(c, 'white'); snowflake(c); out['frost_trap'] = c
    c = icon_base('dgray'); trap(c); burst_icon(c, 'orange', 'yellow'); out['explosive_trap'] = c
    c = icon_base('green_d')
    c.fill(c.line(3, 13, 13, 3, 1.2), I['lgray']); c.fill(c.line(3, 3, 13, 13, 1.2), I['lgray'])
    c.fill(c.ellipse(8, 8, 2.5, 2.5), I['yellow'], I['out'])
    out['deterrence'] = c
    c = icon_base('green_d')
    c.fill(c.poly([(3, 3), (13, 6), (8, 8), (11, 13)]), I['green_l'], I['out'])
    c.px(11, 13, I['white'])
    out['wyvern_sting'] = c
    c = icon_base('green_d')
    for dx in (-3, 0, 3):
        c.fill(c.line(11 + dx, 3, 7 + dx, 13, 1.2), I['white'])
    c.fill(c.ellipse(4, 12, 2, 2), I['yellow'])
    out['counterattack'] = c
    c = icon_base('red_d')
    for dx in (-3, 0, 3):
        c.fill(c.line(5 + dx, 3, 9 + dx, 13, 1.6), I['red'])
    c.fill(c.ellipse(11, 4, 2, 2), I['yellow'])
    out['bestial_wrath'] = c


def main():
    write('target', target_ring(), TARGET_PALETTE, 16)
    write('projectiles', projectiles(), PROJECTILE_PALETTE, 16)
    write('markers', markers(), MARKER_PALETTE, 16)
    write('circle', circle(), CIRCLE_PALETTE, 64)
    write('chest', [chest(False), chest(True)], CHEST_PALETTE, 16)
    write('map_marks', map_marks(), MARKER_PALETTE, 8)
    icon_map = icons()
    write('icons', [c.img for c in icon_map.values()], ICON_PALETTE, 16)

    lines = ['// Generated by tools/gen_effects.py. Do not edit by hand.', '#ifndef GW_ICONS_H',
             '#define GW_ICONS_H', '', '#include "bn_color.h"', '#include "bn_sprite_palette_item.h"', '',
             'namespace gw', '{', '', '// Frames of bn::sprite_items::fx_icons.', 'enum class icon_id : uint8_t',
             '{']
    lines += [f'    {name.upper()},' for name in icon_map]
    lines += ['    COUNT', '};', '']
    gray = gray_palette(ICON_PALETTE)
    colors = ', '.join(f'bn::color({r >> 3}, {g >> 3}, {b >> 3})' for r, g, b in (gba_color(*c) for c in gray))
    lines += ['namespace palettes', '{',
              f'    // fx_icons with every color grayed out, for abilities that can\'t be used yet.',
              f'    constexpr bn::color icons_gray_colors[] = {{ {colors} }};',
              '    constexpr bn::sprite_palette_item icons_gray(icons_gray_colors, bn::bpp_mode::BPP_4);',
              '}', '', '}', '', '#endif', '']
    (INCLUDE / 'gw_icons.h').write_text('\n'.join(lines))
    print(f'effects written, {len(icon_map)} icons')


if __name__ == '__main__':
    main()
