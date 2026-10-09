"""Generate the travel art: the continent pictures of the world map and the flight scene's vehicles.

Outputs:
  graphics/continent_<name>.bmp  128x128 pictures in four 64x64 sprite frames, like the minimaps
  graphics/fx_travel.bmp         16x16 frames: the gryphon, two wing beats
  graphics/fx_vehicles.bmp       32x32 frames of the boat and tram scenes: the boat, the tram car, the moon
  graphics/fx_scene.bmp          64x32 strips that scroll past them: the sea and the tram's tunnel
  include/gw_continents.h        the continents, their zones (name, levels, spot on the picture) and zone_id

The pictures are cartoon maps drawn from polygons: every zone of the Alliance route, and the land
between them, so the coasts look like the Eastern Kingdoms and Kalimdor. Zones not in the game yet
are on the map too; the world map page says when they arrive.
"""

import numpy as np
from PIL import Image

from art_common import GRAPHICS, INCLUDE, PREVIEW, save_indexed_bmp

PICTURE = 128
CONTENT = 112
MARGIN = (PICTURE - CONTENT) // 2

COLORS = {
    'clear': (255, 0, 255),
    'sea': (24, 40, 88),
    'sea_light': (40, 72, 136),
    'coast': (16, 16, 24),
    'border': (56, 44, 32),
    'grass': (80, 144, 64),
    'forest': (40, 96, 56),
    'plague': (120, 128, 96),
    'sand': (208, 184, 104),
    'tan': (176, 128, 80),
    'ash': (152, 64, 40),
    'snow': (224, 232, 240),
    'swamp': (88, 112, 72),
    'city': (144, 152, 176),
    'jungle': (32, 120, 72),
    'night': (56, 72, 96),
}
INDEX = {name: index for index, name in enumerate(COLORS)}
PALETTE = list(COLORS.values())

# name, biome, polygon (x, y in the 112x112 content), levels or None for land without a zone entry,
# the map_id it is in the game as (or NONE).
EASTERN_KINGDOMS = [
    ('Tirisfal Glades', 'forest', [(16, 10), (30, 6), (40, 9), (42, 18), (34, 24), (20, 22), (14, 16)],
     (1, 10), 'NONE'),
    ('Western Plaguelands', 'plague', [(40, 9), (56, 8), (60, 18), (56, 28), (42, 28), (34, 24), (42, 18)],
     (55, 60), 'NONE'),
    ('Eastern Plaguelands', 'plague', [(56, 8), (74, 6), (88, 12), (90, 24), (80, 30), (64, 30), (56, 28),
                                       (60, 18)], (55, 60), 'NONE'),
    ('Silverpine Forest', 'forest', [(14, 16), (20, 22), (34, 24), (36, 34), (30, 42), (18, 40), (12, 28)],
     (20, 25), 'NONE'),
    ('Hillsbrad Foothills', 'grass', [(34, 24), (42, 28), (56, 28), (58, 36), (50, 42), (36, 42), (30, 42),
                                      (36, 34)], (30, 35), 'NONE'),
    ('Arathi Highlands', 'tan', [(56, 28), (64, 30), (80, 30), (84, 38), (76, 46), (60, 46), (58, 36)],
     None, 'NONE'),
    ('Wetlands', 'swamp', [(36, 42), (50, 42), (58, 36), (60, 46), (66, 50), (62, 58), (48, 58), (40, 54)],
     (25, 30), 'NONE'),
    ('Ironforge', 'snow', [(30, 58), (48, 58), (50, 66), (42, 70), (30, 68)], (25, 30), 'NONE'),
    ('Loch Modan', 'grass', [(48, 58), (62, 58), (70, 62), (66, 70), (50, 66)], None, 'NONE'),
    ('Searing Gorge', 'ash', [(36, 70), (42, 70), (50, 66), (54, 74), (42, 76)], None, 'NONE'),
    ('Badlands', 'tan', [(50, 66), (66, 70), (72, 76), (62, 80), (54, 74)], (50, 55), 'NONE'),
    ('Burning Steppes', 'ash', [(36, 76), (42, 76), (54, 74), (62, 80), (60, 84), (40, 84)], (50, 55), 'NONE'),
    ('Elwynn Forest', 'grass', [(28, 84), (40, 84), (46, 84), (46, 92), (32, 92), (26, 90)], (1, 10), 'ELWYNN'),
    ('Stormwind City', 'city', [(25, 82), (31, 79), (34, 85), (29, 88)], (1, 60), 'STORMWIND'),
    ('Redridge Mountains', 'tan', [(46, 84), (60, 84), (68, 88), (62, 94), (46, 92)], (15, 20), 'REDRIDGE'),
    ('Westfall', 'sand', [(20, 92), (26, 90), (32, 92), (32, 102), (22, 104), (18, 98)], (10, 15),
     'WESTFALL'),
    ('Duskwood', 'night', [(32, 92), (46, 92), (54, 96), (50, 102), (32, 102)], (20, 25), 'NONE'),
    ('Swamp of Sorrows', 'swamp', [(54, 96), (62, 94), (70, 96), (72, 102), (58, 104), (50, 102)], (50, 55),
     'NONE'),
    ('Stranglethorn Vale', 'jungle', [(22, 104), (32, 102), (50, 102), (44, 108), (30, 110), (24, 108)],
     (35, 40), 'NONE'),
]

KALIMDOR = [
    ('Teldrassil', 'forest', [(22, 6), (32, 4), (36, 10), (28, 14), (22, 12)], None, 'NONE'),
    ('Darkshore', 'forest', [(30, 18), (42, 14), (48, 20), (44, 30), (36, 34), (30, 28)], (10, 20), 'NONE'),
    ('Ashenvale', 'forest', [(36, 34), (44, 30), (48, 20), (62, 22), (66, 34), (56, 40), (42, 40)], (20, 30),
     'NONE'),
    ('Azshara', 'grass', [(62, 22), (76, 20), (84, 28), (74, 34), (66, 34)], None, 'NONE'),
    ('Desolace', 'tan', [(26, 40), (42, 40), (46, 52), (40, 60), (26, 58), (22, 48)], (45, 50), 'NONE'),
    ('The Barrens', 'sand', [(42, 40), (56, 40), (66, 34), (74, 40), (72, 62), (58, 70), (46, 64), (46, 52)],
     None, 'NONE'),
    ('Dustwallow Marsh', 'swamp', [(72, 62), (74, 40), (84, 48), (86, 62), (78, 70)], None, 'NONE'),
    ('Feralas', 'jungle', [(20, 60), (26, 58), (40, 60), (46, 64), (44, 76), (30, 80), (20, 72)], (45, 50),
     'NONE'),
    ('Thousand Needles', 'tan', [(46, 64), (58, 70), (72, 72), (68, 80), (48, 80), (44, 76)], (40, 45),
     'NONE'),
    ("Un'Goro Crater", 'jungle', [(34, 84), (44, 76), (48, 80), (48, 96), (38, 96)], None, 'NONE'),
    ('Silithus', 'sand', [(24, 86), (34, 84), (38, 96), (30, 102), (22, 96)], None, 'NONE'),
    ('Tanaris', 'sand', [(48, 80), (68, 80), (76, 84), (74, 100), (60, 106), (48, 96)], (40, 45), 'NONE'),
]

CONTINENTS = [('eastern', 'Eastern Kingdoms', EASTERN_KINGDOMS), ('kalimdor', 'Kalimdor', KALIMDOR)]


def poly_mask(points, size=CONTENT):
    ys, xs = np.mgrid[0:size, 0:size]
    px, py = xs + 0.5, ys + 0.5
    mask = np.zeros((size, size), dtype=bool)
    n = len(points)
    for i in range(n):
        x0, y0 = points[i]
        x1, y1 = points[(i + 1) % n]
        cond = (y0 > py) != (y1 > py)
        with np.errstate(divide='ignore', invalid='ignore'):
            xint = (x1 - x0) * (py - y0) / (y1 - y0) + x0
        mask ^= cond & (px < xint)
    return mask


def shifted(mask, dx, dy):
    out = np.zeros_like(mask)
    h, w = mask.shape
    out[max(0, dy):h + min(0, dy), max(0, dx):w + min(0, dx)] = \
        mask[max(0, -dy):h + min(0, -dy), max(0, -dx):w + min(0, -dx)]
    return out


def centroid(mask):
    ys, xs = np.nonzero(mask)
    return int(round(xs.mean())), int(round(ys.mean()))


def draw_continent(zones, seed):
    rng = np.random.default_rng(seed)
    owner = np.full((CONTENT, CONTENT), -1, dtype=np.int16)
    for index, (_, _, points, _, _) in enumerate(zones):
        owner[poly_mask(points)] = index
    land = owner >= 0

    img = np.full((CONTENT, CONTENT), INDEX['sea'], dtype=np.uint8)
    waves = rng.random((CONTENT, CONTENT)) < 0.04
    img[waves] = INDEX['sea_light']
    near = np.zeros_like(land)
    for dx in range(-3, 4):
        for dy in range(-3, 4):
            if dx * dx + dy * dy <= 9:
                near |= shifted(land, dx, dy)
    img[near & ~land] = INDEX['sea_light']

    for index, (_, biome, _, _, _) in enumerate(zones):
        img[owner == index] = INDEX[biome]

    # Borders where two zones meet, a dark coast where the land meets the sea.
    border = np.zeros_like(land)
    for dx, dy in ((1, 0), (0, 1)):
        other = np.full_like(owner, -1)
        other[max(0, -dy):CONTENT - max(0, dy), max(0, -dx):CONTENT - max(0, dx)] = \
            owner[max(0, dy):, max(0, dx):][:CONTENT - max(0, dy), :CONTENT - max(0, dx)]
        border |= land & (other >= 0) & (other != owner)
    img[border] = INDEX['border']
    coast = np.zeros_like(land)
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        coast |= shifted(land, dx, dy)
    img[coast & ~land] = INDEX['coast']

    spots = [centroid(owner == index) for index in range(len(zones))]
    picture = np.zeros((PICTURE, PICTURE), dtype=np.uint8)
    picture[MARGIN:MARGIN + CONTENT, MARGIN:MARGIN + CONTENT] = img
    return picture, [(x + MARGIN, y + MARGIN) for x, y in spots]


def save_picture(name, picture):
    frames = [picture[fy:fy + 64, fx:fx + 64] for fy in (0, 64) for fx in (0, 64)]
    save_indexed_bmp(GRAPHICS / f'continent_{name}.bmp', np.concatenate(frames, axis=0), PALETTE)
    (GRAPHICS / f'continent_{name}.json').write_text('{\n    "type": "sprite",\n    "height": 64\n}\n')
    lut = np.array(PALETTE, dtype=np.uint8)
    lut[0] = (40, 40, 40)
    PREVIEW.mkdir(parents=True, exist_ok=True)
    Image.fromarray(lut[picture], 'RGB').resize((PICTURE * 3, PICTURE * 3), Image.NEAREST).save(
        PREVIEW / f'continent_{name}.png')


# --- the flight scene's gryphon (16x16, facing left) -----------------------------------------------

V = {'clear': 0, 'out': 1, 'body_d': 2, 'body': 3, 'body_l': 4, 'white': 5, 'beak': 6, 'wood_d': 7,
     'wood': 8, 'sail': 9, 'metal_d': 10, 'metal': 11, 'red': 12, 'window': 13}
VEHICLE_PALETTE = [(255, 0, 255), (24, 20, 28), (96, 64, 32), (144, 96, 48), (192, 144, 80), (240, 236, 224),
                   (240, 184, 48), (88, 56, 32), (136, 96, 56), (232, 224, 200), (88, 96, 112),
                   (152, 160, 176), (176, 48, 40), (248, 232, 136), (0, 0, 0), (0, 0, 0)]


def outlined(img):
    mask = img > 0
    ring = np.zeros_like(mask)
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        ring |= shifted(mask, dx, dy)
    img[ring & ~mask] = V['out']
    return img


def vehicle(kind, frame):
    img = np.zeros((16, 16), dtype=np.uint8)
    ys, xs = np.mgrid[0:16, 0:16]

    def ellipse(cx, cy, rx, ry):
        return ((xs + 0.5 - cx) / rx) ** 2 + ((ys + 0.5 - cy) / ry) ** 2 <= 1

    def rect(x0, y0, x1, y1):
        return (xs >= x0) & (xs <= x1) & (ys >= y0) & (ys <= y1)

    if kind == 'gryphon':
        img[ellipse(14, 9, 2.5, 1.2)] = V['body_d']                               # tail
        if frame == 0:
            wing = poly_mask([(6, 8), (9, 1), (12, 3), (11, 8)], 16)
        else:
            wing = poly_mask([(6, 8), (11, 8), (12, 14), (9, 15)], 16)
        img[ellipse(9, 9, 4.5, 2.5)] = V['body']                                  # body
        img[ellipse(9, 10, 3.5, 1.2)] = V['body_l']
        img[ellipse(4.5, 7.5, 2.2, 2)] = V['white']                               # head
        img[rect(1, 7, 2, 8)] = V['beak']
        img[7, 4] = V['out']
        img[wing] = V['body_d'] if frame == 0 else V['body_l']
        img[rect(8, 11, 8, 12) | rect(11, 11, 11, 12)] = V['beak']               # claws
        return outlined(img)
    raise ValueError(kind)


def write_vehicles():
    frames = [vehicle('gryphon', 0), vehicle('gryphon', 1)]
    save_indexed_bmp(GRAPHICS / 'fx_travel.bmp', np.concatenate(frames, axis=0), VEHICLE_PALETTE,
                     allow_duplicates=True)
    (GRAPHICS / 'fx_travel.json').write_text('{\n    "type": "sprite",\n    "height": 16\n}\n')
    lut = np.array(VEHICLE_PALETTE, dtype=np.uint8)
    lut[0] = (40, 72, 136)
    Image.fromarray(lut[np.concatenate(frames, axis=1)], 'RGB').resize((16 * 2 * 6, 16 * 6), Image.NEAREST).save(
        PREVIEW / 'fx_travel.png')


# --- the boat and tram scenes ------------------------------------------------------------------------
# A side view on the dark panel: the vehicle (32x32, facing right, zoomed 2x) stays in the middle while
# strips of sea or tunnel (64x32, seamless left to right) scroll past it.

def big_vehicle(kind):
    img = np.zeros((32, 32), dtype=np.uint8)
    ys, xs = np.mgrid[0:32, 0:32]

    def rect(x0, y0, x1, y1):
        return (xs >= x0) & (xs <= x1) & (ys >= y0) & (ys <= y1)

    def disc(cx, cy, r):
        return (xs + 0.5 - cx) ** 2 + (ys + 0.5 - cy) ** 2 <= r * r

    if kind == 'boat':
        # A cog under one square sail, its bow to the right.
        img[poly_mask([(1, 19), (31, 19), (27, 27), (6, 27)], 32)] = V['wood']
        img[rect(1, 19, 30, 20)] = V['wood_d']
        img[rect(5, 23, 27, 23)] = V['wood_d']
        img[rect(2, 15, 8, 18)] = V['wood']                                   # the stern castle
        img[rect(2, 15, 8, 15)] = V['wood_d']
        img[rect(4, 16, 5, 17)] = V['window']
        img[rect(15, 2, 16, 18)] = V['wood_d']                                # mast
        img[poly_mask([(9, 4), (23, 4), (24, 15), (8, 15)], 32)] = V['sail']
        img[rect(9, 9, 23, 9)] = V['white']
        img[rect(14, 6, 17, 7) | rect(14, 12, 17, 13)] = V['red']            # the lion on the sail
        img[rect(17, 1, 21, 3)] = V['red']                                    # pennant
        img[rect(26, 16, 30, 17)] = V['wood_d']                               # bowsprit
        return outlined(img)
    if kind == 'tram':
        # The Deeprun Tram's car: a riveted carriage with lit windows, its rounded nose to the right.
        img[poly_mask([(1, 8), (26, 8), (31, 14), (31, 24), (1, 24)], 32)] = V['metal']
        img[rect(1, 8, 27, 9)] = V['metal_d']
        img[rect(1, 19, 31, 20)] = V['red']
        img[rect(1, 24, 31, 25)] = V['metal_d']
        for wx in (3, 10, 17):
            img[rect(wx, 11, wx + 4, 16)] = V['window']
        img[poly_mask([(25, 11), (28, 11), (30, 15), (30, 16), (25, 16)], 32)] = V['window']
        for wx in (6, 24):
            img[disc(wx, 26.5, 2.6)] = V['metal_d']
            img[disc(wx, 26.5, 1)] = V['metal']
        img[rect(13, 4, 14, 7)] = V['metal_d']                                # the pole to the wire
        return outlined(img)
    # The moon over the sea.
    img[disc(16, 16, 9)] = V['sail']
    img[disc(13, 13, 2) | disc(19, 19, 1.5) | disc(20, 11, 1)] = V['white']
    return outlined(img)


S = {'clear': 0, 'out': 1, 'sea_d': 2, 'sea_m': 3, 'sea_l': 4, 'foam': 5, 'rock_d': 6, 'rock_m': 7,
     'rock_l': 8, 'rail': 9, 'tie': 10, 'lamp': 11, 'glow': 12}
SCENE_PALETTE = [(255, 0, 255), (16, 16, 24), (16, 32, 72), (32, 64, 128), (72, 112, 176), (208, 224, 240),
                 (40, 36, 44), (68, 62, 70), (100, 92, 96), (160, 164, 176), (88, 60, 36), (248, 208, 96),
                 (152, 120, 64)]


def scene_strip(kind):
    img = np.zeros((32, 64), dtype=np.uint8)
    ys, xs = np.mgrid[0:32, 0:64]
    if kind == 'sea_top':
        # Waves with a crest every 32 pixels; above them, the sky shows through.
        crest = np.round(7 + 2.5 * np.sin(xs * 2 * np.pi / 32)).astype(int)
        img[ys >= crest] = S['sea_m']
        img[(ys >= crest + 1) & (ys <= crest + 2)] = S['sea_l']
        img[ys == crest] = S['foam']
        img[(ys > 12) & ((xs * 3 + ys * 7) % 41 == 0)] = S['sea_l']
        img[(ys > 18) & ((xs * 5 + ys * 3) % 37 < 2 + (ys - 18) // 2)] = S['sea_d']
        img[ys >= 28] = S['sea_d']
        img[(ys >= 28) & ((xs * 7 + ys * 5) % 43 < 3)] = S['sea_m']
        return img
    if kind == 'sea_deep':
        img[:] = S['sea_d']
        img[(xs * 7 + ys * 5) % 43 < 3] = S['sea_m']
        img[(xs * 3 + ys * 11) % 97 == 0] = S['sea_l']
        return img
    if kind == 'tunnel_top':
        # The tunnel's vault: big stones, and a lamp hanging every 64 pixels.
        img[:] = S['rock_m']
        img[ys % 8 == 7] = S['rock_d']
        img[(xs + (ys // 8) * 12) % 24 == 0] = S['rock_d']
        img[(ys % 8 == 0) & ((xs + (ys // 8) * 12) % 24 > 2)] = S['rock_l']
        img[ys >= 24] = 0
        img[ys == 24] = S['out']
        img[(ys >= 24) & (ys <= 26) & (xs >= 31) & (xs <= 32)] = S['out']
        img[(ys >= 26) & (ys <= 30) & (xs >= 28) & (xs <= 35)] = S['out']
        img[(ys >= 27) & (ys <= 29) & (xs >= 29) & (xs <= 34)] = S['lamp']
        img[(ys == 31) & (xs >= 27) & (xs <= 36)] = S['glow']
        return img
    # The tunnel's floor: the rail on its ties, then rock.
    img[:] = S['rock_m']
    img[ys % 8 == 7] = S['rock_d']
    img[(xs + (ys // 8) * 16) % 32 == 0] = S['rock_d']
    img[ys < 8] = 0
    img[(ys >= 4) & (ys <= 7) & (xs % 16 < 6)] = S['tie']
    img[ys == 2] = S['rail']
    img[ys == 3] = S['out']
    img[ys == 8] = S['out']
    return img


def write_scenes():
    frames = [big_vehicle('boat'), big_vehicle('tram'), big_vehicle('moon')]
    save_indexed_bmp(GRAPHICS / 'fx_vehicles.bmp', np.concatenate(frames, axis=0), VEHICLE_PALETTE,
                     allow_duplicates=True)
    (GRAPHICS / 'fx_vehicles.json').write_text('{\n    "type": "sprite",\n    "height": 32\n}\n')
    strips = [scene_strip(kind) for kind in ('sea_top', 'sea_deep', 'tunnel_top', 'tunnel_bottom')]
    save_indexed_bmp(GRAPHICS / 'fx_scene.bmp', np.concatenate(strips, axis=0), SCENE_PALETTE + [(0, 0, 0)] * 3,
                     allow_duplicates=True)
    (GRAPHICS / 'fx_scene.json').write_text('{\n    "type": "sprite",\n    "height": 32\n}\n')

    # Previews of both scenes as the game lays them out, on the panel's color.
    panel = (24, 28, 48)
    for name, rows, vehicle_frame in (('boat', [(None, 0), (0, 1), (1, 2), (1, 3)], 0),
                                      ('tram', [(2, 1), (None, 2), (3, 3)], 1)):
        canvas = np.zeros((160, 256, 3), dtype=np.uint8)
        canvas[:] = panel
        lut = np.array(SCENE_PALETTE, dtype=np.uint8)
        for strip, row in rows:
            if strip is None:
                continue
            tile = np.tile(strips[strip], (1, 4))
            y = row * 32 + (16 if name == 'boat' else 8)
            part = canvas[y:y + 32]
            mask = tile[:part.shape[0]] != 0
            part[mask] = lut[tile[:part.shape[0]]][mask]
        big = np.kron(frames[vehicle_frame], np.ones((2, 2), dtype=np.uint8))
        vlut = np.array(VEHICLE_PALETTE, dtype=np.uint8)
        y = 64 if name == 'boat' else 40
        part = canvas[y:y + 64, 96:160]
        mask = big != 0
        part[mask] = vlut[big][mask]
        Image.fromarray(canvas, 'RGB').resize((512, 320), Image.NEAREST).save(PREVIEW / f'scene_{name}.png')


def zone_enum(name):
    return ''.join(c if c.isalnum() else '_' for c in name.upper().replace("'", '')).replace('__', '_')


def main():
    zone_rows = []
    continent_rows = []
    for index, (name, title, zones) in enumerate(CONTINENTS):
        picture, spots = draw_continent(zones, index + 1)
        save_picture(name, picture)
        continent_rows.append(f'    {{ "{title}", bn::sprite_items::continent_{name} }},')
        for (zone, _, _, levels, map_name), (x, y) in zip(zones, spots):
            if levels is None:
                continue
            zone_rows.append((zone, index, x, y, levels, map_name))
    write_vehicles()
    write_scenes()

    out = ['// Generated by tools/gen_travel.py. Do not edit by hand.',
           '// Include only from the world map and the flight scene: it pulls in both continent pictures.',
           '#ifndef GW_CONTINENTS_H', '#define GW_CONTINENTS_H', '']
    out += [f'#include "bn_sprite_items_continent_{name}.h"' for name, _, _ in CONTINENTS]
    out += ['', '#include "gw_ids.h"', '', 'namespace gw', '{', '',
            '// A 128x128 picture of a continent in four 64x64 frames.',
            'struct continent_def', '{', '    const char* name;', '    const bn::sprite_item& item;', '};', '',
            'constexpr continent_def continents[] = {']
    out += continent_rows
    out += ['};', '', 'constexpr int continent_count = sizeof(continents) / sizeof(continents[0]);', '',
            '// The zones of the route, in continent order. map is NONE until the zone is in the game.',
            'enum class zone_id : uint8_t', '{']
    out += [f'    {zone_enum(row[0])},' for row in zone_rows]
    out += ['    COUNT', '};', '', 'struct zone_def', '{', '    const char* name;', '    uint8_t continent;',
            '    uint8_t x;              // its spot on the continent picture', '    uint8_t y;',
            '    uint8_t min_level;', '    uint8_t max_level;', '    map_id map;', '};', '',
            'constexpr zone_def zones[] = {']
    for zone, continent, x, y, (low, high), map_name in zone_rows:
        out.append(f'    {{ "{zone}", {continent}, {x}, {y}, {low}, {high}, map_id::{map_name} }},')
    out += ['};', '', 'static_assert(sizeof(zones) / sizeof(zones[0]) == int(zone_id::COUNT));', '', '}', '',
            '#endif', '']
    (INCLUDE / 'gw_continents.h').write_text('\n'.join(out))
    print(f'{len(CONTINENTS)} continents, {len(zone_rows)} zones')


if __name__ == '__main__':
    main()
