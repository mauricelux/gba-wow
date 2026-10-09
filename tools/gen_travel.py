"""Generate the travel art: the continent pictures of the world map and the flight scene's vehicles.

Outputs:
  graphics/continent_<name>.bmp  128x128 pictures in four 64x64 sprite frames, like the minimaps
  graphics/fx_travel.bmp         16x16 frames: the gryphon (two wing beats), the boat and the tram
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
    ('Redridge Mountains', 'tan', [(46, 84), (60, 84), (68, 88), (62, 94), (46, 92)], (15, 20), 'NONE'),
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


# --- the flight scene's vehicles (16x16, facing left) -----------------------------------------------

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
    if kind == 'boat':
        bob = frame
        img[poly_mask([(1, 10 + bob), (15, 10 + bob), (13, 14 + bob), (3, 14 + bob)], 16)] = V['wood']
        img[rect(2, 10 + bob, 14, 10 + bob)] = V['wood_d']
        img[rect(7, 1 + bob, 7, 10 + bob)] = V['wood_d']                          # mast
        img[poly_mask([(8, 2 + bob), (13, 7 + bob), (8, 9 + bob)], 16)] = V['sail']
        img[poly_mask([(6, 3 + bob), (6, 9 + bob), (2, 9 + bob)], 16)] = V['sail']
        img[rect(7, 0 + bob, 9, 1 + bob)] = V['red']                              # flag
        return outlined(img)
    # The Deeprun Tram: a car on a rail.
    img[rect(0, 13, 15, 13)] = V['metal_d']
    img[rect(1, 5, 14, 11)] = V['metal']
    img[rect(1, 5, 14, 5)] = V['red']
    for x in (3, 7, 11):
        img[rect(x, 7, x + 2, 9)] = V['window']
    img[ellipse(4, 12 + frame * 0, 1.5, 1.5) | ellipse(12, 12, 1.5, 1.5)] = V['metal_d']
    return outlined(img)


def write_vehicles():
    frames = [vehicle('gryphon', 0), vehicle('gryphon', 1), vehicle('boat', 0), vehicle('boat', 1),
              vehicle('tram', 0), vehicle('tram', 1)]
    save_indexed_bmp(GRAPHICS / 'fx_travel.bmp', np.concatenate(frames, axis=0), VEHICLE_PALETTE,
                     allow_duplicates=True)
    (GRAPHICS / 'fx_travel.json').write_text('{\n    "type": "sprite",\n    "height": 16\n}\n')
    lut = np.array(VEHICLE_PALETTE, dtype=np.uint8)
    lut[0] = (40, 72, 136)
    Image.fromarray(lut[np.concatenate(frames, axis=1)], 'RGB').resize((16 * 6 * 6, 16 * 6), Image.NEAREST).save(
        PREVIEW / 'fx_travel.png')


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
