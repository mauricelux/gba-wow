"""Generate every map in the game (backgrounds, collision, metadata headers).

Run from the tools directory:  python3 gen_world.py

Maps:
  elwynn     2048x2048 outdoor region: Northshire Valley, Goldshire, farms, lake, forests
  abbey      Northshire Abbey interior
  inn        Lion's Pride Inn interior (Goldshire)
  westfall   1024x1024 outdoor region: Sentinel Hill, farms, Moonbrook
  deadmines  dungeon below Moonbrook
  echo_ridge kobold mine in Northshire Valley
  fargodeep  kobold mine south of Goldshire
  stormwind  1024x1024 capital: Valley of Heroes, Trade District, the Keep, Cathedral, Mage Quarter
"""

import numpy as np

import worldgen as wg
from worldgen import Map, Palette

# ---------------------------------------------------------------------------------------------
# Interiors
# ---------------------------------------------------------------------------------------------

INTERIOR = [
    ('outline', (24, 20, 28)), ('floor_d', (96, 64, 40)), ('floor_m', (136, 96, 56)),
    ('floor_l', (168, 124, 76)), ('wall_d', (72, 72, 88)), ('wall_m', (112, 112, 128)),
    ('wall_l', (152, 152, 160)), ('carpet_d', (104, 24, 32)), ('carpet', (160, 40, 48)),
    ('table_d', (72, 44, 28)), ('table_l', (120, 80, 48)), ('flame', (248, 200, 80)),
    ('gold', (224, 176, 64)), ('cloth', (200, 192, 168)), ('stone_floor', (128, 120, 112)),
]

CAVE = [
    ('outline', (16, 12, 16)), ('rock_d', (56, 48, 48)), ('rock_m', (88, 76, 72)),
    ('rock_l', (124, 108, 100)), ('floor_d', (72, 60, 52)), ('floor_m', (100, 84, 72)),
    ('floor_l', (128, 108, 92)), ('beam_d', (80, 52, 32)), ('beam_l', (128, 88, 52)),
    ('rail', (152, 152, 160)), ('water_d', (24, 48, 96)), ('water_m', (40, 80, 136)),
    ('water_l', (96, 144, 192)), ('lamp', (248, 200, 96)), ('red', (152, 40, 40)),
]


def interior_room(m, x, y, w, h, floor='wood'):
    """A room: floor, a tall back wall (top 32 px) and thin side and bottom walls."""
    g = m.ground
    ys, xs = np.mgrid[y:y + h, x:x + w]
    if floor == 'wood':
        pattern = np.full((h, w), m.g('floor_m'), dtype=np.uint8)
        pattern[ys % 8 == 7] = m.g('floor_d')
        pattern[((xs + (ys // 8) * 24) % 32 == 0)] = m.g('floor_d')
        pattern[(ys % 8 == 0)] = m.g('floor_l')
    else:
        pattern = np.full((h, w), m.g('stone_floor'), dtype=np.uint8)
        pattern[(ys % 16 == 15) | (((xs + (ys // 16) * 8) % 16) == 15)] = m.g('wall_d')
        pattern[(ys % 16 == 0) & (((xs + (ys // 16) * 8) % 16) < 12)] = m.g('wall_l')
    g[y:y + h, x:x + w] = pattern
    wg.bricks(m, g, x, y, w, 40, 'wall_d', 'wall_m', 'wall_l')
    g[y + 38:y + 40, x:x + w] = m.g('outline')
    g[y:y + h, x:x + 8] = m.g('wall_d')
    g[y:y + h, x + w - 8:x + w] = m.g('wall_d')
    g[y + h - 8:y + h, x:x + w] = m.g('wall_d')
    g[y:y + h, x + 7] = m.g('outline')
    g[y:y + h, x + w - 8] = m.g('outline')
    g[y + h - 8, x:x + w] = m.g('outline')
    m.block(x, y, w, 40)
    m.block(x, y, 8, h)
    m.block(x + w - 8, y, 8, h)
    m.block(x, y + h - 8, w, 8)


def exit_mat(m, x, y):
    """Doormat in the bottom wall that leads outside. Returns the rectangle to use as a warp."""
    g = m.ground
    g[y:y + 8, x:x + 32] = m.g('carpet_d')
    g[y + 2:y + 6, x + 2:x + 30] = m.g('carpet')
    m.unblock(x, y, 32, 8)
    return (x, y + 4, 32, 4)


def carpet(m, x, y, w, h):
    g = m.ground
    g[y:y + h, x:x + w] = m.g('carpet')
    g[y:y + h, x:x + 2] = m.g('gold')
    g[y:y + h, x + w - 2:x + w] = m.g('gold')


def table(m, x, y, w=32, h=16):
    g = m.ground
    g[y:y + h, x:x + w] = m.g('table_l')
    g[y + h - 4:y + h, x:x + w] = m.g('table_d')
    g[y, x:x + w] = m.g('outline')
    g[y:y + h, x] = m.g('outline')
    g[y:y + h, x + w - 1] = m.g('outline')
    m.block(x, y + 2, w, h - 2)


def candle(m, x, y):
    g = m.ground
    g[y + 4:y + 8, x + 3:x + 5] = m.g('cloth')
    g[y + 1:y + 4, x + 3:x + 5] = m.g('flame')


def pew(m, x, y, w=48):
    g = m.ground
    g[y:y + 8, x:x + w] = m.g('table_d')
    g[y + 1:y + 4, x + 1:x + w - 1] = m.g('table_l')
    m.block(x, y, w, 8)


def gen_abbey():
    m = Map('abbey', 256, 256, Palette([INTERIOR]), Palette([wg.OVERHEAD_LEAVES]))
    interior_room(m, 0, 0, 256, 256, floor='stone')
    carpet(m, 112, 40, 32, 208)
    for py in range(96, 200, 24):
        pew(m, 40, py, 56)
        pew(m, 160, py, 56)
    # Altar with candles at the back.
    table(m, 104, 48, 48, 16)
    for cx in (106, 118, 132, 144):
        candle(m, cx, 44)
    # Banners on the back wall.
    g = m.ground
    for bx in (48, 200):
        g[8:36, bx:bx + 8] = m.g('carpet')
        g[8:10, bx:bx + 8] = m.g('gold')
    exit_rect = exit_mat(m, 112, 248)
    m.warp(*exit_rect, 'elwynn', 'abbey_exit')
    m.point('entry', 128, 236)
    m.point('respawn', 128, 236)
    m.npc('MCBRIDE', 128, 80)
    m.npc('LLANE', 60, 70)
    m.npc('DANIL', 196, 70)
    m.npc('KHELDEN', 56, 222)
    m.npc('THORGAS', 200, 222)
    m.area(0, 0, 256, 256, 'Northshire Abbey')
    m.music = 'TOWN'
    m.save()
    return m


def gen_inn():
    m = Map('inn', 256, 256, Palette([INTERIOR]), Palette([wg.OVERHEAD_LEAVES]))
    interior_room(m, 0, 0, 256, 256, floor='wood')
    g = m.ground
    # Bar counter along the back.
    g[56:72, 24:136] = m.g('table_l')
    g[68:72, 24:136] = m.g('table_d')
    g[56, 24:136] = m.g('outline')
    m.block(24, 58, 112, 14)
    # Barrels behind the bar.
    for bx in (32, 56, 80):
        g[40:56, bx:bx + 16] = m.g('table_d')
        g[42:54, bx + 2:bx + 14] = m.g('table_l')
        g[46:48, bx:bx + 16] = m.g('outline')
    # Fireplace on the back wall.
    g[8:40, 184:232] = m.g('wall_d')
    g[16:40, 192:224] = m.g('outline')
    g[28:40, 196:220] = m.g('flame')
    g[32:40, 200:216] = m.g('carpet')
    m.block(184, 8, 48, 40)
    for tx, ty in ((40, 120), (120, 120), (40, 184), (120, 184), (184, 112), (184, 176)):
        table(m, tx, ty, 32, 16)
        candle(m, tx + 12, ty - 2)
    carpet(m, 112, 208, 32, 40)
    exit_rect = exit_mat(m, 112, 248)
    m.warp(*exit_rect, 'elwynn', 'inn_exit')
    m.point('entry', 128, 236)
    m.point('respawn', 128, 236)
    m.npc('FARLEY', 80, 86)
    m.npc('REMY', 200, 150)
    m.area(0, 0, 256, 256, "Lion's Pride Inn")
    m.music = 'TOWN'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Elwynn Forest
# ---------------------------------------------------------------------------------------------

MIXED = ('oak', 'birch', 'pine', 'oak', 'small', 'birch')
ELWYNN_PROPS = ('bush', 'fern', 'flowers', 'tall_grass', 'flower_bush', 'rock', 'wide_bush', 'fern',
                'log', 'stump', 'big_rock', 'tall_grass', 'bush', 'flowers')
WESTFALL_PROPS = ('tall_grass', 'rock', 'bush', 'tall_grass', 'log', 'stump', 'big_rock', 'fern',
                  'tall_grass', 'rock')

def gen_elwynn():
    m = Map('elwynn', 2048, 2048,
            Palette([wg.TERRAIN_ELWYNN, wg.BUILDINGS, wg.FARM]),
            Palette([wg.OVERHEAD_LEAVES, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    # --- roads -------------------------------------------------------------------------------
    wg.corners_along(m, 'path', [(1024, 320), (1024, 560), (1008, 720), (1024, 900), (1064, 980),
                                (1080, 1040), (1080, 1180)], 1.2)
    wg.corners_along(m, 'path', [(1024, 1240), (1000, 1400), (980, 1560), (976, 1700)], 1.1)
    wg.corners_along(m, 'path', [(960, 1220), (760, 1260), (560, 1330), (340, 1420), (120, 1440),
                                (0, 1440)], 1.1)
    wg.corners_along(m, 'path', [(1090, 1220), (1300, 1240), (1500, 1270), (1650, 1240),
                                (1720, 1230)], 1.0)
    wg.corners_along(m, 'path', [(1024, 470), (1180, 480), (1260, 470)], 0.8)
    wg.corners_along(m, 'path', [(1010, 640), (880, 640), (800, 620)], 0.8)
    wg.corners_along(m, 'path', [(1000, 300), (880, 240), (760, 170)], 0.8)
    wg.corners_along(m, 'path', [(760, 1260), (620, 1150), (440, 1050), (220, 975), (0, 968)], 1.1)
    wg.paint_paths(m)

    # --- water -------------------------------------------------------------------------------
    wg.corners_ellipse(m, 'water', 1400, 1660, 176, 104)
    wg.corners_ellipse(m, 'water', 1520, 1720, 96, 72)
    wg.paint_water(m)

    # --- forest borders and the valley walls -------------------------------------------------
    border = 64
    wg.forest(m, trees, 0, 0, m.width, border)
    wg.forest(m, trees, 0, m.height - border, m.width, border)
    wg.forest(m, trees, 0, 0, border, 928)
    wg.forest(m, trees, 0, 1008, border, 392)
    wg.forest(m, trees, 0, 1480, border, m.height - 1480)
    wg.forest(m, trees, m.width - border, 0, border, m.height)
    # Northshire Valley: x 640..1408, y 64..832, opening south at x 992..1056.
    wg.forest(m, trees, 576, 64, 64, 832)
    wg.forest(m, trees, 1408, 64, 64, 832)
    wg.forest(m, trees, 576, 832, 416, 64)
    wg.forest(m, trees, 1056, 832, 416, 64)

    # --- Northshire Valley ---------------------------------------------------------------------
    door = wg.abbey(m, 944, 160)
    m.chest(7, 972, 186, 3)
    wg.cobbles(m, 992, 288, 64, 32)
    m.warp(1012, 282, 24, 10, 'abbey', 'entry')
    m.point('abbey_exit', 1024, 304)
    m.point('start', door[0], 330)
    m.point('northshire_respawn', 1060, 340)
    m.npc('WILLEM', 1054, 328)
    m.npc('GUARD_NS', 1000, 820)
    m.npc('EAGAN', 1120, 380)
    m.npc('MILLY', 864, 452)

    mine = wg.mine_entrance(m, 680, 96)
    m.warp(mine[0] - 12, mine[1] - 4, 24, 8, 'echo_ridge', 'entry')
    m.point('echo_ridge_exit', mine[0], mine[1] + 24)
    m.area(640, 64, 240, 220, 'Echo Ridge Mine')

    wg.crop_field(m, 672, 400, 160, 112)
    wg.fence(m, 664, 392, 176)
    wg.fence(m, 664, 512, 176)
    m.area(656, 380, 200, 300, 'Northshire Vineyards')
    wg.camp(m, 672, 560, 128, 96, tents=[(680, 568), (752, 568)], fire=(728, 616))
    m.spawn_group('DEFIAS_THUG', 760, 600, 6, 70, seed=2)
    m.spawn_group('DEFIAS_THUG', 860, 520, 3, 40, seed=3)

    m.spawn_group('YOUNG_WOLF', 1250, 430, 6, 130, seed=4)
    m.spawn_group('YOUNG_WOLF', 1220, 650, 4, 100, seed=5)
    wg.grove(m, trees, [(720, 320), (872, 360), (1160, 240), (1300, 280), (1336, 560),
                       (1160, 720), (880, 760), (1288, 740), (680, 720), (1100, 560)], seed=1, kinds=MIXED)
    for (bx, by), kind in zip(((960, 400), (1080, 380), (920, 520), (1120, 600), (1300, 380), (860, 300)),
                              ('bush', 'flower_bush', 'wide_bush', 'bush', 'flower_bush', 'rock')):
        wg.PROPS[kind](m, bx, by)
    m.area(640, 64, 768, 768, 'Northshire Valley')

    # --- Goldshire -------------------------------------------------------------------------------
    wg.cobbles(m, 944, 1168, 160, 72)
    wg.well(m, 1008, 1176)
    inn_door = wg.house(m, 912, 1040, 144, 128, style='stone', roof_colors=('roof_d', 'roof_m', 'roof_l'),
                       roof_ridge='roof_h', roof_outline='outline', door_x=72)
    m.warp(inn_door[0] - 8, 1160, 16, 10, 'inn', 'entry')
    m.point('inn_exit', inn_door[0], 1180)
    wg.house(m, 1104, 1056, 96, 112)
    wg.cobbles(m, 1104, 1168, 96, 32)
    wg.anvil(m, 1128, 1176)
    wg.house(m, 800, 1088, 80, 96, roof_colors=('red_d', 'red_m', 'red_l'))
    wg.house(m, 1224, 1096, 80, 96, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'),
            roof_ridge='thatch_l')
    wg.house(m, 840, 1272, 80, 96, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'),
            roof_ridge='thatch_l')
    m.npc('DUGHAN', 1072, 1216)
    m.npc('LYRIA', 960, 1224)
    m.npc('CORINA', 1176, 1200)
    m.npc('GUARD_GS', 1060, 1000)
    m.npc('ANDREW', 1136, 1222)
    m.point('goldshire_respawn', 1024, 1260)
    wg.grove(m, trees, [(760, 1000), (1300, 1000), (1320, 1300), (720, 1380), (1180, 1340)], seed=3,
             kinds=('oak', 'birch', 'oak', 'small'))
    m.area(780, 980, 560, 420, 'Goldshire')

    # --- Stonefield farm (east) -------------------------------------------------------------------
    wg.house(m, 1680, 1120, 96, 96, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'), roof_ridge='thatch_l')
    m.chest(5, 1752, 1134, 7)
    wg.wheat_field(m, 1520, 1296, 144, 112)
    wg.wheat_field(m, 1736, 1296, 160, 112)
    wg.fence(m, 1512, 1288, 160)
    wg.fence(m, 1728, 1288, 176)
    wg.crop_field(m, 1840, 1120, 96, 80)
    m.npc('MA_STONEFIELD', 1712, 1240)
    m.spawn_group('BOAR', 1600, 1460, 6, 90, seed=6)
    m.spawn_group('BOAR', 1800, 1450, 4, 70, seed=7)
    m.spawn('PRINCESS', 1820, 1520)
    m.area(1480, 1060, 500, 520, 'Stonefield Farm')

    # --- Crystal Lake (murlocs) ---------------------------------------------------------------------
    m.spawn_group('MURLOC', 1250, 1600, 4, 40, seed=8)
    m.spawn_group('MURLOC', 1420, 1800, 4, 50, seed=9)
    m.spawn_group('MURLOC', 1600, 1620, 3, 40, seed=10)
    m.area(1200, 1520, 460, 320, 'Crystal Lake')
    wg.grove(m, trees, [(1624, 1752), (1680, 1744), (1712, 1784)], seed=7, kinds=('birch', 'oak'))
    m.chest(15, 1688, 1840, 9)
    m.chest(16, 1936, 1940, 10)

    # --- Fargodeep Mine (south) ----------------------------------------------------------------------
    mine = wg.mine_entrance(m, 944, 1736)
    m.warp(mine[0] - 12, mine[1] - 4, 24, 8, 'fargodeep', 'entry')
    m.point('fargodeep_exit', mine[0], mine[1] + 24)
    m.area(840, 1560, 300, 260, 'Fargodeep Mine')

    # --- Forest's Edge and Hogger (south-west) ----------------------------------------------------
    wg.forest(m, trees, 64, 1560, 200, 420, kinds=('oak', 'small'), holes=[(112, 1696, 96, 72)],
              secrets=[(200, 1720, 72, 32)])
    m.chest(4, 152, 1736, 10)
    wg.forest(m, trees, 600, 1580, 160, 400, kinds=('oak', 'small'))
    m.spawn_group('RIVERPAW_GNOLL', 420, 1680, 7, 110, seed=12)
    m.spawn('HOGGER', 400, 1860)
    m.area(260, 1560, 340, 420, "Forest's Edge")
    m.npc('GUARD_WEST', 140, 1400)

    # --- woods with timber wolves (west and east) --------------------------------------------------
    # Each wood hides a clearing with a chest, reached by a path under the tree tops.
    wg.forest(m, trees, 160, 160, 320, 360, kinds=('pine',), holes=[(272, 272, 96, 72)],
              secrets=[(304, 344, 32, 184)])
    m.chest(2, 320, 312, 6)
    wg.forest(m, trees, 1600, 160, 320, 320, kinds=('pine',), holes=[(1720, 272, 96, 72)],
              secrets=[(1592, 296, 136, 32)])
    m.chest(3, 1768, 312, 8)
    wg.forest(m, trees, 1600, 600, 200, 280, kinds=('pine', 'birch'), holes=[(1656, 688, 88, 64)],
              secrets=[(1688, 744, 32, 144)])
    m.chest(6, 1700, 728, 9)
    m.spawn_group('TIMBER_WOLF', 420, 760, 6, 110, seed=13)
    m.spawn_group('TIMBER_WOLF', 1650, 960, 5, 120, seed=14)
    m.spawn_group('FOREST_SPIDER', 300, 640, 5, 100, seed=15)
    m.spawn_group('FOREST_SPIDER', 1880, 560, 4, 80, seed=16)
    scatter = []
    rng = np.random.default_rng(42)
    for _ in range(170):
        x, y = int(rng.uniform(80, 1960)), int(rng.uniform(80, 1960))
        if m.area_free(x, y, 40, 56) and not (640 <= x <= 1408 and 64 <= y <= 896) \
                and not (760 <= x <= 1340 and 980 <= y <= 1400):
            scatter.append((x, y))
    # Keep trees off the roads.
    path = wg.corners(m, 'path')
    water = wg.corners(m, 'water')
    kept = []
    for x, y in scatter:
        mx, my = (x + 16) // 16, (y + 40) // 16
        near = path[max(0, my - 3):my + 4, max(0, mx - 3):mx + 4].max() + \
            water[max(0, my - 3):my + 4, max(0, mx - 3):mx + 4].max()
        if near == 0:
            kept.append((x, y))
    # Every eighth tree turns red and gold. Those use the roof bank, so they go in last and only
    # where no green crown shares their tiles.
    autumn = kept[::8]
    wg.grove(m, trees, [p for p in kept if p not in autumn], seed=5, kinds=MIXED)
    wg.grove(m, trees, autumn, seed=0, kinds=('autumn',))
    wg.scatter_props(m, rng, 130, ELWYNN_PROPS, (80, 80, 1888, 1888),
                     avoid=[(760, 980, 580, 420), (1480, 1080, 480, 360)])
    wg.scatter_props(m, rng, 30, ('bush', 'fern', 'flowers', 'flower_bush', 'tall_grass'),
                     (656, 96, 736, 720), avoid=[(904, 128, 240, 240), (656, 380, 200, 300)])

    # The road north-west leads to the gates of Stormwind.
    m.warp(0, 936, 8, 64, 'stormwind', 'from_elwynn')
    m.point('from_stormwind', 24, 968)

    # Road west leads to the Westfall bridge.
    m.warp(0, 1416, 8, 48, 'westfall', 'from_elwynn')
    m.point('from_westfall', 24, 1440)
    m.area(0, 0, 2048, 2048, 'Elwynn Forest')
    m.music = 'ELWYNN'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Westfall
# ---------------------------------------------------------------------------------------------

def gen_westfall():
    m = Map('westfall', 1024, 1024,
            Palette([wg.TERRAIN_WESTFALL, wg.BUILDINGS, wg.FARM]),
            Palette([wg.OVERHEAD_LEAVES, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m, dead=True)

    # River along the east with the bridge from Elwynn.
    wg.corners_along(m, 'water', [(944, 0), (930, 400), (950, 700), (936, 1024)], 2.2)
    wg.paint_water(m)
    wg.corners_along(m, 'path', [(1024, 448), (900, 448), (700, 470), (520, 480), (500, 600),
                                (480, 800), (440, 940)], 1.1)
    wg.corners_along(m, 'path', [(520, 480), (520, 360), (512, 300)], 1.0)
    wg.corners_along(m, 'path', [(700, 470), (760, 640), (780, 760)], 0.9)
    wg.paint_paths(m)
    wg.bridge(m, 896, 424, 96, 48)

    border = 48
    wg.forest(m, trees, 0, 0, m.width, border)
    wg.forest(m, trees, 0, m.height - border, m.width, border)
    wg.forest(m, trees, 0, 0, border, m.height)
    wg.forest(m, trees, m.width - border, 0, border, 424)
    wg.forest(m, trees, m.width - border, 472, border, m.height - 472)
    # Dead woods in the north-west corner with a clearing behind a path under the branches.
    wg.forest(m, trees, 48, 48, 176, 152, holes=[(96, 88, 80, 56)], secrets=[(128, 140, 32, 68)])
    m.chest(8, 136, 128, 11)

    # Sentinel Hill.
    tdoor = wg.tower(m, 480, 168)
    wg.cobbles(m, 448, 304, 128, 48)
    m.npc('GRYAN', 560, 330)
    m.npc('GUARD_WF', 470, 330)
    m.npc('SALMA', 420, 360)
    m.npc('LEWIS', 600, 360)
    m.npc('HEATHER', 540, 392)
    m.point('sentinel_respawn', 512, 370)
    m.area(380, 140, 260, 260, 'Sentinel Hill')

    # Farms with harvest watchers.
    wg.house(m, 160, 320, 96, 96, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'), roof_ridge='thatch_l')
    wg.wheat_field(m, 96, 456, 224, 160)
    wg.crop_field(m, 96, 640, 160, 96)
    m.spawn_group('HARVEST_WATCHER', 200, 540, 8, 110, seed=21)
    m.area(64, 280, 300, 480, 'Saldean\'s Farm')
    wg.house(m, 720, 160, 96, 96, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'), roof_ridge='thatch_l')
    wg.wheat_field(m, 640, 280, 192, 112)
    m.spawn_group('HARVEST_WATCHER', 740, 340, 5, 70, seed=22)
    m.area(620, 120, 260, 300, 'Jangolode Farm')

    # Moonbrook: ruined houses, a defias camp and the mine entrance.
    wg.house(m, 336, 736, 80, 96, style='stone', roof_colors=('dead_0', 'dead_1', 'dead_2'),
            roof_ridge='dead_3', roof_outline='o2')
    m.chest(13, 368, 750, 15)
    wg.house(m, 560, 736, 80, 96, style='stone', roof_colors=('dead_0', 'dead_1', 'dead_2'),
            roof_ridge='dead_3', roof_outline='o2')
    wg.camp(m, 640, 840, 128, 96, tents=[(648, 848), (720, 848)], fire=(700, 900))
    wg.crates(m, 336, 880)
    entrance = wg.mine_entrance(m, 408, 896)
    m.warp(entrance[0] - 12, entrance[1] - 4, 24, 8, 'deadmines', 'entry')
    m.point('deadmines_exit', entrance[0], entrance[1] + 24)
    m.spawn_group('DEFIAS_TRAPPER', 520, 760, 6, 100, seed=23)
    m.spawn_group('DEFIAS_SMUGGLER', 700, 880, 5, 60, seed=24)
    m.area(300, 700, 500, 300, 'Moonbrook', 'MOONBROOK')

    m.spawn_group('GNOLL_BRUTE', 760, 600, 5, 90, seed=25)
    m.spawn_group('GNOLL_BRUTE', 260, 860, 3, 50, seed=26)

    rng = np.random.default_rng(7)
    path = wg.corners(m, 'path')
    for i in range(48):
        x, y = int(rng.uniform(60, 900)), int(rng.uniform(60, 940))
        mx, my = (x + 16) // 16, (y + 40) // 16
        if m.area_free(x, y, 40, 56) and path[max(0, my - 3):my + 4, max(0, mx - 3):mx + 4].max() == 0:
            wg.tree(m, trees, x, y, int(rng.integers(0, 3)), kind=('oak', 'small', 'autumn', 'oak')[i % 4])
    wg.scatter_props(m, rng, 70, WESTFALL_PROPS, (56, 56, 900, 900),
                     avoid=[(380, 140, 260, 260), (300, 700, 500, 300)])

    m.point('from_elwynn', 1000, 448)
    m.npc('FURLBROW', 860, 410)
    m.warp(1016, 424, 8, 48, 'elwynn', 'from_westfall')
    m.area(0, 0, 1024, 1024, 'Westfall')
    m.music = 'WESTFALL'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# The Deadmines
# ---------------------------------------------------------------------------------------------

def cave_floor(m, x, y, w_, h):
    ys, xs = np.mgrid[y:y + h, x:x + w_]
    noise = (xs * 31 + ys * 17 + (xs // 8) * (ys // 8) * 7) % 23
    pattern = np.full((h, w_), m.g('floor_m'), dtype=np.uint8)
    pattern[noise < 3] = m.g('floor_d')
    pattern[noise > 20] = m.g('floor_l')
    m.ground[y:y + h, x:x + w_] = pattern


def gen_deadmines():
    m = Map('deadmines', 1024, 512, Palette([CAVE]), Palette([wg.OVERHEAD_LEAVES]))
    g = m.ground
    # Solid rock everywhere, then carve tunnels and rooms.
    wg.bricks(m, g, 0, 0, 1024, 512, 'rock_d', 'rock_m', 'rock_l')
    m.block(0, 0, 1024, 512)

    def carve(x, y, w_, h):
        cave_floor(m, x, y, w_, h)
        g[y:y + 8, x:x + w_] = m.g('rock_d')
        g[y + 8:y + 10, x:x + w_] = m.g('outline')
        m.unblock(x, y + 16, w_, h - 16)

    rooms = [
        (48, 352, 160, 128),    # entrance hall
        (192, 400, 240, 48),    # tunnel east
        (400, 320, 192, 160),   # mine shaft room
        (560, 360, 160, 48),    # tunnel
        (688, 256, 176, 192),   # goblin foundry
        (720, 120, 48, 160),    # tunnel north
        (560, 32, 432, 144),    # the cove (boss)
        (64, 232, 40, 136),     # a side tunnel off the entrance hall
        (40, 200, 96, 64),      # its dead end
    ]
    for r in rooms:
        carve(*r)
    # Rails through the tunnels.
    for x in range(200, 430, 8):
        g[420:422, x:x + 6] = m.g('rail')
        g[430:432, x:x + 6] = m.g('rail')
        g[418:434, x + 2:x + 4] = m.g('beam_d')
    # Support beams in the rooms.
    for bx, by in ((80, 360), (176, 360), (416, 328), (560, 328), (704, 264), (840, 264)):
        g[by:by + 24, bx:bx + 4] = m.g('beam_l')
        g[by:by + 4, bx - 4:bx + 8] = m.g('beam_d')
    # The cove: water along the top with the ship's deck as the boss arena.
    g[48:80, 560:992] = m.g('water_m')
    g[48:52, 560:992] = m.g('water_l')
    g[96:168, 640:928] = m.g('beam_l')
    for x in range(640, 928, 8):
        g[96:168, x] = m.g('beam_d')
    g[96:100, 640:928] = m.g('outline')
    m.block(560, 40, 432, 48)
    # Lamps
    for lx, ly in ((120, 368), (480, 336), (760, 272), (600, 112), (960, 112)):
        g[ly:ly + 6, lx:lx + 6] = m.g('lamp')
    exit_rect = (96, 472, 48, 8)
    g[472:480, 96:144] = m.g('beam_l')
    m.unblock(96, 464, 48, 16)
    m.warp(*exit_rect, 'westfall', 'deadmines_exit')
    m.point('entry', 120, 452)
    m.point('respawn', 120, 452)
    # Enemies stand alone or in pairs, far enough apart to be pulled one group at a time.
    for x, y in ((90, 396), (172, 410)):
        m.spawn('DEFIAS_MINER', x, y)
    for x, y in ((440, 352), (466, 368), (432, 444), (548, 440)):
        m.spawn('DEFIAS_MINER', x, y)
    for x, y in ((716, 420), (744, 432), (840, 424), (722, 330)):
        m.spawn('GOBLIN_ENGINEER', x, y)
    m.spawn('SNEED', 800, 300)
    for x, y in ((600, 132), (676, 124), (700, 150)):
        m.spawn('DEFIAS_PIRATE', x, y)
    m.spawn('VANCLEEF', 784, 132)
    m.chest(11, 88, 248, 17)
    m.chest(12, 912, 160, 19)
    m.area(0, 0, 1024, 512, 'The Deadmines')
    m.area(560, 32, 432, 144, 'Ironclad Cove', 'IRONCLAD_COVE')
    m.music = 'DUNGEON'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Stormwind City
# ---------------------------------------------------------------------------------------------

# Canals, marble and statues. Water and quay stone share this bank so the canal edges fit in tiles.
CITY = [
    ('w_d', (32, 64, 120)), ('w_m', (48, 96, 160)), ('w_l', (104, 152, 208)),
    ('quay_d', (64, 64, 80)), ('quay_m', (104, 104, 120)), ('quay_l', (148, 148, 160)),
    ('quay_h', (188, 188, 196)), ('marble_d', (164, 160, 156)), ('marble_m', (204, 200, 192)),
    ('marble_l', (236, 232, 224)), ('bronze_d', (96, 72, 48)), ('bronze_m', (152, 120, 72)),
    ('bronze_l', (208, 176, 112)), ('c_outline', (24, 24, 36)), ('c_blue', (40, 64, 152)),
]

# Slate roofs for the keep and the dwarves, purple ones for the Mage Quarter.
OVERHEAD_CITY = [
    ('p_outline', (24, 20, 40)), ('purple_d', (64, 40, 104)), ('purple_m', (96, 64, 144)),
    ('purple_l', (136, 104, 184)), ('purple_h', (184, 160, 216)), ('slate_d', (48, 56, 72)),
    ('slate_m', (72, 84, 104)), ('slate_l', (104, 120, 140)), ('slate_h', (140, 156, 176)),
    ('wall_d', (80, 80, 96)), ('wall_m', (120, 120, 136)), ('wall_l', (160, 160, 168)),
    ('gold_o', (240, 200, 80)), ('blue_o', (32, 56, 144)), ('white_o', (232, 232, 224)),
]

SLATE = dict(roof_colors=('slate_d', 'slate_m', 'slate_l'), roof_ridge='slate_h', roof_outline='p_outline')
PURPLE = dict(roof_colors=('purple_d', 'purple_m', 'purple_l'), roof_ridge='purple_h', roof_outline='p_outline')
BLUE = dict(roof_colors=('roof_d', 'roof_m', 'roof_l'), roof_ridge='roof_h', roof_outline='outline')
RED = dict(roof_colors=('red_d', 'red_m', 'red_l'), roof_ridge='red_l', roof_outline='o2')
THATCH = dict(roof_colors=('thatch_d', 'thatch_m', 'thatch_l'), roof_ridge='thatch_l', roof_outline='o2')


def marble(m, x, y, w, h):
    """Big marble flagstones."""
    ys, xs = np.mgrid[y:y + h, x:x + w]
    row = ys // 16
    pattern = np.full((h, w), m.g('marble_m'), dtype=np.uint8)
    pattern[((xs + (row % 2) * 8) % 16 == 15) | (ys % 16 == 15)] = m.g('marble_d')
    pattern[(ys % 16 == 0) & ((xs + (row % 2) * 8) % 16 < 14)] = m.g('marble_l')
    m.ground[y:y + h, x:x + w] = pattern


def canal(m, x, y, w, h):
    """A stone-lined canal. The water and the quays are solid; bridges open crossings."""
    g = m.ground
    ys, xs = np.mgrid[y:y + h, x:x + w]
    water = np.full((h, w), m.g('w_m'), dtype=np.uint8)
    water[((xs + (ys // 8) * 5) % 16 < 3) & (ys % 8 == 3)] = m.g('w_l')
    water[((xs + (ys // 8) * 3 + 8) % 16 < 2) & (ys % 8 == 6)] = m.g('w_d')
    g[y:y + h, x:x + w] = water
    vertical = h > w
    q = 8
    for qx, qy, qw, qh in ((x, y, q, h), (x + w - q, y, q, h)) if vertical else ((x, y, w, q), (x, y + h - q, w, q)):
        sub_ys, sub_xs = np.mgrid[qy:qy + qh, qx:qx + qw]
        quay = np.full((qh, qw), m.g('quay_m'), dtype=np.uint8)
        quay[(sub_ys % 8 == 7) | (((sub_xs + (sub_ys // 8) * 4) % 8) == 7)] = m.g('quay_d')
        quay[sub_ys % 8 == 0] = m.g('quay_l')
        g[qy:qy + qh, qx:qx + qw] = quay
    if vertical:
        g[y:y + h, x + q] = m.g('w_d')
        g[y:y + h, x] = m.g('quay_h')
        g[y:y + h, x + w - 1] = m.g('c_outline')
    else:
        g[y + q:y + q + 2, x:x + w] = m.g('w_d')
        g[y, x:x + w] = m.g('quay_h')
        g[y + h - 1, x:x + w] = m.g('c_outline')
    m.block(x, y, w, h)


def city_bridge(m, x, y, w, h, east_west):
    """A marble bridge with railings along both sides of the way across."""
    marble(m, x, y, w, h)
    g = m.ground
    m.unblock(x, y, w, h)
    if east_west:
        # Crossing a north-south canal: railings on the top and bottom.
        for ry in (y, y + h - 8):
            g[ry:ry + 8, x:x + w] = m.g('marble_l')
            g[ry + 6:ry + 8, x:x + w] = m.g('marble_d')
            g[ry + 2:ry + 6, x + 4:x + w - 4:8] = m.g('quay_d')
            m.block(x, ry, w, 8)
    else:
        for rx in (x, x + w - 8):
            g[y:y + h, rx:rx + 8] = m.g('marble_l')
            g[y:y + h, rx + 6:rx + 8] = m.g('marble_d')
            g[y + 4:y + h - 4:8, rx + 2:rx + 6] = m.g('quay_d')
            m.block(rx, y, 8, h)


def statue(m, x, y):
    """A hero on a marble pedestal, 32x48, in the city bank. (x, y) is the top-left."""
    g = m.ground
    g[y + 32:y + 48, x:x + 32] = m.g('marble_m')
    g[y + 32:y + 34, x:x + 32] = m.g('marble_l')
    g[y + 44:y + 48, x:x + 32] = m.g('marble_d')
    g[y + 47, x:x + 32] = m.g('c_outline')
    g[y + 32:y + 48, x] = m.g('c_outline')
    g[y + 32:y + 48, x + 31] = m.g('c_outline')
    g[y + 37:y + 41, x + 10:x + 22] = m.g('bronze_l')
    # The figure: head, shoulders, body and a sword held point down.
    body = [(12, 20, 2, 'c_outline'), (12, 20, 3, 'bronze_l'), (11, 21, 4, 'bronze_m'), (11, 21, 6, 'bronze_m'),
            (8, 24, 9, 'bronze_d'), (7, 25, 10, 'bronze_m'), (7, 25, 11, 'bronze_m')]
    for x0, x1, row, c in body:
        g[y + row, x + x0:x + x1] = m.g(c)
    g[y + 2:y + 9, x + 12:x + 20] = m.g('bronze_m')
    g[y + 3:y + 7, x + 13:x + 15] = m.g('bronze_l')
    g[y + 9:y + 32, x + 9:x + 23] = m.g('bronze_m')
    g[y + 9:y + 32, x + 9:x + 12] = m.g('bronze_l')
    g[y + 9:y + 32, x + 20:x + 23] = m.g('bronze_d')
    g[y + 20:y + 23, x + 6:x + 26] = m.g('bronze_l')
    g[y + 12:y + 34, x + 15:x + 17] = m.g('quay_h')
    g[y + 2:y + 32, x + 8] = m.g('c_outline')
    g[y + 2:y + 32, x + 23] = m.g('c_outline')
    m.block(x, y + 24, 32, 24)


def fountain(m, x, y):
    """A round fountain on a 64x64 marble square."""
    marble(m, x, y, 64, 64)
    g = m.ground
    ys, xs = np.mgrid[0:64, 0:64]
    d = np.hypot(xs + 0.5 - 32, (ys + 0.5 - 34) * 1.15)
    area = g[y:y + 64, x:x + 64]
    area[d < 30] = m.g('marble_d')
    area[d < 28] = m.g('marble_l')
    area[d < 25] = m.g('w_d')
    area[d < 23] = m.g('w_m')
    area[(d < 21) & ((xs + ys) % 9 == 0)] = m.g('w_l')
    area[d < 7] = m.g('marble_m')
    area[d < 4] = m.g('marble_l')
    area[(d >= 30) & (d < 31)] = m.g('c_outline')
    area[15:30, 30:34] = m.g('marble_m')
    area[12:18, 28:36] = m.g('w_l')
    m.block(x + 8, y + 12, 48, 44)


def lamp_post(m, x, y):
    """A street lamp on marble, 8x24."""
    g = m.ground
    g[y + 6:y + 24, x + 3:x + 5] = m.g('c_outline')
    g[y:y + 7, x + 1:x + 7] = m.g('c_outline')
    g[y + 1:y + 6, x + 2:x + 6] = m.g('bronze_l')
    g[y + 22:y + 24, x + 1:x + 7] = m.g('c_outline')
    m.block(x, y + 16, 8, 8)


def city_walls(m, gate):
    """Walls around the whole map, with the gate gap (y0, y1) on the east side."""
    g = m.ground
    W, H, t = m.width, m.height, 32
    wg.bricks(m, g, 0, 0, W, 48, 'stone_d', 'stone_m', 'stone_l')
    g[0:8, 0:W] = m.g('stone_l')
    g[0:6, 4:W:16] = m.g('outline')
    g[44:48, 0:W] = m.g('stone_d')
    g[47, 0:W] = m.g('outline')
    m.block(0, 0, W, 48)
    top = [(0, 48, t, H - 48), (W - t, 48, t, gate[0] - 48), (W - t, gate[1], t, H - gate[1]), (0, H - t, W, t)]
    for x, y, w, h in top:
        ys, xs = np.mgrid[y:y + h, x:x + w]
        pattern = np.full((h, w), m.g('stone_m'), dtype=np.uint8)
        pattern[(ys % 16 < 8) & (xs % 16 < 8)] = m.g('stone_l')
        pattern[(ys % 16 == 15) | (xs % 16 == 15)] = m.g('stone_d')
        g[y:y + h, x:x + w] = pattern
        m.block(x, y, w, h)
    g[48:H - t, t - 2:t] = m.g('outline')
    g[48:gate[0], W - t:W - t + 2] = m.g('outline')
    g[gate[1]:H - t, W - t:W - t + 2] = m.g('outline')
    g[H - t:H - t + 2, t:W - t] = m.g('outline')


def gate_tower(m, x, y):
    """One of the two towers flanking the east gate: 48x96 with a blue cone."""
    g = m.ground
    wg.bricks(m, g, x, y + 40, 48, 56, 'stone_d', 'stone_m', 'stone_l')
    g[y + 40:y + 96, x] = m.g('outline')
    g[y + 40:y + 96, x + 47] = m.g('outline')
    g[y + 56:y + 72, x + 20:x + 28] = m.g('outline')
    g[y + 58:y + 70, x + 21:x + 27] = m.g('glass_d')
    g[y + 60:y + 70, x + 22:x + 26:2] = m.g('glass_l')
    for bx in (x + 6, x + 36):
        g[y + 46:y + 76, bx:bx + 6] = m.g('banner')
        g[y + 46:y + 76, bx] = m.g('banner_d')
        g[y + 46:y + 48, bx:bx + 6] = m.g('gold')
    spire(m, x + 24, y, 48, 48, ('roof_d', 'roof_m', 'roof_l'), 'outline', tip='gold')
    m.block(x, y + 40, 48, 56)


def spire(m, cx, top, w, h, colors, outline, tip=None):
    """A cone roof on the overhead layer, lit from the left."""
    o = m.overhead
    for row in range(h):
        half = max(1, int((row + 1) * (w / 2) / h))
        x0, x1 = cx - half, cx + half
        o[top + row, x0:x1] = m.o(colors[1])
        o[top + row, x0:x0 + max(1, half // 2)] = m.o(colors[2])
        o[top + row, cx + half // 3:x1] = m.o(colors[0])
        if row % 6 == 5:
            o[top + row, x0:x1] = m.o(colors[0])
        o[top + row, x0] = m.o(outline)
        o[top + row, x1 - 1] = m.o(outline)
    o[top + h - 1, cx - w // 2:cx + w // 2] = m.o(outline)
    if tip:
        o[top - 4:top + 2, cx - 1:cx + 1] = m.o(tip)


def keep(m, x, y):
    """Stormwind Keep: a wide hall with a blue roof and two corner towers. 288x208."""
    W, H = 288, 208
    g = m.ground
    wall_y = y + 112
    wg.bricks(m, g, x + 24, wall_y, W - 48, H - 112, 'stone_d', 'stone_m', 'stone_l')
    g[wall_y:wall_y + 2, x + 24:x + W - 24] = m.g('stone_h')
    for wx in range(x + 48, x + W - 48, 32):
        if abs(wx + 8 - (x + W // 2)) < 40:
            continue
        g[wall_y + 24:wall_y + 56, wx:wx + 16] = m.g('outline')
        g[wall_y + 26:wall_y + 54, wx + 2:wx + 14] = m.g('glass_d')
        g[wall_y + 32:wall_y + 54, wx + 3:wx + 13:2] = m.g('glass_l')
    dx = x + W // 2 - 20
    g[wall_y + 40:y + H, dx:dx + 40] = m.g('outline')
    g[wall_y + 42:y + H, dx + 2:dx + 38] = m.g('wood_d')
    for px in range(dx + 4, dx + 36, 6):
        g[wall_y + 44:y + H, px:px + 3] = m.g('wood_l')
    g[wall_y + 34:wall_y + 40, dx - 4:dx + 44] = m.g('gold')
    for bx in (dx - 24, dx + 56):
        g[wall_y + 8:wall_y + 64, bx:bx + 8] = m.g('banner')
        g[wall_y + 8:wall_y + 64, bx] = m.g('banner_d')
        g[wall_y + 8:wall_y + 10, bx:bx + 8] = m.g('gold')
        g[wall_y + 28:wall_y + 36, bx + 2:bx + 6] = m.g('gold')
    wg.roof(m, x + 24, y + 24, W - 48, 88, ('roof_d', 'roof_m', 'roof_l'), 'roof_h', 'outline')
    for tx in (x, x + W - 56):
        wg.bricks(m, g, tx, y + 72, 56, H - 72, 'stone_d', 'stone_m', 'stone_l')
        g[y + 72:y + H, tx] = m.g('outline')
        g[y + 72:y + H, tx + 55] = m.g('outline')
        g[y + 100:y + 124, tx + 24:tx + 32] = m.g('outline')
        g[y + 102:y + 122, tx + 25:tx + 31] = m.g('glass_l')
        spire(m, tx + 28, y, 64, 72, ('roof_d', 'roof_m', 'roof_l'), 'outline', tip='gold')
    m.block(x, y + 72, W, H - 72)
    m.block(x + 24, y + 40, W - 48, H - 40)
    return (x + W // 2, y + H + 8)


def cathedral(m, x, y):
    """The Cathedral of Light: white stone, tall windows, a blue roof and two spires. 208x192."""
    W, H = 208, 192
    g = m.ground
    wall_y = y + 96
    wg.bricks(m, g, x, wall_y, W, H - 96, 'stone_l', 'stone_h', 'plaster')
    g[wall_y:wall_y + 2, x:x + W] = m.g('plaster')
    g[wall_y:y + H, x] = m.g('outline')
    g[wall_y:y + H, x + W - 1] = m.g('outline')
    for wx in (x + 16, x + 48, x + 144, x + 176):
        g[wall_y + 12:wall_y + 60, wx:wx + 16] = m.g('outline')
        g[wall_y + 14:wall_y + 58, wx + 2:wx + 14] = m.g('banner_d')
        g[wall_y + 16:wall_y + 58, wx + 4:wx + 12:4] = m.g('glass_l')
        g[wall_y + 20:wall_y + 58:8, wx + 2:wx + 14] = m.g('gold')
    # Rose window and the great door.
    cx = x + W // 2
    g[wall_y + 4:wall_y + 28, cx - 12:cx + 12] = m.g('outline')
    g[wall_y + 6:wall_y + 26, cx - 10:cx + 10] = m.g('banner')
    g[wall_y + 12:wall_y + 20, cx - 4:cx + 4] = m.g('glass_l')
    g[wall_y + 6:wall_y + 26, cx - 1:cx + 1] = m.g('gold')
    g[wall_y + 15:wall_y + 17, cx - 10:cx + 10] = m.g('gold')
    g[wall_y + 40:y + H, cx - 20:cx + 20] = m.g('outline')
    g[wall_y + 42:y + H, cx - 18:cx + 18] = m.g('wood_d')
    for px in range(cx - 16, cx + 16, 6):
        g[wall_y + 44:y + H, px:px + 3] = m.g('wood_l')
    g[wall_y + 36:wall_y + 40, cx - 24:cx + 24] = m.g('gold')
    wg.roof(m, x + 32, y + 32, W - 64, 64, ('roof_d', 'roof_m', 'roof_l'), 'roof_h', 'outline')
    for tx in (x, x + W - 40):
        wg.bricks(m, m.overhead, tx, y + 48, 40, 48, 'stone_d', 'stone_m', 'stone_l', palette=m.o)
        m.overhead[y + 48:y + 96, tx] = m.o('outline')
        m.overhead[y + 48:y + 96, tx + 39] = m.o('outline')
        m.overhead[y + 60:y + 80, tx + 16:tx + 24] = m.o('glass')
        spire(m, tx + 20, y, 40, 48, ('roof_d', 'roof_m', 'roof_l'), 'outline', tip='gold')
    m.block(x, y + 112, W, H - 112)
    return (cx, y + H + 8)


def mage_tower(m, x, y):
    """The Wizard's Sanctum: a stone tower crowned with a purple cone. 80x176."""
    g = m.ground
    wg.bricks(m, g, x, y + 80, 80, 96, 'stone_d', 'stone_m', 'stone_l')
    g[y + 80:y + 176, x] = m.g('outline')
    g[y + 80:y + 176, x + 79] = m.g('outline')
    for wy in (y + 96, y + 128):
        g[wy:wy + 16, x + 36:x + 44] = m.g('outline')
        g[wy + 2:wy + 14, x + 37:x + 43] = m.g('glass_l')
    g[y + 148:y + 176, x + 28:x + 52] = m.g('outline')
    g[y + 150:y + 176, x + 30:x + 50] = m.g('banner_d')
    g[y + 160, x + 44:x + 47] = m.g('gold')
    wg.bricks(m, m.overhead, x, y + 64, 80, 16, 'wall_d', 'wall_m', 'wall_l', palette=m.o)
    m.overhead[y + 64, x:x + 80] = m.o('p_outline')
    spire(m, x + 40, y, 96, 64, ('purple_d', 'purple_m', 'purple_l'), 'p_outline', tip='gold_o')
    m.block(x, y + 96, 80, 80)
    return (x + 40, y + 184)


def stall(m, x, y, colors=('red_m', 'canvas')):
    """A market stall: a striped awning (overhead) over a wooden counter, 40x32."""
    g, o = m.ground, m.overhead
    g[y + 20:y + 28, x + 4:x + 36] = m.g('wood_l')
    g[y + 26:y + 28, x + 4:x + 36] = m.g('wood_d')
    g[y + 20, x + 4:x + 36] = m.g('outline')
    g[y + 21:y + 24, x + 8:x + 32:6] = m.g('gold')
    ys, xs = np.mgrid[y:y + 20, x:x + 40]
    awning = np.where((xs // 8) % 2 == 0, m.o(colors[0]), m.o(colors[1])).astype(np.uint8)
    awning[ys >= y + 16] = np.where((xs[ys >= y + 16] // 8) % 2 == 0, m.o('red_d'), m.o('canvas_d'))
    o[y:y + 20, x:x + 40] = awning
    o[y, x:x + 40] = m.o('o2')
    o[y + 19, x:x + 40] = m.o('o2')
    o[y:y + 20, x] = m.o('o2')
    o[y:y + 20, x + 39] = m.o('o2')
    m.block(x + 4, y + 18, 32, 10)


def gen_stormwind():
    m = Map('stormwind', 1024, 1024,
            Palette([wg.TERRAIN_ELWYNN, wg.BUILDINGS, CITY]),
            Palette([wg.OVERHEAD_LEAVES, wg.OVERHEAD_ROOFS, OVERHEAD_CITY]))
    wg.fill_grass(m)
    trees = wg.Trees(m)
    gate = (464, 560)

    # --- streets and squares (laid first, buildings stand on them) -----------------------------------
    marble(m, 720, 400, 272, 224)                     # Valley of Heroes
    wg.cobbles(m, 32, 456, 624, 64)                   # Trade District main street
    wg.cobbles(m, 304, 296, 64, 160)                  # north to the cathedral and the keep
    wg.cobbles(m, 280, 520, 64, 120)                  # south to the Mage Quarter bridge
    wg.cobbles(m, 32, 520, 624, 120)                  # market
    marble(m, 32, 224, 312, 72)                       # keep courtyard
    marble(m, 360, 232, 288, 96)                      # Cathedral Square
    wg.cobbles(m, 720, 48, 272, 352)                  # Dwarven District
    wg.cobbles(m, 720, 624, 128, 368)                 # Old Town
    wg.cobbles(m, 848, 776, 144, 80)
    wg.cobbles(m, 32, 704, 624, 56)                   # Mage Quarter
    wg.cobbles(m, 320, 760, 64, 224)

    city_walls(m, gate)

    # --- canals and bridges --------------------------------------------------------------------------
    canal(m, 656, 48, 64, 944)
    canal(m, 32, 640, 624, 64)
    city_bridge(m, 656, 456, 64, 112, True)
    city_bridge(m, 656, 176, 64, 64, True)
    city_bridge(m, 656, 784, 64, 64, True)
    city_bridge(m, 280, 640, 64, 64, False)
    city_bridge(m, 496, 640, 48, 64, False)
    wg.cobbles(m, 496, 616, 48, 24)
    wg.cobbles(m, 496, 704, 48, 8)
    marble(m, 600, 176, 56, 64)
    wg.cobbles(m, 600, 784, 56, 64)

    # --- the gate and the Valley of Heroes ------------------------------------------------------------
    gate_tower(m, 976, gate[0] - 96)
    gate_tower(m, 976, gate[1])
    m.unblock(992, gate[0], 32, gate[1] - gate[0])
    marble(m, 992, gate[0], 32, gate[1] - gate[0])
    for sx in (752, 832, 912):
        statue(m, sx, 408)
        statue(m, sx, 568)
    for lx in (736, 800, 880, 960):
        lamp_post(m, lx, 456)
        lamp_post(m, lx, 544)
    m.warp(1016, gate[0] + 8, 8, gate[1] - gate[0] - 16, 'elwynn', 'from_stormwind')
    m.point('from_elwynn', 990, 512)
    m.npc('MARCUS_JONATHAN', 948, 500)
    m.npc('SW_GUARD_GATE', 948, 536)
    m.area(720, 400, 304, 224, 'Valley of Heroes')

    # --- Stormwind Keep and Cathedral Square ----------------------------------------------------------
    keep_door = keep(m, 48, 16)
    m.npc('BOLVAR', keep_door[0], keep_door[1] + 24)
    m.npc('SW_GUARD_KEEP', keep_door[0] - 48, keep_door[1] + 20)
    m.area(32, 48, 312, 248, 'Stormwind Keep')
    church_door = cathedral(m, 400, 40)
    fountain(m, 472, 248)
    m.npc('BENEDICTUS', church_door[0] - 40, church_door[1] + 8)
    m.point('respawn', church_door[0], church_door[1] + 8)
    m.area(344, 48, 312, 280, 'Cathedral Square')

    # --- Trade District ---------------------------------------------------------------------------------
    inn_door = wg.house(m, 48, 344, 160, 112, style='timber', door_x=72, **RED)
    m.npc('ALLISON', inn_door[0] + 24, inn_door[1])
    m.point('inn', inn_door[0], inn_door[1] + 8)
    smith_door = wg.house(m, 392, 360, 104, 96, style='stone', **SLATE)
    m.npc('GUNTHER', smith_door[0] + 24, smith_door[1])
    armor_door = wg.house(m, 520, 360, 104, 96, style='stone', **BLUE)
    m.npc('LINA', armor_door[0] + 24, armor_door[1])
    wg.house(m, 224, 360, 72, 96, style='timber', **THATCH)
    wg.anvil(m, 448, 464)
    for sx, colors in ((64, ('red_m', 'canvas')), (136, ('thatch_m', 'canvas')), (408, ('red_m', 'canvas')),
                       (480, ('thatch_m', 'canvas'))):
        stall(m, sx, 552, colors)
    m.npc('THURMAN', 156, 594)
    m.npc('SW_GUARD_TRADE', 360, 500)
    fountain(m, 216, 560)
    m.area(32, 328, 624, 312, 'Trade District')

    # --- Mage Quarter -----------------------------------------------------------------------------------
    tower_door = mage_tower(m, 432, 720)
    m.npc('JENNEA', tower_door[0] - 32, tower_door[1])
    wg.house(m, 56, 768, 96, 96, style='stone', **PURPLE)
    wg.house(m, 176, 768, 96, 96, style='timber', **PURPLE)
    wg.house(m, 56, 872, 112, 96, style='stone', **PURPLE)
    wg.house(m, 552, 776, 88, 96, style='stone', **PURPLE)
    wg.house(m, 552, 888, 88, 80, style='timber', **PURPLE)
    wg.grove(m, trees, [(192, 888), (232, 912), (400, 904), (512, 920), (272, 880)], seed=2,
             kinds=('birch', 'oak', 'pine'))
    m.chest(9, 632, 984, 9)
    m.area(32, 704, 624, 288, 'Mage Quarter')

    # --- Dwarven District -------------------------------------------------------------------------------
    forge_door = wg.house(m, 760, 48, 160, 128, style='stone', door_x=64, **SLATE)
    for ax in (776, 808, 872, 904):
        wg.anvil(m, ax, 196)
    m.npc('EINRIS', 950, 210)
    m.npc('BRANN', forge_door[0] - 40, forge_door[1] + 4)
    wg.house(m, 744, 296, 104, 96, style='stone', **SLATE)
    wg.house(m, 872, 296, 104, 96, style='stone', **RED)
    m.area(720, 48, 272, 352, 'Dwarven District')

    # --- Old Town -------------------------------------------------------------------------------------------
    wg.house(m, 760, 632, 112, 112, style='timber', **RED)
    wg.house(m, 880, 640, 96, 96, style='timber', **THATCH)
    si7_door = wg.house(m, 872, 872, 112, 96, style='stone', **SLATE)
    wg.house(m, 744, 880, 96, 96, style='timber', **THATCH)
    m.npc('ANDER', 900, 800)
    wg.grove(m, trees, [(880, 744), (944, 736)], seed=4, kinds=('oak', 'birch'))
    m.chest(10, 972, 864, 10)
    m.area(720, 624, 272, 368, 'Old Town')

    # Grass patches get a few trees and props.
    rng = np.random.default_rng(31)
    wg.scatter_props(m, rng, 18, ('bush', 'flowers', 'flower_bush', 'fern'), (40, 300, 960, 680))
    m.area(0, 0, 1024, 1024, 'Stormwind City')
    m.music = 'TOWN'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Kobold mines: Echo Ridge (Northshire) and Fargodeep (south of Goldshire)
# ---------------------------------------------------------------------------------------------

CAVE_OVERHEAD = [
    ('outline', (16, 12, 16)), ('beam_d', (80, 52, 32)), ('beam_l', (128, 88, 52)),
    ('rock_d', (56, 48, 48)), ('rock_m', (88, 76, 72)), ('rock_l', (124, 108, 100)),
    ('lamp', (248, 200, 96)),
]


class Cave:
    """A small cave map: rock everywhere, then rooms and tunnels carved out of it.

    The floor is kept as one flag per 8x8 cell. render() paints the rock, the floor, a 16 px rock
    face along every wall above the floor (solid, so walls look tall) and the edges.
    """

    def __init__(self, name, width, height):
        self.m = Map(name, width, height, Palette([CAVE]), Palette([CAVE_OVERHEAD]))
        self.floor = np.zeros((height // 8, width // 8), dtype=bool)
        self.face = np.zeros_like(self.floor)

    def rect(self, x, y, w, h):
        self.floor[y // 8:(y + h) // 8, x // 8:(x + w) // 8] = True

    def blob(self, cx, cy, rx, ry, seed=0):
        """A rounded chamber with a slightly ragged edge."""
        local = np.random.default_rng(seed)
        ys, xs = np.mgrid[0:self.floor.shape[0], 0:self.floor.shape[1]]
        d = np.hypot((xs * 8 + 4 - cx) / rx, (ys * 8 + 4 - cy) / ry)
        self.floor |= d < 1 + (local.random(d.shape) - 0.5) * 0.18

    def tunnel(self, points, width):
        ys, xs = np.mgrid[0:self.floor.shape[0], 0:self.floor.shape[1]]
        px, py = xs * 8 + 4, ys * 8 + 4
        for (x0, y0), (x1, y1) in zip(points, points[1:]):
            dx, dy = x1 - x0, y1 - y0
            t = np.clip(((px - x0) * dx + (py - y0) * dy) / max(dx * dx + dy * dy, 1), 0, 1)
            self.floor |= np.hypot(px - (x0 + t * dx), py - (y0 + t * dy)) <= width / 2

    def render(self):
        m, g, f = self.m, self.m.ground, self.floor
        rows, cols = f.shape
        wg.bricks(m, g, 0, 0, m.width, m.height, 'rock_d', 'rock_m', 'rock_l')
        m.block(0, 0, m.width, m.height)
        floors = []
        for v in range(4):
            local = np.random.default_rng(300 + v)
            tile = np.full((8, 8), m.g('floor_m'), dtype=np.uint8)
            noise = local.random((8, 8))
            tile[noise < 0.12] = m.g('floor_d')
            tile[noise > 0.92] = m.g('floor_l')
            if v == 3:
                tile[5:7, 2:4] = m.g('rock_m')
                tile[5, 2] = m.g('rock_l')
            floors.append(tile)
        above = np.zeros_like(f)
        above[1:] = f[:-1]
        top_face = f & ~above
        second = np.zeros_like(f)
        second[1:] = top_face[:-1]
        self.face = top_face | (second & f)
        for cy in range(rows):
            for cx in range(cols):
                if not f[cy, cx]:
                    continue
                x, y = cx * 8, cy * 8
                cell = g[y:y + 8, x:x + 8]
                if self.face[cy, cx]:
                    lower = not top_face[cy, cx]
                    cell[:] = m.g('rock_m')
                    cell[:, (x // 8 * 3) % 8] = m.g('rock_d')
                    cell[:, (x // 8 * 5 + 3) % 8] = m.g('rock_l')
                    if not lower:
                        cell[0, :] = m.g('rock_d')
                    else:
                        cell[5:, :] = m.g('rock_d')
                        cell[6:, :] = m.g('outline')
                else:
                    cell[:] = floors[wg.tile_hash(cx, cy, 5) % 4 if wg.tile_hash(cx, cy, 6) % 3 else 0]
                    m.unblock(x, y, 8, 8)
                # Dark edges where the floor meets rock at the sides and below.
                if cx > 0 and not f[cy, cx - 1]:
                    cell[:, 0] = m.g('outline')
                if cx + 1 < cols and not f[cy, cx + 1]:
                    cell[:, 7] = m.g('outline')
                if cy + 1 < rows and not f[cy + 1, cx]:
                    cell[7, :] = m.g('outline')
                    g[y + 8:y + 10, x:x + 8] = m.g('rock_l')

    # --- props (cave bank) ----------------------------------------------------------------------

    def beams(self, x, y, w=32):
        """A timber frame against a rock face: two posts and a crossbeam. y is the face's top."""
        g, m = self.m.ground, self.m
        for px in (x, x + w - 4):
            g[y:y + 16, px:px + 4] = m.g('beam_l')
            g[y:y + 16, px + 3] = m.g('beam_d')
        g[y:y + 4, x - 2:x + w + 2] = m.g('beam_l')
        g[y + 3, x - 2:x + w + 2] = m.g('beam_d')

    def lamp(self, x, y):
        g, m = self.m.ground, self.m
        g[y:y + 2, x + 2:x + 4] = m.g('beam_d')
        g[y + 2:y + 7, x + 1:x + 5] = m.g('outline')
        g[y + 3:y + 6, x + 2:x + 4] = m.g('lamp')

    def ore(self, x, y):
        """Gold flecks in the rock."""
        g, m = self.m.ground, self.m
        for dx, dy in ((0, 0), (3, 2), (1, 4), (5, 1), (6, 5), (2, 6)):
            g[y + dy, x + dx] = m.g('lamp')
            if dy + 1 < 8:
                g[y + dy + 1, x + dx] = m.g('rock_d')

    def rails(self, x0, x1, y):
        g, m = self.m.ground, self.m
        for x in range(x0 // 8 * 8, x1, 8):
            g[y:y + 12, x + 2:x + 4] = m.g('beam_d')
            g[y + 2, x:x + 8] = m.g('rail')
            g[y + 9, x:x + 8] = m.g('rail')

    def rails_v(self, x, y0, y1):
        g, m = self.m.ground, self.m
        for y in range(y0 // 8 * 8, y1, 8):
            g[y + 2:y + 4, x:x + 12] = m.g('beam_d')
            g[y:y + 8, x + 2] = m.g('rail')
            g[y:y + 8, x + 9] = m.g('rail')

    def cart(self, x, y):
        """A minecart full of ore, 24x16."""
        g, m = self.m.ground, self.m
        g[y + 2:y + 13, x:x + 24] = m.g('outline')
        g[y + 3:y + 12, x + 1:x + 23] = m.g('rail')
        g[y + 5:y + 12, x + 1:x + 23] = m.g('beam_d')
        g[y + 1:y + 4, x + 3:x + 21] = m.g('rock_m')
        g[y, x + 6:x + 18] = m.g('rock_l')
        g[y + 2, x + 8:x + 16:3] = m.g('lamp')
        for wx in (x + 3, x + 17):
            g[y + 12:y + 16, wx:wx + 4] = m.g('outline')
            g[y + 13:y + 15, wx + 1:wx + 3] = m.g('rail')
        self.m.block(x, y + 6, 24, 10)

    def crates(self, x, y):
        g, m = self.m.ground, self.m
        for cx, cy in ((x, y + 8), (x + 16, y + 8), (x + 8, y)):
            g[cy:cy + 16, cx:cx + 16] = m.g('outline')
            g[cy + 1:cy + 15, cx + 1:cx + 15] = m.g('beam_l')
            g[cy + 7:cy + 9, cx + 1:cx + 15] = m.g('beam_d')
            g[cy + 1:cy + 15, cx + 7:cx + 9] = m.g('beam_d')
        self.m.block(x, y + 4, 32, 20)

    def puddle(self, cx, cy, rx, ry):
        g, m = self.m.ground, self.m
        ys, xs = np.mgrid[cy - ry:cy + ry, cx - rx:cx + rx]
        d = np.hypot((xs + 0.5 - cx) / rx, (ys + 0.5 - cy) / ry)
        area = g[cy - ry:cy + ry, cx - rx:cx + rx]
        area[d < 1] = m.g('water_d')
        area[d < 0.85] = m.g('water_m')
        area[(d < 0.6) & ((xs + ys) % 7 == 0)] = m.g('water_l')
        self.m.block(cx - rx + 8, cy - ry + 8, rx * 2 - 16, ry * 2 - 16)

    def candles(self, x, y):
        """A kobold's candle stub stuck on a rock."""
        g, m = self.m.ground, self.m
        g[y + 5:y + 8, x:x + 8] = m.g('rock_m')
        g[y + 5, x + 1:x + 7] = m.g('rock_l')
        g[y + 2:y + 5, x + 3:x + 5] = m.g('cloth') if 'cloth' in m.gp.index else m.g('rock_l')
        g[y:y + 2, x + 3:x + 5] = m.g('lamp')
        self.m.block(x, y + 4, 8, 4)

    def crossbeam(self, x, y, w):
        """A beam across a tunnel on the overhead layer, held up by posts at both ends."""
        m, o = self.m, self.m.overhead
        o[y:y + 6, x:x + w] = m.o('beam_l')
        o[y + 5, x:x + w] = m.o('beam_d')
        o[y, x:x + w] = m.o('outline')
        for px in range(x + 6, x + w - 6, 12):
            o[y + 2, px:px + 2] = m.o('beam_d')

    def secret(self, x, y, w, h):
        """Rock drawn over a passage on the overhead layer: it looks like solid wall, but the player
        can walk under it. The same brick pattern as the ground makes it seamless."""
        wg.bricks(self.m, self.m.overhead, x, y, w, h, 'rock_d', 'rock_m', 'rock_l', palette=self.m.o)

    def exit(self, x, y, target, point):
        """The way out at the bottom edge of the map, 48 px wide."""
        g, m = self.m.ground, self.m
        g[y:y + 8, x:x + 48] = m.g('beam_l')
        for px in range(x, x + 48, 8):
            g[y:y + 8, px] = m.g('beam_d')
        m.unblock(x, y - 8, 48, 16)
        m.warp(x, y, 48, 8, target, point)
        m.point('entry', x + 24, y - 20)
        m.point('respawn', x + 24, y - 20)


def gen_echo_ridge():
    c = Cave('echo_ridge', 512, 512)
    m = c.m
    c.rect(232, 400, 48, 112)
    c.blob(256, 384, 112, 64, seed=1)
    c.tunnel([(190, 370), (120, 320)], 56)
    c.blob(110, 300, 84, 72, seed=2)
    c.tunnel([(256, 340), (272, 200)], 56)
    c.blob(280, 168, 124, 72, seed=3)
    c.tunnel([(360, 190), (420, 270)], 56)
    c.blob(420, 310, 72, 80, seed=4)
    # A narrow crawl off the west chamber, easy to miss, ends in a small pocket.
    c.tunnel([(84, 250), (76, 130)], 32)
    c.blob(80, 96, 48, 40, seed=5)
    c.render()
    c.exit(232, 504, 'elwynn', 'echo_ridge_exit')

    for x, y in ((176, 312), (304, 312), (200, 96), (336, 96), (392, 228)):
        c.beams(x, y)
    for x, y in ((150, 330), (240, 104), (430, 238), (80, 70)):
        c.lamp(x, y)
    c.rails(176, 336, 404)
    c.cart(320, 392)
    c.crates(56, 300)
    c.crates(408, 344)
    for x, y in ((200, 160), (232, 200), (330, 150), (372, 210), (96, 344), (452, 300)):
        c.candles(x, y)
    c.crossbeam(232, 336, 56)
    c.crossbeam(240, 216, 64)
    c.secret(56, 136, 48, 88)
    m.chest(0, 80, 112, 3)

    for x, y in ((220, 380), (300, 400), (100, 290), (140, 330), (70, 330), (240, 160),
                 (300, 180), (340, 200), (210, 200), (410, 300), (440, 340), (400, 270)):
        m.spawn('KOBOLD_VERMIN', x, y)
    m.area(0, 0, 512, 512, 'Echo Ridge Mine', 'ECHO_RIDGE')
    m.music = 'DUNGEON'
    m.save()
    return m


def gen_fargodeep():
    c = Cave('fargodeep', 768, 512)
    m = c.m
    c.rect(360, 420, 48, 92)
    c.blob(384, 424, 116, 56, seed=11)
    c.tunnel([(300, 424), (180, 396)], 56)
    c.blob(130, 380, 92, 72, seed=12)
    c.tunnel([(390, 390), (400, 270)], 56)
    c.blob(400, 236, 144, 72, seed=13)
    c.tunnel([(520, 250), (620, 310)], 56)
    c.blob(650, 336, 84, 84, seed=14)
    c.tunnel([(330, 190), (310, 110)], 56)
    c.blob(330, 76, 176, 48, seed=15)
    # A crawl behind the east chamber.
    c.tunnel([(690, 280), (712, 150)], 32)
    c.blob(708, 112, 44, 40, seed=16)
    c.render()
    c.exit(360, 504, 'elwynn', 'fargodeep_exit')

    for x, y in ((320, 360), (440, 368), (88, 312), (344, 168), (472, 168), (224, 32), (432, 32),
                 (616, 264)):
        c.beams(x, y)
    for x, y in ((392, 372), (128, 318), (400, 176), (330, 40), (650, 270), (708, 86)):
        c.lamp(x, y)
    for x, y in ((240, 40), (280, 44), (360, 40), (408, 46), (456, 40), (192, 50), (500, 52),
                 (560, 184), (264, 176)):
        c.ore(x, y)
    c.rails(176, 360, 412)
    c.rails_v(394, 280, 392)
    c.cart(96, 396)
    c.cart(296, 268)
    c.crates(170, 330)
    c.crates(680, 360)
    c.puddle(470, 248, 40, 24)
    for x, y in ((300, 230), (520, 270), (600, 340), (260, 90), (400, 100)):
        c.candles(x, y)
    c.crossbeam(392, 312, 64)
    c.crossbeam(300, 136, 64)
    c.secret(688, 160, 48, 88)
    m.chest(1, 708, 128, 8)

    for x, y in ((330, 430), (440, 440), (110, 370), (160, 400), (80, 420), (330, 240),
                 (400, 260), (360, 220), (540, 280), (630, 330), (680, 380), (620, 390),
                 (250, 80), (330, 90), (420, 80)):
        m.spawn('KOBOLD_TUNNELER', x, y)
    m.area(0, 0, 768, 512, 'Fargodeep Mine')
    m.area(150, 24, 360, 104, 'The Deep Vein', 'FARGODEEP')
    m.music = 'DUNGEON'
    m.save()
    return m


GENERATORS = {
    'abbey': gen_abbey,
    'inn': gen_inn,
    'elwynn': gen_elwynn,
    'westfall': gen_westfall,
    'deadmines': gen_deadmines,
    'echo_ridge': gen_echo_ridge,
    'fargodeep': gen_fargodeep,
    'stormwind': gen_stormwind,
}


def main():
    maps = {name: generate() for name, generate in GENERATORS.items()}
    ids = [chest[0] for m in maps.values() for chest in m.chests]
    if len(ids) != len(set(ids)) or max(ids) >= 64:
        raise SystemExit(f'chest ids must be unique and below 64: {sorted(ids)}')
    print(f'{len(ids)} treasure chests')
    starts = {'elwynn': 'start', 'westfall': 'from_elwynn', 'stormwind': 'from_elwynn'}
    for name, m in maps.items():
        m.check_reachable(starts.get(name, 'entry'))
    write_minimaps(maps)


# Maps with a picture on the world map page, in the order D-pad left and right go through them.
# Interiors show the map their door leads to.
MINIMAPS = ['elwynn', 'stormwind', 'westfall', 'echo_ridge', 'fargodeep', 'deadmines']


def write_minimaps(maps):
    rows = []
    for name in MINIMAPS:
        left, top, size = maps[name].save_minimap()
        rows.append(f'    {{ map_id::{name.upper()}, bn::sprite_items::minimap_{name}, {left}, {top}, {size} }},')
    out = ['// Generated by tools/gen_world.py. Do not edit by hand.',
           '// Include only from gw_menu_map.cpp: it pulls in every minimap picture.',
           '#ifndef GW_MINIMAPS_H', '#define GW_MINIMAPS_H', '']
    out += [f'#include "bn_sprite_items_minimap_{name}.h"' for name in sorted(MINIMAPS)]
    out += ['', '#include "gw_ids.h"', '', 'namespace gw', '{', '',
            '// A 128x128 picture of a map in four 64x64 frames. A world pixel (x, y) is the picture pixel',
            '// (left + x * minimap_content / size, top + y * minimap_content / size).',
            'struct minimap_def', '{', '    map_id map;', '    const bn::sprite_item& item;', '    int left;',
            '    int top;', '    int size;', '};', '', 'constexpr int minimap_picture = 128;',
            'constexpr int minimap_content = 112;', '', 'constexpr minimap_def minimaps[] = {']
    out += rows
    out += ['};', '', 'constexpr int minimap_count = sizeof(minimaps) / sizeof(minimaps[0]);', '', '}', '',
            '#endif', '']
    (wg.INCLUDE / 'gw_minimaps.h').write_text('\n'.join(out))
    for m in maps.values():
        m.write_header(maps)


if __name__ == '__main__':
    main()
