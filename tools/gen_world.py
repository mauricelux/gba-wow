"""Generate every map in the game (backgrounds, collision, metadata headers).

Run from the tools directory:  python3 gen_world.py

Maps:
  elwynn     2048x2048 outdoor region: Northshire Valley, Goldshire, farms, lake, forests
  abbey      Northshire Abbey interior
  inn        Lion's Pride Inn interior (Goldshire)
  westfall   1024x1024 outdoor region: Sentinel Hill, farms, Moonbrook
  deadmines  dungeon below Moonbrook
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
    wg.paint_paths(m)

    # --- water -------------------------------------------------------------------------------
    wg.corners_ellipse(m, 'water', 1400, 1660, 176, 104)
    wg.corners_ellipse(m, 'water', 1520, 1720, 96, 72)
    wg.paint_water(m)

    # --- forest borders and the valley walls -------------------------------------------------
    border = 64
    wg.forest(m, trees, 0, 0, m.width, border)
    wg.forest(m, trees, 0, m.height - border, m.width, border)
    wg.forest(m, trees, 0, 0, border, 1400)
    wg.forest(m, trees, 0, 1480, border, m.height - 1480)
    wg.forest(m, trees, m.width - border, 0, border, m.height)
    # Northshire Valley: x 640..1408, y 64..832, opening south at x 992..1056.
    wg.forest(m, trees, 576, 64, 64, 832)
    wg.forest(m, trees, 1408, 64, 64, 832)
    wg.forest(m, trees, 576, 832, 416, 64)
    wg.forest(m, trees, 1056, 832, 416, 64)

    # --- Northshire Valley ---------------------------------------------------------------------
    door = wg.abbey(m, 944, 160)
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
    m.area(640, 64, 240, 220, 'Echo Ridge Mine', 'ECHO_RIDGE')
    m.spawn_group('KOBOLD_VERMIN', 780, 240, 8, 90, seed=1)

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
                       (1160, 720), (880, 760), (1288, 740), (680, 720), (1100, 560)], seed=1)
    for bx, by in ((960, 400), (1080, 380), (920, 520), (1120, 600), (1300, 380), (860, 300)):
        wg.bush(m, bx, by)
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
    wg.grove(m, trees, [(760, 1000), (1300, 1000), (1320, 1300), (720, 1380), (1180, 1340)], seed=3)
    m.area(780, 980, 560, 420, 'Goldshire')

    # --- Stonefield farm (east) -------------------------------------------------------------------
    wg.house(m, 1680, 1120, 96, 96, roof_colors=('thatch_d', 'thatch_m', 'thatch_l'), roof_ridge='thatch_l')
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

    # --- Fargodeep Mine (south) ----------------------------------------------------------------------
    wg.mine_entrance(m, 944, 1736)
    m.spawn_group('KOBOLD_TUNNELER', 980, 1660, 8, 110, seed=11)
    m.area(840, 1560, 300, 260, 'Fargodeep Mine', 'FARGODEEP')

    # --- Forest's Edge and Hogger (south-west) ----------------------------------------------------
    wg.forest(m, trees, 64, 1560, 200, 420, solid=True)
    wg.forest(m, trees, 600, 1580, 160, 400, solid=True)
    m.spawn_group('RIVERPAW_GNOLL', 420, 1680, 7, 110, seed=12)
    m.spawn('HOGGER', 400, 1860)
    m.area(260, 1560, 340, 420, "Forest's Edge")
    m.npc('GUARD_WEST', 140, 1400)

    # --- woods with timber wolves (west and east) --------------------------------------------------
    wg.forest(m, trees, 160, 160, 320, 360, solid=True)
    wg.forest(m, trees, 1600, 160, 320, 320, solid=True)
    wg.forest(m, trees, 1600, 600, 200, 280, solid=True)
    m.spawn_group('TIMBER_WOLF', 400, 900, 6, 140, seed=13)
    m.spawn_group('TIMBER_WOLF', 1650, 960, 5, 120, seed=14)
    m.spawn_group('FOREST_SPIDER', 300, 640, 5, 100, seed=15)
    m.spawn_group('FOREST_SPIDER', 1880, 560, 4, 80, seed=16)
    scatter = []
    rng = np.random.default_rng(42)
    for _ in range(140):
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
    wg.grove(m, trees, kept, seed=5)
    for i in range(40):
        x, y = int(rng.uniform(100, 1940)), int(rng.uniform(100, 1940))
        mx, my = x // 16, y // 16
        if m.area_free(x, y, 16, 16) and path[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max() == 0 \
                and water[max(0, my - 2):my + 3, max(0, mx - 2):mx + 3].max() == 0:
            (wg.bush if i % 3 else wg.stump)(m, x, y)

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
    for _ in range(40):
        x, y = int(rng.uniform(60, 900)), int(rng.uniform(60, 940))
        mx, my = (x + 16) // 16, (y + 40) // 16
        if m.area_free(x, y, 40, 56) and path[max(0, my - 3):my + 4, max(0, mx - 3):mx + 4].max() == 0:
            wg.tree(m, trees, x, y, int(rng.integers(0, 3)))

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
    m.area(0, 0, 1024, 512, 'The Deadmines')
    m.area(560, 32, 432, 144, 'Ironclad Cove', 'IRONCLAD_COVE')
    m.music = 'DUNGEON'
    m.save()
    return m


GENERATORS = {
    'abbey': gen_abbey,
    'inn': gen_inn,
    'elwynn': gen_elwynn,
    'westfall': gen_westfall,
    'deadmines': gen_deadmines,
}


def main():
    maps = {name: generate() for name, generate in GENERATORS.items()}
    for m in maps.values():
        m.write_header(maps)


if __name__ == '__main__':
    main()
