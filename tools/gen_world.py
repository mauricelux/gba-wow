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
    wg.forest(m, trees, 0, m.height - border, 1056, border, holes=[(144, m.height - border, 48, border)])
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

    # The road south to Stranglethorn came later than the woods: the trees and props are placed as
    # before it existed (so old saves stand where they stood), minus the ones in its way.
    south_road = [(128, 864), (132, 900), (152, 944), (168, 1024)]
    wg.corners_along(m, 'south_road', south_road, 1.1)
    road = wg.corners(m, 'south_road')
    gap = (144, m.height - border, 48, border)
    m.block(*gap)
    rng = np.random.default_rng(23)
    path = wg.corners(m, 'path')
    water = wg.corners(m, 'water')
    clear = [(1080, 260, 420, 420), (80, 280, 304, 224), (170, 620, 260, 260), (56, 750, 140, 110),
             (450, 600, 130, 140), (380, 800, 260, 180), (700, 720, 260, 260), (1100, 730, 290, 240),
             (900, 140, 220, 220), (440, 180, 120, 100), (690, 300, 140, 140)]
    placed = 0
    skipped = []
    for i in range(3000):
        if placed >= 400:
            break
        x, y = int(rng.uniform(40, 1460)), int(rng.uniform(120, 960))
        mx, my = (x + 16) // 16, (y + 40) // 16
        near = path[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max() + \
            water[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max()
        if near == 0 and m.area_free(x, y, 40, 56) and \
                not any(cx - 40 <= x <= cx + cw and cy - 56 <= y <= cy + ch for cx, cy, cw, ch in clear):
            variant = int(rng.integers(0, 3))
            if road[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max():
                foot = (x // 8 * 8 + 8, y // 8 * 8 + 32, 16, 16)
                m.block(*foot)
                skipped.append(foot)
            elif placed % 7 == 3:
                wg.tree(m, dead, x, y, variant, kind='small')
            else:
                wg.tree(m, trees, x, y, variant, kind=('oak', 'pine', 'oak', 'small')[placed % 4])
            placed += 1
    for foot in skipped + [gap]:
        m.unblock(*foot)
    wg.paint_paths(m, 'south_road')
    wg.scatter_props(m, rng, 90, DUSKWOOD_PROPS, (56, 120, 1400, 840),
                     avoid=[(1080, 260, 420, 420), (80, 280, 304, 224), (96, 848, 96, 112)])

    m.point('from_elwynn', 568, 24)
    m.warp(544, 0, 48, 8, 'elwynn', 'from_duskwood')
    m.point('from_redridge', 1336, 24)
    m.warp(1312, 0, 48, 8, 'redridge', 'from_duskwood')
    m.point('from_stranglethorn', 168, 996)
    m.warp(144, 1016, 48, 8, 'stranglethorn', 'from_duskwood')
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
    m.point('from_hillsbrad', 640, 24)
    m.warp(624, 0, 32, 8, 'hillsbrad', 'from_wetlands')
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


# ---------------------------------------------------------------------------------------------
# Hillsbrad Foothills (Southshore, the Alterac foothills and Durnholde Keep)
# ---------------------------------------------------------------------------------------------

HILLSBRAD_PROPS = ('flowers', 'tall_grass', 'bush', 'rock', 'fern', 'flower_bush', 'wide_bush', 'stump',
                   'big_rock')


def gen_hillsbrad():
    m = Map('hillsbrad', 1280, 1024,
            Palette([wg.TERRAIN_HILLSBRAD, wg.BUILDINGS, wg.FARM, wg.ROCK_HILLSBRAD]),
            Palette([wg.OVERHEAD_LEAVES, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    # --- the Alterac Mountains along the north; the Thandol road comes in from the east ------------
    rock = wg.corners(m, 'rock')
    rock[0:5, :] = 1
    rock[0:9, 0:8] = 1
    rock[:, 0:3] = 1
    rock[:, 78:] = 1
    rock[40:47, 78:] = 0                # the road east to the Thandol Span
    rock[5:12, 62:78] = 1               # the crag over Growless Cave
    wg.paint_cliffs(m)

    # --- the sea along the south coast --------------------------------------------------------------
    wg.corners_rect(m, 'water', 0, 944, 1280, 80)
    wg.corners_along(m, 'water', [(0, 912), (240, 928), (480, 920), (680, 936), (1000, 928), (1280, 912)],
                     1.6)
    wg.paint_water(m)

    # --- roads ----------------------------------------------------------------------------------------
    roads = [
        [(1280, 688), (1120, 688), (1000, 760), (900, 800)],                 # from the Thandol Span
        [(880, 720), (840, 600), (820, 520), (760, 380), (680, 260)],        # north to Strahnbrad
        [(820, 520), (620, 520), (440, 520), (260, 480)],                    # west to the fields
        [(440, 520), (360, 400), (300, 300)],                                # up to Crushridge Hold
        [(820, 520), (960, 500), (1100, 540)],                               # east to Durnholde
        [(760, 380), (900, 320), (1040, 300)],                               # to Growless Cave
        [(640, 840), (520, 860), (400, 870)],                                # the Western Strand
    ]
    for points in roads:
        wg.corners_along(m, 'path', points, 1.1)
    wg.paint_paths(m)

    # --- Southshore ---------------------------------------------------------------------------------
    wg.cobbles(m, 720, 816, 256, 72)
    hall = wg.house(m, 848, 696, 112, 112, style='stone', roof_colors=('roof_d', 'roof_m', 'roof_l'),
                    roof_ridge='roof_h', roof_outline='outline')
    inn = wg.house(m, 720, 712, 112, 96, roof_colors=('red_d', 'red_m', 'red_l'), roof_ridge='red_l',
                   roof_outline='o2')
    shop = wg.house(m, 984, 736, 80, 80, roof_colors=('roof_d', 'roof_m', 'roof_l'), roof_ridge='roof_h',
                    roof_outline='outline')
    chapel = wg.house(m, 608, 712, 96, 96, style='stone', roof_colors=('roof_d', 'roof_m', 'roof_l'),
                      roof_ridge='roof_h', roof_outline='outline')
    wg.anvil(m, 728, 832)
    wg.well(m, 800, 832)
    wg.pier(m, 896, 888, 32, 104)
    m.npc('MALEB', hall[0], hall[1] + 4)
    m.npc('REDPATH', 896, 852)
    m.npc('ANDERSON', inn[0], inn[1] + 4)
    m.npc('DIBBS', 952, 864)
    m.npc('SOUTHSHORE_VENDOR', shop[0], shop[1] + 4)
    m.npc('SOUTHSHORE_SMITH', 744, 868)
    m.npc('RALEIGH', chapel[0], chapel[1] + 4)
    m.npc('DARLA', 1064, 848)
    m.point('flight', 1064, 868)
    m.npc('DARREN_MALVEW', 1104, 720)
    m.npc('SOUTHSHORE_GUARD', 1040, 700)
    m.point('southshore_respawn', 856, 876)
    m.area(600, 680, 520, 260, 'Southshore')

    # --- Hillsbrad Fields: the farms the Forsaken have taken ---------------------------------------
    wg.wheat_field(m, 168, 408, 128, 64)
    wg.wheat_field(m, 168, 584, 112, 56)
    wg.crop_field(m, 304, 600, 96, 48)
    wg.house(m, 320, 392, 96, 96, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'), roof_ridge='thatch_l',
             roof_outline='o2')
    wg.house(m, 424, 576, 96, 80, roof_colors=('red_d', 'red_m', 'red_l'), roof_ridge='red_l', roof_outline='o2')
    wg.fence(m, 160, 400, 144)
    wg.fence(m, 160, 656, 248)
    m.spawn_group('FORSAKEN_THUG', 260, 520, 5, 70, seed=141)
    m.spawn_group('FORSAKEN_HERBALIST', 420, 520, 3, 50, seed=142)
    m.spawn_group('FORSAKEN_THUG', 380, 680, 2, 30, seed=143)
    m.spawn('FORSAKEN_COURIER', 240, 552)
    m.area(140, 380, 400, 300, 'Hillsbrad Fields')

    # --- the Western Strand: the Torn Fin murlocs ---------------------------------------------------
    m.spawn_group('TORN_FIN_TIDEHUNTER', 300, 880, 4, 70, seed=144)
    m.spawn_group('TORN_FIN_ORACLE', 460, 890, 3, 50, seed=145)
    m.chest(37, 112, 880, 30)
    m.area(80, 820, 500, 120, 'Western Strand')

    # --- Darrow Hill: bears and mountain lions ------------------------------------------------------
    m.spawn_group('GRAY_BEAR', 600, 400, 4, 80, seed=146)
    m.spawn_group('GRAY_BEAR', 980, 640, 2, 40, seed=147)
    m.spawn_group('MOUNTAIN_LION', 700, 280, 3, 60, seed=148)
    m.spawn_group('MOUNTAIN_LION', 560, 700, 3, 50, seed=149)
    m.area(500, 300, 340, 280, 'Darrow Hill')

    # --- Crushridge Hold: the Alterac ogres ---------------------------------------------------------
    wg.camp(m, 168, 176, 176, 96, tents=[(184, 184), (296, 184)], fire=(248, 224))
    m.spawn_group('CRUSHRIDGE_OGRE', 260, 300, 5, 80, seed=150)
    m.spawn_group('CRUSHRIDGE_MAGE', 220, 230, 3, 40, seed=151)
    m.spawn_group('CRUSHRIDGE_ENFORCER', 340, 240, 2, 30, seed=152)
    m.area(140, 150, 300, 220, 'Crushridge Hold')

    # --- Strahnbrad: the Syndicate's camp and Gravis Slipknot --------------------------------------
    ruin(m, 528, 104, 96, 32, seed=11)
    ruin(m, 720, 104, 80, 32, seed=12)
    wg.camp(m, 576, 144, 176, 96, tents=[(592, 152), (704, 152)], fire=(648, 192))
    m.spawn_group('SYNDICATE_FOOTPAD', 640, 290, 5, 80, seed=153)
    m.spawn_group('SYNDICATE_THIEF', 560, 240, 3, 50, seed=154)
    m.spawn('GRAVIS_SLIPKNOT', 664, 168)
    m.area(520, 90, 300, 220, 'Strahnbrad')

    # --- Durnholde Keep: the Syndicate's stronghold ------------------------------------------------
    keep_wall(m, 1008, 384, 192, 64, gate_w=40)
    wg.camp(m, 1000, 480, 208, 96, tents=[(1016, 488), (1160, 488)], fire=(1096, 520))
    ruin(m, 984, 592, 96, 32, seed=13)
    ruin(m, 1136, 592, 80, 32, seed=14)
    m.spawn_group('SYNDICATE_ENFORCER', 1080, 560, 4, 70, seed=155)
    m.spawn_group('SYNDICATE_SHADOW_MAGE', 1100, 500, 3, 50, seed=156)
    m.spawn_group('SYNDICATE_FOOTPAD', 960, 460, 2, 30, seed=157)
    m.chest(38, 1192, 472, 33)
    m.area(970, 340, 270, 290, 'Durnholde Keep')

    # --- Growless Cave: the yetis -------------------------------------------------------------------
    cave = wg.mine_entrance(m, 1080, 152, 64, 48)
    m.block(cave[0] - 16, cave[1] - 24, 32, 32)
    m.spawn_group('MOUNTAIN_YETI', 1060, 280, 5, 70, seed=158)
    m.spawn('BLOODFANG', cave[0], cave[1] + 32)
    m.area(980, 180, 260, 160, 'Growless Cave')

    # --- trees, and a clearing in the woods above the fields ---------------------------------------
    wg.forest(m, trees, 48, 320, 96, 160, kinds=('oak', 'birch'), holes=[(64, 352, 56, 48)],
              secrets=[(104, 368, 48, 32)])
    m.chest(36, 88, 380, 32)
    rng = np.random.default_rng(41)
    path = wg.corners(m, 'path')
    water = wg.corners(m, 'water')
    clear = [(600, 680, 520, 260), (140, 380, 400, 300), (140, 150, 300, 220), (520, 90, 300, 220),
             (970, 340, 270, 290), (980, 180, 260, 160), (40, 310, 110, 180)]
    placed = 0
    for _ in range(3000):
        if placed >= 130:
            break
        x, y = int(rng.uniform(56, 1180), ), int(rng.uniform(100, 860))
        mx, my = (x + 16) // 16, (y + 40) // 16
        near = path[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max() + \
            water[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max()
        if near == 0 and m.area_free(x, y, 40, 56) and \
                not any(cx - 40 <= x <= cx + cw and cy - 56 <= y <= cy + ch for cx, cy, cw, ch in clear):
            wg.tree(m, trees, x, y, int(rng.integers(0, 3)), kind=('oak', 'birch', 'small', 'oak')[placed % 4])
            placed += 1
    wg.scatter_props(m, rng, 90, HILLSBRAD_PROPS, (56, 100, 1160, 780),
                     avoid=[(600, 680, 520, 260), (140, 380, 400, 300)])

    m.point('from_wetlands', 1232, 688)
    m.warp(1272, 664, 8, 48, 'wetlands', 'from_hillsbrad')
    m.area(1140, 640, 140, 100, "Thoradin's Wall")
    m.area(0, 0, 1280, 1024, 'Hillsbrad Foothills')
    m.music = 'HILLSBRAD'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Tirisfal Glades (the grounds of the Scarlet Monastery)
# ---------------------------------------------------------------------------------------------

def gen_tirisfal():
    m = Map('tirisfal', 768, 512,
            Palette([wg.TERRAIN_TIRISFAL, wg.BUILDINGS_SCARLET, wg.FARM, wg.ROCK_TIRISFAL]),
            Palette([wg.OVERHEAD_LEAVES_TIRISFAL, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    rock = wg.corners(m, 'rock')
    rock[0:5, :] = 1
    rock[0:12, 0:4] = 1
    rock[0:12, 44:] = 1
    wg.paint_cliffs(m)

    wg.corners_along(m, 'path', [(400, 470), (400, 400), (432, 320), (432, 240)], 1.1)
    wg.corners_along(m, 'path', [(420, 330), (300, 320), (200, 300)], 0.9)
    wg.paint_paths(m)
    wg.corners_along(m, 'water', [(768, 260), (700, 300), (680, 400), (720, 512)], 1.3)
    wg.paint_water(m)

    # The Monastery's front: the Library's doors in the middle, the Armory's in the west wing and the
    # Cathedral's in the east wing; the Graveyard's crypt further west.
    door = wg.abbey(m, 352, 96)
    for x, wing in ((256, 'sm_armory'), (512, 'sm_cathedral')):
        wg.bricks(m, m.ground, x, 160, 96, 64, 'stone_d', 'stone_m', 'stone_l')
        m.ground[160:163, x:x + 96] = m.g('stone_h')
        m.ground[223, x:x + 96] = m.g('outline')
        m.block(x, 160, 96, 64)
        wg.door(m, x + 40, 196, 28)
        for bx in (x + 16, x + 72):
            m.ground[172:204, bx:bx + 8] = m.g('banner')
            m.ground[172:204, bx] = m.g('banner_d')
            m.ground[172:174, bx:bx + 8] = m.g('gold')
        m.warp(x + 36, 216, 24, 8, wing, 'entry')
        m.point(wing[3:] + '_exit', x + 48, 244)
    m.warp(door[0] - 12, door[1] - 14, 24, 8, 'sm_library', 'entry')
    m.point('library_exit', door[0], door[1] + 14)
    graveyard(m, 112, 240, 176, 96, seed=21, gaps=[(176, 240, 48, 96)])
    crypt_door = crypt(m, 160, 144, 80, 80)
    m.warp(crypt_door[0] - 12, crypt_door[1] - 14, 24, 8, 'sm_graveyard', 'entry')
    m.point('graveyard_exit', crypt_door[0], crypt_door[1] + 14)
    m.area(96, 80, 560, 260, 'The Scarlet Monastery')

    # The Argent Dawn's watch on the Monastery.
    wg.camp(m, 304, 400, 176, 80, tents=[(320, 408)], fire=(392, 432))
    m.npc('ARGENT_SCOUT', 440, 424)
    m.npc('GRYPHON_TIRISFAL', 488, 448)
    m.point('flight', 488, 468)
    m.point('tirisfal_respawn', 400, 476)
    m.area(290, 380, 220, 120, 'The Argent Watch')

    m.spawn_group('SCARLET_CONVERT', 560, 300, 3, 40, seed=161)
    m.spawn('SCARLET_SCOUT', 300, 300)
    m.spawn('SCARLET_SCOUT', 600, 260)
    m.spawn('SCARLET_CONVERT', 340, 262)
    m.spawn('SCARLET_CONVERT', 492, 262)
    m.spawn('SCARLET_SCOUT', 640, 200)

    wg.forest(m, trees, 592, 360, 96, 120, kinds=('oak',), holes=[(616, 392, 48, 48)],
              secrets=[(568, 400, 56, 32)])
    m.chest(39, 640, 432, 31)
    rng = np.random.default_rng(43)
    path = wg.corners(m, 'path')
    placed = 0
    for _ in range(400):
        if placed >= 24:
            break
        x, y = int(rng.uniform(40, 700)), int(rng.uniform(240, 440))
        mx, my = (x + 16) // 16, (y + 40) // 16
        if path[max(0, my - 3):my + 4, max(0, mx - 3):mx + 4].max() == 0 and m.area_free(x, y, 40, 56) \
                and not (280 <= x <= 520 and 360 <= y <= 512) and not (96 <= x <= 300 and 220 <= y <= 350):
            wg.tree(m, trees, x, y, placed, kind=('oak', 'small')[placed % 2])
            placed += 1
    wg.scatter_props(m, rng, 20, ('tall_grass', 'rock', 'stump', 'log', 'fern'), (40, 240, 680, 220),
                     avoid=[(290, 380, 220, 120), (96, 220, 220, 140)])
    m.area(0, 0, 768, 512, 'Tirisfal Glades')
    m.music = 'DUSKWOOD'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# The Scarlet Monastery: the Graveyard and the Library
# ---------------------------------------------------------------------------------------------

MONASTERY = [
    ('outline', (24, 18, 20)), ('top_d', (44, 36, 36)), ('top_m', (62, 52, 50)),
    ('wall_d', (104, 92, 84)), ('wall_m', (144, 130, 116)), ('wall_l', (184, 170, 150)),
    ('floor_d', (96, 86, 80)), ('floor_m', (128, 116, 106)), ('floor_l', (160, 148, 134)),
    ('iron_d', (40, 36, 44)), ('iron_l', (150, 150, 164)), ('straw', (216, 176, 80)),
    ('wood', (112, 70, 42)), ('flame', (252, 224, 120)), ('red', (160, 28, 36)),
]

MONASTERY_OVERHEAD = [
    ('outline', (24, 18, 20)), ('top_d', (44, 36, 36)), ('top_m', (62, 52, 50)),
    ('wall_d', (104, 92, 84)), ('wall_m', (144, 130, 116)), ('wall_l', (184, 170, 150)),
]

CRYPT = [
    ('outline', (14, 14, 20)), ('top_d', (26, 28, 36)), ('top_m', (38, 40, 50)),
    ('wall_d', (56, 58, 68)), ('wall_m', (80, 82, 94)), ('wall_l', (112, 114, 124)),
    ('floor_d', (60, 64, 64)), ('floor_m', (78, 78, 86)), ('floor_l', (102, 102, 110)),
    ('iron_d', (32, 32, 40)), ('iron_l', (140, 144, 156)), ('straw', (200, 196, 176)),
    ('wood', (88, 60, 40)), ('flame', (176, 248, 176)), ('red', (136, 24, 32)),
]

CRYPT_OVERHEAD = [
    ('outline', (14, 14, 20)), ('top_d', (26, 28, 36)), ('top_m', (38, 40, 50)),
    ('wall_d', (56, 58, 68)), ('wall_m', (80, 82, 94)), ('wall_l', (112, 114, 124)),
]


class Monastery(Castle):
    """The Scarlet Monastery: pale stone halls, crimson carpets and banners, books. The Graveyard's
    crypts use the same props in cold stone with ghostly green candles ('straw' is bone there)."""

    def __init__(self, name, width, height, crypt=False):
        self.m = Map(name, width, height, Palette([CRYPT if crypt else MONASTERY]),
                     Palette([CRYPT_OVERHEAD if crypt else MONASTERY_OVERHEAD]))
        self.floor = np.zeros((height // 8, width // 8), dtype=bool)
        self.face = np.zeros_like(self.floor)

    def bookshelf(self, x, y, w):
        """Shelves of books along a wall face (y is the face's top, 24 px tall)."""
        g, m = self.m.ground, self.m
        g[y + 2:y + 24, x:x + w] = m.g('wood')
        g[y + 2:y + 24, x] = m.g('outline')
        g[y + 2:y + 24, x + w - 1] = m.g('outline')
        books = ('red', 'straw', 'iron_l', 'wall_l', 'red', 'top_m')
        for shelf in (y + 4, y + 13):
            for bx in range(x + 1, x + w - 1):
                kind = (bx * 7 + shelf * 3) % 11
                if kind < 9:
                    g[shelf + (kind % 2):shelf + 8, bx] = m.g(books[kind % len(books)])
            g[shelf + 8, x:x + w] = m.g('outline')
        self.m.block(x, y + 8, w, 16)

    def slab(self, x, y):
        """A grave slab set in the floor, 16x24."""
        g, m = self.m.ground, self.m
        g[y:y + 24, x:x + 16] = m.g('outline')
        g[y + 1:y + 23, x + 1:x + 15] = m.g('wall_m')
        g[y + 1:y + 3, x + 1:x + 15] = m.g('wall_l')
        g[y + 6:y + 16, x + 7:x + 9] = m.g('wall_d')
        g[y + 9:y + 11, x + 4:x + 12] = m.g('wall_d')

    def bones(self, x, y):
        """A heap of bones and a skull on the floor, 16x8."""
        g, m = self.m.ground, self.m
        g[y + 4, x:x + 16] = m.g('straw')
        g[y + 6, x + 2:x + 12] = m.g('straw')
        g[y + 2:y + 5, x + 10:x + 15] = m.g('straw')
        g[y + 3, x + 11] = g[y + 3, x + 13] = m.g('outline')
        g[y + 7, x:x + 16] = m.g('floor_d')

    def candles(self, x, y):
        """A stand of candles, 8x16."""
        g, m = self.m.ground, self.m
        g[y + 6:y + 16, x + 3:x + 5] = m.g('iron_d')
        g[y + 14:y + 16, x + 1:x + 7] = m.g('iron_d')
        for cx in (x + 1, x + 3, x + 5):
            g[y + 3:y + 6, cx:cx + 2] = m.g('wall_l')
            g[y + 1:y + 3, cx:cx + 2] = m.g('flame')
        self.m.block(x, y + 10, 8, 6)

    def dummy(self, x, y):
        """A straw training dummy on a post, 16x24."""
        g, m = self.m.ground, self.m
        g[y + 10:y + 24, x + 7:x + 9] = m.g('wood')
        g[y + 22:y + 24, x + 3:x + 13] = m.g('wood')
        g[y + 2:y + 14, x + 3:x + 13] = m.g('outline')
        g[y + 3:y + 13, x + 4:x + 12] = m.g('straw')
        g[y + 7, x + 1:x + 15] = m.g('wood')
        g[y:y + 3, x + 5:x + 11] = m.g('outline')
        g[y + 1:y + 3, x + 6:x + 10] = m.g('straw')
        g[y + 6:y + 9, x + 6:x + 10] = m.g('red')
        self.m.block(x + 2, y + 14, 12, 10)

    def altar(self, x, y, w=64, h=24):
        """A stone altar with a crimson cloth."""
        g, m = self.m.ground, self.m
        g[y:y + h, x:x + w] = m.g('outline')
        g[y + 1:y + h - 1, x + 1:x + w - 1] = m.g('wall_l')
        g[y + 4:y + h - 4, x + 4:x + w - 4] = m.g('red')
        g[y + h - 5:y + h - 1, x + 1:x + w - 1] = m.g('wall_d')
        g[y + 6:y + 10, x + w // 2 - 2:x + w // 2 + 2] = m.g('straw')
        self.m.block(x, y + 4, w, h - 4)


def gen_sm_graveyard():
    c = Monastery('sm_graveyard', 1024, 512, crypt=True)
    m = c.m
    c.rect(448, 400, 128, 88)       # the crypt's door hall
    c.rect(488, 488, 48, 24)        # the steps up to the cemetery
    c.rect(320, 416, 128, 48)       # corridor west
    c.rect(96, 352, 224, 136)       # the torture chamber, Vishas's
    c.rect(128, 296, 48, 56)        # a hidden niche behind the racks
    c.rect(488, 304, 48, 96)        # corridor north
    c.rect(320, 224, 384, 80)       # the hall of the dead, Azshir's
    c.rect(704, 232, 64, 48)        # corridor east
    c.rect(768, 160, 208, 208)      # the chapel, Thalnos's
    c.rect(488, 184, 48, 40)        # corridor north
    c.rect(336, 32, 352, 152)       # the ossuary, Ironspine's
    c.render()
    c.exit(488, 504, 'tirisfal', 'graveyard_exit')

    # The torture chamber: cells, racks and chains.
    for x in (112, 176, 240):
        c.chains(x, 360)
    c.rack(200, 356)
    c.bars(232, 352, 72)
    c.secret(128, 296, 48, 56)
    m.chest(40, 152, 340, 32)
    for x, y in ((136, 440), (264, 400)):
        c.straw(x, y)
    # The hall of the dead: grave slabs in rows.
    for x in range(344, 690, 40):
        c.slab(x, 260)
    for x in (336, 680):
        c.candles(x, 236)
    # The chapel: an altar, candles and pews.
    c.altar(840, 176)
    for y in (232, 272, 312):
        c.table(792, y, 56, 16)
        c.table(896, y, 56, 16)
    for x in (800, 944):
        c.candles(x, 176)
    # The ossuary: bones everywhere.
    for x, y in ((360, 80), (420, 140), (600, 70), (640, 150), (500, 110), (380, 160), (660, 110)):
        c.bones(x, y)
    for x in (360, 512, 664):
        c.torch(x, 36)

    m.spawn('SCARLET_TORTURER', 464, 424)
    m.spawn('SCARLET_TORTURER', 560, 424)
    m.spawn('SCARLET_TORTURER', 160, 430)
    m.spawn('SCARLET_TORTURER', 260, 450)
    m.spawn('INTERROGATOR_VISHAS', 208, 404)
    m.spawn('HAUNTING_PHANTASM', 512, 330)
    for x, y in ((360, 280), (440, 250), (600, 250), (660, 286)):
        m.spawn('HAUNTING_PHANTASM' if x in (360, 600) else 'UNFETTERED_SPIRIT', x, y)
    m.spawn('AZSHIR_THE_SLEEPLESS', 512, 248)
    for x, y in ((800, 340), (940, 340), (870, 290)):
        m.spawn('SCARLET_TORTURER' if x == 870 else 'UNFETTERED_SPIRIT', x, y)
    m.spawn('BLOODMAGE_THALNOS', 872, 216)
    for x, y in ((400, 120), (624, 120), (512, 160)):
        m.spawn('HAUNTING_PHANTASM' if x == 512 else 'UNFETTERED_SPIRIT', x, y)
    m.spawn('IRONSPINE', 512, 72)
    m.area(0, 0, 1024, 512, 'Scarlet Monastery: Graveyard')
    m.area(96, 352, 224, 136, 'The Torture Chamber')
    m.area(320, 224, 384, 80, 'The Hall of the Dead')
    m.area(768, 160, 208, 208, 'The Chapel of the Dead')
    m.area(336, 32, 352, 152, 'The Ossuary')
    m.music = 'MONASTERY'
    m.save()
    return m


def gen_sm_library():
    c = Monastery('sm_library', 1024, 512)
    m = c.m
    c.rect(448, 400, 128, 88)       # the vestibule
    c.rect(488, 488, 48, 24)        # the doors out
    c.rect(488, 320, 48, 80)        # corridor north
    c.rect(288, 216, 448, 104)      # the Gallery
    c.rect(264, 232, 24, 56)        # the kennel door
    c.rect(64, 184, 200, 176)       # the kennels, Loksey's
    c.rect(736, 232, 24, 56)        # the stacks' door
    c.rect(760, 184, 224, 192)      # the stacks
    c.rect(912, 376, 48, 24)        # a hidden reading room
    c.rect(896, 400, 80, 64)
    c.rect(488, 168, 48, 48)        # corridor north
    c.rect(304, 24, 416, 144)       # the Athenaeum, Doan's
    c.render()
    c.exit(488, 504, 'tirisfal', 'library_exit')

    c.rug(496, 216, 32, 184)
    c.rug(312, 248, 400, 40)
    for x in (296, 360, 632, 696):
        c.bookshelf(x, 216, 40)
    for x in (456, 552):
        c.banner(x, 220)
    for x in (72, 216):
        c.straw(x, 320)
    c.bucket(120, 320)
    for x in (768, 840, 920):
        c.bookshelf(x, 184, 56)
    for y in (264, 320):
        c.table(784, y, 64, 20)
        c.table(888, y, 64, 20)
    c.secret(912, 376, 48, 0)
    c.desk(900, 440, 32, 20)
    m.chest(41, 952, 452, 34)
    for x in (320, 392, 464, 560, 632, 680):
        c.bookshelf(x, 24, 40 if x != 680 else 32)
    c.rug(472, 72, 80, 96)
    c.desk(488, 40, 48, 24)
    for x in (336, 680):
        c.candles(x, 120)

    m.spawn('SCARLET_GALLANT', 464, 424)
    m.spawn('SCARLET_ADEPT', 560, 424)
    for x, y in ((340, 300), (420, 296), (600, 296), (690, 300), (512, 250)):
        m.spawn(('SCARLET_MONK', 'SCARLET_GALLANT', 'SCARLET_ADEPT', 'SCARLET_GALLANT', 'SCARLET_MONK')[
            (x // 80) % 5], x, y)
    m.spawn('SCARLET_BEASTMASTER', 200, 320)
    for x, y in ((120, 240), (200, 240)):
        m.spawn('SCARLET_TRACKING_HOUND', x, y)
    m.spawn('HOUNDMASTER_LOKSEY', 160, 270)
    for x, y in ((800, 240), (940, 240), (820, 350), (930, 350)):
        m.spawn('SCARLET_DIVINER' if x in (800, 930) else 'SCARLET_CHAPLAIN', x, y)
    m.spawn('SCARLET_MONK', 512, 192)
    for x, y in ((360, 100), (664, 100)):
        m.spawn('SCARLET_CHAPLAIN' if x < 512 else 'SCARLET_DIVINER', x, y)
    m.spawn('ARCANIST_DOAN', 512, 84)
    m.area(0, 0, 1024, 512, 'Scarlet Monastery: Library')
    m.area(288, 216, 448, 104, 'The Gallery')
    m.area(64, 184, 200, 176, 'The Kennels')
    m.area(760, 184, 224, 192, 'The Stacks', 'THE_STACKS')
    m.area(304, 24, 416, 144, 'The Athenaeum')
    m.music = 'MONASTERY'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Stranglethorn Vale
# ---------------------------------------------------------------------------------------------

STRANGLETHORN_PROPS = ('fern', 'bush', 'wide_bush', 'tall_grass', 'flower_bush', 'fern', 'rock', 'log')


def gen_stranglethorn():
    m = Map('stranglethorn', 1024, 1536,
            Palette([wg.TERRAIN_STRANGLETHORN, wg.BUILDINGS_JUNGLE, wg.FARM, wg.ROCK_STRANGLETHORN]),
            Palette([wg.OVERHEAD_LEAVES_JUNGLE, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    # --- the mountains along the east, and the cliffs around Booty Bay with a tunnel through them ------
    rock = wg.corners(m, 'rock')
    rock[:93, 60:] = 1
    rock[73:79, 37:60] = 1
    rock[73:79, 47:51] = 0              # the tunnel down to Booty Bay
    rock[73:93, 36:39] = 1              # the bay's west cliff
    wg.paint_cliffs(m)

    # --- the sea along the west and south coasts, Booty Bay's harbor and Lake Nazferiti -------------
    wg.corners_rect(m, 'water', 0, 608, 24, 928)
    wg.corners_along(m, 'water', [(0, 560), (40, 700), (56, 900), (40, 1100), (56, 1300), (24, 1536)], 1.8)
    wg.corners_rect(m, 'water', 0, 1488, 1024, 48)
    wg.corners_along(m, 'water', [(0, 1456), (200, 1472), (400, 1464), (576, 1480)], 1.4)
    wg.corners_rect(m, 'water', 608, 1456, 352, 80)
    wg.corners_ellipse(m, 'water', 784, 1472, 150, 40)
    wg.corners_ellipse(m, 'water', 800, 540, 72, 44)
    wg.paint_water(m)

    # --- roads --------------------------------------------------------------------------------------
    roads = [
        [(304, 0), (304, 120), (312, 200)],                                          # from Duskwood
        [(312, 200), (480, 250), (640, 290), (696, 330)],                            # to Nesingwary's
        [(480, 250), (440, 420), (420, 600), (460, 800), (520, 980), (600, 1100), (720, 1150), (776, 1200),
         (776, 1376)],                                                                # south to Booty Bay
        [(420, 600), (300, 630), (200, 640)],                                        # Zul'Kunda
        [(460, 800), (620, 780), (720, 760)],                                        # Mistvale Valley
        [(520, 980), (380, 1080), (260, 1160)],                                      # the Bloodsail Compound
        [(696, 330), (800, 300), (860, 240)],                                        # the Kal'ai Ruins
    ]
    for points in roads:
        wg.corners_along(m, 'path', points, 1.1)
    wg.paint_paths(m)

    # --- the jungle's edge: the way back north to Duskwood --------------------------------------------
    border = 48
    wg.forest(m, trees, 0, 0, 1024, border, kinds=('oak', 'palm'), holes=[(280, 0, 48, border)])
    wg.forest(m, trees, 0, border, border, 528, kinds=('oak', 'palm'))

    # --- the Rebel Camp -----------------------------------------------------------------------------
    wg.camp(m, 216, 120, 192, 96, tents=[(224, 128), (360, 128)], fire=(256, 176))
    wg.crates(m, 368, 176)
    m.npc('DOREN', 312, 144)
    m.npc('BLUTH', 344, 196)
    m.npc('REBEL_SOLDIER', 280, 204)
    m.point('rebel_camp_respawn', 312, 236)
    m.area(200, 100, 230, 140, 'Rebel Camp')

    # --- Nesingwary's Expedition --------------------------------------------------------------------
    wg.camp(m, 600, 288, 192, 96, tents=[(608, 296), (752, 296)], fire=(640, 344))
    wg.crates(m, 720, 352)
    m.npc('HEMET', 696, 316)
    m.npc('AJECK', 664, 372)
    m.npc('ERLGADIN', 760, 352)
    m.npc('BARNIL', 700, 372)
    m.area(580, 270, 240, 140, "Nesingwary's Expedition")

    # --- the Kal'ai Ruins: King Bangalash ---------------------------------------------------------------
    ruin(m, 816, 160, 96, 32, seed=21)
    ruin(m, 880, 224, 64, 32, seed=22)
    m.spawn('KING_BANGALASH', 864, 208)
    m.spawn('STRANGLETHORN_TIGER', 824, 216)
    m.spawn('STRANGLETHORN_TIGER', 920, 204)
    m.area(790, 120, 170, 160, "Kal'ai Ruins")

    # --- Lake Nazferiti: the tigers -----------------------------------------------------------------
    m.spawn_group('STRANGLETHORN_TIGER', 568, 540, 6, 64, seed=171)
    m.spawn_group('STRANGLETHORN_TIGER', 720, 620, 4, 50, seed=172)
    m.area(560, 420, 380, 240, 'Lake Nazferiti')

    # --- the Shadowmaw Thicket: the panthers --------------------------------------------------------
    m.spawn_group('SHADOWMAW_PANTHER', 150, 410, 7, 80, seed=173)
    m.area(60, 290, 300, 200, 'Shadowmaw Thicket')

    # --- Zul'Kunda: the Bloodscalp trolls -----------------------------------------------------------
    ruin(m, 96, 560, 112, 32, seed=23)
    ruin(m, 248, 576, 96, 32, seed=24)
    ruin(m, 112, 704, 96, 32, seed=25)
    ruin(m, 256, 712, 80, 32, seed=26)
    m.spawn_group('BLOODSCALP_WARRIOR', 210, 660, 6, 80, seed=174)
    m.spawn_group('BLOODSCALP_SHAMAN', 150, 640, 3, 40, seed=175)
    m.spawn_group('BLOODSCALP_HEADHUNTER', 290, 780, 5, 60, seed=176)
    m.spawn('MOGH_THE_UNDYING', 176, 616)
    m.chest(43, 168, 772, 37)
    m.area(60, 520, 340, 320, "Zul'Kunda")

    # --- the Ruins of Jubuwal: raptors ------------------------------------------------------------------
    ruin(m, 528, 704, 96, 32, seed=27)
    m.spawn_group('STRANGLETHORN_RAPTOR', 560, 680, 7, 80, seed=177)
    m.area(470, 620, 200, 180, 'Ruins of Jubuwal')

    # --- Mistvale Valley: the gorillas ----------------------------------------------------------------
    for bx, by in ((680, 840), (900, 720), (912, 900), (760, 920)):
        wg.big_rock(m, bx, by)
    m.spawn_group('MISTVALE_GORILLA', 780, 820, 6, 80, seed=178)
    m.spawn_group('ELDER_MISTVALE_GORILLA', 860, 880, 3, 50, seed=179)
    m.area(640, 700, 320, 260, 'Mistvale Valley')

    # --- the Crystalvein Mine: lashtail raptors -------------------------------------------------------
    mine = wg.mine_entrance(m, 272, 896, 64, 48)
    m.block(mine[0] - 16, mine[1] - 24, 32, 32)
    m.spawn_group('LASHTAIL_RAPTOR', 360, 980, 7, 80, seed=180)
    m.area(220, 860, 300, 200, 'Crystalvein Mine')

    # --- the Bloodsail Compound on the Wild Shore ---------------------------------------------------
    wg.camp(m, 96, 1200, 208, 96, tents=[(104, 1208), (256, 1208)], fire=(176, 1256))
    wg.crates(m, 152, 1208)
    m.spawn_group('BLOODSAIL_SWASHBUCKLER', 240, 1330, 6, 80, seed=181)
    m.spawn_group('BLOODSAIL_MAGE', 140, 1310, 3, 40, seed=182)
    m.spawn_group('BLOODSAIL_SEA_DOG', 360, 1380, 5, 60, seed=183)
    m.spawn('FLEET_MASTER_FIRALLON', 208, 1232)
    m.area(64, 1140, 400, 320, 'The Bloodsail Compound')

    # --- Booty Bay ------------------------------------------------------------------------------------
    wg.cobbles(m, 624, 1376, 320, 48)
    inn = wg.house(m, 632, 1272, 112, 96, roof_colors=('red_d', 'red_m', 'red_l'), roof_ridge='red_l',
                   roof_outline='o2')
    baron = wg.house(m, 816, 1272, 120, 96, style='stone', roof_colors=('thatch_d', 'thatch_m', 'thatch_l'),
                     roof_ridge='thatch_l', roof_outline='o2')
    wg.pier(m, 704, 1424, 32, 72)
    wg.pier(m, 848, 1424, 32, 72)
    wg.anvil(m, 904, 1392)
    wg.crates(m, 768, 1408)
    m.npc('SKINDLE', inn[0], inn[1] + 4)
    m.npc('REVILGAZ', baron[0], baron[1] + 4)
    m.npc('BRUISER', 808, 1300)
    m.npc('GYLL', 656, 1400)
    m.point('flight', 656, 1416)
    m.npc('BOOTY_BAY_VENDOR', 752, 1404)
    m.npc('BOOTY_BAY_SMITH', 920, 1412)
    m.npc('KEBOK', 808, 1404)
    m.npc('SEAHORN', 864, 1404)
    m.point('booty_bay_respawn', 776, 1396)
    m.point('from_tanaris', 720, 1440)
    m.warp(708, 1488, 24, 8, 'tanaris', 'from_booty_bay', ride='boat')
    m.area(600, 1260, 360, 220, 'Booty Bay')

    # --- trees, and a hidden glade under the eastern cliffs -------------------------------------------
    wg.forest(m, trees, 864, 1000, 96, 128, kinds=('oak', 'palm'), holes=[(888, 1032, 48, 48)],
              secrets=[(840, 1048, 56, 32)])
    m.chest(42, 912, 1068, 38)
    rng = np.random.default_rng(47)
    path = wg.corners(m, 'path')
    water = wg.corners(m, 'water')
    clear = [(200, 100, 230, 140), (580, 270, 240, 140), (790, 140, 170, 190), (80, 548, 280, 220),
             (80, 1180, 260, 150), (580, 1160, 380, 376), (240, 880, 120, 80), (820, 990, 140, 150)]
    placed = 0
    for _ in range(8000):
        if placed >= 520:
            break
        x, y = int(rng.uniform(56, 920)), int(rng.uniform(56, 1420))
        mx, my = (x + 16) // 16, (y + 40) // 16
        near = path[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max() + \
            water[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max()
        if near == 0 and m.area_free(x, y, 40, 56) and \
                not any(cx - 40 <= x <= cx + cw and cy - 56 <= y <= cy + ch for cx, cy, cw, ch in clear):
            wg.tree(m, trees, x, y, int(rng.integers(0, 3)), kind=('oak', 'palm', 'small', 'oak', 'palm')[placed % 5])
            placed += 1
    wg.scatter_props(m, rng, 110, STRANGLETHORN_PROPS, (56, 60, 900, 1380),
                     avoid=[(200, 100, 230, 140), (580, 270, 240, 140), (580, 1160, 380, 376)])

    m.point('from_duskwood', 304, 28)
    m.warp(280, 0, 48, 8, 'duskwood', 'from_stranglethorn')
    m.area(0, 0, 1024, 1536, 'Stranglethorn Vale')
    m.music = 'STRANGLETHORN'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# The Scarlet Monastery: the Armory and the Cathedral
# ---------------------------------------------------------------------------------------------

def gen_sm_armory():
    c = Monastery('sm_armory', 1024, 512)
    m = c.m
    c.rect(448, 400, 128, 88)       # the gatehouse
    c.rect(488, 488, 48, 24)        # the doors out
    c.rect(488, 336, 48, 64)        # corridor north
    c.rect(224, 216, 576, 120)      # the training grounds
    c.rect(160, 248, 64, 56)        # corridor west
    c.rect(48, 184, 112, 208)       # the barracks
    c.rect(800, 248, 48, 56)        # corridor east
    c.rect(848, 168, 144, 224)      # the armory
    c.rect(896, 392, 48, 48)        # a hidden storeroom
    c.rect(880, 440, 96, 56)
    c.rect(488, 168, 48, 48)        # corridor north
    c.rect(288, 24, 448, 144)       # the Hall of Champions, Herod's
    c.render()
    c.exit(488, 504, 'tirisfal', 'armory_exit')

    c.rug(496, 216, 32, 184)
    for x in (264, 328, 680, 744):
        c.dummy(x, 256)
    for x in (456, 552):
        c.banner(x, 220)
    for y in (232, 288, 344):
        c.table(64, y, 48, 16)
    for x in (864, 912, 952):
        c.rack(x, 172)
    for y in (240, 304):
        c.table(872, y, 96, 20)
    c.secret(896, 392, 48, 0)
    for x in (896, 928):
        c.barrel(x, 456)
    m.chest(44, 952, 484, 38)
    c.rug(472, 64, 80, 104)
    for x in (320, 376, 640, 696):
        c.rack(x, 28)
    for x in (456, 560):
        c.banner(x, 28)
    for x in (304, 712):
        c.torch(x, 30)

    m.spawn('SCARLET_SOLDIER', 464, 424)
    m.spawn('SCARLET_SOLDIER', 560, 424)
    for x, y in ((300, 300), (380, 280), (440, 300), (600, 300), (660, 280), (720, 300)):
        m.spawn('SCARLET_TRAINEE' if x in (380, 660) else 'SCARLET_SOLDIER', x, y)
    for x, y in ((90, 240), (120, 300), (90, 360)):
        m.spawn('SCARLET_MYRMIDON' if y != 300 else 'SCARLET_DEFENDER', x, y)
    for x, y in ((900, 220), (940, 290), (900, 360)):
        m.spawn('SCARLET_DEFENDER' if y != 290 else 'SCARLET_MYRMIDON', x, y)
    m.spawn('SCARLET_MYRMIDON', 512, 196)
    for x, y in ((360, 100), (664, 100)):
        m.spawn('SCARLET_DEFENDER', x, y)
    m.spawn('HEROD', 512, 84)
    # Where the trainees come running from when Herod falls.
    m.patrol = [(456, 240), (512, 236), (568, 240), (512, 300)]
    m.area(0, 0, 1024, 512, 'Scarlet Monastery: Armory')
    m.area(224, 216, 576, 120, 'The Training Grounds')
    m.area(48, 184, 112, 208, 'The Barracks')
    m.area(848, 168, 144, 224, 'The Armory')
    m.area(288, 24, 448, 144, 'The Hall of Champions')
    m.music = 'MONASTERY'
    m.save()
    return m


def gen_sm_cathedral():
    c = Monastery('sm_cathedral', 1024, 512)
    m = c.m
    c.rect(448, 400, 128, 88)       # the narthex
    c.rect(488, 488, 48, 24)        # the doors out
    c.rect(488, 368, 48, 32)        # steps north
    c.rect(192, 264, 640, 104)      # the Chapel Gardens
    c.rect(488, 216, 48, 48)        # corridor north
    c.rect(256, 24, 512, 192)       # the nave
    c.rect(200, 88, 56, 56)         # the door to the side chapel
    c.rect(48, 56, 152, 160)        # the Chamber of Atonement, Fairbanks's
    c.rect(768, 104, 48, 48)        # a hidden vestry
    c.rect(816, 72, 112, 96)
    c.render()
    c.exit(488, 504, 'tirisfal', 'cathedral_exit')

    # The Chapel Gardens: candles and banners along the wall.
    for x in (224, 352, 640, 768):
        c.candles(x, 268)
    for x in (288, 416, 600, 728):
        c.banner(x, 268)
    # The nave: pews either side of a long carpet up to the altar.
    c.rug(488, 72, 48, 144)
    for y in (112, 144, 176):
        for x in (296, 376, 568, 648):
            c.table(x, y, 64, 16)
    c.altar(480, 36, 64, 24)
    for x in (432, 584):
        c.candles(x, 32)
    for x in (280, 360, 648, 728):
        c.banner(x, 28)
    c.secret(768, 104, 48, 0)
    m.chest(45, 872, 156, 39)
    c.bookshelf(832, 72, 80)
    # The Chamber of Atonement.
    c.altar(88, 64, 64, 24)
    for x in (64, 168):
        c.chains(x, 60)
    c.bones(80, 176)

    m.spawn('SCARLET_CENTURION', 464, 424)
    m.spawn('SCARLET_CENTURION', 560, 424)
    for x, y in ((240, 330), (320, 310), (400, 340), (620, 340), (700, 310), (780, 330)):
        m.spawn(('SCARLET_CHAMPION', 'SCARLET_ABBOT', 'SCARLET_WIZARD')[(x // 80) % 3], x, y)
    m.spawn('SCARLET_CHAMPION', 512, 240)
    for x, y in ((320, 92), (700, 92), (312, 196), (712, 196)):
        m.spawn('SCARLET_WIZARD' if y < 150 else 'SCARLET_CHAMPION', x, y)
    for x, y in ((80, 140), (160, 190)):
        m.spawn('SCARLET_ABBOT', x, y)
    m.spawn('HIGH_INQUISITOR_FAIRBANKS', 120, 120)
    m.spawn('SCARLET_COMMANDER_MOGRAINE', 512, 100)
    m.spawn('HIGH_INQUISITOR_WHITEMANE', 512, 72)
    m.area(0, 0, 1024, 512, 'Scarlet Monastery: Cathedral')
    m.area(192, 264, 640, 104, 'The Chapel Gardens')
    m.area(256, 24, 512, 192, 'The Crimson Cathedral')
    m.area(48, 56, 152, 160, 'The Chamber of Atonement')
    m.music = 'MONASTERY'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Tanaris and Thousand Needles
# ---------------------------------------------------------------------------------------------

DESERT_PROPS = ('rock', 'tall_grass', 'rock', 'big_rock', 'tall_grass', 'stump')


def cactus(m, x, y, variant=0):
    """A saguaro, 16x32, on the overhead layer with a shadow at its foot. Solid at the foot. Returns
    False where it would share a tile with another overhead bank or stand on a road."""
    x, y = x // 8 * 8, y // 8 * 8
    paths = m.corners.get('path')
    if paths is not None:
        near = paths[max(y // 16 - 1, 0):(y + 32) // 16 + 2, max(x // 16 - 1, 0):(x + 16) // 16 + 2]
        if near.max() > 0:
            return False
    inside = np.zeros((32, 16), dtype=bool)
    inside[2:31, 5:11] = True
    if variant % 2 == 0:
        inside[8:17, 1:4] = True
        inside[14:17, 1:6] = True
        inside[5:14, 12:15] = True
        inside[11:14, 10:15] = True
    else:
        inside[10:19, 12:15] = True
        inside[16:19, 10:15] = True
    part = np.zeros((32, 16), dtype=np.uint8)
    part[inside] = m.o('leaf_2')
    cols = np.arange(16)[None, :].repeat(32, axis=0)
    part[inside & ((cols == 6) | (cols == 2) | (cols == 13))] = m.o('leaf_3')
    part[inside & ((cols == 9) | (cols == 3))] = m.o('leaf_1')
    part[wg.outline_of(inside)] = m.o('outline')
    if not wg.bank_fits(m.overhead, part, x, y):
        return False
    m.stamp(m.overhead, part, x, y)
    m.ground[y + 30:y + 32, x + 3:x + 13] = m.g('shadow')
    m.block(x + 4, y + 24, 8, 8)
    return True


def paint_salt(m, kind='salt'):
    """Salt flats, painted like paths: 'foam' white with 'flower' grey speckles and a pale rim. Paint
    them after the roads."""
    c = wg.corners(m, kind)
    for my in range(m.metas_y):
        for mx in range(m.metas_x):
            if c[my:my + 2, mx:mx + 2].max() == 0:
                continue
            tile = m.ground[my * 16:(my + 1) * 16, mx * 16:(mx + 1) * 16]
            f = np.ones((16, 16)) if c[my:my + 2, mx:mx + 2].min() == 1 else wg.corner_field(c, mx, my)
            # Roads painted before keep their dirt; the salt takes the ground around them.
            ground = np.isin(tile, [m.g(role) for role in ('grass_d', 'grass_m', 'grass_l', 'grass_h', 'flower',
                                                            'foam', 'shadow')])
            inner = (f > 0.6) & ground
            tile[(f > 0.42) & (f <= 0.6) & ground] = m.g('grass_h')
            tile[inner] = m.g('foam')
            tile[inner & (wg.SPECKLE > 0.9)] = m.g('flower')


def adobe_wall(m, x, y, w, h, gate=48):
    """Gadgetzan's town wall: a ring of adobe 16 px thick, with brick faces 24 px tall on the north and
    south sides, a gate in the middle of each side and square towers at the corners. Solid."""
    g = m.ground
    gx, gy = x + w // 2 - gate // 2, y + h // 2 - gate // 2
    for wy in (y, y + h - 24):
        for sx, sw in ((x, gx - x), (gx + gate, x + w - gx - gate)):
            wg.bricks(m, g, sx, wy, sw, 24, 'stone_d', 'stone_m', 'stone_l')
            g[wy:wy + 3, sx:sx + sw] = m.g('stone_h')
            g[wy + 23, sx:sx + sw] = m.g('outline')
            m.block(sx, wy, sw, 24)
    for wx in (x, x + w - 16):
        for sy, sh in ((y + 24, gy - y - 24), (gy + gate, y + h - 24 - gy - gate)):
            g[sy:sy + sh, wx:wx + 16] = m.g('stone_m')
            g[sy:sy + sh, wx + 2:wx + 14] = m.g('stone_l')
            g[sy:sy + sh:8, wx + 2:wx + 14] = m.g('stone_m')
            g[sy:sy + sh, wx] = m.g('outline')
            g[sy:sy + sh, wx + 15] = m.g('outline')
            m.block(wx, sy, 16, sh)
    for tx, ty in ((x, y), (x + w - 32, y), (x, y + h - 40), (x + w - 32, y + h - 40)):
        wg.bricks(m, g, tx, ty, 32, 40, 'stone_d', 'stone_m', 'stone_l')
        g[ty:ty + 16, tx:tx + 32] = m.g('stone_h')
        g[ty + 4:ty + 12, tx + 4:tx + 28] = m.g('stone_l')
        g[ty + 15, tx:tx + 32] = m.g('stone_d')
        g[ty:ty + 40, tx] = m.g('outline')
        g[ty:ty + 40, tx + 31] = m.g('outline')
        g[ty + 39, tx:tx + 32] = m.g('outline')
        m.block(tx, ty, 32, 40)
    # Steamwheedle banners either side of each gate on the north and south walls.
    for wy in (y, y + h - 24):
        for bx in (gx - 16, gx + gate + 8):
            g[wy + 4:wy + 22, bx:bx + 8] = m.g('banner')
            g[wy + 4:wy + 22, bx] = m.g('banner_d')
            g[wy + 10:wy + 14, bx + 2:bx + 6] = m.g('gold')


def bramble_gate(m, x, y, w, h):
    """A mound of giant thorns with a dark hole at its foot: the way into the Razorfen warrens. x, y,
    w, h are multiples of 8. Returns the opening's bottom-center."""
    g = m.ground
    ys, xs = np.mgrid[y:y + h, x:x + w]
    d = np.hypot((xs + 0.5 - x - w / 2) / (w / 2), (ys + 0.5 - y - h) / h)
    mound = d < 1
    vines = np.full((h, w), m.g('trunk_m'), dtype=np.uint8)
    u, v = xs % 16, ys % 16
    vines[(u + v) % 16 < 2] = m.g('trunk_d')
    vines[(u - v) % 16 == 7] = m.g('trunk_d')
    vines[((u + v) % 16 == 2) & (v % 4 == 1)] = m.g('grass_h')
    vines[((u * 3 + v * 5) % 16 == 0)] = m.g('grass_d')
    area = g[y:y + h, x:x + w]
    area[mound] = vines[mound]
    area[mound & ~np.pad(mound, 1)[2:, 1:-1]] = m.g('shadow')
    ox = x + w // 2 - 16
    g[y + h - 32:y + h, ox:ox + 32] = m.g('shadow')
    g[y + h - 34:y + h - 32, ox + 4:ox + 28] = m.g('trunk_d')
    for py in range(y, y + h, 8):
        for px in range(x, x + w, 8):
            if mound[py - y:py - y + 8, px - x:px - x + 8].sum() >= 32:
                m.block(px, py, 8, 8)
    m.unblock(ox, y + h - 24, 32, 24)
    return (ox + 16, y + h)


def gen_tanaris():
    m = Map('tanaris', 1024, 1536,
            Palette([wg.TERRAIN_TANARIS, wg.BUILDINGS_GADGETZAN, wg.FARM, wg.ROCK_TANARIS]),
            Palette([wg.OVERHEAD_LEAVES_DESERT, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    # --- sandstone: Zul'Farrak's plateau in the northwest, the ridge to the north, the south edge ----
    rock = wg.corners(m, 'rock')
    rock[0:20, 0:24] = 1
    rock[20:28, 0:4] = 1
    rock[:, 0:2] = 1
    rock[0:2, :] = 1
    rock[0:3, 36:42] = 0                # the pass north into Thousand Needles
    rock[91:, :] = 1
    for cx, cy, rx, ry in ((340, 760, 40, 28), (160, 980, 56, 32), (720, 880, 36, 24), (580, 1360, 48, 30),
                           (880, 160, 40, 30), (400, 600, 28, 20)):
        wg.corners_ellipse(m, 'rock', cx, cy, rx, ry)
    wg.paint_cliffs(m)

    # --- the sea along the east coast, Steamwheedle Port's bay and Lost Rigger Cove ------------------
    wg.corners_rect(m, 'water', 976, 0, 48, 1536)
    wg.corners_along(m, 'water', [(1008, 0), (968, 300), (984, 500), (960, 640), (976, 900), (960, 1100),
                                  (984, 1300), (1008, 1440)], 2.0)
    wg.corners_ellipse(m, 'water', 948, 704, 92, 52)
    wg.corners_ellipse(m, 'water', 944, 1296, 112, 64)
    wg.corners_ellipse(m, 'water', 656, 700, 56, 32)     # Waterspring Field's pond
    wg.paint_water(m)

    # --- roads ----------------------------------------------------------------------------------------
    roads = [
        [(624, 0), (624, 120), (624, 240)],                                          # from Thousand Needles
        [(624, 512), (624, 600), (560, 700), (520, 820), (480, 960), (440, 1060)],    # south to Dunemaul
        [(784, 376), (840, 420), (872, 520), (872, 600)],                            # Steamwheedle Port
        [(840, 420), (880, 330)],                                                    # Noonshade Ruins
        [(464, 376), (360, 400), (260, 400), (208, 384)],                            # Zul'Farrak's gate
        [(520, 820), (680, 960), (760, 1100), (820, 1240)],                          # Lost Rigger Cove
        [(260, 400), (200, 520)],                                                    # Sandsorrow Watch
    ]
    for points in roads:
        wg.corners_along(m, 'path', points, 1.1)
    wg.paint_paths(m)

    # --- Gadgetzan --------------------------------------------------------------------------------------
    adobe_wall(m, 464, 240, 320, 272)
    rust = dict(roof_colors=('roof_d', 'roof_m', 'roof_l'), roof_ridge='roof_h', roof_outline='outline')
    inn = wg.house(m, 488, 272, 112, 96, style='stone', **rust)
    hall = wg.house(m, 656, 272, 104, 96, style='stone', **rust)
    shop = wg.house(m, 488, 392, 80, 80, style='stone', roof_colors=('red_d', 'red_m', 'red_l'),
                    roof_ridge='red_l', roof_outline='o2')
    wg.well(m, 608, 400)
    wg.cobbles(m, 672, 424, 64, 48)
    wg.anvil(m, 680, 432)
    wg.crates(m, 736, 448)
    m.npc('BILGEWHIZZLE', hall[0], hall[1] + 4)
    m.npc('FIZZGRIMBLE', inn[0], inn[1] + 4)
    m.npc('BLIZRIK', shop[0], shop[1] + 4)
    m.npc('KRINKLE', 712, 456)
    m.npc('TRENTON', 600, 452)
    m.npc('FIZZLEDOWSER', 576, 384)
    m.npc('TRANREK', 528, 386)
    m.npc('BERA', 744, 400)
    m.point('flight', 744, 416)
    m.npc('GADGETZAN_BRUISER', 588, 482)
    m.npc('GADGETZAN_BRUISER', 664, 228)
    m.point('gadgetzan_respawn', 624, 492)
    m.area(464, 240, 320, 272, 'Gadgetzan')

    # --- Steamwheedle Port --------------------------------------------------------------------------------
    port = wg.house(m, 808, 488, 104, 96, style='stone', **rust)
    wg.pier(m, 896, 616, 32, 72)
    wg.crates(m, 816, 600)
    m.npc('STOLEY', port[0], port[1] + 4)
    m.point('steamwheedle_respawn', 872, 604)
    m.point('from_booty_bay', 912, 628)
    m.warp(900, 680, 24, 8, 'stranglethorn', 'from_tanaris', ride='boat')
    m.area(790, 470, 200, 260, 'Steamwheedle Port')

    # --- Zul'Farrak's gate under the plateau ----------------------------------------------------------
    gate = mountain_gate(m, 144, 304, 128, 72, gate_w=40)
    m.warp(gate[0] - 16, gate[1] - 12, 32, 8, 'zul_farrak', 'entry')
    m.point('zf_exit', gate[0], gate[1] + 18)
    m.area(0, 0, 384, 400, "Zul'Farrak")

    # --- Sandsorrow Watch: the Sandfury trolls ---------------------------------------------------------
    for rx, ry, seed in ((96, 480, 31), (224, 560, 32), (112, 600, 33)):
        ruin(m, rx, ry, 80, 32, seed=seed)
    m.spawn_group('SANDFURY_HIDESKINNER', 180, 520, 5, 70, seed=201)
    m.spawn_group('SANDFURY_AXE_THROWER', 150, 620, 4, 50, seed=202)
    m.area(60, 430, 300, 240, 'Sandsorrow Watch')

    # --- Noonshade Ruins: the Wastewander bandits -------------------------------------------------------
    ruin(m, 824, 232, 96, 32, seed=34)
    ruin(m, 896, 296, 64, 32, seed=35)
    m.spawn_group('WASTEWANDER_BANDIT', 860, 320, 5, 60, seed=203)
    m.spawn_group('WASTEWANDER_THIEF', 820, 280, 3, 40, seed=204)
    m.spawn('CALIPH_SCORPIDSTING', 920, 260)
    m.area(800, 200, 176, 220, 'Noonshade Ruins')

    # --- Waterspring Field: the shadow mages at the pond -------------------------------------------------
    m.spawn_group('WASTEWANDER_SHADOW_MAGE', 640, 760, 3, 50, seed=205)
    m.spawn_group('WASTEWANDER_BANDIT', 540, 680, 3, 40, seed=206)
    m.area(480, 600, 260, 220, 'Waterspring Field')

    # --- the open desert: hyenas west, scorpids in the middle ---------------------------------------------
    m.spawn_group('BLISTERPAW_HYENA', 200, 800, 6, 90, seed=207)
    m.area(60, 700, 300, 260, 'Broken Pillar')
    m.spawn_group('SCORPID_HUNTER', 400, 860, 7, 90, seed=208)
    m.area(320, 780, 200, 200, 'The Gaping Chasm')

    # --- Dunemaul Compound: the ogres and Omgorn the Lost -----------------------------------------------
    wg.camp(m, 296, 1072, 224, 112, tents=[(304, 1080), (472, 1080)], fire=(392, 1128))
    wg.crates(m, 352, 1152)
    m.spawn_group('DUNEMAUL_BRUTE', 400, 1150, 6, 90, seed=209)
    m.spawn_group('DUNEMAUL_OGRE_MAGE', 330, 1180, 3, 50, seed=210)
    m.spawn('OMGORN_THE_LOST', 408, 1100)
    m.area(240, 1020, 340, 220, 'Dunemaul Compound')

    # --- the Eastmoon Ruins: scorpid reavers ---------------------------------------------------------------
    ruin(m, 584, 1168, 96, 32, seed=36)
    m.spawn_group('SCORPID_REAVER', 640, 1220, 6, 90, seed=211)
    m.area(540, 1120, 220, 200, 'Eastmoon Ruins')

    # --- Lost Rigger Cove: the Southsea pirates ---------------------------------------------------------
    wg.camp(m, 712, 1248, 128, 96, tents=[(720, 1256), (800, 1256)], fire=(768, 1304))
    wg.pier(m, 840, 1280, 32, 64)
    m.spawn_group('SOUTHSEA_PIRATE', 760, 1330, 5, 60, seed=212)
    m.spawn_group('SOUTHSEA_CANNONEER', 820, 1220, 3, 40, seed=213)
    m.spawn('ANDRE_FIREBEARD', 776, 1272)
    m.chest(47, 728, 1336, 44)
    m.area(700, 1200, 300, 200, 'Lost Rigger Cove')

    # --- palms at the pond and the port, a hidden grove in the far south --------------------------------
    for i, (px, py) in enumerate(((600, 624), (704, 628), (720, 704), (576, 712), (776, 568), (936, 448))):
        wg.tree(m, trees, px, py, i, kind='palm')
    wg.forest(m, trees, 64, 1320, 128, 112, kinds=('palm', 'small'), holes=[(96, 1352, 48, 48)],
              secrets=[(144, 1368, 56, 32)])
    m.chest(46, 120, 1388, 43)
    rng = np.random.default_rng(53)
    placed = 0
    for _ in range(3000):
        if placed >= 60:
            break
        x, y = int(rng.uniform(48, 940)), int(rng.uniform(420, 1420))
        if m.area_free(x, y, 16, 32) and not any(cx - 16 <= x <= cx + cw and cy - 32 <= y <= cy + ch for
                                                    cx, cy, cw, ch in ((464, 240, 320, 272), (700, 1200, 300, 200),
                                                                       (240, 1020, 340, 220), (790, 470, 200, 260))):
            if cactus(m, x, y, placed):
                placed += 1
    wg.scatter_props(m, rng, 70, DESERT_PROPS, (48, 420, 900, 1000),
                     avoid=[(464, 240, 320, 272), (790, 470, 200, 260), (700, 1200, 300, 200)])

    m.point('from_needles', 624, 28)
    m.warp(600, 0, 48, 8, 'thousand_needles', 'from_tanaris')
    m.area(0, 0, 1024, 1536, 'Tanaris')
    m.music = 'TANARIS'
    m.save()
    return m


def gen_thousand_needles():
    m = Map('thousand_needles', 1024, 1024,
            Palette([wg.TERRAIN_NEEDLES, wg.BUILDINGS_GADGETZAN, wg.FARM, wg.ROCK_NEEDLES]),
            Palette([wg.OVERHEAD_LEAVES_DESERT, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    # --- the mesas and needles of the western canyons; cliffs along the north --------------------------
    rock = wg.corners(m, 'rock')
    rock[0:4, :] = 1
    rock[:, 0:2] = 1
    rock[:, 63:] = 1
    rock[62:, 0:42] = 1
    rock[4:6, 4:16] = 1
    rock[0:6, 20:32] = 1
    rock[4:7, 0:4] = 1
    rock[48:53, 0:3] = 0                # the pass west into Feralas
    rng = np.random.default_rng(61)
    needles = [(96, 360), (200, 300), (320, 260), (440, 330), (120, 520), (260, 760), (420, 880), (140, 880),
               (480, 640), (340, 420), (80, 700), (220, 940), (600, 200), (760, 140), (900, 230), (680, 300),
               (540, 120)]
    for cx, cy in needles:
        wg.corners_ellipse(m, 'rock', cx, cy, int(rng.integers(24, 44)), int(rng.integers(18, 30)))
    wg.paint_cliffs(m)

    # --- roads ------------------------------------------------------------------------------------------------
    roads = [
        [(744, 1024), (744, 900), (768, 760)],                                       # from Tanaris
        [(672, 656), (560, 600), (400, 560), (300, 600), (240, 640)],                # west into the canyons
        [(400, 560), (380, 400), (400, 220), (432, 132)],                            # north to the Downs
        [(380, 400), (260, 220), (176, 172)],                                        # northwest to the Kraul
        [(240, 640), (190, 790), (0, 800)],                                          # west to Feralas
    ]
    for points in roads:
        wg.corners_along(m, 'path', points, 1.0)
    wg.paint_paths(m)

    # --- the Shimmering Flats -------------------------------------------------------------------------------
    wg.corners_ellipse(m, 'salt', 810, 690, 190, 290)
    wg.corners_along(m, 'salt', [(720, 430), (860, 400), (960, 470), (984, 700), (950, 900), (860, 984),
                                 (720, 1000), (650, 900), (624, 700), (650, 520), (720, 430)], 3.0)
    paint_salt(m)

    # --- the Mirage Raceway: a track around the goblins' and gnomes' camp ------------------------------------
    # Drawn on bare salt and mirrored about a tile corner, so its tiles repeat in flips.
    g = m.ground
    quarter = wg.SPECKLE[:8, :8]
    speckle = np.block([[quarter, quarter[:, ::-1]], [quarter[::-1], quarter[::-1, ::-1]]])
    g[552:760, 664:936] = np.where(np.tile(speckle, (13, 17)) > 0.9, m.g('flower'), m.g('foam'))
    ys, xs = np.mgrid[0:m.height, 0:m.width]
    d = np.hypot((xs + 0.5 - 800) / 128.0, (ys + 0.5 - 656) / 96.0)
    g[(d > 0.84) & (d <= 1.0)] = m.g('dirt_m')
    g[((d > 0.81) & (d <= 0.84)) | ((d > 1.0) & (d <= 1.04))] = m.g('dirt_d')
    g[(d > 0.91) & (d <= 0.93) & np.isin(xs % 8, (2, 3, 4, 5))] = m.g('dirt_l')
    wg.camp(m, 736, 616, 128, 80, tents=[(744, 624), (824, 624)], fire=(792, 664))
    wg.crates(m, 744, 664)
    m.npc('POZZIK', 800, 644)
    m.npc('KRAVEL', 800, 716)
    m.npc('FIZZLE_BRASSBOLTS', 728, 700)
    m.npc('WIZZLE_BRASSBOLTS', 752, 710)
    m.npc('ARGENT_GUARD_DALEN', 864, 704)
    m.point('raceway_respawn', 816, 728)
    m.area(648, 536, 304, 240, 'Mirage Raceway')
    m.spawn_group('SALTSTONE_BASILISK', 820, 880, 6, 90, seed=221)
    m.spawn_group('SALTSTONE_BASILISK', 880, 500, 3, 50, seed=222)
    m.spawn_group('GALAK_WINDCHASER', 760, 240, 3, 60, seed=226)
    m.area(560, 360, 464, 664, 'The Shimmering Flats')

    # --- Camp E'thok: the Galak centaurs --------------------------------------------------------------------
    wg.camp(m, 192, 600, 176, 88, tents=[(200, 608), (320, 608)], fire=(264, 648))
    m.spawn_group('GALAK_SCOUT', 280, 700, 5, 70, seed=223)
    m.spawn_group('GALAK_WINDCHASER', 220, 560, 4, 60, seed=224)
    m.spawn_group('GALAK_SCOUT', 520, 480, 2, 40, seed=225)
    m.area(150, 520, 300, 240, "Camp E'thok")

    # --- the Razorfen gates -----------------------------------------------------------------------------
    kraul = bramble_gate(m, 112, 112, 128, 64)
    m.warp(kraul[0] - 16, kraul[1] - 12, 32, 8, 'razorfen_kraul', 'entry')
    m.point('rfk_exit', kraul[0], kraul[1] + 18)
    m.spawn('RAZORFEN_QUILGUARD', kraul[0] - 40, kraul[1] + 24)
    m.spawn('RAZORFEN_QUILGUARD', kraul[0] + 40, kraul[1] + 24)
    m.area(64, 64, 240, 200, 'Razorfen Kraul')
    downs = bramble_gate(m, 368, 72, 128, 64)
    m.warp(downs[0] - 16, downs[1] - 12, 32, 8, 'razorfen_downs', 'entry')
    m.point('rfd_exit', downs[0], downs[1] + 18)
    m.spawn('DEATHS_HEAD_ACOLYTE', downs[0] - 40, downs[1] + 28)
    m.area(320, 64, 220, 180, 'Razorfen Downs')

    # --- a few dry trees, and a cache in a dead-end canyon ------------------------------------------------
    for i, (tx, ty) in enumerate(((160, 400), (520, 360), (300, 860), (480, 760), (120, 760))):
        wg.tree(m, trees, tx, ty, i, kind='small')
    m.chest(48, 56, 600, 41)
    placed = 0
    for _ in range(2000):
        if placed >= 24:
            break
        x, y = int(rng.uniform(40, 560)), int(rng.uniform(180, 960))
        if m.area_free(x, y, 16, 32) and not (150 <= x + 16 and x <= 450 and 520 <= y + 32 and y <= 760):
            if cactus(m, x, y, placed):
                placed += 1
    wg.scatter_props(m, rng, 50, DESERT_PROPS, (40, 180, 520, 780), avoid=[(150, 520, 300, 240)])

    m.point('from_tanaris', 744, 988)
    m.warp(720, 1016, 48, 8, 'tanaris', 'from_needles')
    m.point('from_feralas', 24, 800)
    m.warp(0, 776, 8, 48, 'feralas', 'from_needles')
    m.area(0, 0, 1024, 1024, 'Thousand Needles')
    m.music = 'TANARIS'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Razorfen Kraul, Razorfen Downs and Zul'Farrak
# ---------------------------------------------------------------------------------------------

# The Kraul: bramble masses for wall tops, earthen walls with roots, a packed-earth floor.
KRAUL = [
    ('outline', (20, 16, 12)), ('top_d', (36, 52, 24)), ('top_m', (60, 80, 36)),
    ('wall_d', (72, 52, 36)), ('wall_m', (104, 76, 48)), ('wall_l', (136, 104, 64)),
    ('floor_d', (92, 72, 48)), ('floor_m', (120, 96, 64)), ('floor_l', (148, 122, 84)),
    ('iron_d', (40, 36, 30)), ('iron_l', (204, 196, 172)), ('straw', (196, 168, 96)),
    ('wood', (112, 72, 40)), ('flame', (244, 184, 72)), ('red', (152, 44, 36)),
]

KRAUL_OVERHEAD = [
    ('outline', (20, 16, 12)), ('top_d', (36, 52, 24)), ('top_m', (60, 80, 36)),
    ('wall_d', (72, 52, 36)), ('wall_m', (104, 76, 48)), ('wall_l', (136, 104, 64)),
]

# The Downs: the quilboar's barrows taken by the Scourge. Cold grey stone, icy candles, violet cloth.
DOWNS = [
    ('outline', (14, 16, 22)), ('top_d', (30, 36, 46)), ('top_m', (46, 54, 66)),
    ('wall_d', (64, 72, 88)), ('wall_m', (92, 102, 118)), ('wall_l', (128, 138, 152)),
    ('floor_d', (70, 72, 80)), ('floor_m', (94, 96, 106)), ('floor_l', (120, 122, 132)),
    ('iron_d', (34, 36, 44)), ('iron_l', (192, 196, 204)), ('straw', (200, 168, 96)),
    ('wood', (90, 64, 44)), ('flame', (140, 220, 248)), ('red', (104, 52, 120)),
]

DOWNS_OVERHEAD = [
    ('outline', (14, 16, 22)), ('top_d', (30, 36, 46)), ('top_m', (46, 54, 66)),
    ('wall_d', (64, 72, 88)), ('wall_m', (92, 102, 118)), ('wall_l', (128, 138, 152)),
]

# Zul'Farrak: a troll city of sandstone under the sun, gold and red paint.
ZULFARRAK = [
    ('outline', (40, 24, 16)), ('top_d', (96, 60, 36)), ('top_m', (124, 84, 52)),
    ('wall_d', (156, 108, 64)), ('wall_m', (192, 144, 92)), ('wall_l', (220, 180, 124)),
    ('floor_d', (176, 136, 88)), ('floor_m', (204, 168, 116)), ('floor_l', (228, 196, 148)),
    ('iron_d', (60, 40, 28)), ('iron_l', (176, 168, 152)), ('straw', (236, 196, 72)),
    ('wood', (120, 80, 44)), ('flame', (252, 228, 136)), ('red', (176, 56, 40)),
]

ZULFARRAK_OVERHEAD = [
    ('outline', (40, 24, 16)), ('top_d', (96, 60, 36)), ('top_m', (124, 84, 52)),
    ('wall_d', (156, 108, 64)), ('wall_m', (192, 144, 92)), ('wall_l', (220, 180, 124)),
]

# Gahz'rilla's pool, in a bank of its own.
ZF_WATER = [
    ('water_d', (24, 72, 104)), ('water_m', (40, 104, 140)), ('water_l', (96, 164, 196)),
    ('lip', (236, 204, 152)), ('lip_d', (144, 100, 60)), ('lip_o', (32, 20, 12)),
]


class Temple(Monastery):
    """The Monastery's halls and props in another dungeon's colors. The Kraul is thorny: its wall tops
    are brambles and its walls are earth held together by roots."""

    def __init__(self, name, width, height, palette, overhead, extra=(), thorny=False):
        self.m = Map(name, width, height, Palette([palette, *extra]), Palette([overhead]))
        self.floor = np.zeros((height // 8, width // 8), dtype=bool)
        self.face = np.zeros_like(self.floor)
        self.thorny = thorny

    def _top(self, layer, x, y, w, h, pal):
        if not self.thorny:
            super()._top(layer, x, y, w, h, pal)
            return
        ys, xs = np.mgrid[y:y + h, x:x + w]
        u, v = xs % 16, ys % 16
        pattern = np.full((h, w), pal('top_m'), dtype=np.uint8)
        pattern[(u + v) % 16 < 2] = pal('top_d')
        pattern[(u - v) % 16 == 5] = pal('top_d')
        pattern[((u + v) % 16 == 2) & (v % 4 == 0)] = pal('outline')
        layer[y:y + h, x:x + w] = pattern

    def _face(self, layer, x, y, w, row, pal):
        if not self.thorny:
            super()._face(layer, x, y, w, row, pal)
            return
        ys, xs = np.mgrid[y:y + 8, x:x + w]
        u = xs % 16
        band = np.full((8, w), pal('wall_m'), dtype=np.uint8)
        band[(u * 7 + (ys % 8) * 3) % 13 == 0] = pal('wall_l')
        band[(u == 3) | ((u == 11) & (ys % 8 < 5))] = pal('wall_d')
        layer[y:y + 8, x:x + w] = band
        if row == 0:
            layer[y, x:x + w] = pal('outline')
            layer[y + 1, x:x + w:3] = pal('top_m')
        if row == self.FACE - 1:
            layer[y + 6, x:x + w] = pal('wall_d')
            layer[y + 7, x:x + w] = pal('outline')

    # --- props ---------------------------------------------------------------------------------------

    def thorns(self, x, y, w, h):
        """A wall of brambles grown across the floor. Solid."""
        g, m = self.m.ground, self.m
        ys, xs = np.mgrid[y:y + h, x:x + w]
        u, v = xs % 16, ys % 16
        part = np.full((h, w), m.g('top_m'), dtype=np.uint8)
        part[(u + v) % 16 < 2] = m.g('top_d')
        part[(u - v) % 16 == 5] = m.g('outline')
        part[((u * 5 + v * 3) % 16 == 0)] = m.g('red')
        part[-2:, :] = m.g('outline')
        g[y:y + h, x:x + w] = part
        self.m.block(x, y, w, h)

    def totem(self, x, y):
        """A quilboar totem pole, 8x32: carved wood, red paint and a tusked top."""
        g, m = self.m.ground, self.m
        g[y:y + 32, x + 1:x + 7] = m.g('outline')
        g[y + 1:y + 31, x + 2:x + 6] = m.g('wood')
        for ty in (y + 6, y + 16):
            g[ty:ty + 3, x + 2:x + 6] = m.g('red')
        g[y + 2:y + 4, x:x + 2] = m.g('iron_l')
        g[y + 2:y + 4, x + 6:x + 8] = m.g('iron_l')
        g[y + 10:y + 12, x + 3:x + 5] = m.g('straw')
        self.m.block(x, y + 24, 8, 8)

    def gong(self, x, y):
        """A great bronze gong in a wooden frame, 32x32."""
        g, m = self.m.ground, self.m
        for px in (x, x + 28):
            g[y:y + 32, px:px + 4] = m.g('wood')
            g[y:y + 32, px + 3] = m.g('outline')
        g[y:y + 4, x - 2:x + 34] = m.g('wood')
        g[y + 3, x - 2:x + 34] = m.g('outline')
        ys, xs = np.mgrid[0:24, 0:22]
        d = np.hypot(xs - 10.5, ys - 11.5)
        disc = g[y + 5:y + 29, x + 5:x + 27]
        disc[d < 11] = m.g('outline')
        disc[d < 10] = m.g('straw')
        disc[(d < 6) & (d >= 4)] = m.g('wood')
        disc[d < 2] = m.g('flame')
        self.m.block(x - 2, y + 20, 36, 12)

    def cage(self, x, y, w, h):
        """An iron cage seen from the front: a roof bar, bars and a floor rail. Solid."""
        g, m = self.m.ground, self.m
        g[y:y + 4, x:x + w] = m.g('iron_d')
        g[y, x:x + w] = m.g('outline')
        for bx in range(x, x + w, 6):
            g[y + 4:y + h, bx] = m.g('iron_l')
            g[y + 4:y + h, bx + 1] = m.g('iron_d')
        g[y + h - 3:y + h, x:x + w] = m.g('iron_d')
        g[y + h - 1, x:x + w] = m.g('outline')
        self.m.block(x, y + 8, w, h - 8)

    def idol(self, x, y):
        """A troll idol on a plinth, 16x32, with a gold mask."""
        g, m = self.m.ground, self.m
        g[y + 20:y + 32, x:x + 16] = m.g('outline')
        g[y + 21:y + 31, x + 1:x + 15] = m.g('wall_m')
        g[y + 21, x + 1:x + 15] = m.g('wall_l')
        g[y:y + 21, x + 3:x + 13] = m.g('outline')
        g[y + 1:y + 20, x + 4:x + 12] = m.g('wall_l')
        g[y + 3:y + 9, x + 4:x + 12] = m.g('straw')
        g[y + 5, x + 5:x + 7] = m.g('outline')
        g[y + 5, x + 9:x + 11] = m.g('outline')
        g[y + 8:y + 12, x + 3] = m.g('iron_l')
        g[y + 8:y + 12, x + 12] = m.g('iron_l')
        self.m.block(x, y + 20, 16, 12)

    def web(self, x, y):
        """A spider web strung across a corner, 16x16."""
        g, m = self.m.ground, self.m
        for i in range(16):
            g[y + i, x + i // 2] = m.g('iron_l')
            g[y + i // 2, x + i] = m.g('iron_l')
            g[y + i, x + i] = m.g('iron_l')
        for r in (5, 10):
            g[y + r, x:x + r + 1:2] = m.g('iron_l')
            g[y:y + r + 1:2, x + r] = m.g('iron_l')

    def pyramid(self, x, y, w, h, top_w, top_h, steps):
        """A stepped pyramid seen from above: rings of sandstone around a flat top, with a stairway
        steps px wide down the middle of its south side. Solid but for the top and the stairs."""
        g, m = self.m.ground, self.m
        cx = x + w // 2
        rings = 4
        for r in range(rings):
            rx0 = x + r * (w - top_w) // (2 * rings)
            rx1 = x + w - r * (w - top_w) // (2 * rings)
            ry0 = y + r * (h - top_h) // (2 * rings)
            ry1 = y + h - r * (h - top_h) // (2 * rings)
            g[ry0:ry1, rx0:rx1] = m.g('wall_m' if r % 2 == 0 else 'wall_l')
            g[ry0, rx0:rx1] = m.g('wall_l')
            g[ry1 - 3:ry1, rx0:rx1] = m.g('wall_d')
            g[ry1 - 1, rx0:rx1] = m.g('outline')
            g[ry0:ry1, rx0] = m.g('outline')
            g[ry0:ry1, rx1 - 1] = m.g('outline')
        tx, ty = cx - top_w // 2, y + (h - top_h) // 2
        g[ty:ty + top_h, tx:tx + top_w] = m.g('floor_l')
        g[ty:ty + top_h:8, tx:tx + top_w] = m.g('floor_m')
        sx = cx - steps // 2
        for sy in range(ty + top_h, y + h, 4):
            g[sy:sy + 4, sx:sx + steps] = m.g('wall_l' if (sy // 4) % 2 == 0 else 'wall_m')
            g[sy + 3, sx:sx + steps] = m.g('wall_d')
        g[ty + top_h:y + h, sx] = m.g('outline')
        g[ty + top_h:y + h, sx + steps - 1] = m.g('outline')
        self.m.block(x, y, w, h)
        self.m.unblock(tx, ty, top_w, top_h)
        self.m.unblock(sx + 2, ty + top_h, steps - 4, y + h - ty - top_h)

    def pool(self, x, y, w, h):
        """Deep water with a sandstone lip, drawn in the water bank. Solid."""
        g, m = self.m.ground, self.m
        ys, xs = np.mgrid[y:y + h, x:x + w]
        water = np.full((h, w), m.g('water_m'), dtype=np.uint8)
        water[((xs % 16) * 3 + (ys % 16) * 5) % 16 == 0] = m.g('water_l')
        water[(ys - y) < 8] = m.g('water_d')
        g[y:y + h, x:x + w] = water
        g[y:y + 3, x:x + w] = m.g('lip')
        g[y + 3, x:x + w] = m.g('lip_o')
        g[y + h - 3:y + h, x:x + w] = m.g('lip')
        g[y + h - 1, x:x + w] = m.g('lip_d')
        g[y:y + h, x:x + 3] = m.g('lip')
        g[y:y + h, x + w - 3:x + w] = m.g('lip')
        g[y + 4:y + h - 3, x + 3] = m.g('lip_d')
        self.m.block(x, y + 4, w, h - 6)
        self.m.water[(y + 8) // 8:(y + h - 8) // 8, (x + 8) // 8:(x + w - 8) // 8] = True


def gen_razorfen_kraul():
    c = Temple('razorfen_kraul', 1024, 768, KRAUL, KRAUL_OVERHEAD, thorny=True)
    m = c.m
    c.rect(448, 640, 128, 104)      # the warren's mouth
    c.rect(488, 744, 48, 24)        # the way out
    c.rect(488, 560, 48, 80)        # tunnel north
    c.rect(352, 400, 320, 160)      # the central warren, Jargba's
    c.rect(256, 456, 96, 48)        # tunnel west
    c.rect(64, 384, 192, 224)       # the boar pens, Aggem's
    c.rect(672, 456, 96, 48)        # tunnel east
    c.rect(768, 360, 208, 248)      # the war hall, Ramtusk's
    c.rect(848, 608, 48, 24)        # a hidden burrow
    c.rect(816, 632, 112, 88)
    c.rect(488, 320, 48, 80)        # tunnel north
    c.rect(384, 184, 256, 136)      # the beast pen, Agathelos's
    c.rect(296, 224, 88, 48)        # tunnel west
    c.rect(64, 40, 232, 248)        # the sanctum, Charlga's
    c.render()
    c.exit(488, 760, 'thousand_needles', 'rfk_exit')

    # Thorn walls narrow the tunnels.
    c.thorns(488, 592, 16, 16)
    c.thorns(520, 608, 16, 16)
    c.thorns(352, 520, 64, 24)
    c.thorns(600, 432, 64, 16)
    c.thorns(152, 496, 104, 16)
    c.thorns(800, 520, 72, 16)
    c.thorns(904, 440, 72, 16)
    c.thorns(520, 232, 120, 16)
    c.thorns(72, 160, 96, 16)
    for x, y in ((392, 404), (624, 404), (464, 652), (552, 652)):
        c.torch(x, y)
    for x in (96, 200):
        c.straw(x, 560)
    c.bucket(160, 568)
    for x in (432, 576):
        c.totem(x, 440)
    for x in (800, 928):
        c.rack(x, 364)
    c.table(840, 432, 80, 20)
    c.secret(848, 608, 48, 0)
    m.chest(49, 896, 708, 41)
    c.crate(840, 680)
    for x, y in ((416, 260), (560, 270)):
        c.bones(x, y)
    for x in (104, 248):
        c.totem(x, 60)
    c.altar(148, 64, 64, 24)
    for x in (80, 264):
        c.torch(x, 44)

    m.spawn('RAZORFEN_QUILGUARD', 472, 680)
    m.spawn('RAZORFEN_QUILGUARD', 552, 680)
    for x, y in ((400, 460), (460, 520), (620, 520), (640, 460)):
        m.spawn('RAZORFEN_GEOMANCER' if x in (460, 640) else 'RAZORFEN_QUILGUARD', x, y)
    m.spawn('DEATH_SPEAKER_JARGBA', 512, 456)
    for x, y in ((120, 440), (200, 470), (110, 530)):
        m.spawn('RAGING_AGAMAR', x, y)
    m.spawn('AGGEM_THORNCURSE', 168, 420)
    for x, y in ((820, 400), (930, 420), (820, 480), (900, 560)):
        m.spawn('RAZORFEN_QUILGUARD' if y < 450 else 'RAZORFEN_TOTEMIC', x, y)
    m.spawn('OVERLORD_RAMTUSK', 872, 470)
    m.spawn('RAZORFEN_QUILGUARD', 512, 352)
    for x, y in ((430, 240), (590, 290)):
        m.spawn('RAGING_AGAMAR', x, y)
    m.spawn('AGATHELOS_THE_RAGING', 512, 280)
    for x, y in ((100, 120), (260, 120), (110, 230), (250, 230)):
        m.spawn('RAZORFEN_TOTEMIC' if y < 200 else 'RAZORFEN_GEOMANCER', x, y)
    m.spawn('CHARLGA_RAZORFLANK', 180, 104)
    m.area(0, 0, 1024, 768, 'Razorfen Kraul')
    m.area(352, 400, 320, 160, 'The Central Warren')
    m.area(64, 384, 192, 224, 'The Boar Pens')
    m.area(768, 360, 208, 248, 'The War Hall')
    m.area(384, 184, 256, 136, 'The Beast Pen')
    m.area(64, 40, 232, 248, "Charlga's Sanctum")
    m.music = 'DUNGEON'
    m.save()
    return m


def gen_razorfen_downs():
    c = Temple('razorfen_downs', 1024, 768, DOWNS, DOWNS_OVERHEAD)
    m = c.m
    c.rect(448, 640, 128, 104)      # the barrow's door
    c.rect(488, 744, 48, 24)        # the way out
    c.rect(488, 544, 48, 96)        # corridor north
    c.rect(384, 352, 256, 192)      # the gong chamber
    c.rect(256, 424, 128, 48)       # corridor west
    c.rect(48, 320, 208, 256)       # the bone pits, Mordresh's
    c.rect(640, 424, 128, 48)       # corridor east
    c.rect(768, 320, 208, 256)      # the larder, Glutton's
    c.rect(848, 576, 48, 24)        # a hidden crypt
    c.rect(816, 600, 112, 80)
    c.rect(488, 256, 48, 96)        # corridor north
    c.rect(320, 40, 384, 216)       # the Coldbringer's crypt
    c.render()
    c.exit(488, 760, 'thousand_needles', 'rfd_exit')

    c.gong(408, 356)
    for x, y in ((464, 356), (560, 356), (616, 356)):
        c.candles(x, y)
    for x, y in ((392, 520), (616, 520)):
        c.web(x, y)
    for x in (72, 136, 200):
        c.slab(x, 360)
    for x, y in ((80, 480), (180, 520), (120, 420)):
        c.bones(x, y)
    for x in (800, 920):
        c.chains(x, 324)
    c.table(816, 420, 96, 20)
    c.barrel(936, 520)
    c.barrel(784, 520)
    c.secret(848, 576, 48, 0)
    m.chest(50, 896, 668, 43)
    c.slab(840, 620)
    c.rug(488, 96, 48, 160)
    c.altar(480, 52, 64, 24)
    for x in (360, 648):
        c.candles(x, 44)
    for x in (344, 400, 600, 656):
        c.slab(x, 140)
    for x, y in ((336, 60), (672, 60)):
        c.web(x, y)

    m.spawn('DEATHS_HEAD_ACOLYTE', 472, 680)
    m.spawn('WITHERED_QUILGUARD', 552, 680)
    for x, y in ((420, 470), (600, 470)):
        m.spawn('WITHERED_QUILGUARD', x, y)
    for x, y in ((80, 380), (220, 400), (140, 540)):
        m.spawn('SKELETAL_FROSTWEAVER' if x == 140 else 'SPLINTERBONE_WARRIOR', x, y)
    m.spawn('MORDRESH_FIRE_EYE', 152, 460)
    for x, y in ((800, 380), (940, 400), (860, 540)):
        m.spawn('WITHERED_QUILGUARD' if y < 500 else 'DEATHS_HEAD_ACOLYTE', x, y)
    m.spawn('GLUTTON', 872, 460)
    m.spawn('SPLINTERBONE_WARRIOR', 512, 300)
    for x, y in ((360, 200), (660, 200), (420, 100), (600, 100)):
        m.spawn('SKELETAL_FROSTWEAVER' if y < 150 else 'SPLINTERBONE_WARRIOR', x, y)
    m.spawn('AMNENNAR_THE_COLDBRINGER', 512, 92)
    # The gong: struck, it calls three waves of spiders and Tuten'kash from the chamber's corners.
    m.point('wave_a', 400, 424)
    m.point('wave_b', 624, 424)
    m.point('wave_c', 400, 528)
    m.point('wave_d', 624, 528)
    m.point('event_boss', 512, 520)
    m.area(392, 352, 72, 64, '', 'GONG')
    m.area(0, 0, 1024, 768, 'Razorfen Downs')
    m.area(384, 352, 256, 192, 'The Gong Chamber')
    m.area(48, 320, 208, 256, 'The Bone Pits')
    m.area(768, 320, 208, 256, "Glutton's Larder")
    m.area(320, 40, 384, 216, "The Coldbringer's Crypt")
    m.music = 'DUNGEON'
    m.save()
    return m


def gen_zul_farrak():
    c = Temple('zul_farrak', 1024, 1024, ZULFARRAK, ZULFARRAK_OVERHEAD, extra=(ZF_WATER,))
    m = c.m
    c.rect(384, 832, 256, 168)      # the gate court
    c.rect(488, 1000, 48, 24)       # the way out
    c.rect(320, 880, 64, 56)        # passage west
    c.rect(48, 600, 272, 336)       # Gahz'rilla's pool
    c.rect(152, 520, 56, 80)        # passage north
    c.rect(48, 232, 288, 288)       # the scorpid basin, Antu'sul's
    c.rect(488, 752, 48, 80)        # passage north
    c.rect(352, 336, 320, 416)      # the plaza and the pyramid
    c.rect(672, 480, 32, 56)        # passage east
    c.rect(704, 400, 272, 224)      # the martyr's court, Theka's
    c.rect(904, 624, 48, 24)        # a hidden shrine
    c.rect(872, 648, 112, 88)
    c.rect(808, 336, 56, 64)        # passage north
    c.rect(704, 96, 272, 240)       # the graveyard, Zum'rah's
    c.rect(488, 256, 48, 80)        # passage north
    c.rect(384, 40, 256, 216)       # the throne of Ukorz
    c.render()
    c.exit(488, 1016, 'tanaris', 'zf_exit')

    for x in (392, 616):
        c.idol(x, 836)
    c.pool(96, 640, 176, 120)
    c.gong(264, 776)
    for x in (64, 288):
        c.torch(x, 604)
    for x, y in ((80, 260), (290, 300), (120, 460), (260, 440)):
        c.bones(x, y)
    c.pyramid(400, 384, 224, 248, 112, 72, 48)
    c.cage(480, 432, 64, 32)
    for x in (360, 648):
        c.idol(x, 344)
    for x in (712, 952):
        c.torch(x, 404)
    c.altar(896, 404, 64, 24)
    c.secret(904, 624, 48, 0)
    m.chest(51, 952, 724, 45)
    c.idol(888, 660)
    for x in (728, 792, 856, 920):
        c.slab(x, 160)
        c.slab(x, 240)
    for x in (712, 952):
        c.candles(x, 100)
    c.rug(488, 96, 48, 160)
    c.altar(480, 48, 64, 24)
    for x in (400, 616):
        c.idol(x, 48)
    for x in (456, 552):
        c.banner(x, 44)

    m.spawn('SANDFURY_HIDESKINNER', 440, 880)
    m.spawn('SANDFURY_AXE_THROWER', 584, 880)
    for x, y in ((120, 820), (240, 880), (80, 900)):
        m.spawn('SANDFURY_SHADOWCASTER' if x == 240 else 'SANDFURY_BLOOD_DRINKER', x, y)
    for x, y in ((100, 300), (240, 320), (160, 420)):
        m.spawn('SCARAB' if x != 160 else 'SANDFURY_BLOOD_DRINKER', x, y)
    for x, y in ((112, 384), (280, 436), (232, 484), (88, 468)):
        m.spawn('SCARAB', x, y)
    m.spawn('ANTU_SUL', 192, 280)
    for x, y in ((380, 700), (640, 700), (380, 400), (640, 400)):
        m.spawn('SANDFURY_BLOOD_DRINKER' if y > 500 else 'SANDFURY_SHADOWCASTER', x, y)
    for x, y in ((740, 560), (940, 560), (760, 460)):
        m.spawn('SANDFURY_HIDESKINNER' if x != 760 else 'SANDFURY_SHADOWCASTER', x, y)
    m.spawn('THEKA_THE_MARTYR', 880, 480)
    for x, y in ((740, 300), (940, 300), (800, 200), (900, 140)):
        m.spawn('ZULFARRAK_ZOMBIE', x, y)
    m.spawn('WITCH_DOCTOR_ZUM_RAH', 840, 128)
    m.spawn('SANDFURY_BLOOD_DRINKER', 512, 292)
    for x, y in ((420, 120), (604, 120)):
        m.spawn('SANDFURY_SHADOWCASTER' if x < 512 else 'SANDFURY_BLOOD_DRINKER', x, y)
    m.spawn('RUUZLU', 456, 96)
    m.spawn('CHIEF_UKORZ_SANDSCALP', 512, 88)
    # The cage on the pyramid: opened, it brings three waves of trolls up the stairs, then Nekrum and
    # Sezz'ziz. The gong at the pool calls Gahz'rilla out of the water.
    m.point('wave_a', 432, 700)
    m.point('wave_b', 512, 720)
    m.point('wave_c', 592, 700)
    m.point('event_boss', 512, 690)
    m.point('gahzrilla', 184, 784)
    m.area(472, 416, 80, 72, '', 'CAGE')
    m.area(248, 760, 64, 64, '', 'GONG')
    m.area(0, 0, 1024, 1024, "Zul'Farrak")
    m.area(48, 600, 272, 336, "Gahz'rilla's Pool")
    m.area(48, 232, 288, 288, 'The Scarab Basin')
    m.area(352, 336, 320, 416, 'The Pyramid')
    m.area(704, 400, 272, 224, "The Martyr's Court")
    m.area(704, 96, 272, 240, 'The Graveyard')
    m.area(384, 40, 256, 216, 'The Throne of Ukorz')
    m.music = 'DUNGEON'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Feralas and Desolace
# ---------------------------------------------------------------------------------------------

def paint_beach(m, kind='beach'):
    """Sand along a coast, painted like paths: 'foam' with a few 'dirt_l' grains and a pale rim. Paint
    it after the water and the roads; it only takes grass."""
    c = wg.corners(m, kind)
    grass = [m.g(role) for role in ('grass_d', 'grass_m', 'grass_l', 'grass_h', 'flower', 'shadow')]
    for my in range(m.metas_y):
        for mx in range(m.metas_x):
            if c[my:my + 2, mx:mx + 2].max() == 0:
                continue
            tile = m.ground[my * 16:(my + 1) * 16, mx * 16:(mx + 1) * 16]
            f = np.ones((16, 16)) if c[my:my + 2, mx:mx + 2].min() == 1 else wg.corner_field(c, mx, my)
            ground = np.isin(tile, grass)
            inner = (f > 0.6) & ground
            tile[(f > 0.42) & (f <= 0.6) & ground] = m.g('grass_h')
            tile[inner] = m.g('foam')
            tile[inner & (wg.SPECKLE > 0.92)] = m.g('dirt_l')


def great_tree(m, trees, x, y, variant=0):
    """One of Feralas's giant trees: three crowns over a trunk twice as wide as an oak's, 64x80. Solid at
    its foot. Returns False where it doesn't fit."""
    x, y = x // 8 * 8, y // 8 * 8
    crowns = ((x, y + 16), (x + 32, y + 16), (x + 16, y))
    kinds = trees.kinds['oak']
    for i, (cx, cy) in enumerate(crowns):
        if not wg.bank_fits(m.overhead, kinds[(variant + i) % 3][0], cx, cy):
            return False
    trunk = wg.make_trunk(m, height=36, width=14)
    wg.plain_grass_under(m, x + 16, y + 44, 32, 36)
    m.stamp(m.ground, trunk, x + 16, y + 44)
    for i, (cx, cy) in enumerate(crowns):
        m.stamp(m.overhead, kinds[(variant + i) % 3][0], cx, cy)
    m.block(x + 20, y + 64, 24, 16)
    return True


def cave_mouth(m, x, y, w, h):
    """A cave in a mound of rock (rock bank) with a dark mouth at its foot. x, y, w, h are multiples of
    8. Returns the mouth's bottom-center."""
    g = m.ground
    ys, xs = np.mgrid[y:y + h, x:x + w]
    d = np.hypot((xs + 0.5 - x - w / 2) / (w / 2), (ys + 0.5 - y - h) / h)
    mound = d < 1
    u, v = xs % 16, ys % 16
    tex = np.full((h, w), m.g('rock_2'), dtype=np.uint8)
    tex[(u * 3 + v * 5) % 16 < 3] = m.g('rock_1')
    tex[(u + v) % 16 == 0] = m.g('rock_3')
    tex[(ys - y) < h // 3] = m.g('rock_3')
    tex[((ys - y) < h // 3) & ((u * 5 + v * 3) % 16 < 2)] = m.g('rock_4')
    area = g[y:y + h, x:x + w]
    area[mound] = tex[mound]
    edge = mound & ~(np.pad(mound, 1)[2:, 1:-1] & np.pad(mound, 1)[:-2, 1:-1] & np.pad(mound, 1)[1:-1, 2:] &
                     np.pad(mound, 1)[1:-1, :-2])
    area[edge] = m.g('rock_0')
    ox = x + w // 2 - 16
    hole = np.hypot((np.arange(32)[None, :] + 0.5 - 16) / 16, (np.arange(32)[:, None] + 0.5 - 32) / 30) < 1
    mouth = g[y + h - 32:y + h, ox:ox + 32]
    mouth[hole] = m.g('rock_0')
    for py in range(y, y + h, 8):
        for px in range(x, x + w, 8):
            if mound[py - y:py - y + 8, px - x:px - x + 8].sum() >= 32:
                m.block(px, py, 8, 8)
    m.unblock(ox, y + h - 24, 32, 24)
    return (ox + 16, y + h)


def zone_trees(m, trees, rng, count, area, avoid, great=False):
    """Scatters trees (or giant trees) over area, away from roads, water and the avoid rectangles."""
    x0, y0, w, h = area
    paths = m.corners.get('path')
    placed = 0
    size = (64, 80) if great else (32, 48)
    for _ in range(count * 40):
        if placed >= count:
            break
        x, y = int(rng.uniform(x0, x0 + w - size[0])), int(rng.uniform(y0, y0 + h - size[1]))
        if any(wg.overlaps((x - 8, y - 8, size[0] + 16, size[1] + 16), a) for a in avoid):
            continue
        near = paths[max(y // 16 - 1, 0):(y + size[1]) // 16 + 2, max(x // 16 - 1, 0):(x + size[0]) // 16 + 2]
        if near.max() > 0 or not m.area_free(x, y + size[1] - 24, size[0], 24):
            continue
        if great:
            ok = great_tree(m, trees, x, y, placed)
        else:
            ok = wg.tree(m, trees, x, y, placed, kind=('oak', 'pine', 'oak')[placed % 3])
        placed += bool(ok)


def gen_feralas():
    m = Map('feralas', 1024, 1536,
            Palette([wg.TERRAIN_FERALAS, wg.BUILDINGS, wg.FARM, wg.ROCK_FERALAS]),
            Palette([wg.OVERHEAD_LEAVES_FERALAS, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m)

    # --- cliffs along the north and east edges, with passes to Desolace and Thousand Needles; the Twin
    # Colossals' two great stacks in the northwest ------------------------------------------------------
    rock = wg.corners(m, 'rock')
    rock[0:3, 14:] = 1
    rock[0:3, 36:41] = 0                # the pass north into Desolace
    rock[:, 62:] = 1
    rock[66:71, 62:] = 0                # the pass east into Thousand Needles
    rock[94:, 14:] = 1
    for cx, cy, rx, ry in ((336, 144, 40, 36), (472, 112, 36, 40), (760, 1180, 30, 24), (408, 1080, 28, 22)):
        wg.corners_ellipse(m, 'rock', cx, cy, rx, ry)
    wg.paint_cliffs(m)

    # --- the sea along the west, Feathermoon's island and the Isle of Dread, the Forgotten Coast's bay ----
    wg.corners_rect(m, 'water', 0, 0, 240, 1536)
    wg.corners_along(m, 'water', [(240, 0), (256, 200), (240, 420), (264, 640), (248, 900), (264, 1140),
                                  (240, 1400), (256, 1536)], 1.6)
    wg.corners_ellipse(m, 'water', 300, 760, 52, 36)     # the Forgotten Coast's bay
    water = wg.corners(m, 'water')
    ys, xs = np.mgrid[0:water.shape[0], 0:water.shape[1]]
    water[((xs - 112 / 16) / (88 / 16)) ** 2 + ((ys - 488 / 16) / (128 / 16)) ** 2 <= 1] = 0    # Feathermoon
    water[((xs - 104 / 16) / (80 / 16)) ** 2 + ((ys - 1320 / 16) / (88 / 16)) ** 2 <= 1] = 0   # Isle of Dread
    water[81:85, 10:17] = 0                                                                  # the sandbar
    wg.paint_water(m)

    # --- roads ---------------------------------------------------------------------------------------
    roads = [
        [(1024, 1088), (880, 1088), (760, 1020), (620, 940), (480, 880), (360, 760), (320, 700)],  # from the Needles
        [(620, 940), (640, 760), (704, 600), (704, 488)],                                          # to Dire Maul
        [(704, 600), (704, 488)],
        [(704, 488), (640, 300), (608, 120), (608, 0)],                                            # to Desolace
        [(480, 880), (520, 1060), (580, 1220)],                                                    # Isildien
        [(704, 600), (860, 680)],                                                                  # the Grimtotem
        [(360, 760), (330, 960), (300, 1150), (272, 1290), (232, 1320)],                           # down the coast
        [(640, 300), (820, 200), (880, 160)],                                                      # the Dream Bough
        [(608, 120), (440, 216), (320, 300)],                                                      # the Colossals
        [(760, 1020), (880, 960)],                                                                 # Lariss Pavilion
        [(620, 940), (760, 1220), (840, 1280)],                                                    # the Highlands
    ]
    for points in roads:
        wg.corners_along(m, 'path', points, 1.1)
    wg.paint_paths(m)

    # --- the beaches ----------------------------------------------------------------------------------
    wg.corners_along(m, 'beach', [(264, 0), (280, 200), (264, 420), (284, 640), (272, 900), (288, 1140),
                                  (264, 1400), (280, 1536)], 1.4)
    wg.corners_ellipse(m, 'beach', 112, 488, 96, 136)
    wg.corners_ellipse(m, 'beach', 104, 1320, 88, 96)
    wg.corners_along(m, 'beach', [(170, 1320), (240, 1320)], 1.6)
    paint_beach(m)

    # --- Feathermoon Stronghold on its island ---------------------------------------------------------
    elf = dict(roof_colors=('roof_d', 'roof_m', 'roof_l'), roof_ridge='roof_h', roof_outline='outline')
    inn = wg.house(m, 40, 384, 80, 80, style='stone', **elf)
    hall = wg.house(m, 128, 400, 64, 64, style='stone', **elf)
    moonwell(m, 96, 488)
    wg.cobbles(m, 144, 544, 32, 32)
    wg.anvil(m, 152, 552)
    wg.pier(m, 96, 600, 32, 64)
    m.npc('SHYRIA', inn[0], inn[1] + 4)
    m.npc('SHANDRIS', hall[0], hall[1] + 4)
    m.npc('LATRONICUS', 64, 496)
    m.npc('ANGELAS', 152, 496)
    m.npc('PRATT', 56, 544)
    m.npc('KINDAL', 176, 528)
    m.npc('VIVIANNA', 80, 572)
    m.npc('BRANNOL', 160, 576)
    m.npc('FYLDREN', 136, 560)
    m.point('flight', 136, 576)
    m.npc('FEATHERMOON_SENTINEL', 72, 592)
    m.point('feathermoon_respawn', 112, 536)
    m.point('island_pier', 112, 612)
    m.warp(100, 652, 24, 8, 'feralas', 'coast_pier', ride='boat')
    m.area(16, 352, 200, 280, 'Feathermoon Stronghold')

    # --- the Forgotten Coast: the pier to Feathermoon, the Hatecrest naga down the shore ------------------
    wg.pier(m, 280, 704, 32, 72)
    wg.crates(m, 320, 672)
    m.point('coast_pier', 296, 712)
    m.point('coast_respawn', 336, 712)
    m.warp(284, 768, 24, 8, 'feralas', 'island_pier', ride='boat')
    m.spawn_group('HATECREST_WARRIOR', 320, 1000, 5, 60, seed=301)
    m.spawn_group('HATECREST_SIREN', 300, 1100, 3, 50, seed=302)
    m.spawn_group('HATECREST_WARRIOR', 330, 1200, 3, 40, seed=303)
    m.area(240, 600, 180, 760, 'The Forgotten Coast')

    # --- the Isle of Dread: Lord Shalzaru --------------------------------------------------------------
    ruin(m, 48, 1264, 96, 32, seed=41)
    m.spawn('HATECREST_WARRIOR', 152, 1336)
    m.spawn('HATECREST_SIREN', 120, 1380)
    m.spawn('HATECREST_WARRIOR', 72, 1360)
    m.spawn('LORD_SHALZARU', 104, 1312)
    m.chest(53, 64, 1384, 48)
    m.area(16, 1220, 220, 200, 'Isle of Dread')

    # --- the Twin Colossals: shore striders and Zorbin's camp ---------------------------------------------
    wg.camp(m, 272, 312, 96, 64, tents=[(280, 320)], fire=(328, 344))
    m.npc('ZORBIN', 344, 328)
    m.spawn_group('SHORE_STRIDER', 300, 220, 3, 60, seed=304)
    m.spawn_group('SHORE_STRIDER', 420, 220, 3, 50, seed=305)
    m.area(256, 48, 320, 320, 'Twin Colossals')

    # --- Feral Scar Vale: the yetis, and a hidden clearing in the woods to its west ---------------------
    m.spawn_group('RAGE_SCAR_YETI', 480, 520, 6, 80, seed=306)
    m.spawn('OLD_GRIZZLEGUT', 520, 580)
    m.area(400, 420, 200, 220, 'Feral Scar Vale')
    wg.forest(m, trees, 288, 400, 104, 176, kinds=('oak', 'pine'), holes=[(304, 440, 56, 56)],
              secrets=[(352, 456, 40, 32)])
    m.chest(52, 328, 476, 47)

    # --- Dire Maul: the great gate of Eldre'Thalas and the ogres who keep it ------------------------------
    for rx, ry, seed in ((600, 360, 42), (784, 360, 43), (608, 520, 44), (776, 520, 45)):
        ruin(m, rx, ry, 64, 32, seed=seed)
    gate = mountain_gate(m, 648, 384, 112, 80, gate_w=40)
    m.warp(gate[0] - 16, gate[1] - 12, 32, 8, 'dire_maul', 'entry')
    m.point('dm_exit', gate[0], gate[1] + 18)
    m.point('maul_respawn', gate[0], gate[1] + 48)
    m.spawn('GORDUNNI_OGRE', gate[0] - 48, gate[1] + 24)
    m.spawn('GORDUNNI_OGRE', gate[0] + 48, gate[1] + 24)
    m.area(592, 340, 240, 220, 'Dire Maul')

    # --- the Dream Bough: the Emerald Dream's portal and its green dragonkin -------------------------------
    moonwell(m, 904, 104)
    for rx, ry, seed in ((856, 64, 51), (936, 64, 52)):
        ruin(m, rx, ry, 48, 32, seed=seed)
    m.spawn_group('JADEMIR_ECHOSPAWN', 880, 160, 4, 50, seed=307)
    m.spawn_group('JADEMIR_BOUGHGUARD', 940, 120, 3, 40, seed=308)
    m.area(832, 40, 176, 200, 'The Dream Bough')

    # --- the Grimtotem Compound --------------------------------------------------------------------------
    wg.camp(m, 840, 624, 160, 112, tents=[(848, 632), (952, 632)], fire=(904, 680))
    m.spawn_group('GRIMTOTEM_RAIDER', 900, 740, 6, 70, seed=309)
    m.spawn_group('GRIMTOTEM_NATURALIST', 880, 660, 4, 50, seed=310)
    m.area(820, 600, 190, 240, 'Grimtotem Compound')

    # --- Lariss Pavilion: Azj'Tordin --------------------------------------------------------------------
    ruin(m, 856, 912, 96, 32, seed=46)
    moonwell(m, 896, 952)
    m.npc('AZJ_TORDIN', 872, 968)
    m.area(840, 880, 170, 130, 'Lariss Pavilion')

    # --- the Ruins of Isildien: the Gordunni ogres and their warlord ---------------------------------------
    for rx, ry, seed in ((472, 1224, 47), (632, 1240, 48), (512, 1368, 49), (664, 1376, 50)):
        ruin(m, rx, ry, 80, 32, seed=seed)
    m.spawn_group('GORDUNNI_OGRE', 560, 1300, 6, 80, seed=311)
    m.spawn_group('GORDUNNI_MAGE', 620, 1330, 4, 60, seed=312)
    m.spawn('GORDUNNI_WARLORD', 600, 1272)
    m.area(440, 1180, 300, 260, 'Ruins of Isildien')

    # --- the Frayfeather Highlands: the wildkin ------------------------------------------------------------
    m.spawn_group('ENRAGED_WILDKIN', 860, 1280, 6, 80, seed=313)
    m.spawn_group('WILDKIN_ORACLE', 920, 1360, 3, 50, seed=314)
    m.area(780, 1160, 230, 290, 'Frayfeather Highlands')

    # --- the forest: giant trees everywhere the roads and camps leave room, the edges thick with them -----
    wg.forest(m, trees, 976, 0, 48, 1056, kinds=('oak', 'pine'))
    wg.forest(m, trees, 976, 1120, 48, 416, kinds=('oak', 'pine'))
    zones = [(16, 352, 200, 280), (256, 600, 120, 200), (256, 48, 320, 330), (400, 420, 200, 220),
             (592, 340, 240, 220), (832, 40, 176, 200), (820, 600, 190, 240), (840, 880, 170, 130),
             (440, 1180, 300, 260), (780, 1160, 230, 290), (16, 1220, 220, 200), (288, 400, 104, 176),
             (240, 900, 120, 380)]
    rng = np.random.default_rng(71)
    zone_trees(m, trees, rng, 22, (300, 40, 680, 1460), zones, great=True)
    zone_trees(m, trees, rng, 70, (300, 40, 680, 1460), zones)
    wg.scatter_props(m, rng, 50, ('rock', 'tall_grass', 'stump', 'tall_grass', 'big_rock'), (300, 60, 660, 1420),
                     avoid=zones)

    m.point('from_needles', 1000, 1088)
    m.warp(1016, 1064, 8, 48, 'thousand_needles', 'from_feralas')
    m.point('from_desolace', 608, 24)
    m.warp(584, 0, 48, 8, 'desolace', 'from_feralas')
    m.area(0, 0, 1024, 1536, 'Feralas')
    m.music = 'FERALAS'
    m.save()
    return m


def gen_desolace():
    m = Map('desolace', 768, 1024,
            Palette([wg.TERRAIN_DESOLACE, wg.BUILDINGS, wg.FARM, wg.ROCK_DESOLACE]),
            Palette([wg.OVERHEAD_LEAVES_DARKSHORE, wg.OVERHEAD_ROOFS]))
    wg.fill_grass(m)
    trees = wg.Trees(m, dead=True)

    rock = wg.corners(m, 'rock')
    rock[0:3, :] = 1
    rock[:, 0:2] = 1
    rock[:, 46:] = 1
    rock[61:, :] = 1
    rock[61:, 22:26] = 0                # the pass south into Feralas
    for cx, cy, rx, ry in ((200, 300, 60, 40), (560, 440, 44, 30), (300, 760, 50, 34), (120, 860, 40, 30),
                           (620, 900, 40, 30)):
        wg.corners_ellipse(m, 'rock', cx, cy, rx, ry)
    wg.paint_cliffs(m)
    wg.corners_ellipse(m, 'water', 440, 620, 60, 28)
    wg.paint_water(m)

    roads = [
        [(384, 1024), (384, 840), (400, 640), (480, 420), (560, 260), (600, 200)],   # from Feralas
        [(400, 640), (280, 560), (176, 520)],                                       # Maraudon
        [(480, 420), (600, 560), (620, 620)],                                       # Magram Village
    ]
    for points in roads:
        wg.corners_along(m, 'path', points, 1.1)
    wg.paint_paths(m)

    # --- Nijel's Point -----------------------------------------------------------------------------------
    elf = dict(roof_colors=('roof_d', 'roof_m', 'roof_l'), roof_ridge='roof_h', roof_outline='outline')
    inn = wg.house(m, 504, 72, 96, 80, style='stone', **elf)
    wg.house(m, 624, 72, 80, 80, style='stone', **elf)
    moonwell(m, 576, 184)
    m.npc('LYSHAERYA', inn[0], inn[1] + 4)
    m.npc('TALENDRIA', 640, 176)
    m.npc('MARANDIS', 520, 200)
    m.npc('WILLOW', 664, 216)
    m.npc('BARITANAS', 688, 176)
    m.point('flight', 688, 192)
    m.point('nijels_respawn', 600, 240)
    m.area(480, 48, 260, 220, "Nijel's Point")

    # --- Magram Village: the Magram centaurs ------------------------------------------------------------
    wg.camp(m, 560, 600, 160, 96, tents=[(568, 608), (672, 608)], fire=(624, 648))
    m.spawn_group('MAGRAM_WRANGLER', 620, 700, 6, 80, seed=321)
    m.spawn_group('MAGRAM_STORMER', 640, 560, 4, 50, seed=322)
    m.area(520, 520, 240, 240, 'Magram Village')

    # --- Maraudon's mouth in the western cliffs, and Cavindra's camp beside it --------------------------
    cave = cave_mouth(m, 64, 400, 144, 96)
    m.warp(cave[0] - 16, cave[1] - 12, 32, 8, 'maraudon', 'entry')
    m.point('maraudon_exit', cave[0], cave[1] + 20)
    wg.camp(m, 192, 528, 80, 56, tents=[(200, 536)], fire=(240, 552))
    m.npc('CAVINDRA', 216, 592)
    m.spawn('PUTRIDUS_SATYR', 120, 560)
    m.spawn('PUTRIDUS_SATYR', 176, 600)
    m.area(40, 360, 280, 300, 'Maraudon')

    # --- the Valley of Spears: bones of the centaur wars, and a cache behind the boulders ----------------
    for bx, by in ((240, 860), (360, 900), (480, 800)):
        wg.big_rock(m, bx, by)
    wg.forest(m, trees, 40, 720, 120, 96, kinds=('oak', 'small'), holes=[(64, 744, 48, 40)],
              secrets=[(112, 752, 48, 32)])
    m.chest(54, 88, 776, 46)
    m.spawn_group('MAGRAM_WRANGLER', 300, 880, 3, 50, seed=323)
    m.area(160, 700, 400, 260, 'Valley of Spears')

    zones = [(480, 48, 260, 220), (520, 520, 240, 240), (40, 360, 280, 300), (40, 720, 120, 96)]
    rng = np.random.default_rng(73)
    zone_trees(m, trees, rng, 30, (40, 60, 690, 900), zones)
    wg.scatter_props(m, rng, 50, ('rock', 'stump', 'big_rock', 'rock'), (40, 60, 690, 900), avoid=zones)

    m.point('from_feralas', 384, 1000)
    m.warp(360, 1016, 48, 8, 'feralas', 'from_desolace')
    m.area(0, 0, 768, 1024, 'Desolace')
    m.music = 'FERALAS'
    m.save()
    return m


# ---------------------------------------------------------------------------------------------
# Maraudon and Dire Maul
# ---------------------------------------------------------------------------------------------

# Maraudon: violet crystal caves grown over with vines, the earth fouled by Theradras.
MARAUDON = [
    ('outline', (16, 12, 20)), ('top_d', (36, 56, 28)), ('top_m', (64, 88, 40)),
    ('wall_d', (64, 44, 80)), ('wall_m', (92, 68, 112)), ('wall_l', (128, 100, 148)),
    ('floor_d', (72, 60, 64)), ('floor_m', (96, 82, 84)), ('floor_l', (122, 106, 104)),
    ('iron_d', (44, 36, 52)), ('iron_l', (196, 184, 220)), ('straw', (156, 196, 88)),
    ('wood', (96, 68, 44)), ('flame', (208, 152, 240)), ('red', (168, 64, 148)),
]

MARAUDON_OVERHEAD = MARAUDON[:6]

# Its clear pools, and the green slime of the Foulspore Cavern, in a bank of their own.
MARAUDON_WATER = [
    ('water_d', (24, 60, 92)), ('water_m', (40, 92, 128)), ('water_l', (100, 160, 196)),
    ('lip', (136, 108, 156)), ('lip_o', (24, 16, 32)), ('lip_d', (80, 56, 96)),
    ('slime_d', (56, 96, 24)), ('slime_m', (96, 148, 32)), ('slime_l', (176, 216, 72)),
]

# Dire Maul: the Highborne city of Eldre'Thalas, pale stone overgrown, arcane teal and violet.
DIRE_MAUL = [
    ('outline', (18, 20, 30)), ('top_d', (40, 56, 48)), ('top_m', (60, 80, 64)),
    ('wall_d', (84, 88, 112)), ('wall_m', (120, 124, 148)), ('wall_l', (164, 166, 184)),
    ('floor_d', (88, 96, 96)), ('floor_m', (112, 120, 118)), ('floor_l', (140, 148, 142)),
    ('iron_d', (48, 44, 64)), ('iron_l', (200, 200, 216)), ('straw', (200, 176, 104)),
    ('wood', (96, 68, 52)), ('flame', (140, 232, 216)), ('red', (112, 64, 152)),
]

DIRE_MAUL_OVERHEAD = DIRE_MAUL[:6]

DM_WATER = [
    ('water_d', (24, 72, 96)), ('water_m', (40, 108, 136)), ('water_l', (104, 172, 200)),
    ('lip', (176, 176, 196)), ('lip_o', (24, 28, 40)), ('lip_d', (100, 104, 128)),
]


class Ruins(Temple):
    """Temple halls with the props of Maraudon and Dire Maul."""

    def poison(self, x, y, w, h):
        """A pool of green slime with a stone lip, in the water bank. Walkable, and it burns. x, y, w, h
        are multiples of 8."""
        g, m = self.m.ground, self.m
        ys, xs = np.mgrid[y:y + h, x:x + w]
        slime = np.full((h, w), m.g('slime_m'), dtype=np.uint8)
        slime[((xs % 16) * 5 + (ys % 16) * 3) % 16 < 2] = m.g('slime_l')
        slime[((xs % 16) * 3 + (ys % 16) * 7) % 16 == 9] = m.g('slime_d')
        g[y:y + h, x:x + w] = slime
        g[y:y + 2, x:x + w] = m.g('lip')
        g[y + 2, x:x + w] = m.g('slime_d')
        g[y + h - 2:y + h, x:x + w] = m.g('lip_d')
        g[y:y + h, x:x + 2] = m.g('lip')
        g[y:y + h, x + w - 2:x + w] = m.g('lip')
        m.area(x + 4, y + 4, w - 8, h - 8, '', 'RADIATION')

    def crystal(self, x, y):
        """A cluster of violet crystals, 16x24. Solid at its foot."""
        g, m = self.m.ground, self.m
        for cx, top, width in ((x + 2, y + 8, 4), (x + 6, y, 5), (x + 11, y + 6, 4)):
            g[top:y + 22, cx - 1:cx + width + 1] = m.g('outline')
            g[top + 1:y + 21, cx:cx + width] = m.g('flame')
            g[top + 2:y + 20, cx + width - 1] = m.g('red')
            g[top + 1:top + 4, cx] = m.g('iron_l')
        g[y + 20:y + 24, x:x + 16] = m.g('wall_d')
        g[y + 23, x:x + 16] = m.g('outline')
        self.m.block(x, y + 14, 16, 10)

    def field(self, cx, cy, r):
        """A circle of arcane runes on the floor, Immol'thar's prison. Walkable."""
        g, m = self.m.ground, self.m
        ys, xs = np.mgrid[cy - r:cy + r, cx - r:cx + r]
        d = np.hypot(xs + 0.5 - cx, ys + 0.5 - cy)
        area = g[cy - r:cy + r, cx - r:cx + r]
        area[(d < r) & (d >= r - 2)] = m.g('flame')
        area[(d < r - 2) & (d >= r - 3)] = m.g('outline')
        area[(d < r - 8) & (d >= r - 9)] = m.g('red')
        angle = np.arctan2(ys + 0.5 - cy, xs + 0.5 - cx)
        runes = (d < r - 3) & (d >= r - 8) & (np.round(angle * 8 / np.pi) % 2 == 0) & ((xs + ys) % 3 == 0)
        area[runes] = m.g('flame')

    def pylon(self, chest_id, x, y):
        """A crystal pylon the hero shuts down, kept like a brazier: bottom-center at (x, y)."""
        self.m.brazier(chest_id, x, y)


def gen_maraudon():
    c = Ruins('maraudon', 1024, 1024, MARAUDON, MARAUDON_OVERHEAD, extra=(MARAUDON_WATER,), thorny=True)
    m = c.m
    c.rect(448, 880, 128, 120)      # the cave mouth
    c.rect(488, 1000, 48, 24)       # the way out
    c.rect(456, 720, 112, 160)      # the crossing
    c.rect(352, 760, 104, 48)       # passage west
    c.rect(48, 640, 304, 256)       # the Foulspore Cavern, Noxxion's
    c.rect(152, 560, 48, 80)        # passage north
    c.rect(48, 336, 256, 224)       # the satyr den, Vyletongue's
    c.rect(568, 760, 104, 48)       # passage east
    c.rect(672, 640, 304, 256)      # the Wicked Grotto, Razorlash's and Gizlock's
    c.rect(904, 896, 48, 24)        # a hidden cave
    c.rect(872, 920, 112, 88)
    c.rect(824, 560, 48, 80)        # passage north
    c.rect(720, 336, 256, 224)      # Celebras's pool
    c.rect(488, 560, 48, 160)       # passage north
    c.rect(352, 304, 320, 256)      # Earth Song Falls
    c.rect(488, 264, 48, 40)        # passage north
    c.rect(304, 40, 416, 224)       # Zaetar's Grave, Theradras's
    c.render()
    c.exit(488, 1016, 'desolace', 'maraudon_exit')

    # Vines narrow the passages; crystals light the caves.
    c.thorns(352, 760, 40, 16)
    c.thorns(632, 760, 40, 16)
    c.thorns(152, 600, 24, 16)
    c.thorns(848, 584, 24, 16)
    for x, y in ((464, 888), (544, 888), (464, 728), (544, 728)):
        c.crystal(x, y)
    c.poison(80, 680, 96, 64)
    c.poison(232, 800, 96, 64)
    c.poison(232, 664, 64, 48)
    for x, y in ((64, 360), (272, 360), (160, 520)):
        c.crystal(x, y)
    for x, y in ((90, 440), (240, 470)):
        c.bones(x, y)
    for x, y in ((688, 660), (944, 660), (800, 860)):
        c.crystal(x, y)
    c.thorns(760, 720, 72, 16)
    c.crate(904, 700)
    c.barrel(928, 700)
    c.secret(904, 896, 48, 0)
    m.chest(55, 928, 980, 47)
    c.crystal(888, 944)
    c.pool(760, 368, 176, 72)
    for x, y in ((736, 470), (944, 470)):
        c.crystal(x, y)
    c.pool(376, 344, 96, 64)
    c.pool(552, 344, 96, 64)
    for x, y in ((368, 440), (640, 440), (368, 520), (640, 520)):
        c.crystal(x, y)
    for x, y in ((320, 56), (688, 56), (400, 120), (608, 120)):
        c.crystal(x, y)
    c.thorns(320, 200, 64, 16)
    c.thorns(640, 200, 64, 16)

    m.spawn('PUTRIDUS_SATYR', 476, 940)
    m.spawn('PUTRIDUS_SATYR', 548, 940)
    m.spawn('CREEPING_SLUDGE', 484, 780)
    m.spawn('BARBED_LASHER', 540, 840)
    for x, y in ((120, 790), (300, 720), (80, 860), (200, 680)):
        m.spawn('CREEPING_SLUDGE' if x != 300 else 'NOXXIOUS_SPAWN', x, y)
    m.spawn('NOXXION', 190, 760)
    for x, y in ((90, 400), (250, 420), (120, 500), (230, 520)):
        m.spawn('PUTRIDUS_SHADOWSTALKER' if y > 480 else 'PUTRIDUS_SATYR', x, y)
    m.spawn('LORD_VYLETONGUE', 176, 400)
    for x, y in ((720, 700), (920, 760), (700, 840), (860, 680)):
        m.spawn('BARBED_LASHER' if x < 800 else 'DEEPROT_STOMPER', x, y)
    m.spawn('RAZORLASH', 790, 780)
    m.spawn('TINKERER_GIZLOCK', 920, 840)
    for x, y in ((760, 480), (900, 480), (840, 520)):
        m.spawn('DEEPROT_STOMPER' if x != 840 else 'BARBED_LASHER', x, y)
    m.spawn('CELEBRAS_THE_CURSED', 848, 460)
    for x, y in ((400, 460), (620, 460), (460, 520), (580, 520)):
        m.spawn('THERADRIM_SHARDLING' if y < 500 else 'SUBTERRANEAN_DIEMETRADON', x, y)
    m.spawn('LANDSLIDE', 430, 430)
    m.spawn('ROTGRIP', 600, 430)
    m.spawn('THERADRIM_GUARDIAN', 512, 500)
    for x, y in ((360, 160), (660, 160), (420, 220), (600, 220)):
        m.spawn('THERADRIM_GUARDIAN' if y < 200 else 'THERADRIM_SHARDLING', x, y)
    m.spawn('PRINCESS_THERADRAS', 512, 110)
    m.point('theradras', 512, 230)
    m.area(0, 0, 1024, 1024, 'Maraudon')
    m.area(48, 640, 304, 256, 'Foulspore Cavern')
    m.area(48, 336, 256, 224, 'The Satyr Den')
    m.area(672, 640, 304, 256, 'The Wicked Grotto')
    m.area(720, 336, 256, 224, "Celebras' Pool")
    m.area(352, 304, 320, 256, 'Earth Song Falls')
    m.area(304, 40, 416, 224, "Zaetar's Grave")
    m.music = 'DUNGEON'
    m.save()
    return m


def gen_dire_maul():
    c = Ruins('dire_maul', 1024, 1024, DIRE_MAUL, DIRE_MAUL_OVERHEAD, extra=(DM_WATER,))
    m = c.m
    c.rect(416, 832, 192, 168)      # the gate court
    c.rect(488, 1000, 48, 24)       # the way out
    c.rect(400, 600, 224, 232)      # the Courtyard
    c.rect(624, 680, 64, 48)        # passage east
    c.rect(688, 600, 312, 208)      # the Warpwood Quarter, Zevrim's
    c.rect(840, 520, 48, 80)        # passage north
    c.rect(720, 312, 280, 208)      # the Conservatory, Hydrospawn's
    c.rect(840, 248, 48, 64)        # passage north
    c.rect(688, 40, 312, 208)       # the Shrine of Eldretharr, Lethtendris's and Alzzin's
    c.rect(336, 680, 64, 48)        # passage west
    c.rect(24, 600, 312, 208)       # the Capital Gardens, Tendris's
    c.rect(152, 808, 48, 24)        # a hidden vault
    c.rect(120, 832, 112, 88)
    c.rect(152, 520, 48, 80)        # passage north
    c.rect(24, 312, 280, 208)       # Immol'thar's prison
    c.rect(152, 248, 48, 64)        # passage north
    c.rect(24, 40, 312, 208)        # the Athenaeum, Tortheldrin's
    c.rect(488, 520, 48, 80)        # passage north
    c.rect(368, 312, 288, 208)      # Gordok Commons
    c.rect(488, 248, 48, 64)        # passage north
    c.rect(368, 40, 288, 208)       # the Gordok throne room
    c.render()
    c.exit(488, 1016, 'feralas', 'dm_exit')

    for x in (424, 584):
        c.torch(x, 836)
    for x in (456, 552):
        c.banner(x, 834)
    for x, y in ((424, 640), (584, 640), (424, 760), (584, 760)):
        c.slab(x, y)
    c.rug(488, 640, 48, 160)
    c.thorns(704, 700, 64, 16)
    c.thorns(880, 760, 96, 16)
    c.thorns(800, 620, 48, 16)
    c.pool(784, 360, 160, 72)
    for x in (736, 968):
        c.torch(x, 316)
    c.altar(912, 52, 64, 24)
    c.thorns(704, 160, 72, 16)
    c.thorns(920, 200, 64, 16)
    for x in (720, 768):
        c.candles(x, 48)
    # The Capital Gardens: Tendris's warped grove, with two of the pylons that hold Immol'thar.
    c.thorns(40, 700, 64, 16)
    c.thorns(240, 760, 80, 16)
    c.pylon(57, 72, 640)
    c.pylon(58, 288, 640)
    c.secret(152, 808, 48, 0)
    m.chest(56, 176, 892, 49)
    c.slab(136, 856)
    c.slab(200, 856)
    # The prison: Immol'thar paces in a circle of runes, held by the four pylons.
    c.field(164, 432, 40)
    c.pylon(59, 56, 360)
    c.pylon(60, 272, 360)
    for x in (40, 280):
        c.candles(x, 476)
    c.bookshelf(48, 40, 96)
    c.bookshelf(216, 40, 96)
    c.rug(160, 64, 32, 168)
    c.desk(96, 140, 48, 24)
    c.desk(208, 140, 48, 24)
    for x in (384, 624):
        c.torch(x, 316)
    for x, y in ((392, 400), (600, 440)):
        c.crate(x, y)
    c.barrel(624, 400)
    c.straw(400, 480)
    c.rug(488, 80, 48, 160)
    c.altar(480, 48, 64, 24)
    for x in (392, 616):
        c.rack(x, 44)
    for x in (456, 552):
        c.banner(x, 44)

    m.npc('SHENDRALAR_ANCIENT', 424, 704)
    m.spawn('WILDSPAWN_SATYR', 452, 900)
    m.spawn('WILDSPAWN_SATYR', 572, 900)
    m.spawn('WARPWOOD_CRUSHER', 560, 700)
    m.spawn('WHIP_LASHER', 470, 800)
    for x, y in ((740, 660), (960, 660), (760, 760), (940, 740)):
        m.spawn('WILDSPAWN_FELSWORN' if x > 900 else 'WILDSPAWN_SATYR', x, y)
    m.spawn('ZEVRIM_THORNHOOF', 860, 700)
    for x, y in ((760, 470), (900, 470), (960, 380)):
        m.spawn('HYDROLING', x, y)
    m.spawn('HYDROSPAWN', 860, 470)
    for x, y in ((740, 120), (960, 140), (800, 200)):
        m.spawn('WHIP_LASHER' if x != 800 else 'WARPWOOD_CRUSHER', x, y)
    m.spawn('LETHTENDRIS', 760, 90)
    m.spawn('ALZZIN_THE_WILDSHAPER', 944, 110)
    for x, y in ((80, 700), (280, 700), (120, 780), (240, 780)):
        m.spawn('PETRIFIED_TREANT' if y < 750 else 'IRONBARK_PROTECTOR', x, y)
    m.spawn('TENDRIS_WARPWOOD', 180, 690)
    for x, y in ((60, 440), (270, 440), (100, 500), (230, 500)):
        m.spawn('ARCANE_ABERRATION' if y < 470 else 'ELDRETH_SPECTRE', x, y)
    for x, y in ((120, 400), (210, 400)):
        m.spawn('EYE_OF_IMMOL_THAR', x, y)
    for x, y in ((60, 120), (280, 120), (100, 200), (240, 200)):
        m.spawn('HIGHBORNE_SUMMONER' if y < 150 else 'ELDRETH_SPECTRE', x, y)
    m.spawn('PRINCE_TORTHELDRIN', 176, 100)
    for x, y in ((400, 360), (620, 360), (430, 470), (600, 480)):
        m.spawn('GORDOK_BRUTE' if x < 500 else 'GORDOK_MASTIFF', x, y)
    m.spawn('GORDOK_MAGE_LORD', 512, 400)
    m.spawn('GORDOK_BRUTE', 512, 280)
    for x, y in ((400, 140), (620, 140)):
        m.spawn('GORDOK_BRUTE', x, y)
    m.spawn('CHO_RUSH_THE_OBSERVER', 440, 110)
    m.spawn('KING_GORDOK', 512, 100)
    m.point('event_boss', 164, 428)
    m.point('prison', 164, 500)
    m.point('gordok_throne', 512, 200)
    m.area(124, 392, 80, 80, '', 'PRISON')
    m.area(0, 0, 1024, 1024, 'Dire Maul')
    m.area(400, 600, 224, 232, 'The Courtyard')
    m.area(688, 600, 312, 208, 'Warpwood Quarter')
    m.area(720, 312, 280, 208, 'The Conservatory')
    m.area(688, 40, 312, 208, 'Shrine of Eldretharr')
    m.area(24, 600, 312, 208, 'Capital Gardens')
    m.area(24, 312, 280, 208, "Immol'thar's Prison")
    m.area(24, 40, 312, 208, 'The Athenaeum')
    m.area(368, 312, 288, 208, 'Gordok Commons')
    m.area(368, 40, 288, 208, 'The Gordok Throne')
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
    'hillsbrad': gen_hillsbrad,
    'tirisfal': gen_tirisfal,
    'sm_graveyard': gen_sm_graveyard,
    'sm_library': gen_sm_library,
    'stranglethorn': gen_stranglethorn,
    'sm_armory': gen_sm_armory,
    'sm_cathedral': gen_sm_cathedral,
    'tanaris': gen_tanaris,
    'thousand_needles': gen_thousand_needles,
    'razorfen_kraul': gen_razorfen_kraul,
    'razorfen_downs': gen_razorfen_downs,
    'zul_farrak': gen_zul_farrak,
    'feralas': gen_feralas,
    'desolace': gen_desolace,
    'maraudon': gen_maraudon,
    'dire_maul': gen_dire_maul,
}


def main():
    maps = {name: generate() for name, generate in GENERATORS.items()}
    ids = [chest[0] for m in maps.values() for chest in m.chests]
    if len(ids) != len(set(ids)) or max(ids) >= 256:
        raise SystemExit(f'chest ids must be unique and below 256: {sorted(ids)}')
    print(f'{len(ids)} treasure chests')
    starts = {'elwynn': 'start', 'westfall': 'from_elwynn', 'stormwind': 'from_elwynn', 'redridge': 'from_elwynn',
              'duskwood': 'from_elwynn', 'silverpine': 'flight', 'dun_morogh': 'from_ironforge',
              'wetlands': 'from_dun_morogh', 'darkshore': 'from_menethil', 'hillsbrad': 'from_wetlands',
              'tirisfal': 'flight', 'stranglethorn': 'from_duskwood', 'tanaris': 'from_booty_bay',
              'thousand_needles': 'from_tanaris', 'feralas': 'from_needles', 'desolace': 'from_feralas'}
    for name, m in maps.items():
        m.check_reachable(starts.get(name, 'entry'))
    write_minimaps(maps)


# Maps with a picture on the world map page, in the order D-pad left and right go through them.
# Interiors show the map their door leads to.
MINIMAPS = ['elwynn', 'stormwind', 'westfall', 'redridge', 'duskwood', 'silverpine', 'ironforge', 'dun_morogh',
            'wetlands', 'darkshore', 'hillsbrad', 'tirisfal', 'stranglethorn', 'tanaris', 'thousand_needles',
            'feralas', 'desolace', 'echo_ridge', 'fargodeep', 'deadmines', 'stockade', 'shadowfang',
            'blackfathom_deeps', 'gnomeregan', 'sm_graveyard', 'sm_library', 'sm_armory', 'sm_cathedral',
            'razorfen_kraul', 'razorfen_downs', 'zul_farrak', 'maraudon', 'dire_maul']


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
