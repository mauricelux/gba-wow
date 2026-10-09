"""Generate every map in the game (backgrounds, collision, metadata headers).

Run from the tools directory:  python3 gen_world.py

Maps:
  elwynn     2048x2048 outdoor region: Northshire Valley, Goldshire, farms, lake, forests
  abbey      Northshire Abbey interior
  inn        Lion's Pride Inn interior (Goldshire)
  westfall   1024x1024 outdoor region: Sentinel Hill, farms, Moonbrook
  redridge   1280x1024 outdoor region: Lakeshire, Lake Everstill, Stonewatch Keep, Render's Valley
  deadmines  dungeon below Moonbrook
  echo_ridge kobold mine in Northshire Valley
  fargodeep  kobold mine south of Goldshire
  stormwind  1024x1024 capital: Valley of Heroes, Trade District, the Keep, Cathedral, Mage Quarter
  stockade   Stormwind's prison, entered from the Mage Quarter
  deeprun_tram  the tram between Stormwind and Ironforge
  duskwood, silverpine, shadowfang   the 20-26 chapter (M18)
  ironforge  the dwarven capital, inside the mountain
  dun_morogh the snowy valley in front of Ironforge and Gnomeregan
  wetlands   1280x1024 outdoor region: Menethil Harbor, Dun Modr, the Dragonmaw
  darkshore  Auberdine and the Zoram Strand, by boat from Menethil
  blackfathom_deeps, gnomeregan   the 24-30 dungeons
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
    # On past the mine and south through the woods to Duskwood.
    wg.corners_along(m, 'path', [(976, 1700), (1000, 1800), (1040, 1900), (1056, 2048)], 1.1)
    wg.corners_along(m, 'path', [(960, 1220), (760, 1260), (560, 1330), (340, 1420), (120, 1440),
                                (0, 1440)], 1.1)
    wg.corners_along(m, 'path', [(1090, 1220), (1300, 1240), (1500, 1270), (1650, 1240),
                                (1720, 1236), (1900, 1240), (2048, 1240)], 1.0)
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
    wg.forest(m, trees, 0, m.height - border, 1024, border)
    wg.forest(m, trees, 1088, m.height - border, m.width - 1088, border)
    wg.forest(m, trees, 0, 0, border, 928)
    wg.forest(m, trees, 0, 1008, border, 392)
    wg.forest(m, trees, 0, 1480, border, m.height - 1480)
    wg.forest(m, trees, m.width - border, 0, border, 1200)
    wg.forest(m, trees, m.width - border, 1280, border, m.height - 1280)
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

    # The road east, past Stonefield, leads to Redridge.
    m.warp(2040, 1208, 8, 64, 'redridge', 'from_elwynn')
    m.point('from_redridge', 2020, 1240)

    # The road south leads to Duskwood.
    m.warp(1032, 2040, 48, 8, 'duskwood', 'from_elwynn')
    m.point('from_duskwood', 1056, 2016)

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
    m.npc('THOR', 604, 300)
    m.point('flight', 604, 322)
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
# Redridge Mountains
# ---------------------------------------------------------------------------------------------

REDRIDGE_PROPS = ('tall_grass', 'rock', 'bush', 'fern', 'big_rock', 'flowers', 'rock', 'stump', 'tall_grass')


def border_rock(m, thickness, seed):
    """Mountains around the map: rock corners thickness metatiles deep (top, bottom, left, right),
    wobbling by a metatile or two."""
    c = wg.corners(m, 'rock')
    ny, nx = c.shape

    def wobble(n, base, amp, salt):
        points = np.random.default_rng(seed * 10 + salt).uniform(-1, 1, n // 5 + 2)
        return base + amp * np.interp(np.linspace(0, len(points) - 1, n), np.arange(len(points)), points)

    top, bottom = wobble(nx, thickness[0], 1.5, 1), wobble(nx, thickness[1], 1.5, 2)
    left, right = wobble(ny, thickness[2], 1.2, 3), wobble(ny, thickness[3], 1.5, 4)
    for cy in range(ny):
        for cx in range(nx):
            if cy < top[cx] or cy > ny - 1 - bottom[cx] or cx < left[cy] or cx > nx - 1 - right[cy]:
                c[cy, cx] = 1


def stonewatch_keep(m, x, y):
    """A walled courtyard with a tower on its north wall, gate in the south wall. 224x200.

    Returns the gate's center on the outside."""
    W, H, t = 224, 200, 24
    g = m.ground
    face = 48
    # The north wall faces the courtyard.
    wg.bricks(m, g, x, y, W, face, 'stone_d', 'stone_m', 'stone_l')
    g[y:y + 3, x:x + W] = m.g('stone_h')
    g[y + face - 1, x:x + W] = m.g('outline')
    for wx in (x + 24, x + W - 40):
        g[y + 16:y + 32, wx:wx + 16] = m.g('outline')
        g[y + 18:y + 32, wx + 2:wx + 14] = m.g('glass_d')
    for bx in (x + 56, x + W - 64):
        # Blackrock banners, black with a dull orange mark.
        g[y + 8:y + 40, bx:bx + 8] = m.g('outline')
        g[y + 8:y + 40, bx + 7] = m.g('stone_d')
        g[y + 18:y + 24, bx + 2:bx + 6] = m.g('wood_l')
        g[y + 36:y + 40, bx + 2:bx + 6] = m.g('stone_m')
    m.block(x, y, W, face)
    wg.cobbles(m, x + t, y + face, W - 2 * t, H - face - t)

    def wall_top(wx, wy, ww, wh):
        ys, xs = np.mgrid[wy:wy + wh, wx:wx + ww]
        pattern = np.full((wh, ww), m.g('stone_m'), dtype=np.uint8)
        pattern[(ys % 16 < 8) & (xs % 16 < 8)] = m.g('stone_l')
        pattern[(ys % 16 == 15) | (xs % 16 == 15)] = m.g('stone_d')
        g[wy:wy + wh, wx:wx + ww] = pattern
        m.block(wx, wy, ww, wh)

    wall_top(x, y + face, t, H - face)
    wall_top(x + W - t, y + face, t, H - face)
    g[y + face:y + H, x + t - 2:x + t] = m.g('outline')
    g[y + face:y + H, x + W - t:x + W - t + 2] = m.g('outline')
    # The south wall: its top, then its outer face, with the gate in the middle.
    gate = 48
    gx = x + W // 2 - gate // 2
    for wx, ww in ((x, gx - x), (gx + gate, x + W - gx - gate)):
        wall_top(wx, y + H - t, ww, 8)
        wg.bricks(m, g, wx, y + H - t + 8, ww, t - 8, 'stone_d', 'stone_m', 'stone_l')
        g[y + H - t + 8, wx:wx + ww] = m.g('outline')
        g[y + H - 1, wx:wx + ww] = m.g('outline')
        m.block(wx, y + H - t, ww, t)
    wg.cobbles(m, gx, y + H - t, gate, t)
    # Rubble where the orcs broke the wall.
    for rx, ry in ((x + 32, y + face + 24), (x + W - 56, y + H - t - 40)):
        g[ry:ry + 8, rx:rx + 16] = m.g('stone_d')
        g[ry + 1:ry + 7, rx + 1:rx + 15] = m.g('stone_m')
        g[ry + 2:ry + 4, rx + 3:rx + 8] = m.g('stone_l')
        m.block(rx, ry + 2, 16, 6)
    # The tower stands behind the north wall and rises over it.
    tx = x + W // 2 - 40
    wg.bricks(m, g, tx, y - 40, 80, face + 40, 'stone_d', 'stone_m', 'stone_l')
    g[y - 40:y + face, tx] = m.g('outline')
    g[y - 40:y + face, tx + 79] = m.g('outline')
    wg.window(m, tx + 36, y - 24)
    wg.door(m, tx + 32, y + face - 24)
    wg.roof(m, tx - 8, y - 88, 96, 56, ('roof_d', 'roof_m', 'roof_l'), 'roof_h', 'outline')
    m.block(tx, y - 40, 80, face + 40)
    return (x + W // 2, y + H + 8)


def broken_bridge(m, x, y0, y1, gap):
    """The Everstill bridge: planks from both shores, its middle span gone into the lake."""
    mid = (y0 + y1) // 2
    wg.pier(m, x, y0, 32, mid - gap // 2 - y0)
    south_y = mid + gap // 2
    for py in range(south_y, y1):
        for px in range(x, x + 32):
            c = m.g('trunk_m') if (py % 8) not in (0, 7) else m.g('trunk_d')
            if px in (x, x + 1, x + 30, x + 31):
                c = m.g('trunk_d')
            m.ground[py, px] = c
    m.unblock(x + 4, south_y + 4, 24, y1 - south_y - 4)
    m.block(x, south_y, 32, 4)
    m.block(x, south_y, 4, y1 - south_y)
    m.block(x + 28, south_y, 4, y1 - south_y)
    # Broken plank ends.
    for py, sign in ((mid - gap // 2 - 1, 1), (south_y, -1)):
        for px in range(x + 2, x + 30, 5):
            m.ground[py:py + 2, px:px + 2] = m.g('shadow')


def gen_redridge():
    m = Map('redridge', 1280, 1024,
            Palette([wg.TERRAIN_REDRIDGE, wg.BUILDINGS, wg.FARM, wg.ROCK]),
            Palette([wg.OVERHEAD_LEAVES, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    # --- mountains ----------------------------------------------------------------------------
    border_rock(m, (6, 5, 4, 5), seed=3)
    rock = wg.corners(m, 'rock')
    # The pass west to Elwynn (Three Corners) and the Lakeridge Highway south to Duskwood.
    rock[41:47, 0:8] = 0
    rock[57:, 26:31] = 0
    # Ridges between the valleys.
    wg.corners_along(m, 'rock', [(300, 40), (296, 170), (270, 250)], 2.2)
    wg.corners_along(m, 'rock', [(920, 40), (912, 150), (880, 210)], 2.0)
    wg.corners_along(m, 'rock', [(720, 1024), (712, 900), (690, 840)], 2.2)
    wg.corners_along(m, 'rock', [(1180, 640), (1100, 680)], 1.8)
    wg.paint_cliffs(m)

    # --- roads ----------------------------------------------------------------------------------
    roads = [
        [(0, 704), (160, 700), (300, 690), (400, 668)],             # Three Corners to the broken bridge
        [(160, 700), (130, 620), (150, 520), (230, 450), (330, 420), (420, 400)],  # around the lake
        [(420, 400), (600, 392), (760, 400), (900, 380), (1000, 372), (1096, 360), (1096, 344)],  # Stonewatch
        [(400, 668), (430, 760), (440, 880), (440, 1024)],          # Lakeridge Highway
        [(430, 760), (600, 720), (800, 740), (940, 790)],           # to Render's Valley
        [(600, 392), (640, 300), (700, 220)],                       # up to Alther's Mill
    ]
    for points in roads:
        wg.corners_along(m, 'path', points, 1.1)
    wg.paint_paths(m)

    # --- Lake Everstill -------------------------------------------------------------------------
    wg.corners_along(m, 'water', [(250, 584), (420, 584), (600, 572), (720, 556)], 3.3)
    wg.corners_ellipse(m, 'water', 900, 548, 250, 84)
    wg.corners_along(m, 'water', [(960, 160), (960, 300), (940, 420), (930, 480)], 0.9)
    wg.paint_water(m)
    wg.bridge(m, 936, 352, 48, 40)
    broken_bridge(m, 384, 504, 664, 48)
    wg.pier(m, 528, 496, 32, 72)
    m.area(180, 470, 980, 220, 'Lake Everstill', 'LAKE_EVERSTILL')
    m.area(900, 140, 120, 300, 'Stonewatch Falls')

    # --- Lakeshire ------------------------------------------------------------------------------
    wg.cobbles(m, 432, 376, 176, 48)
    hall = wg.house(m, 424, 248, 128, 128, style='stone', roof_colors=('red_d', 'red_m', 'red_l'),
                    roof_ridge='red_l', roof_outline='o2')
    inn = wg.house(m, 568, 264, 112, 112, roof_colors=('red_d', 'red_m', 'red_l'), door_x=40)
    wg.house(m, 320, 288, 88, 96, roof_colors=('red_d', 'red_m', 'red_l'))
    wg.house(m, 640, 408, 80, 80, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'), roof_ridge='thatch_l')
    wg.house(m, 248, 392, 72, 80, roof_colors=('red_d', 'red_m', 'red_l'))
    wg.well(m, 568, 384)
    wg.crop_field(m, 344, 448, 32, 32)
    wg.fence(m, 336, 440, 48)
    m.npc('SOLOMON', hall[0] + 20, hall[1] + 10)
    m.npc('MARRIS', 470, 412)
    m.npc('BRIANNA', inn[0] + 18, inn[1] + 8)
    m.npc('BREANNA', inn[0] - 18, inn[1] + 8)
    m.npc('KAREN', 400, 420)
    m.npc('OSLOW', 400, 496)
    m.npc('BAREN', 496, 494)
    m.npc('BRAY', 580, 496)
    m.npc('OSGOOD', 392, 470)
    m.npc('BERTON', 336, 500)
    m.npc('ARIENA', 712, 352)
    m.point('flight', 712, 374)
    m.point('lakeshire_respawn', 500, 440)
    m.area(232, 232, 520, 290, 'Lakeshire')

    # --- Redridge Canyons (gnolls, Ribchaser) --------------------------------------------------
    m.spawn_group('REDRIDGE_MONGREL', 170, 230, 6, 80, seed=31)
    m.spawn_group('REDRIDGE_MONGREL', 130, 330, 4, 40, seed=32)
    m.spawn('RIBCHASER', 120, 150)
    wg.camp(m, 112, 168, 96, 64, tents=[(120, 176)], fire=(176, 200))
    m.chest(18, 236, 128, 18)
    m.area(64, 96, 220, 340, 'Redridge Canyons')

    # --- Alther's Mill (Shadowhide gnolls) ----------------------------------------------------
    wg.house(m, 664, 120, 96, 96, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'), roof_ridge='thatch_l')
    wg.camp(m, 784, 160, 96, 80, tents=[(792, 168), (840, 168)], fire=(824, 216))
    m.spawn_group('SHADOWHIDE_GNOLL', 810, 280, 6, 60, seed=33)
    m.spawn_group('SHADOWHIDE_MYSTIC', 828, 214, 4, 30, seed=34)
    m.spawn_group('TARANTULA', 520, 170, 5, 70, seed=35)
    m.area(600, 96, 300, 200, "Alther's Mill")

    # --- Stonewatch Keep (Blackrock orcs, Gath'Ilzogg) ------------------------------------------
    stonewatch_keep(m, 984, 136)
    m.spawn('GATH_ILZOGG', 1096, 212)
    # Placed one by one, a little more than social range apart, so a pull brings one or two orcs.
    for x, y in ((1028, 204), (1164, 204), (1176, 252), (1030, 300), (1162, 300), (1064, 400), (1132, 404),
                 (904, 296)):
        m.spawn('BLACKROCK_RENEGADE', x, y)
    for x, y in ((1060, 268), (1132, 268), (1016, 252), (1010, 416), (1068, 452), (912, 236)):
        m.spawn('BLACKROCK_SUMMONER', x, y)
    m.spawn_group('TARANTULA', 330, 150, 3, 40, seed=40)
    # A pine grove by the keep hides a clearing, reached under the branches from the lake shore.
    wg.forest(m, trees, 1112, 392, 80, 96, kinds=('pine',), holes=[(1128, 408, 48, 40)],
              secrets=[(1136, 448, 24, 56)])
    m.chest(19, 1152, 436, 19)
    m.area(970, 80, 260, 360, 'Stonewatch Keep')

    # --- Render's Valley (the Blackrock camp) ---------------------------------------------------
    wg.camp(m, 896, 744, 192, 128, tents=[(904, 752), (952, 752), (1032, 752), (1000, 816)], fire=(968, 808))
    wg.crates(m, 1048, 832)
    m.spawn_group('BLACKROCK_GRUNT', 980, 820, 7, 120, seed=41)
    m.spawn_group('BLACKROCK_SHADOWCASTER', 1040, 790, 4, 60, seed=42)
    m.chest(20, 1176, 900, 18)
    m.area(760, 680, 460, 280, "Render's Valley")

    # --- the hills south of the lake -------------------------------------------------------------
    m.spawn_group('BLACKROCK_OUTRUNNER', 600, 800, 6, 90, seed=43)
    m.spawn_group('BLACKROCK_OUTRUNNER', 820, 680, 3, 40, seed=44)
    m.spawn_group('GREAT_GORETUSK', 220, 830, 6, 90, seed=45)
    m.spawn_group('GREAT_GORETUSK', 560, 900, 3, 50, seed=46)
    m.spawn('BELLYGRUB', 300, 920)
    m.spawn_group('MURLOC_FLESHEATER', 620, 664, 5, 50, seed=47)
    m.spawn_group('MURLOC_FLESHEATER', 1130, 560, 4, 40, seed=48)
    m.spawn_group('MURLOC_FLESHEATER', 820, 462, 3, 30, seed=49)
    m.area(64, 720, 380, 260, 'Three Corners')
    m.area(400, 740, 120, 284, 'Lakeridge Highway')

    # The Lakeridge Highway leads south to Duskwood, watched by Lakeshire's guard.
    m.npc('GUARD_LAKERIDGE', 472, 944)
    m.warp(416, 1016, 64, 8, 'duskwood', 'from_redridge')
    m.point('from_duskwood', 440, 992)

    rng = np.random.default_rng(17)
    path = wg.corners(m, 'path')
    water = wg.corners(m, 'water')
    placed = 0
    for _ in range(400):
        if placed >= 80:
            break
        x, y = int(rng.uniform(60, 1200)), int(rng.uniform(80, 960))
        mx, my = (x + 16) // 16, (y + 40) // 16
        near = path[max(0, my - 3):my + 4, max(0, mx - 3):mx + 4].max() + \
            water[max(0, my - 3):my + 4, max(0, mx - 3):mx + 4].max()
        clear = [(200, 200, 580, 340), (960, 40, 260, 340), (100, 150, 120, 100), (770, 140, 130, 120),
                 (880, 720, 230, 170)]
        if near == 0 and m.area_free(x, y, 40, 56) and \
                not any(cx - 40 <= x <= cx + cw and cy - 56 <= y <= cy + ch for cx, cy, cw, ch in clear):
            wg.tree(m, trees, x, y, int(rng.integers(0, 2)), kind=('pine', 'oak', 'pine', 'birch')[placed % 4])
            placed += 1
    wg.scatter_props(m, rng, 70, REDRIDGE_PROPS, (64, 64, 1150, 900),
                     avoid=[(200, 200, 580, 340), (960, 40, 260, 340), (880, 720, 230, 170)])

    m.point('from_elwynn', 24, 704)
    m.warp(0, 672, 8, 64, 'elwynn', 'from_redridge')
    m.area(0, 0, 1280, 1024, 'Redridge Mountains')
    m.music = 'REDRIDGE'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Duskwood
# ---------------------------------------------------------------------------------------------

DUSKWOOD_PROPS = ('fern', 'tall_grass', 'rock', 'bush', 'stump', 'log', 'fern', 'big_rock', 'tall_grass')


def gravestone(m, x, y, kind=0):
    """A gravestone on the grass, 16x16, drawn in the rock bank: harmonize_rock turns the grass
    around it into the bank's twins. kind 0 is a headstone, 1 a cross, 2 a slab on the ground."""
    x, y, t = wg._prop(m, x, y, 16, 16)
    o, d, mid, lit, hi = m.g('rock_0'), m.g('rock_1'), m.g('rock_2'), m.g('rock_3'), m.g('rock_4')
    if kind == 0:
        t[4:14, 4:12] = mid
        t[3, 5:11] = mid
        t[2, 6:10] = mid
        t[4:13, 4] = lit
        t[3, 5] = lit
        t[4:14, 11] = d
        t[6, 6:10] = d
        t[8, 6:10] = d
        t[1, 6:10] = o
        t[2, 5] = t[2, 10] = o
        t[3, 4] = t[3, 11] = o
        t[4:14, 3] = o
        t[4:14, 12] = o
        t[14, 3:13] = d
        t[15, 2:14] = m.g('shadow')
        m.block(x + 4, y + 8, 8, 8)
    elif kind == 1:
        t[2:14, 7:9] = mid
        t[5:7, 4:12] = mid
        t[2:14, 7] = lit
        t[5, 4:12] = hi
        t[1, 7:9] = o
        t[2:5, 6] = t[2:5, 9] = o
        t[7:14, 6] = t[7:14, 9] = o
        t[4, 3:7] = t[4, 9:13] = o
        t[7, 3:7] = t[7, 9:13] = o
        t[5:7, 3] = t[5:7, 12] = o
        t[14, 4:12] = d
        t[15, 4:12] = m.g('shadow')
        m.block(x + 6, y + 10, 4, 6)
    else:
        t[7:14, 2:14] = mid
        t[7:9, 2:14] = lit
        t[13, 2:14] = d
        t[6, 2:14] = o
        t[14, 2:14] = o
        t[6:15, 1] = t[6:15, 14] = o
        t[9:12, 7:9] = hi
        t[10, 5:11] = hi
        t[15, 2:15] = m.g('shadow')
        m.block(x + 2, y + 8, 12, 6)


def graveyard(m, x, y, w, h, seed, gaps=()):
    """Rows of gravestones every 32x32 px inside the rectangle, a few missing; gaps are rectangles left
    clear for paths and the dead."""
    local = np.random.default_rng(seed)
    for gy in range(y, y + h - 15, 32):
        for gx in range(x + (gy // 32 % 2) * 16, x + w - 15, 32):
            if local.random() < 0.2 or any(wg.overlaps((gx, gy, 16, 16), g) for g in gaps):
                continue
            gravestone(m, gx, gy, int(local.choice([0, 0, 0, 1, 1, 2])))


def ns_bridge(m, x, y, w, h):
    """A plank bridge running north-south over a river: planks across, rails along both sides."""
    for py in range(y, y + h):
        for px in range(x, x + w):
            c = m.g('trunk_m') if (py % 8) not in (0, 7) else m.g('trunk_d')
            if px in (x, x + 1, x + w - 2, x + w - 1):
                c = m.g('trunk_d')
            m.ground[py, px] = c
    for py in range(y + 4, y + h, 16):
        m.ground[py:py + 4, x:x + 3] = m.g('shadow')
        m.ground[py:py + 4, x + w - 3:x + w] = m.g('shadow')
    m.unblock(x + 4, y, w - 8, h)
    m.block(x, y, 4, h)
    m.block(x + w - 4, y, 4, h)


def crypt(m, x, y, w=80, h=80):
    """A stone crypt with a slate roof and its door broken open. Returns the doorway's bottom-center."""
    door = wg.house(m, x, y, w, h, style='stone', roof_colors=('roof_d', 'roof_m', 'roof_l'),
                    roof_ridge='roof_h', roof_outline='outline', windows=False)
    g = m.ground
    dx = door[0] - 8
    dy = y + h - 24
    g[dy + 2:dy + 24, dx + 2:dx + 14] = m.g('outline')
    g[dy:dy + 2, dx - 2:dx + 18] = m.g('stone_h')
    for bx in (x + 8, x + w - 16):
        g[y + h // 2 + 8:y + h - 4, bx:bx + 8] = m.g('stone_l')
        g[y + h // 2 + 8:y + h - 4, bx + 7] = m.g('stone_d')
    return door


def dusk_lamp(m, x, y):
    """A Darkshire lamp post, 8x32, on cobbles (building bank)."""
    g = m.ground
    g[y + 8:y + 30, x + 3:x + 5] = m.g('outline')
    g[y + 29:y + 31, x + 1:x + 7] = m.g('stone_d')
    g[y:y + 9, x + 1:x + 7] = m.g('outline')
    g[y + 2:y + 7, x + 2:x + 6] = m.g('glass_l')
    g[y, x + 3:x + 5] = m.g('stone_d')
    m.block(x + 1, y + 24, 6, 8)


def gen_duskwood():
    m = Map('duskwood', 1536, 1024,
            Palette([wg.TERRAIN_DUSKWOOD, wg.BUILDINGS_DUSK, wg.FARM, wg.ROCK_DUSK]),
            Palette([wg.OVERHEAD_LEAVES_DUSK, wg.OVERHEAD_ROOFS_DUSK]))
    wg.fill_grass(m)
    trees = wg.Trees(m)
    dead = wg.Trees(m, dead=True)

    # --- mountains: Redridge's foothills to the north-east, Deadwind's to the east and south-east ----
    rock = wg.corners(m, 'rock')
    rock[0:10, 70:] = 1
    rock[:, 93:] = 1
    rock[59:, 66:] = 1
    rock[0:10, 81:86] = 0                   # the pass north to Redridge
    wg.corners_along(m, 'rock', [(1536, 700), (1504, 760), (1480, 860), (1420, 960)], 1.6)
    wg.corners_ellipse(m, 'rock', 824, 792, 104, 52)    # Vul'Gol Ogre Mound
    wg.paint_cliffs(m)

    # --- roads -------------------------------------------------------------------------------------
    roads = [
        [(568, 0), (568, 170), (588, 270), (616, 400), (640, 548)],                     # north to Elwynn
        [(120, 616), (260, 600), (420, 580), (640, 548), (840, 530), (1000, 500), (1096, 470),
         (1200, 464)],                                                                   # Raven Hill Road
        [(1336, 0), (1332, 100), (1290, 170), (1212, 250), (1208, 340), (1208, 440)],   # north to Redridge
        [(1228, 496), (1228, 660), (1232, 748)],                                         # to Tranquil Gardens
        [(840, 530), (836, 640), (820, 740), (820, 880)],                               # to Vul'Gol
        [(260, 600), (280, 690), (300, 760)],                                            # Raven Hill
        [(252, 598), (232, 544), (224, 500)],                                            # cemetery gate
        [(120, 616), (112, 700), (120, 790)],                                            # Sven's camp
        [(420, 580), (440, 700), (470, 860)],                                            # Yorgen Farmstead
        [(1000, 500), (1020, 380), (1040, 300)],                                         # Mistmantle Manor
    ]
    for points in roads:
        wg.corners_along(m, 'path', points, 1.1)
    wg.paint_paths(m)

    # --- the Darkened Bank: the river along the north --------------------------------------------
    wg.corners_along(m, 'water', [(0, 96), (300, 104), (560, 96), (820, 108), (1060, 100), (1130, 112)], 1.5)
    wg.paint_water(m)
    ns_bridge(m, 552, 56, 32, 88)
    m.area(0, 56, 1140, 96, 'The Darkened Bank')

    # --- forest borders ---------------------------------------------------------------------------
    border = 48
    wg.forest(m, trees, 0, 0, 544, border)
    wg.forest(m, trees, 592, 0, 528, border)
    wg.forest(m, trees, 0, 0, border, m.height)
    wg.forest(m, trees, 0, m.height - border, 1056, border)
    # Old woods between the zones, with hidden clearings.
    wg.forest(m, trees, 48, 144, 224, 112, kinds=('pine', 'oak'), holes=[(128, 168, 64, 48)],
              secrets=[(144, 208, 32, 56)])
    m.chest(21, 160, 196, 22)
    wg.forest(m, trees, 1040, 600, 48, 160, kinds=('pine', 'oak'))
    wg.forest(m, trees, 560, 880, 160, 96, kinds=('pine', 'oak'), holes=[(600, 904, 64, 40)],
              secrets=[(616, 864, 32, 48)])
    m.chest(22, 632, 932, 23)
    # The deep woods between the roads.
    wg.forest(m, trees, 872, 296, 112, 144, kinds=('pine', 'oak'))
    wg.forest(m, trees, 560, 612, 112, 96, kinds=('oak', 'pine'))
    wg.forest(m, trees, 880, 576, 192, 160, kinds=('pine', 'oak'), holes=[(864, 600, 176, 112)])
    wg.forest(m, trees, 280, 520, 88, 48, kinds=('oak', 'small'))

    # --- Darkshire ----------------------------------------------------------------------------------
    wg.cobbles(m, 1192, 432, 256, 64)
    smithy = wg.house(m, 1112, 344, 80, 80, style='stone', roof_colors=('roof_d', 'roof_m', 'roof_l'),
                      roof_ridge='roof_h', roof_outline='outline')
    hall = wg.house(m, 1224, 296, 128, 128, style='stone', roof_colors=('red_d', 'red_m', 'red_l'),
                    roof_ridge='red_l', roof_outline='o2')
    inn = wg.house(m, 1368, 320, 112, 112, roof_colors=('red_d', 'red_m', 'red_l'), roof_ridge='red_l',
                   roof_outline='o2', door_x=40)
    eva = wg.house(m, 1128, 528, 80, 80, roof_colors=('roof_d', 'roof_m', 'roof_l'), roof_ridge='roof_h',
                   roof_outline='outline')
    sirra = wg.house(m, 1248, 528, 80, 80, roof_colors=('red_d', 'red_m', 'red_l'), roof_ridge='red_l',
                     roof_outline='o2')
    barracks = wg.house(m, 1352, 520, 112, 96, style='stone', roof_colors=('roof_d', 'roof_m', 'roof_l'),
                        roof_ridge='roof_h', roof_outline='outline')
    wg.anvil(m, 1200, 440)
    for lx in (1192, 1336, 1440):
        dusk_lamp(m, lx, 432)
    # The town gate: a palisade along the west side, guarded by the Night Watch.
    wg.fence(m, 1096, 280, 160, vertical=True)
    wg.fence(m, 1096, 496, 176, vertical=True)
    m.npc('ELLO', hall[0] + 14, hall[1] + 12)
    m.npc('TRELAYNE', inn[0] - 18, inn[1] + 10)
    m.npc('CALOR', smithy[0] + 16, smithy[1] + 10)
    m.npc('EVA', eva[0] + 20, eva[1] + 10)
    m.npc('SIRRA', sirra[0] + 20, sirra[1] + 10)
    m.npc('ALTHEA', barracks[0] - 20, barracks[1] + 10)
    m.npc('FELICIA', 1440, 476)
    m.point('flight', 1440, 496)
    m.npc('NIGHT_WATCH_GATE', 1116, 452)
    m.npc('NIGHT_WATCH_SQUARE', 1328, 476)
    m.npc('DARKSHIRE_VENDOR', 1376, 456)
    m.point('darkshire_respawn', 1300, 480)
    m.area(1096, 260, 400, 420, 'Darkshire')

    # --- Mistmantle Manor (Nightbane worgen, Stalvan) ---------------------------------------------
    wg.house(m, 976, 152, 112, 96, style='stone', roof_colors=('dead_0', 'dead_1', 'dead_2'),
             roof_ridge='dead_3', roof_outline='o2')
    wg.grove(m, dead, [(920, 168), (1104, 176), (936, 264), (1096, 280)], seed=2, kinds=('oak', 'small'))
    m.spawn('STALVAN_MISTMANTLE', 1040, 284)
    m.spawn_group('NIGHTBANE_TAINTED_ONE', 1010, 320, 5, 60, seed=51)
    m.area(900, 140, 220, 220, 'Mistmantle Manor', 'MISTMANTLE_MANOR')

    # --- Brightwood Grove (Nightbane worgen) --------------------------------------------------------
    wg.forest(m, trees, 352, 152, 64, 96, kinds=('pine', 'oak'))
    wg.forest(m, trees, 688, 152, 96, 64, kinds=('pine', 'oak'))
    wg.camp(m, 448, 192, 96, 64, tents=[], fire=(480, 216))
    m.spawn_group('NIGHTBANE_DARK_RUNNER', 480, 260, 6, 90, seed=52)
    m.spawn_group('NIGHTBANE_SHADOW_WEAVER', 520, 200, 4, 60, seed=53)
    m.area(340, 140, 460, 220, 'Brightwood Grove')

    # --- Twilight Grove (spiders) -----------------------------------------------------------------
    # A ring of old trees around a clearing, open to the road on its south side.
    wg.forest(m, trees, 664, 280, 192, 176, kinds=('oak', 'pine'), holes=[(704, 320, 112, 104), (736, 424, 48, 40)])
    wg.big_rock(m, 744, 352)
    m.spawn_group('VENOM_WEB_SPIDER', 760, 370, 4, 36, seed=54)
    m.spawn_group('VENOM_WEB_SPIDER', 900, 400, 4, 60, seed=55)
    m.area(650, 260, 300, 220, 'Twilight Grove')

    # --- Raven Hill Cemetery (skeletons, Mor'Ladim) -----------------------------------------------
    wg.fence(m, 80, 288, 304)
    wg.fence(m, 80, 288, 216, vertical=True)
    wg.fence(m, 376, 288, 216, vertical=True)
    wg.fence(m, 80, 496, 112)
    wg.fence(m, 256, 496, 128)
    crypt(m, 192, 296, 80, 64)
    graveyard(m, 104, 360, 264, 128, seed=61, gaps=[(208, 360, 48, 140)])
    wg.grove(m, dead, [(96, 296), (320, 296), (104, 440)], seed=4, kinds=('oak', 'small'))
    m.spawn_group('SKELETAL_WARRIOR', 236, 420, 7, 90, seed=56)
    m.spawn_group('SKELETAL_MAGE', 236, 390, 4, 70, seed=57)
    m.spawn('MOR_LADIM', 232, 380)
    m.area(80, 280, 304, 224, 'Raven Hill Cemetery')

    # --- Raven Hill (ghouls) ---------------------------------------------------------------------------
    wg.house(m, 192, 640, 80, 80, roof_colors=('dead_0', 'dead_1', 'dead_2'), roof_ridge='dead_3',
             roof_outline='o2')
    wg.house(m, 320, 664, 80, 80, style='stone', roof_colors=('dead_0', 'dead_1', 'dead_2'),
             roof_ridge='dead_3', roof_outline='o2')
    wg.house(m, 224, 776, 80, 80, roof_colors=('dead_0', 'dead_1', 'dead_2'), roof_ridge='dead_3',
             roof_outline='o2')
    m.chest(23, 360, 758, 22)
    m.spawn_group('ROTTING_GHOUL', 300, 740, 7, 100, seed=58)
    m.area(170, 620, 260, 260, 'Raven Hill')

    # --- Sven's camp -----------------------------------------------------------------------------------
    wg.camp(m, 64, 768, 112, 80, tents=[(72, 776)], fire=(128, 808))
    m.npc('SVEN', 152, 788)
    m.point('raven_hill_respawn', 116, 836)
    m.area(56, 750, 140, 110, "Sven's Camp")

    # --- Abercrombie's hut, where Stitches begins his walk to Darkshire ------------------------------
    hut = wg.house(m, 472, 616, 72, 80, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'), roof_ridge='thatch_l')
    m.npc('ABERCROMBIE', hut[0] + 16, hut[1] + 8)
    m.spawn('STITCHES', 536, 720)
    m.patrol = [(536, 720), (448, 722), (444, 600), (520, 574), (700, 545), (860, 528), (1000, 500), (1072, 472)]
    m.area(450, 600, 130, 140, "Addle's Stead")

    # --- Yorgen Farmstead (rabid wolves) -----------------------------------------------------------------
    wg.house(m, 392, 824, 96, 96, roof_colors=('dead_0', 'dead_1', 'dead_2'), roof_ridge='dead_3',
             roof_outline='o2')
    wg.crop_field(m, 504, 856, 64, 72)
    wg.fence(m, 496, 848, 80)
    m.spawn_group('RABID_DIRE_WOLF', 470, 950, 4, 40, seed=59)
    m.spawn_group('RABID_DIRE_WOLF', 600, 820, 3, 40, seed=60)
    m.area(380, 800, 260, 180, 'Yorgen Farmstead')

    # --- Vul'Gol Ogre Mound ------------------------------------------------------------------------------
    cave = wg.mine_entrance(m, 792, 808, 64, 48)
    wg.camp(m, 736, 872, 176, 80, tents=[(744, 880), (872, 880)], fire=(808, 912))
    m.spawn_group('SPLINTER_FIST_OGRE', 820, 920, 6, 80, seed=62)
    m.spawn_group('SPLINTER_FIST_TASKMASTER', 824, 880, 3, 40, seed=63)
    m.chest(24, cave[0], cave[1] - 2, 24)
    m.area(700, 720, 260, 260, "Vul'Gol Ogre Mound")

    # --- Tranquil Gardens Cemetery (Morbent Fel) ---------------------------------------------------------
    wg.fence(m, 1104, 744, 112)
    wg.fence(m, 1256, 744, 120)
    wg.fence(m, 1104, 744, 208, vertical=True)
    door = crypt(m, 1272, 760, 80, 80)
    graveyard(m, 1128, 776, 232, 160, seed=64, gaps=[(1208, 744, 56, 220), (1264, 744, 96, 120)])
    wg.grove(m, dead, [(1112, 896), (1352, 880)], seed=6, kinds=('small', 'oak'))
    m.spawn('MORBENT_FEL', door[0], door[1] + 18)
    m.spawn_group('PLAGUE_SPREADER', 1180, 860, 6, 80, seed=65)
    m.area(1100, 730, 290, 240, 'Tranquil Gardens Cemetery')

    # --- the wolves of the woods --------------------------------------------------------------------------
    m.spawn_group('DIRE_WOLF', 950, 656, 6, 60, seed=66)
    m.spawn_group('DIRE_WOLF', 420, 470, 4, 50, seed=67)
    m.spawn_group('DIRE_WOLF', 640, 700, 4, 60, seed=68)

    rng = np.random.default_rng(23)
    path = wg.corners(m, 'path')
    water = wg.corners(m, 'water')
    clear = [(1080, 260, 420, 420), (80, 280, 304, 224), (170, 620, 260, 260), (56, 750, 140, 110),
             (450, 600, 130, 140), (380, 800, 260, 180), (700, 720, 260, 260), (1100, 730, 290, 240),
             (900, 140, 220, 220), (440, 180, 120, 100), (690, 300, 140, 140)]
    placed = 0
    for i in range(3000):
        if placed >= 400:
            break
        x, y = int(rng.uniform(40, 1460)), int(rng.uniform(120, 960))
        mx, my = (x + 16) // 16, (y + 40) // 16
        near = path[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max() + \
            water[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max()
        if near == 0 and m.area_free(x, y, 40, 56) and \
                not any(cx - 40 <= x <= cx + cw and cy - 56 <= y <= cy + ch for cx, cy, cw, ch in clear):
            if placed % 7 == 3:
                wg.tree(m, dead, x, y, int(rng.integers(0, 3)), kind='small')
            else:
                wg.tree(m, trees, x, y, int(rng.integers(0, 3)), kind=('oak', 'pine', 'oak', 'small')[placed % 4])
            placed += 1
    wg.scatter_props(m, rng, 90, DUSKWOOD_PROPS, (56, 120, 1400, 840),
                     avoid=[(1080, 260, 420, 420), (80, 280, 304, 224)])

    m.point('from_elwynn', 568, 24)
    m.warp(544, 0, 48, 8, 'elwynn', 'from_duskwood')
    m.point('from_redridge', 1336, 24)
    m.warp(1312, 0, 48, 8, 'redridge', 'from_duskwood')
    m.area(0, 0, 1536, 1024, 'Duskwood')
    m.music = 'DUSKWOOD'
    m.night = True
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


def stockade_house(m, x, y):
    """The Stockade's gatehouse: a squat stone block with barred windows, an iron gate and a slate
    roof. 144x96. Returns the gate's bottom-center point."""
    W, H = 144, 96
    g = m.ground
    wall_y = y + 48
    wg.bricks(m, g, x, wall_y, W, H - 48, 'stone_d', 'stone_m', 'stone_l')
    g[wall_y:wall_y + 2, x:x + W] = m.g('stone_h')
    g[y + H - 2:y + H, x:x + W] = m.g('stone_d')
    g[wall_y:y + H, x] = m.g('outline')
    g[wall_y:y + H, x + W - 1] = m.g('outline')
    for wx in (x + 16, x + 40, x + 96, x + 120):
        g[wall_y + 10:wall_y + 22, wx:wx + 8] = m.g('outline')
        g[wall_y + 11:wall_y + 21, wx + 1:wx + 7] = m.g('glass_d')
        g[wall_y + 11:wall_y + 21, wx + 2:wx + 7:2] = m.g('stone_l')
    cx = x + W // 2
    g[wall_y + 8:y + H, cx - 16:cx + 16] = m.g('outline')
    g[wall_y + 10:y + H, cx - 14:cx + 14] = m.g('glass_d')
    for px in range(cx - 13, cx + 14, 4):
        g[wall_y + 10:y + H - 10, px] = m.g('stone_l')
    for py in range(wall_y + 13, y + H - 10, 6):
        g[py, cx - 14:cx + 14] = m.g('stone_l')
    g[y + H - 11, cx - 14:cx + 14] = m.g('stone_d')
    g[wall_y + 6:wall_y + 10, cx - 20:cx + 20] = m.g('stone_h')
    for bx in (cx - 32, cx + 24):
        g[wall_y + 8:wall_y + 36, bx:bx + 8] = m.g('banner')
        g[wall_y + 8:wall_y + 36, bx] = m.g('banner_d')
        g[wall_y + 8:wall_y + 10, bx:bx + 8] = m.g('gold')
        g[wall_y + 18:wall_y + 24, bx + 2:bx + 6] = m.g('gold')
    wg.roof(m, x, y, W, 48, SLATE['roof_colors'], SLATE['roof_ridge'], SLATE['roof_outline'])
    m.block(x, y + 16, W, H - 16)
    m.unblock(cx - 12, y + H - 16, 24, 16)
    return (cx, y + H)


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
    m.npc('DUNGAR', 808, 440)
    m.point('flight', 808, 462)
    m.npc('RANDAL', 808, 600)
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
    gate = stockade_house(m, 40, 872)
    m.warp(gate[0] - 12, gate[1] - 8, 24, 8, 'stockade', 'entry')
    m.point('stockade_exit', gate[0], gate[1] + 12)
    m.npc('THELWATER', gate[0] + 32, gate[1] + 8)
    m.npc('SW_GUARD_STOCKADE', gate[0] - 32, gate[1] + 8)
    m.area(32, 856, 176, 136, 'The Stockade')
    wg.house(m, 552, 776, 88, 96, style='stone', **PURPLE)
    wg.house(m, 552, 888, 88, 80, style='timber', **PURPLE)
    wg.grove(m, trees, [(240, 912), (400, 904), (512, 920), (272, 880)], seed=2,
             kinds=('birch', 'oak', 'pine'))
    m.chest(9, 632, 984, 9)
    m.area(32, 704, 624, 288, 'Mage Quarter')

    # --- Dwarven District -------------------------------------------------------------------------------
    forge_door = wg.house(m, 760, 48, 160, 128, style='stone', door_x=64, **SLATE)
    for ax in (776, 808, 872, 904):
        wg.anvil(m, ax, 196)
    m.npc('EINRIS', 950, 210)
    m.npc('BRANN', forge_door[0] - 40, forge_door[1] + 4)
    # The Deeprun Tram's station house: stairs down to the tram to Ironforge.
    tram_door = wg.house(m, 744, 296, 104, 96, style='stone', **SLATE)
    m.warp(tram_door[0] - 12, tram_door[1] - 8, 24, 8, 'deeprun_tram', 'entry')
    m.point('tram_exit', tram_door[0], tram_door[1] + 12)
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


# ---------------------------------------------------------------------------------------------
# The Stockade: Stormwind's prison, in the Mage Quarter
# ---------------------------------------------------------------------------------------------

PRISON = [
    ('outline', (16, 14, 20)), ('top_d', (30, 30, 40)), ('top_m', (46, 46, 60)),
    ('wall_d', (58, 58, 72)), ('wall_m', (88, 88, 104)), ('wall_l', (124, 124, 140)),
    ('floor_d', (72, 68, 68)), ('floor_m', (100, 96, 92)), ('floor_l', (128, 124, 116)),
    ('iron_d', (36, 40, 48)), ('iron_l', (152, 156, 172)), ('straw', (176, 144, 64)),
    ('wood', (112, 74, 44)), ('flame', (248, 196, 80)), ('red', (152, 36, 40)),
]

PRISON_OVERHEAD = [
    ('outline', (16, 14, 20)), ('top_d', (30, 30, 40)), ('top_m', (46, 46, 60)),
    ('wall_d', (58, 58, 72)), ('wall_m', (88, 88, 104)), ('wall_l', (124, 124, 140)),
]


class Prison(Cave):
    """Stone corridors and cells. Like a cave, the floor is one flag per 8x8 cell, but walls are
    brick faces 24 px tall and the floor is flagstones. Cells line the north side of corridors:
    their bars stand where the corridor's wall face would be."""

    FACE = 3

    def __init__(self, name, width, height):
        self.m = Map(name, width, height, Palette([PRISON]), Palette([PRISON_OVERHEAD]))
        self.floor = np.zeros((height // 8, width // 8), dtype=bool)
        self.face = np.zeros_like(self.floor)

    def _top(self, layer, x, y, w, h, pal):
        """The tops of the walls: big dark blocks."""
        ys, xs = np.mgrid[y:y + h, x:x + w]
        pattern = np.full((h, w), pal('top_m'), dtype=np.uint8)
        pattern[(ys % 16 == 15) | ((xs + (ys // 16 % 2) * 8) % 16 == 15)] = pal('top_d')
        layer[y:y + h, x:x + w] = pattern

    def _face(self, layer, x, y, w, row, pal):
        """One 8 px band of brick face; row 0 is the top band, FACE - 1 the bottom one."""
        wg.bricks(self.m, layer, x, y, w, 8, 'wall_d', 'wall_m', 'wall_l', palette=pal)
        if row == 0:
            layer[y, x:x + w] = pal('outline')
        if row == self.FACE - 1:
            layer[y + 6, x:x + w] = pal('wall_d')
            layer[y + 7, x:x + w] = pal('outline')

    def render(self):
        m, g, f = self.m, self.m.ground, self.floor
        rows, cols = f.shape
        self._top(g, 0, 0, m.width, m.height, m.g)
        m.block(0, 0, m.width, m.height)
        above = np.zeros_like(f)
        above[1:] = f[:-1]
        face_row = np.full(f.shape, -1, dtype=np.int8)
        band = f & ~above
        for r in range(self.FACE):
            face_row[band & (face_row < 0)] = r
            nxt = np.zeros_like(band)
            nxt[1:] = band[:-1]
            band = nxt & f
        self.face = face_row >= 0
        ys, xs = np.mgrid[0:16, 0:16]
        stone = np.full((16, 16), m.g('floor_m'), dtype=np.uint8)
        stone[(ys == 15) | (xs == 15)] = m.g('floor_d')
        stone[(ys == 0) & (xs < 14)] = m.g('floor_l')
        stone[(xs == 0) & (ys < 14)] = m.g('floor_l')
        cracked = stone.copy()
        cracked[5, 4:7] = m.g('floor_d')
        cracked[6, 7:10] = m.g('floor_d')
        cracked[7, 10] = m.g('floor_d')
        for cy in range(rows):
            for cx in range(cols):
                if not f[cy, cx]:
                    continue
                x, y = cx * 8, cy * 8
                cell = g[y:y + 8, x:x + 8]
                if face_row[cy, cx] >= 0:
                    self._face(g, x, y, 8, face_row[cy, cx], m.g)
                else:
                    sx, sy = cx // 2, cy // 2
                    src = cracked if wg.tile_hash(sx, sy, 9) % 7 == 0 else stone
                    cell[:] = src[(y % 16):(y % 16) + 8, (x % 16):(x % 16) + 8]
                    m.unblock(x, y, 8, 8)
                if cx > 0 and not f[cy, cx - 1]:
                    cell[:, 0] = m.g('outline')
                if cx + 1 < cols and not f[cy, cx + 1]:
                    cell[:, 7] = m.g('outline')
                if cy + 1 < rows and not f[cy + 1, cx]:
                    cell[7, :] = m.g('outline')
                    g[y + 8:y + 10, x:x + 8] = m.g('wall_l')
                    g[y + 10, x:x + 8] = m.g('wall_d')

    # --- props (prison bank) --------------------------------------------------------------------

    def bars(self, x, y, w, door=None):
        """A cell front across a corridor's face band (y is the band's top, 24 px tall). door is
        the x of a 16 px opening, or None for a locked cell."""
        g, m = self.m.ground, self.m
        g[y:y + 4, x:x + w] = m.g('wall_m')
        g[y, x:x + w] = m.g('outline')
        g[y + 3, x:x + w] = m.g('wall_d')
        for bx in range(x, x + w, 4):
            if door is not None and door <= bx < door + 16:
                continue
            g[y + 4:y + 24, bx + 1] = m.g('iron_l')
            g[y + 4:y + 24, bx + 2] = m.g('iron_d')
        for ry in (y + 9, y + 19):
            for bx in range(x, x + w, 8):
                if door is not None and door <= bx < door + 16:
                    continue
                g[ry, bx:bx + 8] = m.g('iron_l')
                g[ry + 1, bx:bx + 8] = m.g('iron_d')
        g[y + 23, x:x + w] = m.g('outline')
        m.block(x, y + 8, w, 16)
        if door is not None:
            g[y + 4:y + 24, door] = m.g('iron_d')
            g[y + 4:y + 24, door + 15] = m.g('iron_d')
            g[y + 23, door:door + 16] = m.g('floor_d')
            m.unblock(door, y + 8, 16, 16)

    def straw(self, x, y):
        g, m = self.m.ground, self.m
        g[y + 2:y + 8, x:x + 24] = m.g('straw')
        g[y + 1, x + 3:x + 21] = m.g('straw')
        for sx in range(x + 1, x + 23, 3):
            g[y + 3 + sx % 4, sx] = m.g('wood')
        g[y + 7, x:x + 24] = m.g('wood')

    def bucket(self, x, y):
        g, m = self.m.ground, self.m
        g[y + 1:y + 8, x + 1:x + 7] = m.g('outline')
        g[y + 2:y + 7, x + 2:x + 6] = m.g('wood')
        g[y + 4, x + 1:x + 7] = m.g('iron_d')
        self.m.block(x, y + 4, 8, 4)

    def chains(self, x, y):
        """Shackles hanging on a wall face."""
        g, m = self.m.ground, self.m
        for cx in (x, x + 6):
            for cy in range(y, y + 10, 2):
                g[cy, cx] = m.g('iron_l')
                g[cy + 1, cx + 1] = m.g('iron_d')
            g[y + 10:y + 12, cx - 1:cx + 3] = m.g('iron_l')

    def torch(self, x, y):
        """A torch in an iron bracket on a wall face."""
        g, m = self.m.ground, self.m
        g[y + 6:y + 12, x + 3:x + 5] = m.g('wood')
        g[y + 9, x + 1:x + 7] = m.g('iron_d')
        g[y + 2:y + 6, x + 2:x + 6] = m.g('flame')
        g[y:y + 2, x + 3:x + 5] = m.g('flame')
        g[y + 5, x + 2:x + 6] = m.g('red')

    def banner(self, x, y):
        """A red Defias rag hung over a wall face, 8x16."""
        g, m = self.m.ground, self.m
        g[y:y + 16, x:x + 8] = m.g('red')
        g[y:y + 16, x] = m.g('outline')
        g[y:y + 16, x + 7] = m.g('outline')
        g[y, x:x + 8] = m.g('iron_d')
        g[y + 14:y + 16, x + 2:x + 6] = m.g('outline')
        g[y + 6:y + 9, x + 3:x + 5] = m.g('top_d')

    def desk(self, x, y, w=48, h=24):
        g, m = self.m.ground, self.m
        g[y:y + h, x:x + w] = m.g('outline')
        g[y + 1:y + h - 5, x + 1:x + w - 1] = m.g('wood')
        g[y + h - 5:y + h - 1, x + 1:x + w - 1] = m.g('top_m')
        g[y + 3:y + 6, x + 6:x + 14] = m.g('floor_l')
        g[y + 4:y + 7, x + w - 12:x + w - 8] = m.g('flame')
        self.m.block(x, y + 4, w, h - 4)

    def barrel(self, x, y):
        g, m = self.m.ground, self.m
        g[y:y + 16, x + 1:x + 15] = m.g('outline')
        g[y + 1:y + 15, x + 2:x + 14] = m.g('wood')
        g[y + 1:y + 4, x + 3:x + 13] = m.g('straw')
        for by in (y + 5, y + 11):
            g[by, x + 2:x + 14] = m.g('iron_d')
        self.m.block(x, y + 6, 16, 10)

    def crate(self, x, y):
        g, m = self.m.ground, self.m
        g[y:y + 16, x:x + 16] = m.g('outline')
        g[y + 1:y + 15, x + 1:x + 15] = m.g('wood')
        g[y + 1:y + 3, x + 1:x + 15] = m.g('straw')
        for d in range(1, 15):
            g[y + d, x + d] = m.g('top_m')
        self.m.block(x, y + 4, 16, 12)

    def rack(self, x, y):
        """A weapon rack against a wall, 24x24."""
        g, m = self.m.ground, self.m
        g[y + 4:y + 6, x:x + 24] = m.g('wood')
        g[y + 18:y + 20, x:x + 24] = m.g('wood')
        for bx in (x + 4, x + 11, x + 18):
            g[y:y + 20, bx] = m.g('iron_l')
            g[y:y + 20, bx + 1] = m.g('iron_d')
            g[y + 14:y + 16, bx - 1:bx + 3] = m.g('wood')
        self.m.block(x, y + 16, 24, 8)

    def secret(self, x, y, w, rock_h):
        """A passage hidden by the overhead layer: rock_h px of wall top, then a brick face. It looks
        like the wall around it, but the player walks under it."""
        o, m = self.m.overhead, self.m
        self._top(o, x, y, w, rock_h, m.o)
        for r in range(self.FACE):
            self._face(o, x, y + rock_h + r * 8, w, r, m.o)

    def exit(self, x, y, target, point):
        """Stairs up to the street at the bottom edge of the map, 48 px wide."""
        g, m = self.m.ground, self.m
        for r, sy in enumerate(range(y - 8, y + 8, 4)):
            g[sy:sy + 4, x:x + 48] = m.g('wall_l' if r % 2 == 0 else 'wall_m')
            g[sy + 3, x:x + 48] = m.g('wall_d')
        g[y - 8:y + 8, x] = m.g('outline')
        g[y - 8:y + 8, x + 47] = m.g('outline')
        m.unblock(x, y - 8, 48, 16)
        m.warp(x, y, 48, 8, target, point)
        m.point('entry', x + 24, y - 24)
        m.point('respawn', x + 24, y - 24)


def gen_stockade():
    c = Prison('stockade', 1024, 512)
    m = c.m
    west_cells = (120, 176, 232, 288, 344)
    east_cells = (632, 688, 744, 800, 856)
    c.rect(432, 384, 160, 112)      # entry hall
    c.rect(488, 496, 48, 16)        # stairs up to the street
    c.rect(480, 272, 64, 120)       # corridor north
    c.rect(400, 176, 224, 104)      # guard room
    c.rect(104, 216, 304, 64)       # west cell block
    c.rect(624, 216, 304, 64)       # east cell block
    for x in west_cells + east_cells:
        c.rect(x, 136, 48, 80)
    c.rect(24, 120, 80, 264)        # the west hall
    c.rect(920, 40, 80, 344)        # the east hall
    c.rect(664, 40, 264, 64)        # north passage
    c.rect(344, 16, 328, 104)       # the warden's hall, where Bazil Thredd holds out
    c.rect(480, 120, 64, 56)        # barred window from the guard room into the hall
    c.rect(32, 32, 72, 64)          # a hidden cell above the west hall
    c.rect(48, 96, 32, 24)
    c.render()
    c.exit(488, 504, 'stormwind', 'stockade_exit')

    # Cells: most locked, a few broken open by the riot.
    open_cells = {176: 192, 288: 304, 688: 704, 800: 816}
    for x in west_cells + east_cells:
        c.bars(x, 216, 48, open_cells.get(x))
        c.straw(x + 4 + (x // 8 % 3) * 4, 176)
        c.bucket(x + 36, 196)
        c.chains(x + 20, 140)
    c.bars(480, 176, 64)
    # Torches on the faces, banners where the Defias took over.
    for x, y in ((448, 392), (568, 392), (424, 184), (592, 184), (32, 124), (88, 124), (952, 44),
                 (976, 44), (712, 44), (816, 44), (376, 20), (632, 20)):
        c.torch(x, y)
    for x in (108, 396, 624, 912):
        c.torch(x, 224)
    for x, y in ((448, 20), (568, 20), (936, 44), (408, 184), (608, 184)):
        c.banner(x, y)
    # The guard room: the warden's desk, overturned crates.
    c.desk(424, 216)
    c.crate(584, 216)
    c.crate(600, 232)
    c.barrel(408, 256)
    # The west and east halls.
    c.crate(32, 352)
    c.barrel(80, 360)
    c.rack(960, 360)
    c.crate(928, 360)
    c.barrel(944, 48 + 24)
    # The warden's hall.
    c.desk(488, 56, 48, 24)
    for x in (360, 392, 624):
        c.barrel(x, 96)
    c.rack(600, 44)
    c.rack(400, 44)
    c.crate(440, 64)
    # The hidden cell: its way in is behind the west hall's back wall.
    c.straw(40, 72)
    c.secret(48, 96, 32, 24)
    m.chest(17, 80, 72, 20)

    for x, y in ((440, 248), (592, 256), (512, 300)):
        m.spawn('DEFIAS_CONVICT', x, y)
    for x, y in ((168, 256), (256, 264), (352, 252), (200, 184), (312, 184)):
        m.spawn('DEFIAS_PRISONER' if x % 3 else 'DEFIAS_CONVICT', x, y)
    for x, y in ((680, 256), (776, 264), (872, 252), (712, 184), (824, 184)):
        m.spawn('DEFIAS_CONVICT' if x % 3 else 'DEFIAS_PRISONER', x, y)
    for x, y in ((48, 180), (80, 232), (952, 136), (976, 216), (744, 84), (856, 84)):
        m.spawn('DEFIAS_INSURGENT', x, y)
    m.spawn('TARGORR', 64, 320)
    m.spawn('KAM_DEEPFURY', 960, 300)
    for x, y in ((408, 84), (616, 84)):
        m.spawn('DEFIAS_INSURGENT', x, y)
    m.spawn('BAZIL_THREDD', 512, 96)
    m.area(0, 0, 1024, 512, 'The Stockade')
    m.area(344, 16, 328, 104, "The Warden's Hall")
    m.music = 'DUNGEON'
    m.save()
    return m


class Station(Prison):
    """The Deeprun Tram's stations: a stone hall with the track along its north wall and the tram
    car waiting at the platform."""

    def track(self, x0, x1, y):
        """The pit with the rails, 32 px deep, under the hall's north wall. Solid."""
        g, m = self.m.ground, self.m
        g[y:y + 32, x0:x1] = m.g('top_d')
        for x in range(x0, x1, 8):
            g[y + 4:y + 30, x + 2:x + 5] = m.g('wood')
            g[y + 4:y + 30, x + 4] = m.g('outline')
        for ry in (y + 9, y + 23):
            g[ry, x0:x1] = m.g('iron_l')
            g[ry + 1, x0:x1] = m.g('iron_d')
        # The platform's edge, with a yellow line to stand behind.
        g[y + 30:y + 32, x0:x1] = m.g('outline')
        g[y + 34:y + 36, x0:x1] = m.g('flame')
        m.block(x0, y, x1 - x0, 32)

    def car(self, x, y, w=112):
        """The tram car on the track, 48 px tall from its roof at y. Returns the door's x: a 24 px
        opening in the middle of its side."""
        g, m = self.m.ground, self.m
        door = x + w // 2 - 12
        g[y:y + 44, x:x + w] = m.g('outline')
        g[y + 1:y + 6, x + 2:x + w - 2] = m.g('top_m')                 # roof
        g[y + 6:y + 30, x + 1:x + w - 1] = m.g('wall_l')                # side
        g[y + 30:y + 42, x + 1:x + w - 1] = m.g('wall_m')
        g[y + 26:y + 30, x + 1:x + w - 1] = m.g('red')                  # stripe
        g[y + 41, x + 1:x + w - 1] = m.g('wall_d')
        left = list(range(door - 18, x + 5, -16))
        for wx in left + [2 * door + 24 - 10 - lx for lx in left]:      # the same on both sides
            g[y + 10:y + 22, wx:wx + 10] = m.g('outline')
            g[y + 11:y + 21, wx + 1:wx + 9] = m.g('flame')             # lit windows
            g[y + 11:y + 13, wx + 1:wx + 9] = m.g('straw')
        g[y + 8:y + 44, door:door + 24] = m.g('outline')                # the open door
        g[y + 9:y + 44, door + 1:door + 23] = m.g('top_d')
        g[y + 9:y + 11, door + 1:door + 23] = m.g('iron_d')
        for wx in (x + 10, x + 26, x + w - 34, x + w - 18):             # wheels on the rails
            g[y + 40:y + 47, wx:wx + 8] = m.g('outline')
            g[y + 41:y + 46, wx + 1:wx + 7] = m.g('iron_d')
            g[y + 43, wx + 3:wx + 5] = m.g('iron_l')
        return door

    def bench(self, x, y):
        """A wooden bench on the platform, 32x12."""
        g, m = self.m.ground, self.m
        g[y:y + 6, x:x + 32] = m.g('outline')
        g[y + 1:y + 5, x + 1:x + 31] = m.g('wood')
        g[y + 1, x + 1:x + 31] = m.g('straw')
        for lx in (x + 3, x + 26):
            g[y + 6:y + 12, lx:lx + 3] = m.g('outline')
        m.block(x, y + 4, 32, 8)


def gen_deeprun_tram():
    c = Station('deeprun_tram', 768, 256)
    m = c.m
    halls = {'stormwind': 48, 'ironforge': 432}
    for x in halls.values():
        c.rect(x, 48, 288, 136)
    c.rect(168, 184, 48, 64)        # stairs up to Stormwind's Dwarven District
    c.rect(552, 184, 48, 64)        # stairs up to Ironforge's Tinker Town
    c.render()
    # The way out first: the world map shows the map an interior's first warp leads to.
    c.exit(168, 248, 'stormwind', 'tram_exit')
    saved = dict(m.points)
    c.exit(552, 248, 'ironforge', 'tram_exit')
    m.points = saved
    m.point('from_ironforge', 576, 220)

    other = {'stormwind': 'ironforge', 'ironforge': 'stormwind'}
    for name, x in halls.items():
        c.track(x, x + 288, 72)
        door = c.car(x + 64, 56, 160)
        m.warp(door, 96, 24, 16, 'deeprun_tram', f'{other[name]}_platform', ride='tram')
        m.point(f'{name}_platform', door + 12, 140)
        for tx in (x + 24, x + 248):
            c.torch(tx, 52)
        c.bench(x + 16, 152)
        c.bench(x + 240, 152)
    m.npc('MONTY', 624, 172)
    m.area(0, 0, 768, 256, 'Deeprun Tram')
    m.area(32, 32, 320, 224, 'Stormwind Station')
    m.area(416, 32, 320, 224, 'Ironforge Station')
    m.music = 'TOWN'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Silverpine Forest (the way to Shadowfang Keep)
# ---------------------------------------------------------------------------------------------

def keep_wall(m, x, y, w, h, gate_w=48):
    """Shadowfang Keep's outer wall seen from the south: a tall brick face with towers at both ends and
    a dark gate in the middle. Returns the gate's bottom-center."""
    g = m.ground
    wg.bricks(m, g, x, y, w, h, 'stone_d', 'stone_m', 'stone_l')
    g[y:y + 3, x:x + w] = m.g('stone_h')
    g[y + h - 1, x:x + w] = m.g('outline')
    for tx in (x, x + w - 48):
        wg.bricks(m, g, tx, y - 24, 48, h + 24, 'stone_d', 'stone_m', 'stone_l')
        g[y - 24:y + h, tx] = m.g('outline')
        g[y - 24:y + h, tx + 47] = m.g('outline')
        wg.window(m, tx + 20, y + 8)
        wg.roof(m, tx - 8, y - 64, 64, 48, ('roof_d', 'roof_m', 'roof_l'), 'roof_h', 'outline')
    gx = x + w // 2 - gate_w // 2
    g[y + h - 40:y + h, gx:gx + gate_w] = m.g('outline')
    g[y + h - 44:y + h - 40, gx - 4:gx + gate_w + 4] = m.g('stone_h')
    for bx in range(gx + 4, gx + gate_w - 2, 6):
        g[y + h - 40:y + h - 16, bx] = m.g('stone_d')
    for bx in (gx - 24, gx + gate_w + 16):
        g[y + 8:y + 40, bx:bx + 8] = m.g('banner')
        g[y + 8:y + 40, bx] = m.g('banner_d')
        g[y + 18:y + 24, bx + 2:bx + 6] = m.g('stone_h')
    m.block(x, y - 24, w, h + 24)
    m.unblock(gx, y + h - 16, gate_w, 16)
    return (gx + gate_w // 2, y + h)


def gen_silverpine():
    m = Map('silverpine', 768, 512,
            Palette([wg.TERRAIN_SILVERPINE, wg.BUILDINGS, wg.FARM, wg.ROCK_SILVERPINE]),
            Palette([wg.OVERHEAD_LEAVES_SILVERPINE, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    rock = wg.corners(m, 'rock')
    rock[0:7, :] = 1
    rock[0:13, 0:10] = 1
    rock[0:13, 38:] = 1
    wg.paint_cliffs(m)

    wg.corners_along(m, 'path', [(384, 470), (380, 420), (360, 330), (384, 240), (384, 176)], 1.1)
    wg.corners_along(m, 'path', [(380, 420), (250, 430), (180, 440)], 0.9)
    wg.paint_paths(m)
    wg.corners_along(m, 'water', [(768, 300), (680, 330), (640, 420), (660, 512)], 1.4)
    wg.paint_water(m)

    gate = keep_wall(m, 224, 104, 320, 72)
    m.warp(gate[0] - 16, gate[1] - 12, 32, 8, 'shadowfang', 'entry')
    m.point('shadowfang_exit', gate[0], gate[1] + 18)
    m.area(200, 40, 368, 200, 'Shadowfang Keep')

    border = 48
    wg.forest(m, trees, 0, 160, border, 352, kinds=('pine',))
    wg.forest(m, trees, 0, m.height - border, m.width, border, kinds=('pine',))
    wg.forest(m, trees, 560, 200, 64, 96, kinds=('pine',))
    wg.forest(m, trees, 48, 224, 96, 112, kinds=('pine',), holes=[(64, 256, 56, 48)], secrets=[(112, 264, 40, 32)])
    m.chest(25, 92, 284, 23)

    # The Alliance scouts' camp below the keep.
    wg.camp(m, 152, 376, 112, 80, tents=[(160, 384)], fire=(216, 416))
    m.npc('VALDAN', 240, 398)
    m.npc('GRYPHON_SILVERPINE', 316, 420)
    m.point('flight', 316, 440)
    m.point('silverpine_respawn', 236, 446)
    m.area(140, 360, 200, 104, "Scouts' Camp")

    m.spawn_group('BLEAK_WORG', 520, 360, 3, 50, seed=71)
    m.spawn_group('BLEAK_WORG', 280, 280, 2, 30, seed=72)

    rng = np.random.default_rng(29)
    path = wg.corners(m, 'path')
    placed = 0
    for _ in range(300):
        if placed >= 26:
            break
        x, y = int(rng.uniform(40, 700)), int(rng.uniform(180, 440))
        mx, my = (x + 16) // 16, (y + 40) // 16
        if path[max(0, my - 3):my + 4, max(0, mx - 3):mx + 4].max() == 0 and m.area_free(x, y, 40, 56) \
                and not (120 <= x <= 340 and 320 <= y <= 500):
            wg.tree(m, trees, x, y, placed, kind='pine')
            placed += 1
    wg.scatter_props(m, rng, 24, ('fern', 'rock', 'tall_grass', 'stump'), (48, 200, 660, 280),
                     avoid=[(140, 360, 200, 104)])
    m.area(0, 0, 768, 512, 'Silverpine Forest')
    m.music = 'DUSKWOOD'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Shadowfang Keep
# ---------------------------------------------------------------------------------------------

CASTLE = [
    ('outline', (14, 14, 20)), ('top_d', (28, 30, 36)), ('top_m', (42, 46, 52)),
    ('wall_d', (54, 60, 64)), ('wall_m', (82, 90, 92)), ('wall_l', (116, 124, 124)),
    ('floor_d', (62, 62, 66)), ('floor_m', (88, 86, 90)), ('floor_l', (114, 112, 114)),
    ('iron_d', (32, 34, 42)), ('iron_l', (140, 144, 160)), ('straw', (144, 120, 64)),
    ('wood', (96, 62, 40)), ('flame', (232, 220, 120)), ('red', (88, 40, 112)),
]

CASTLE_OVERHEAD = [
    ('outline', (14, 14, 20)), ('top_d', (28, 30, 36)), ('top_m', (42, 46, 52)),
    ('wall_d', (54, 60, 64)), ('wall_m', (82, 90, 92)), ('wall_l', (116, 124, 124)),
]


class Castle(Prison):
    """Shadowfang Keep: the prison's walls and flagstones in colder stone, with Arugal's violet banners."""

    def __init__(self, name, width, height):
        self.m = Map(name, width, height, Palette([CASTLE]), Palette([CASTLE_OVERHEAD]))
        self.floor = np.zeros((height // 8, width // 8), dtype=bool)
        self.face = np.zeros_like(self.floor)

    def rug(self, x, y, w, h):
        """A long violet carpet with an iron-grey border."""
        g, m = self.m.ground, self.m
        g[y:y + h, x:x + w] = m.g('red')
        g[y:y + h, x] = m.g('iron_d')
        g[y:y + h, x + w - 1] = m.g('iron_d')
        g[y, x:x + w] = m.g('iron_d')
        g[y + h - 1, x:x + w] = m.g('iron_d')
        g[y + 2:y + h - 2:6, x + 2:x + w - 2] = m.g('outline')

    def ledge(self, x, y, w, h):
        """A raised stone platform; its steps face south."""
        g, m = self.m.ground, self.m
        g[y:y + h, x:x + w] = m.g('wall_l')
        g[y:y + h:8, x:x + w] = m.g('wall_m')
        g[y + h - 6:y + h - 3, x:x + w] = m.g('wall_m')
        g[y + h - 3:y + h, x:x + w] = m.g('wall_d')
        g[y:y + h, x] = m.g('outline')
        g[y:y + h, x + w - 1] = m.g('outline')

    def table(self, x, y, w=64, h=24):
        g, m = self.m.ground, self.m
        g[y:y + h, x:x + w] = m.g('outline')
        g[y + 1:y + h - 4, x + 1:x + w - 1] = m.g('wood')
        g[y + h - 4:y + h - 1, x + 1:x + w - 1] = m.g('top_m')
        for px in range(x + 6, x + w - 6, 14):
            g[y + 4:y + 8, px:px + 6] = m.g('iron_l')
        self.m.block(x, y + 4, w, h - 4)


def gen_shadowfang():
    c = Castle('shadowfang', 1024, 768)
    m = c.m
    c.rect(448, 600, 128, 128)      # entry hall
    c.rect(488, 728, 48, 32)        # the stairs down to the gate
    c.rect(256, 640, 192, 48)       # corridor west
    c.rect(96, 568, 160, 152)       # the dungeon, Rethilgore's
    for x in (104, 152, 200):
        c.rect(x, 504, 40, 64)      # cells
    c.rect(488, 520, 48, 80)        # corridor north
    c.rect(400, 392, 224, 128)      # the dining hall, Silverlaine's
    c.rect(224, 400, 160, 104)      # the kitchen, Razorclaw's
    c.rect(384, 448, 16, 40)        # kitchen door
    c.rect(624, 448, 80, 48)        # corridor east
    c.rect(704, 360, 224, 200)      # the courtyard chapel, Springvale's
    c.rect(840, 200, 48, 160)       # stairs up to the ramparts
    c.rect(560, 152, 368, 48)       # the ramparts, Odo's
    c.rect(512, 152, 48, 48)        # the tower door
    c.rect(256, 40, 256, 176)       # Arugal's chamber
    c.rect(720, 256, 72, 64)        # a hidden room above the chapel
    c.rect(736, 320, 32, 40)
    c.render()
    c.exit(488, 760, 'silverpine', 'shadowfang_exit')

    # The dungeon and its cells.
    for x in (104, 152, 200):
        c.bars(x, 568, 40, 116 if x == 104 else None)
        c.straw(x + 8, 528)
        c.chains(x + 16, 508)
    for x, y in ((112, 576), (224, 576), (448, 608), (544, 608), (272, 648), (416, 648)):
        c.torch(x, y)
    # The kitchen: tables, barrels and a butcher's rack.
    c.table(248, 440, 48, 24)
    c.barrel(232, 480)
    c.barrel(352, 408)
    c.rack(320, 404)
    # The dining hall: a long table on a carpet, banners on the north face.
    c.rug(432, 424, 160, 72)
    c.table(448, 448, 128, 24)
    for x in (408, 600):
        c.torch(x, 396)
    for x in (456, 552):
        c.banner(x, 396)
    # The chapel: pews, an altar on a dais.
    c.ledge(768, 368, 96, 32)
    for y in (432, 472, 512):
        c.table(728, y, 56, 16)
        c.table(848, y, 56, 16)
    c.banner(744, 364)
    c.banner(880, 364)
    # The ramparts.
    for x in (600, 712, 824):
        c.torch(x, 156)
    # Arugal's chamber: three ledges he steps between, and his carpet.
    c.ledge(272, 56, 64, 40)
    c.ledge(432, 56, 64, 40)
    c.ledge(352, 48, 64, 32)
    c.rug(360, 104, 48, 104)
    for x in (264, 496):
        c.banner(x, 44)
    for x in (320, 448):
        c.torch(x, 44)
    # The hidden room.
    c.secret(736, 320, 32, 40)
    c.crate(728, 288)
    m.chest(26, 776, 292, 24)

    # The entry hall's guards stand at the far end, so the hero steps in safely.
    for x, y in ((480, 628), (548, 636)):
        m.spawn('SHADOWFANG_MOONWALKER', x, y)
    m.spawn('HAUNTED_SERVITOR', 512, 616)
    m.spawn('SHADOWFANG_DARKCASTER', 336, 668)
    for x, y in ((136, 680), (224, 640)):
        m.spawn('SHADOWFANG_WOLFGUARD', x, y)
    m.spawn('RETHILGORE', 176, 616)
    m.spawn('HAUNTED_SERVITOR', 512, 556)
    for x, y in ((432, 500), (592, 500)):
        m.spawn('WAILING_GUARDSMAN', x, y)
    m.spawn('SHADOWFANG_DARKCASTER', 600, 420)
    m.spawn('BARON_SILVERLAINE', 512, 416)
    m.spawn('HAUNTED_SERVITOR', 340, 480)
    m.spawn('RAZORCLAW_THE_BUTCHER', 288, 424)
    for x, y in ((648, 472), (680, 476)):
        m.spawn('BLEAK_WORG', x, y)
    for x, y in ((736, 420), (896, 420), (816, 540)):
        m.spawn('SHADOWFANG_WOLFGUARD' if x != 816 else 'SHADOWFANG_DARKCASTER', x, y)
    for x, y in ((744, 540), (896, 540)):
        m.spawn('WAILING_GUARDSMAN', x, y)
    m.spawn('COMMANDER_SPRINGVALE', 816, 412)
    for x, y in ((864, 300), (864, 230)):
        m.spawn('BLEAK_WORG', x, y)
    m.spawn('ODO_THE_BLINDWATCHER', 664, 180)
    for x, y in ((760, 180), (588, 184)):
        m.spawn('SHADOWFANG_MOONWALKER', x, y)
    for x, y in ((300, 180), (468, 180)):
        m.spawn('SHADOWFANG_DARKCASTER' if x < 400 else 'SHADOWFANG_MOONWALKER', x, y)
    m.spawn('ARUGAL', 384, 74)
    # Where Arugal blinks to: the west, middle and east ledges, and the floor in front of them.
    m.patrol = [(304, 82), (384, 72), (464, 82), (384, 150)]
    m.area(0, 0, 1024, 768, 'Shadowfang Keep')
    m.area(256, 40, 256, 176, "Arugal's Chamber")
    m.area(400, 392, 224, 128, 'The Dining Hall')
    m.area(704, 360, 224, 200, 'The Chapel')
    m.area(560, 152, 368, 48, 'The Ramparts')
    m.music = 'DUNGEON'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Ironforge
# ---------------------------------------------------------------------------------------------

FORGE = [
    ('outline', (20, 14, 12)), ('top_d', (42, 32, 28)), ('top_m', (60, 48, 42)),
    ('wall_d', (84, 64, 52)), ('wall_m', (118, 94, 74)), ('wall_l', (156, 128, 100)),
    ('floor_d', (90, 80, 74)), ('floor_m', (122, 108, 98)), ('floor_l', (154, 140, 126)),
    ('iron_d', (44, 44, 54)), ('iron_l', (150, 154, 170)), ('straw', (248, 148, 40)),
    ('wood', (112, 74, 44)), ('flame', (252, 228, 120)), ('red', (176, 44, 20)),
]

FORGE_OVERHEAD = [
    ('outline', (20, 14, 12)), ('top_d', (42, 32, 28)), ('top_m', (60, 48, 42)),
    ('wall_d', (84, 64, 52)), ('wall_m', (118, 94, 74)), ('wall_l', (156, 128, 100)),
]


class Forge(Prison):
    """Ironforge: halls cut into the mountain, warm stone, lava and anvils. 'straw' is the lava's
    middle color here."""

    def __init__(self, name, width, height):
        self.m = Map(name, width, height, Palette([FORGE]), Palette([FORGE_OVERHEAD]))
        self.floor = np.zeros((height // 8, width // 8), dtype=bool)
        self.face = np.zeros_like(self.floor)

    def lava(self, x, y, w, h):
        """A pool of molten metal behind a low stone rim. Solid."""
        g, m = self.m.ground, self.m
        g[y:y + h, x:x + w] = m.g('wall_m')
        g[y:y + 2, x:x + w] = m.g('wall_l')
        g[y + h - 3:y + h, x:x + w] = m.g('wall_d')
        g[y + h - 1, x:x + w] = m.g('outline')
        g[y:y + h, x] = m.g('outline')
        g[y:y + h, x + w - 1] = m.g('outline')
        ys, xs = np.mgrid[y + 6:y + h - 6, x + 6:x + w - 6]
        pool = np.full(xs.shape, m.g('straw'), dtype=np.uint8)
        pool[((xs * 3 + ys * 5) // 7) % 5 == 0] = m.g('red')
        pool[((xs + ys * 2) % 11 == 0) | ((xs * 2 + ys) % 13 == 0)] = m.g('flame')
        g[y + 6:y + h - 6, x + 6:x + w - 6] = pool
        g[y + 5, x + 5:x + w - 5] = m.g('outline')
        g[y + 5:y + h - 5, x + 5] = m.g('outline')
        g[y + 5:y + h - 5, x + w - 6] = m.g('outline')
        g[y + h - 6, x + 5:x + w - 5] = m.g('outline')
        self.m.block(x, y + 4, w, h - 4)

    def anvil(self, x, y):
        """An iron anvil on a stone block, 24x16."""
        g, m = self.m.ground, self.m
        g[y + 8:y + 16, x + 4:x + 20] = m.g('outline')
        g[y + 9:y + 15, x + 5:x + 19] = m.g('wall_m')
        g[y + 2:y + 9, x:x + 24] = m.g('outline')
        g[y + 3:y + 8, x + 1:x + 23] = m.g('iron_d')
        g[y + 3, x + 2:x + 22] = m.g('iron_l')
        g[y + 4:y + 6, x:x + 4] = m.g('iron_d')
        self.m.block(x, y + 6, 24, 10)

    def pillar(self, x, y, h=40):
        """A square stone pillar, 16 wide, standing on the floor."""
        g, m = self.m.ground, self.m
        wg.bricks(m, g, x, y, 16, h, 'wall_d', 'wall_m', 'wall_l')
        g[y:y + h, x] = m.g('outline')
        g[y:y + h, x + 15] = m.g('outline')
        g[y, x:x + 16] = m.g('outline')
        g[y + h - 1, x:x + 16] = m.g('outline')
        g[y + 2:y + 4, x + 1:x + 15] = m.g('iron_l')
        self.m.block(x, y + h - 16, 16, 16)

    def throne(self, x, y):
        """Magni's throne: a high golden-iron seat on a dais, 48x40."""
        g, m = self.m.ground, self.m
        g[y:y + 40, x:x + 48] = m.g('outline')
        g[y + 1:y + 39, x + 1:x + 47] = m.g('wall_l')
        g[y + 30:y + 39, x + 1:x + 47] = m.g('wall_m')
        g[y + 4:y + 26, x + 12:x + 36] = m.g('outline')
        g[y + 5:y + 25, x + 13:x + 35] = m.g('red')
        g[y + 2:y + 6, x + 10:x + 38] = m.g('flame')
        g[y + 20:y + 26, x + 8:x + 40] = m.g('iron_l')
        self.m.block(x, y, 48, 28)

    def table(self, x, y, w=48, h=20):
        g, m = self.m.ground, self.m
        g[y:y + h, x:x + w] = m.g('outline')
        g[y + 1:y + h - 4, x + 1:x + w - 1] = m.g('wood')
        g[y + h - 4:y + h - 1, x + 1:x + w - 1] = m.g('top_m')
        for px in range(x + 6, x + w - 6, 12):
            g[y + 3:y + 7, px:px + 4] = m.g('flame')
            g[y + 6, px:px + 4] = m.g('wall_d')
        self.m.block(x, y + 4, w, h - 4)

    def gears(self, x, y):
        """A big cog on a wall face, 24x24 (Tinker Town)."""
        g, m = self.m.ground, self.m
        ys, xs = np.mgrid[0:24, 0:24]
        d = np.hypot(xs - 11.5, ys - 11.5)
        a = np.arctan2(ys - 11.5, xs - 11.5)
        teeth = (d < 12) & (np.cos(a * 8) > 0.3)
        body = d < 9
        part = g[y:y + 24, x:x + 24]
        part[teeth | body] = m.g('outline')
        part[(d < 8) | (teeth & (d < 11))] = m.g('iron_l')
        part[(d < 6)] = m.g('iron_d')
        part[d < 3] = m.g('outline')

    def rug(self, x, y, w, h):
        """A long red carpet with a gold-iron border."""
        g, m = self.m.ground, self.m
        g[y:y + h, x:x + w] = m.g('red')
        g[y:y + h, x] = m.g('flame')
        g[y:y + h, x + w - 1] = m.g('flame')
        g[y + 2:y + h - 2:8, x + 3:x + w - 3] = m.g('outline')

    def exit(self, x, y, target, point, points=True):
        """Stairs out; points=False keeps 'entry' and 'respawn' where an earlier exit put them."""
        saved = dict(self.m.points)
        super().exit(x, y, target, point)
        if not points:
            self.m.points = saved


def gen_ironforge():
    c = Forge('ironforge', 1024, 768)
    m = c.m
    c.rect(448, 608, 128, 120)      # the Gates
    c.rect(488, 728, 48, 40)        # the steps out to Dun Morogh
    c.rect(480, 560, 64, 48)        # the hall up to the Commons
    c.rect(304, 272, 416, 288)      # the Commons around the Great Forge
    c.rect(480, 208, 64, 64)        # the hall to the High Seat
    c.rect(400, 56, 224, 152)       # the High Seat
    c.rect(720, 384, 80, 48)        # the hall east
    c.rect(800, 272, 192, 248)      # Tinker Town
    c.rect(872, 520, 48, 48)        # the stairs down to the Deeprun Tram
    c.rect(224, 384, 80, 48)        # the hall west
    c.rect(32, 272, 192, 248)       # the Military Ward
    c.render()
    c.exit(488, 760, 'dun_morogh', 'from_ironforge')
    c.exit(872, 560, 'deeprun_tram', 'from_ironforge', points=False)
    m.point('tram_exit', 896, 528)

    # The Great Forge: lava in the middle of the Commons, anvils around it.
    c.lava(440, 352, 144, 120)
    for x, y in ((400, 360), (600, 360), (400, 448), (600, 448)):
        c.anvil(x, y)
    for x in (320, 688):
        for y in (288, 512):
            c.pillar(x, y - 24)
    # The Stonefire Tavern's tables in the Commons' south-west corner.
    c.table(344, 488, 56, 20)
    c.barrel(408, 484)
    for x in (360, 456, 552, 648):
        c.torch(x, 276)
    for x in (464, 552):
        c.torch(x, 612)
    # The High Seat: Magni's throne and the royal guard.
    c.throne(488, 72)
    for x in (432, 576):
        c.pillar(x, 112, 48)
    c.rug(496, 112, 32, 96)
    for x in (416, 592):
        c.torch(x, 60)
    # Tinker Town: cogs on the walls, crates of parts.
    for x in (824, 896, 960):
        c.gears(x, 276)
    c.crate(944, 480)
    c.crate(960, 480)
    c.barrel(816, 480)
    # The Military Ward: weapon racks.
    for x in (48, 96, 176):
        c.rack(x, 280)
    c.table(120, 456, 64, 20)

    m.npc('MAGNI', 512, 124)
    m.npc('IRONFORGE_GUARD_SEAT', 456, 172)
    m.npc('GERRIG', 584, 176)
    m.npc('IRONFORGE_GUARD_GATE', 472, 692)
    m.npc('GRYTH', 512, 312)
    m.point('flight', 512, 332)
    m.npc('FIREBREW', 344, 528)
    m.point('ironforge_respawn', 512, 520)
    m.npc('IRONFORGE_VENDOR', 680, 528)
    m.npc('IRONFORGE_SMITH', 400, 400)
    m.npc('MEKKATORQUE', 896, 320)
    m.npc('SHONI', 848, 400)
    m.npc('KELSTRUM', 80, 336)
    m.npc('OLMIN', 128, 400)
    m.npc('JULI', 176, 336)
    m.area(0, 0, 1024, 768, 'Ironforge')
    m.area(440, 600, 144, 168, 'The Gates')
    m.area(304, 272, 416, 288, 'The Commons')
    m.area(432, 344, 160, 136, 'The Great Forge')
    m.area(400, 56, 224, 152, 'The High Seat')
    m.area(800, 272, 192, 296, 'Tinker Town')
    m.area(32, 272, 192, 248, 'The Military Ward')
    m.music = 'IRONFORGE'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Dun Morogh (the snowy valley in front of Ironforge and Gnomeregan)
# ---------------------------------------------------------------------------------------------

def mountain_gate(m, x, y, w, h, gate_w=48):
    """A dwarven gate cut into a mountain: a stone face with gold trim, two braziers and a dark arch
    in the middle. Returns the gate's bottom-center. x, y, w, h are multiples of 8."""
    g = m.ground
    wg.bricks(m, g, x, y, w, h, 'stone_d', 'stone_m', 'stone_l')
    g[y:y + 3, x:x + w] = m.g('stone_h')
    g[y + 6, x:x + w] = m.g('gold')
    g[y + h - 1, x:x + w] = m.g('outline')
    g[y:y + h, x] = m.g('outline')
    g[y:y + h, x + w - 1] = m.g('outline')
    gx = x + w // 2 - gate_w // 2
    g[y + h - 48:y + h, gx:gx + gate_w] = m.g('outline')
    g[y + h - 52:y + h - 48, gx - 4:gx + gate_w + 4] = m.g('gold')
    g[y + h - 48:y + h, gx - 4:gx] = m.g('stone_h')
    g[y + h - 48:y + h, gx + gate_w:gx + gate_w + 4] = m.g('stone_h')
    for bx in (x + 16, x + w - 32):
        g[y + h - 32:y + h - 8, bx:bx + 16] = m.g('stone_d')
        g[y + h - 36:y + h - 32, bx - 2:bx + 18] = m.g('gold')
        g[y + h - 44:y + h - 36, bx + 4:bx + 12] = m.g('glass_l')
    m.block(x, y, w, h)
    m.unblock(gx, y + h - 24, gate_w, 24)
    return (gx + gate_w // 2, y + h)


def gen_dun_morogh():
    m = Map('dun_morogh', 1024, 768,
            Palette([wg.TERRAIN_SNOW, wg.BUILDINGS, wg.FARM, wg.ROCK_SNOW]),
            Palette([wg.OVERHEAD_LEAVES_SNOW, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    rock = wg.corners(m, 'rock')
    rock[0:8, :] = 1
    rock[0:14, 0:16] = 1
    rock[37:, :] = 1
    rock[30:, 0:12] = 1
    rock[0:20, 58:] = 1
    rock[34:, 50:] = 1
    wg.paint_cliffs(m)

    wg.corners_along(m, 'path', [(512, 196), (512, 300), (520, 400), (640, 420), (800, 410), (1024, 408)], 1.1)
    wg.corners_along(m, 'path', [(520, 400), (380, 380), (260, 320), (160, 280), (152, 252)], 1.1)
    wg.paint_paths(m)
    wg.corners_ellipse(m, 'water', 700, 540, 110, 40)
    wg.paint_water(m)

    gate = mountain_gate(m, 416, 112, 192, 88)
    m.warp(gate[0] - 20, gate[1] - 12, 40, 8, 'ironforge', 'entry')
    m.point('from_ironforge', gate[0], gate[1] + 18)
    m.npc('IRONFORGE_GUARD_OUTSIDE', gate[0] + 40, gate[1] + 16)
    m.area(400, 96, 224, 160, 'The Gates of Ironforge')

    cave = wg.mine_entrance(m, 120, 200, 64, 56)
    m.warp(cave[0] - 12, cave[1] - 8, 24, 8, 'gnomeregan', 'entry')
    m.point('gnomeregan_exit', cave[0], cave[1] + 20)
    m.area(80, 180, 200, 140, 'Gnomeregan')

    # The camp the gnomes fled to when Gnomeregan fell.
    wg.camp(m, 256, 368, 112, 72, tents=[(264, 376)], fire=(320, 400))
    m.npc('OZZIE', 344, 384)
    m.npc('MOUNTAINEER', 248, 344)
    m.point('dun_morogh_respawn', 312, 448)
    m.area(240, 340, 150, 120, "The Tinkers' Camp")

    m.spawn_group('LEPER_GNOME', 200, 300, 3, 40, seed=81)
    m.spawn_group('IRRADIATED_PILLAGER', 120, 330, 2, 30, seed=82)

    rng = np.random.default_rng(31)
    path = wg.corners(m, 'path')
    placed = 0
    for _ in range(600):
        if placed >= 34:
            break
        x, y = int(rng.uniform(40, 940), ), int(rng.uniform(180, 520))
        mx, my = (x + 16) // 16, (y + 40) // 16
        if path[max(0, my - 3):my + 4, max(0, mx - 3):mx + 4].max() == 0 and m.area_free(x, y, 40, 56) \
                and not (230 <= x <= 400 and 320 <= y <= 470) and not (380 <= x <= 640 and y <= 280):
            wg.tree(m, trees, x, y, placed, kind='pine')
            placed += 1
    wg.scatter_props(m, rng, 18, ('rock', 'big_rock', 'rock'), (48, 200, 900, 300),
                     avoid=[(240, 340, 150, 120)])

    m.point('from_wetlands', 1004, 408)
    m.warp(1016, 384, 8, 48, 'wetlands', 'from_dun_morogh')
    m.area(860, 340, 164, 140, 'The Pass to the Wetlands')
    m.area(0, 0, 1024, 768, 'Dun Morogh')
    m.music = 'IRONFORGE'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# The Wetlands (Menethil Harbor, Dun Modr, the Dragonmaw)
# ---------------------------------------------------------------------------------------------

WETLANDS_PROPS = ('tall_grass', 'fern', 'rock', 'bush', 'log', 'stump', 'tall_grass', 'big_rock', 'fern')


def ruin(m, x, y, w, h, seed):
    """Broken dwarven walls: stubs of brick with gaps, on the building bank. Solid where they stand."""
    g = m.ground
    local = np.random.default_rng(seed)
    for bx in range(x, x + w, 16):
        if local.random() < 0.35:
            continue
        top = int(local.integers(0, 3)) * 8
        wg.bricks(m, g, bx, y + top, 16, h - top, 'stone_d', 'stone_m', 'stone_l')
        g[y + top, bx:bx + 16] = m.g('stone_h')
        g[y + h - 1, bx:bx + 16] = m.g('outline')
        m.block(bx, y + top + 8, 16, h - top - 8)


def dig_site(m, x, y, w, h):
    """An excavation: a pit of bare earth (farm bank) with plank ramps and crates."""
    ys, xs = np.mgrid[y:y + h, x:x + w]
    pattern = np.full((h, w), m.g('soil_m'), dtype=np.uint8)
    pattern[(xs * 5 + ys * 3) % 17 == 0] = m.g('soil_d')
    pattern[(ys - y) % 24 < 2] = m.g('soil_d')
    m.ground[y:y + h, x:x + w] = pattern
    for px in range(x + 16, x + w - 16, 48):
        m.ground[y + 8:y + h - 8, px:px + 2] = m.g('canvas_d')


def gen_wetlands():
    m = Map('wetlands', 1280, 1024,
            Palette([wg.TERRAIN_WETLANDS, wg.BUILDINGS, wg.FARM, wg.ROCK_WETLANDS]),
            Palette([wg.OVERHEAD_LEAVES_WETLANDS, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    # --- the hills: Arathi to the north, Grim Batol to the east, Dun Morogh's peaks to the south -----
    rock = wg.corners(m, 'rock')
    rock[0:5, 12:] = 1
    rock[0:5, 38:44] = 0                # the gorge under the Thandol Span
    rock[:, 74:] = 1
    rock[16:30, 70:] = 1                # Grim Batol's foothills
    rock[60:, 20:] = 1
    rock[60:, 48:54] = 0                # Dun Algaz, the pass south
    wg.paint_cliffs(m)

    # --- the sea along the west coast and the marsh ponds ------------------------------------------
    wg.corners_along(m, 'water', [(0, 0), (40, 200), (60, 420), (80, 620), (100, 820), (60, 1024)], 4.2)
    wg.corners_ellipse(m, 'water', 0, 500, 140, 560)
    wg.corners_ellipse(m, 'water', 440, 690, 64, 40)
    wg.corners_ellipse(m, 'water', 690, 890, 60, 36)
    wg.corners_ellipse(m, 'water', 280, 470, 56, 32)
    wg.corners_ellipse(m, 'water', 760, 640, 72, 40)
    wg.corners_along(m, 'water', [(640, 0), (640, 80)], 2.2)
    wg.paint_water(m)

    # --- roads ----------------------------------------------------------------------------------------
    roads = [
        [(300, 840), (420, 820), (560, 790), (640, 760), (816, 800), (816, 1024)],      # Menethil to Dun Algaz
        [(560, 790), (580, 640), (560, 480), (600, 360), (640, 260), (640, 96)],       # north to Dun Modr
        [(600, 360), (760, 380), (920, 440), (1000, 480)],                             # east to the Dragonmaw
        [(580, 640), (440, 560), (380, 420), (420, 320)],                              # the excavation
    ]
    for points in roads:
        wg.corners_along(m, 'path', points, 1.1)
    wg.paint_paths(m)

    # --- Menethil Harbor ----------------------------------------------------------------------------
    wg.cobbles(m, 192, 760, 224, 112)
    keep = wg.house(m, 296, 648, 112, 112, style='stone', roof_colors=('roof_d', 'roof_m', 'roof_l'),
                    roof_ridge='roof_h', roof_outline='outline')
    inn = wg.house(m, 192, 664, 96, 96, roof_colors=('red_d', 'red_m', 'red_l'), roof_ridge='red_l',
                   roof_outline='o2')
    shop = wg.house(m, 224, 880, 80, 80, roof_colors=('roof_d', 'roof_m', 'roof_l'), roof_ridge='roof_h',
                    roof_outline='outline')
    wg.house(m, 320, 880, 80, 80, roof_colors=('red_d', 'red_m', 'red_l'), roof_ridge='red_l', roof_outline='o2')
    wg.well(m, 336, 800)
    wg.pier(m, 160, 800, 32, 120)
    m.npc('STOUTFIST', keep[0] + 18, keep[1] + 10)
    m.npc('HELBREK', inn[0] + 18, inn[1] + 10)
    m.npc('MENETHIL_VENDOR', shop[0] + 18, shop[1] + 10)
    m.npc('SHELLEI', 392, 790)
    m.point('flight', 392, 810)
    m.npc('HALLORAN', 256, 790)
    m.npc('HARBORMASTER', 216, 820)
    m.npc('MENETHIL_GUARD', 424, 760)
    m.point('menethil_respawn', 296, 840)
    m.point('from_darkshore', 176, 880)
    m.warp(164, 912, 24, 8, 'darkshore', 'from_menethil', ride='boat')
    m.area(120, 640, 320, 330, 'Menethil Harbor')

    # --- Whelgar's Excavation Site (gnolls looting the dig) -----------------------------------------
    dig_site(m, 352, 272, 160, 104)
    wg.camp(m, 424, 400, 96, 64, tents=[(432, 408)], fire=(480, 424))
    m.npc('WHELGAR', 496, 448)
    m.npc('ORMER', 416, 456)
    m.spawn_group('MOSSHIDE_GNOLL', 420, 320, 5, 70, seed=91)
    m.spawn_group('MOSSHIDE_MYSTIC', 380, 300, 3, 50, seed=92)
    m.chest(33, 376, 288, 27)
    m.area(330, 250, 220, 230, "Whelgar's Excavation Site")

    # --- the bogs: crocolisks around the ponds, raptors on the ridge ---------------------------------
    m.spawn_group('YOUNG_CROCOLISK', 440, 620, 4, 70, seed=93)
    m.spawn_group('YOUNG_CROCOLISK', 290, 540, 3, 50, seed=94)
    m.spawn_group('GIANT_CROCOLISK', 760, 700, 4, 70, seed=95)
    m.spawn_group('GIANT_CROCOLISK', 690, 940, 2, 40, seed=96)
    m.spawn_group('MOTTLED_RAPTOR', 860, 220, 4, 80, seed=97)
    m.spawn_group('MOTTLED_SCREECHER', 960, 260, 4, 70, seed=98)
    m.spawn('SARLTOOTH', 1000, 200)
    m.area(800, 140, 300, 220, 'Raptor Ridge')
    m.spawn_group('BLUEGILL_MURLOC', 200, 300, 5, 80, seed=99)
    m.area(120, 160, 240, 320, 'Bluegill Marsh')

    # --- Dun Modr and the Thandol Span (Dark Iron dwarves) -------------------------------------------
    ruin(m, 528, 136, 96, 40, seed=7)
    ruin(m, 672, 136, 96, 40, seed=8)
    ruin(m, 528, 232, 64, 32, seed=9)
    ruin(m, 704, 232, 64, 32, seed=10)
    ns_bridge(m, 624, 0, 32, 104)
    m.spawn_group('DARK_IRON_DWARF', 640, 200, 5, 70, seed=100)
    m.spawn_group('DARK_IRON_SABOTEUR', 600, 260, 3, 50, seed=101)
    m.spawn('BALGARAS_THE_FOUL', 640, 120)
    m.area(500, 100, 300, 200, 'Dun Modr')
    m.area(600, 0, 80, 110, 'Thandol Span', 'THANDOL_SPAN')

    # --- the Dragonmaw: Angerfang Encampment and the gates of Grim Batol ----------------------------
    wg.camp(m, 904, 480, 176, 104, tents=[(912, 488), (1040, 488)], fire=(984, 528))
    m.spawn_group('DRAGONMAW_GRUNT', 980, 560, 6, 80, seed=102)
    m.spawn_group('DRAGONMAW_SHADOWWARDER', 980, 500, 3, 50, seed=103)
    m.area(880, 460, 240, 180, 'Angerfang Encampment')
    gate = mountain_gate(m, 1064, 384, 112, 72, gate_w=40)
    m.block(gate[0] - 20, gate[1] - 24, 40, 24)
    m.spawn('NEK_ROSH', gate[0], gate[1] + 24)
    m.spawn_group('DRAGONMAW_GRUNT', gate[0] - 40, gate[1] + 40, 2, 20, seed=104)
    m.area(1040, 360, 160, 140, 'Grim Batol')

    # --- trees: willows and oaks of the marsh ---------------------------------------------------------
    rng = np.random.default_rng(37)
    path = wg.corners(m, 'path')
    water = wg.corners(m, 'water')
    clear = [(110, 630, 340, 350), (330, 250, 220, 230), (500, 90, 310, 210), (880, 460, 240, 180),
             (1040, 360, 160, 140)]
    placed = 0
    for _ in range(3000):
        if placed >= 150:
            break
        x, y = int(rng.uniform(120, 1150)), int(rng.uniform(100, 940))
        mx, my = (x + 16) // 16, (y + 40) // 16
        near = path[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max() + \
            water[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max()
        if near == 0 and m.area_free(x, y, 40, 56) and \
                not any(cx - 40 <= x <= cx + cw and cy - 56 <= y <= cy + ch for cx, cy, cw, ch in clear):
            wg.tree(m, trees, x, y, int(rng.integers(0, 3)), kind=('oak', 'small', 'oak', 'birch')[placed % 4])
            placed += 1
    wg.scatter_props(m, rng, 80, WETLANDS_PROPS, (120, 100, 1040, 860),
                     avoid=[(110, 630, 340, 350), (330, 250, 220, 230)])

    m.point('from_dun_morogh', 816, 1000)
    m.warp(792, 1016, 48, 8, 'dun_morogh', 'from_wetlands')
    m.area(760, 940, 120, 84, 'Dun Algaz')
    m.area(0, 0, 1280, 1024, 'Wetlands')
    m.music = 'WETLANDS'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Darkshore (Auberdine and the road to Blackfathom Deeps)
# ---------------------------------------------------------------------------------------------

def moonwell(m, x, y):
    """A night elf moonwell: a ring of pale stone around glowing water on a paved square, 32x32
    (building bank). x and y are multiples of 8."""
    g = m.ground
    wg.cobbles(m, x, y, 32, 32)
    ys, xs = np.mgrid[0:32, 0:32]
    d = np.hypot(xs - 15.5, ys - 15.5)
    part = g[y:y + 32, x:x + 32]
    part[d < 15] = m.g('outline')
    part[d < 14] = m.g('stone_l')
    part[d < 11] = m.g('glass_d')
    part[d < 9] = m.g('banner')
    part[(d < 6) & ((xs + ys) % 3 == 0)] = m.g('stone_h')
    m.block(x + 4, y + 8, 24, 20)


def gen_darkshore():
    m = Map('darkshore', 768, 1024,
            Palette([wg.TERRAIN_DARKSHORE, wg.BUILDINGS, wg.FARM, wg.ROCK_DARKSHORE]),
            Palette([wg.OVERHEAD_LEAVES_DARKSHORE, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    rock = wg.corners(m, 'rock')
    rock[0:6, :] = 1
    rock[:, 42:] = 1
    rock[50:, 30:] = 1
    rock[52:, 0:18] = 1
    rock[60:, :] = 1
    rock[0:28, 36:] = 1
    wg.paint_cliffs(m)

    wg.corners_along(m, 'water', [(0, 80), (40, 300), (60, 500), (30, 700), (0, 896)], 4.0)
    wg.paint_water(m)
    wg.corners_along(m, 'path', [(200, 300), (320, 360), (400, 480), (420, 640), (440, 780), (444, 880),
                                 (352, 904)], 1.1)
    wg.paint_paths(m)

    # --- Auberdine -----------------------------------------------------------------------------------
    elf = dict(roof_colors=('roof_d', 'roof_m', 'roof_l'), roof_ridge='roof_h', roof_outline='outline')
    inn = wg.house(m, 192, 160, 112, 96, style='stone', **elf)
    hall = wg.house(m, 336, 152, 96, 96, style='stone', **elf)
    wg.house(m, 464, 184, 80, 80, style='stone', **elf)
    moonwell(m, 280, 296)
    wg.pier(m, 120, 296, 32, 104)
    m.npc('SHAUSSIY', inn[0] + 20, inn[1] + 10)
    m.npc('SHAEDLASS', hall[0] + 20, hall[1] + 10)
    m.npc('CAYLAIS', 440, 320)
    m.point('flight', 440, 340)
    m.npc('SENTINEL', 232, 344)
    m.point('auberdine_respawn', 352, 336)
    m.point('from_menethil', 136, 340)
    m.warp(124, 392, 24, 8, 'wetlands', 'from_darkshore', ride='boat')
    m.area(100, 140, 460, 260, 'Auberdine')

    # --- the Zoram Strand: Blackfathom Deeps' gate in the cliffs of Ashenvale ------------------------
    # Against the western cliffs; the road goes round its east side to the door.
    gate = mountain_gate(m, 288, 800, 120, 72, gate_w=40)
    m.warp(gate[0] - 16, gate[1] - 12, 32, 8, 'blackfathom_deeps', 'entry')
    m.point('bfd_exit', gate[0], gate[1] + 18)
    m.spawn_group('BLACKFATHOM_MYRMIDON', 380, 700, 3, 60, seed=111)
    m.spawn_group('BLACKFATHOM_TIDE_PRIESTESS', 300, 720, 2, 40, seed=112)
    m.area(240, 640, 300, 320, 'The Zoram Strand')

    wg.forest(m, trees, 560, 448, 112, 240, kinds=('pine', 'oak'), holes=[(584, 520, 56, 48)],
              secrets=[(560, 528, 40, 32)])
    m.chest(35, 612, 556, 26)
    rng = np.random.default_rng(41)
    path = wg.corners(m, 'path')
    water = wg.corners(m, 'water')
    clear = [(100, 140, 460, 280), (240, 640, 300, 320), (560, 448, 112, 240)]
    placed = 0
    for _ in range(2000):
        if placed >= 60:
            break
        x, y = int(rng.uniform(120, 640)), int(rng.uniform(120, 760))
        mx, my = (x + 16) // 16, (y + 40) // 16
        near = path[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max() + \
            water[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max()
        if near == 0 and m.area_free(x, y, 40, 56) and \
                not any(cx - 40 <= x <= cx + cw and cy - 56 <= y <= cy + ch for cx, cy, cw, ch in clear):
            wg.tree(m, trees, x, y, int(rng.integers(0, 3)), kind=('oak', 'pine')[placed % 2])
            placed += 1
    wg.scatter_props(m, rng, 30, ('fern', 'rock', 'tall_grass', 'bush'), (120, 420, 520, 360),
                     avoid=[(240, 640, 300, 320)])
    m.area(0, 0, 768, 1024, 'Darkshore')
    m.music = 'WETLANDS'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Blackfathom Deeps
# ---------------------------------------------------------------------------------------------

DEEPS = [
    ('outline', (10, 16, 24)), ('top_d', (20, 32, 40)), ('top_m', (30, 46, 56)),
    ('wall_d', (40, 64, 72)), ('wall_m', (60, 92, 100)), ('wall_l', (88, 124, 128)),
    ('floor_d', (56, 72, 76)), ('floor_m', (78, 98, 100)), ('floor_l', (104, 126, 124)),
    ('water_d', (16, 40, 72)), ('water_m', (28, 64, 108)), ('water_l', (72, 116, 160)),
    ('moss', (56, 96, 64)), ('flame', (140, 200, 248)), ('iron_d', (36, 40, 52)),
]

DEEPS_OVERHEAD = [
    ('outline', (10, 16, 24)), ('top_d', (20, 32, 40)), ('top_m', (30, 46, 56)),
    ('wall_d', (40, 64, 72)), ('wall_m', (60, 92, 100)), ('wall_l', (88, 124, 128)),
]


class Deeps(Prison):
    """Blackfathom Deeps: a drowned night elf temple. Mossy blue-green stone, black water."""

    def __init__(self, name, width, height):
        self.m = Map(name, width, height, Palette([DEEPS]), Palette([DEEPS_OVERHEAD]))
        self.floor = np.zeros((height // 8, width // 8), dtype=bool)
        self.face = np.zeros_like(self.floor)

    def pool(self, x, y, w, h):
        """Deep water with a stone lip. Solid."""
        g, m = self.m.ground, self.m
        ys, xs = np.mgrid[y:y + h, x:x + w]
        water = np.full((h, w), m.g('water_m'), dtype=np.uint8)
        water[((xs % 16) * 3 + (ys % 16) * 5) % 16 == 0] = m.g('water_l')
        water[(ys - y) < 6] = m.g('water_d')
        g[y:y + h, x:x + w] = water
        g[y:y + 3, x:x + w] = m.g('wall_l')
        g[y + 3, x:x + w] = m.g('outline')
        g[y + h - 2:y + h, x:x + w] = m.g('wall_l')
        g[y:y + h, x:x + 2] = m.g('wall_l')
        g[y:y + h, x + w - 2:x + w] = m.g('wall_l')
        self.m.block(x, y + 4, w, h - 6)

    def moss(self, x, y, w, h, seed=0):
        """Moss on some of the flagstones: two fixed patches, so the tiles repeat."""
        g, m = self.m.ground, self.m
        patches = [np.array([[0, 1, 1, 0, 0, 0, 1, 0], [1, 1, 1, 1, 0, 1, 1, 1], [0, 1, 1, 0, 0, 0, 1, 0],
                             [0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 1, 0, 0, 0, 0, 0], [0, 1, 1, 1, 0, 0, 1, 1],
                             [0, 0, 1, 1, 0, 1, 1, 1], [0, 0, 0, 0, 0, 0, 1, 0]], dtype=bool)]
        patches.append(patches[0][::-1, ::-1].copy())
        floors = [m.g('floor_m'), m.g('floor_l'), m.g('floor_d')]
        for cy in range(y // 8, (y + h) // 8):
            for cx in range(x // 8, (x + w) // 8):
                kind = wg.tile_hash(cx, cy, 40 + seed) % 5
                if kind >= 2:
                    continue
                cell = g[cy * 8:cy * 8 + 8, cx * 8:cx * 8 + 8]
                cell[patches[kind] & np.isin(cell, floors)] = m.g('moss')

    def statue(self, x, y):
        """A broken night elf statue on a plinth, 16x32."""
        g, m = self.m.ground, self.m
        g[y + 20:y + 32, x:x + 16] = m.g('outline')
        g[y + 21:y + 31, x + 1:x + 15] = m.g('wall_m')
        g[y + 21, x + 1:x + 15] = m.g('wall_l')
        g[y:y + 21, x + 4:x + 12] = m.g('outline')
        g[y + 1:y + 20, x + 5:x + 11] = m.g('wall_l')
        g[y + 4:y + 8, x + 5:x + 11] = m.g('moss')
        self.m.block(x, y + 20, 16, 12)

    def altar(self, x, y, w=64, h=32):
        g, m = self.m.ground, self.m
        g[y:y + h, x:x + w] = m.g('outline')
        g[y + 1:y + h - 1, x + 1:x + w - 1] = m.g('wall_l')
        g[y + h - 6:y + h - 1, x + 1:x + w - 1] = m.g('wall_m')
        g[y + 6:y + 12, x + w // 2 - 6:x + w // 2 + 6] = m.g('flame')
        self.m.block(x, y + 4, w, h - 8)

    def door(self, x, y, w=48):
        """Aku'mai's door on a north wall face: carved stone with a dark slit."""
        g, m = self.m.ground, self.m
        g[y:y + 24, x:x + w] = m.g('outline')
        g[y + 1:y + 24, x + 2:x + w - 2] = m.g('wall_d')
        g[y + 4:y + 24, x + w // 2 - 1:x + w // 2 + 1] = m.g('outline')
        for px in (x + 8, x + w - 12):
            g[y + 6:y + 10, px:px + 4] = m.g('flame')

    def torch(self, x, y):
        """A blue-flamed sconce on a wall face."""
        g, m = self.m.ground, self.m
        g[y + 6:y + 12, x + 3:x + 5] = m.g('iron_d')
        g[y + 9, x + 1:x + 7] = m.g('iron_d')
        g[y + 2:y + 6, x + 2:x + 6] = m.g('flame')
        g[y:y + 2, x + 3:x + 5] = m.g('water_l')


def gen_blackfathom_deeps():
    c = Deeps('blackfathom_deeps', 1024, 1024)
    m = c.m
    c.rect(456, 872, 112, 96)       # the entrance hall
    c.rect(488, 960, 48, 64)        # the stairs up to the Zoram Strand
    c.rect(488, 800, 48, 72)
    c.rect(320, 640, 384, 160)      # Ghamoo-ra's pool
    c.rect(488, 560, 48, 80)
    c.rect(384, 432, 256, 128)      # the Pool of Ask'ar
    c.rect(640, 464, 96, 64)        # Thaelrid's alcove
    c.rect(320, 480, 64, 48)
    c.rect(96, 400, 224, 176)       # Lady Sarevess's ledge
    c.rect(128, 576, 48, 64)        # a hidden passage
    c.rect(96, 640, 112, 64)
    c.rect(704, 672, 48, 48)
    c.rect(752, 600, 224, 192)      # Gelihast's grotto
    c.rect(488, 336, 48, 96)
    c.rect(320, 160, 384, 176)      # the Moonshrine
    c.rect(736, 24, 256, 176)       # Aku'mai's lair
    c.render()
    c.exit(488, 1016, 'darkshore', 'bfd_exit')

    # Walkways stay open around each pool: the halls' doors face them.
    c.pool(408, 696, 208, 64)
    c.pool(456, 480, 112, 40)
    c.pool(784, 696, 96, 64)
    c.pool(800, 48, 128, 40)
    c.moss(320, 640, 384, 160, seed=1)
    c.moss(96, 400, 224, 176, seed=2)
    c.moss(752, 600, 224, 192, seed=3)
    for x, y in ((344, 648), (664, 648), (416, 440), (592, 440)):
        c.statue(x, y)
    for x in (352, 456, 552, 648):
        c.torch(x, 164)
    for x in (120, 280):
        c.torch(x, 404)
    c.altar(480, 232)
    c.door(488, 160)
    c.secret(128, 552, 48, 24)
    m.chest(31, 152, 680, 26)

    # The four braziers of the Moonshrine open Aku'mai's door.
    for chest, x, y in ((27, 352, 216), (28, 672, 216), (29, 352, 320), (30, 672, 320)):
        m.brazier(chest, x, y)
    m.warp(488, 184, 48, 8, 'blackfathom_deeps', 'akumai_entry', sealed=True)
    m.point('moonshrine_door', 512, 204)
    m.point('akumai_entry', 864, 176)
    m.warp(840, 192, 48, 8, 'blackfathom_deeps', 'moonshrine_door')

    m.npc('THAELRID', 704, 496)

    m.spawn_group('BLACKFATHOM_MYRMIDON', 512, 920, 2, 30, seed=121)
    m.spawn('AKU_MAI_SNAPJAW', 360, 768)
    m.spawn('AKU_MAI_SNAPJAW', 660, 768)
    m.spawn('BLACKFATHOM_TIDE_PRIESTESS', 400, 768)
    m.spawn('GHAMOO_RA', 512, 768)
    m.spawn_group('BLACKFATHOM_MYRMIDON', 512, 528, 2, 30, seed=122)
    m.spawn('BLACKFATHOM_TIDE_PRIESTESS', 420, 530)
    for x, y in ((200, 470), (260, 520), (140, 520)):
        m.spawn('BLACKFATHOM_MYRMIDON' if x != 260 else 'BLACKFATHOM_TIDE_PRIESTESS', x, y)
    m.spawn('LADY_SAREVESS', 200, 432)
    for x, y in ((800, 640), (920, 660), (800, 776), (930, 770)):
        m.spawn('BLINDLIGHT_MURLOC', x, y)
    m.spawn('GELIHAST', 880, 640)
    for x, y in ((392, 252), (632, 252), (440, 304), (584, 304), (512, 324)):
        m.spawn('TWILIGHT_ACOLYTE' if x in (392, 632, 512) else 'TWILIGHT_REAVER', x, y)
    m.spawn('TWILIGHT_LORD_KELRIS', 512, 284)
    # Aku'mai waits at the water's edge, out of reach of the door it is entered by.
    m.spawn('AKU_MAI_SERVANT', 776, 120)
    m.spawn('AKU_MAI_SERVANT', 952, 120)
    m.spawn('AKU_MAI', 864, 108)
    m.area(0, 0, 1024, 1024, 'Blackfathom Deeps')
    m.area(320, 640, 384, 160, "Ghamoo-ra's Pool")
    m.area(96, 400, 224, 176, "Sarevess's Ledge")
    m.area(752, 600, 224, 192, "Gelihast's Grotto")
    m.area(320, 160, 384, 176, 'The Moonshrine')
    m.area(736, 24, 256, 176, "Aku'mai's Lair")
    m.music = 'DUNGEON'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Gnomeregan
# ---------------------------------------------------------------------------------------------

GNOMER = [
    ('outline', (14, 16, 22)), ('top_d', (30, 34, 44)), ('top_m', (46, 52, 64)),
    ('wall_d', (64, 70, 84)), ('wall_m', (96, 104, 120)), ('wall_l', (136, 144, 160)),
    ('floor_d', (70, 72, 80)), ('floor_m', (100, 102, 110)), ('floor_l', (132, 134, 140)),
    ('iron_d', (40, 44, 52)), ('iron_l', (176, 180, 192)), ('straw', (96, 200, 56)),
    ('wood', (168, 128, 56)), ('flame', (216, 248, 120)), ('red', (184, 48, 40)),
]

GNOMER_OVERHEAD = [
    ('outline', (14, 16, 22)), ('top_d', (30, 34, 44)), ('top_m', (46, 52, 64)),
    ('wall_d', (64, 70, 84)), ('wall_m', (96, 104, 120)), ('wall_l', (136, 144, 160)),
]


class Gnomer(Forge):
    """Gnomeregan: steel plates, brass pipes and glowing green fallout. 'straw' is the fallout here,
    'wood' the brass."""

    def __init__(self, name, width, height):
        self.m = Map(name, width, height, Palette([GNOMER]), Palette([GNOMER_OVERHEAD]))
        self.floor = np.zeros((height // 8, width // 8), dtype=bool)
        self.face = np.zeros_like(self.floor)

    def fallout(self, x, y, w, h, seed=0):
        """A puddle of radioactive slime on the floor. Walkable, and it burns."""
        g, m = self.m.ground, self.m
        local = np.random.default_rng(seed)
        ys, xs = np.mgrid[y:y + h, x:x + w]
        cx, cy = x + w / 2, y + h / 2
        d = np.hypot((xs + 0.5 - cx) / (w / 2), (ys + 0.5 - cy) / (h / 2))
        d += (local.random(d.shape) - 0.5) * 0.15
        area = g[y:y + h, x:x + w]
        area[d < 1] = m.g('outline')
        area[d < 0.92] = m.g('straw')
        area[(d < 0.7) & ((xs * 5 + ys * 3) % 7 == 0)] = m.g('flame')
        self.m.area(x + 4, y + 4, w - 8, h - 8, '', 'RADIATION')

    def pipes(self, x, y, w):
        """Two brass pipes along a wall face."""
        g, m = self.m.ground, self.m
        for py in (y + 4, y + 13):
            g[py:py + 4, x:x + w] = m.g('wood')
            g[py, x:x + w] = m.g('wall_l')
            g[py + 3, x:x + w] = m.g('outline')
            for px in range(x + 8, x + w, 32):
                g[py - 1:py + 5, px:px + 3] = m.g('iron_d')

    def stripes(self, x, y, w, h):
        """Red and dark warning stripes on the floor."""
        g = self.m.ground
        ys, xs = np.mgrid[y:y + h, x:x + w]
        g[y:y + h, x:x + w] = np.where(((xs + ys) // 6) % 2 == 0, self.m.g('red'), self.m.g('iron_d'))

    def console(self, x, y, w=48):
        """A control panel with blinking lights against a wall, 16 high."""
        g, m = self.m.ground, self.m
        g[y:y + 16, x:x + w] = m.g('outline')
        g[y + 1:y + 15, x + 1:x + w - 1] = m.g('wall_m')
        g[y + 1:y + 4, x + 1:x + w - 1] = m.g('wall_l')
        for px in range(x + 4, x + w - 4, 8):
            g[y + 7:y + 10, px:px + 3] = m.g('flame') if (px // 8) % 2 else m.g('red')
        self.m.block(x, y + 4, w, 12)


def gen_gnomeregan():
    c = Gnomer('gnomeregan', 1024, 1024)
    m = c.m
    c.rect(448, 864, 128, 104)      # the way in
    c.rect(488, 960, 48, 64)        # the ramp up to Dun Morogh
    c.rect(416, 896, 32, 48)
    c.rect(160, 832, 256, 144)      # the trogg tunnels, Grubbis's
    c.rect(488, 752, 48, 112)
    c.rect(320, 560, 384, 192)      # the Hall of Gears, Viscous Fallout's
    c.rect(288, 600, 32, 48)
    c.rect(64, 448, 224, 208)       # the Dormitory
    c.rect(704, 608, 32, 48)
    c.rect(736, 512, 224, 224)      # the Engineering Labs, the Electrocutioner's
    c.rect(488, 448, 48, 112)
    c.rect(352, 288, 320, 160)      # the Launch Bay, the Crowd Pummeler's
    c.rect(488, 208, 48, 80)
    c.rect(320, 48, 384, 160)       # the Control Room, Thermaplugg's
    c.rect(80, 656, 48, 48)         # a hidden storeroom off the Dormitory
    c.rect(64, 704, 96, 64)
    c.render()
    c.exit(488, 1016, 'dun_morogh', 'gnomeregan_exit')

    for x, y, w, h, seed in ((380, 600, 72, 40, 1), (560, 660, 88, 48, 2), (420, 700, 64, 36, 3),
                             (220, 880, 56, 32, 4), (840, 660, 64, 40, 5), (120, 560, 56, 32, 6),
                             (580, 600, 56, 32, 7)):
        c.fallout(x, y, w, h, seed)
    c.pipes(320, 560, 384)
    c.pipes(736, 512, 224)
    c.pipes(320, 48, 384)
    for x in (360, 456, 552):
        c.gears(x, 288)
    c.stripes(384, 392, 256, 16)
    for x in (360, 456, 560):
        c.console(x, 72)
    c.console(760, 524)
    c.console(880, 524)
    c.crate(96, 480)
    c.crate(112, 480)
    c.crate(240, 470)
    c.secret(80, 632, 48, 24)
    m.chest(32, 112, 744, 29)

    m.spawn_group('CAVERNDEEP_BURROWER', 240, 900, 3, 50, seed=131)
    m.spawn('IRRADIATED_PILLAGER', 340, 860)
    m.spawn('GRUBBIS', 200, 870)
    m.spawn('LEPER_GNOME', 470, 900)
    m.spawn('LEPER_GNOME', 550, 900)
    m.spawn_group('IRRADIATED_SLIME', 460, 640, 3, 80, seed=132)
    m.spawn_group('LEPER_GNOME', 640, 720, 2, 30, seed=133)
    m.spawn('VISCOUS_FALLOUT', 600, 690)
    m.spawn_group('LEPER_GNOME', 160, 520, 3, 60, seed=134)
    m.spawn_group('DARK_IRON_AGENT', 200, 600, 2, 40, seed=135)
    m.spawn_group('ARCANE_NULLIFIER', 820, 620, 2, 40, seed=136)
    m.spawn('MECHANO_TANK', 900, 700)
    m.spawn('ELECTROCUTIONER_6000', 848, 580)
    m.spawn('MECHANO_TANK', 420, 330)
    m.spawn('MECHANO_TANK', 600, 330)
    m.spawn('ARCANE_NULLIFIER', 512, 420)
    m.spawn('CROWD_PUMMELER', 512, 352)
    m.spawn('DARK_IRON_AGENT', 400, 180)
    m.spawn('DARK_IRON_AGENT', 620, 180)
    m.spawn('MEKGINEER_THERMAPLUGG', 512, 128)
    # Where Thermaplugg's bombs come out: the hatches along the Control Room's walls.
    m.patrol = [(344, 120), (680, 120), (344, 196), (680, 196)]
    m.area(0, 0, 1024, 1024, 'Gnomeregan')
    m.area(160, 832, 256, 144, 'The Tunnels')
    m.area(320, 560, 384, 192, 'The Hall of Gears')
    m.area(64, 448, 224, 208, 'The Dormitory')
    m.area(736, 512, 224, 224, 'The Engineering Labs')
    m.area(352, 288, 320, 160, 'The Launch Bay')
    m.area(320, 48, 384, 160, 'The Control Room')
    m.music = 'DUNGEON'
    m.save()
    return m


GENERATORS = {
    'abbey': gen_abbey,
    'inn': gen_inn,
    'elwynn': gen_elwynn,
    'westfall': gen_westfall,
    'redridge': gen_redridge,
    'deadmines': gen_deadmines,
    'echo_ridge': gen_echo_ridge,
    'fargodeep': gen_fargodeep,
    'stormwind': gen_stormwind,
    'stockade': gen_stockade,
    'deeprun_tram': gen_deeprun_tram,
    'duskwood': gen_duskwood,
    'silverpine': gen_silverpine,
    'shadowfang': gen_shadowfang,
    'ironforge': gen_ironforge,
    'dun_morogh': gen_dun_morogh,
    'wetlands': gen_wetlands,
    'darkshore': gen_darkshore,
    'blackfathom_deeps': gen_blackfathom_deeps,
    'gnomeregan': gen_gnomeregan,
}


def main():
    maps = {name: generate() for name, generate in GENERATORS.items()}
    ids = [chest[0] for m in maps.values() for chest in m.chests]
    if len(ids) != len(set(ids)) or max(ids) >= 256:
        raise SystemExit(f'chest ids must be unique and below 256: {sorted(ids)}')
    print(f'{len(ids)} treasure chests')
    starts = {'elwynn': 'start', 'westfall': 'from_elwynn', 'stormwind': 'from_elwynn', 'redridge': 'from_elwynn',
              'duskwood': 'from_elwynn', 'silverpine': 'flight', 'dun_morogh': 'from_ironforge',
              'wetlands': 'from_dun_morogh', 'darkshore': 'from_menethil'}
    for name, m in maps.items():
        m.check_reachable(starts.get(name, 'entry'))
    write_minimaps(maps)


# Maps with a picture on the world map page, in the order D-pad left and right go through them.
# Interiors show the map their door leads to.
MINIMAPS = ['elwynn', 'stormwind', 'westfall', 'redridge', 'duskwood', 'silverpine', 'ironforge', 'dun_morogh',
            'wetlands', 'darkshore', 'echo_ridge', 'fargodeep', 'deadmines', 'stockade', 'shadowfang',
            'blackfathom_deeps', 'gnomeregan']


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
