"""Generate the Northshire test map: two background layers and a collision grid.

Outputs:
  graphics/northshire_ground.bmp    ground layer (drawn under characters), 1024x1024, 2 palette banks
  graphics/northshire_overhead.bmp  overhead layer (tree tops, roofs; drawn over characters)
  include/gw_northshire_collision.h 128x128 grid of 8x8 px cells, 1 bit each

The map is built from 16x16 px metatiles. Paths and the pond use a dual grid (each metatile
is shaped by its four corners), which keeps the number of unique 8x8 tiles low enough for
the GBA (Butano allows up to 1024 per background).

Placeholder art until the world moves to Tiled in milestone 2.
"""

import numpy as np

from art_common import (GRAPHICS, INCLUDE, PREVIEW, check_tile_banks, save_indexed_bmp,
                        save_preview_png, unique_tile_count)

SIZE = 1024
META = 16
METAS = SIZE // META
CELL = 8
CELLS = SIZE // CELL

# Ground palette, bank 0: grass, dirt, water, tree trunks.
G_BACKDROP, G_SHADOW, G_GRASS_D, G_GRASS_M, G_GRASS_L, G_GRASS_H = 0, 1, 2, 3, 4, 5
G_DIRT_D, G_DIRT_M, G_DIRT_L = 6, 7, 8
G_WATER_D, G_WATER_M, G_WATER_L, G_FOAM = 9, 10, 11, 12
G_TRUNK_D, G_TRUNK_M, G_YELLOW = 13, 14, 15
# Ground palette, bank 1: the abbey (stone, wood, glass, banners).
S_OUTLINE, S_STONE_D, S_STONE_M, S_STONE_L, S_STONE_H = 17, 18, 19, 20, 21
S_WOOD_D, S_WOOD_L, S_GLASS_D, S_GLASS_L = 22, 23, 24, 25
S_BANNER_D, S_BANNER, S_GOLD, S_PATH_D, S_PATH_L = 26, 27, 28, 29, 30

GROUND_PALETTE = [
    (40, 72, 40), (32, 64, 40), (56, 104, 48), (80, 136, 56), (112, 168, 72), (160, 200, 96),
    (120, 88, 56), (160, 120, 72), (192, 152, 96),
    (40, 72, 144), (64, 112, 184), (112, 160, 216), (224, 232, 232),
    (72, 48, 32), (112, 80, 48), (240, 208, 72),
    (255, 0, 255), (32, 32, 48), (88, 88, 104), (128, 128, 144), (168, 168, 176), (208, 208, 200),
    (88, 56, 32), (144, 96, 56), (40, 48, 80), (240, 200, 96),
    (32, 48, 128), (56, 88, 184), (232, 184, 64), (112, 104, 96), (184, 176, 160), (0, 248, 248),
]

# Overhead palette: tree canopies and roofs.
O_OUTLINE, O_LEAF_0, O_LEAF_1, O_LEAF_2, O_LEAF_3, O_LEAF_4 = 1, 2, 3, 4, 5, 6
O_ROOF_D, O_ROOF_M, O_ROOF_L, O_STONE_D, O_STONE_M, O_STONE_L = 7, 8, 9, 10, 11, 12
O_GOLD, O_GLASS, O_ROOF_H = 13, 14, 15

OVERHEAD_PALETTE = [
    (255, 0, 255), (16, 32, 32), (24, 56, 48), (40, 88, 56), (64, 120, 64), (96, 152, 72),
    (144, 192, 96),
    (48, 56, 96), (72, 88, 136), (104, 128, 176), (88, 88, 104), (128, 128, 144), (168, 168, 176),
    (232, 184, 64), (40, 48, 80), (144, 168, 208),
]

rng = np.random.default_rng(1337)


def tile_hash(x, y, salt=0):
    h = (x * 73856093) ^ (y * 19349663) ^ (salt * 83492791)
    return (h ^ (h >> 13)) & 0xFFFF


class World:
    def __init__(self):
        self.ground = np.zeros((SIZE, SIZE), dtype=np.uint8)
        self.overhead = np.zeros((SIZE, SIZE), dtype=np.uint8)
        self.solid = np.zeros((CELLS, CELLS), dtype=bool)
        self.path_corners = np.zeros((METAS + 1, METAS + 1), dtype=float)
        self.water_corners = np.zeros((METAS + 1, METAS + 1), dtype=float)
        self.stone_rects = []

    def block(self, x, y, w, h):
        """Mark a pixel rectangle as solid (rounded out to whole 8x8 cells)."""
        self.solid[max(0, y // CELL):(y + h + CELL - 1) // CELL,
                   max(0, x // CELL):(x + w + CELL - 1) // CELL] = True


# ---------------------------------------------------------------------------------------------
# Grass
# ---------------------------------------------------------------------------------------------

def make_grass_variants():
    variants = []
    for v in range(6):
        local = np.random.default_rng(100 + v)
        tile = np.full((META, META), G_GRASS_M, dtype=np.uint8)
        noise = local.random((META, META))
        tile[noise < 0.07] = G_GRASS_D
        tile[noise > 0.95] = G_GRASS_L
        # Grass tufts: a light blade over a dark root.
        for _ in range(2 + v % 3):
            x, y = local.integers(1, META - 1), local.integers(2, META)
            tile[y - 1, x] = G_GRASS_H
            tile[y, x] = G_GRASS_D
            if x + 1 < META:
                tile[y - 1, x + 1] = G_GRASS_L
        if v == 4:   # yellow flowers
            for _ in range(3):
                x, y = local.integers(2, META - 2), local.integers(2, META - 2)
                tile[y, x] = G_YELLOW
                tile[y + 1, x] = G_GRASS_D
        if v == 5:   # white flowers
            for _ in range(3):
                x, y = local.integers(2, META - 2), local.integers(2, META - 2)
                tile[y, x] = G_FOAM
                tile[y + 1, x] = G_GRASS_D
        variants.append(tile)
    return variants


def paint_grass(world):
    variants = make_grass_variants()
    for my in range(METAS):
        for mx in range(METAS):
            h = tile_hash(mx, my) % 100
            v = 0 if h < 30 else 1 if h < 55 else 2 if h < 75 else 3 if h < 90 else 4 if h < 95 else 5
            world.ground[my * META:(my + 1) * META, mx * META:(mx + 1) * META] = variants[v]


# ---------------------------------------------------------------------------------------------
# Dual-grid terrain (paths, pond)
# ---------------------------------------------------------------------------------------------

EDGE_NOISE = np.random.default_rng(7).random((META, META)) * 0.24 - 0.12
SPECKLE = np.random.default_rng(8).random((META, META))


def corner_field(corners, mx, my):
    tl, tr = corners[my, mx], corners[my, mx + 1]
    bl, br = corners[my + 1, mx], corners[my + 1, mx + 1]
    u = (np.arange(META) + 0.5) / META
    uu, vv = np.meshgrid(u, u)
    return tl * (1 - uu) * (1 - vv) + tr * uu * (1 - vv) + bl * (1 - uu) * vv + br * uu * vv + EDGE_NOISE


def set_corners_along(corners, points, radius):
    """Set dual-grid corners within radius (in metatiles) of a polyline."""
    ys, xs = np.mgrid[0:corners.shape[0], 0:corners.shape[1]]
    for (x0, y0), (x1, y1) in zip(points, points[1:]):
        dx, dy = x1 - x0, y1 - y0
        length2 = dx * dx + dy * dy
        t = np.clip(((xs - x0) * dx + (ys - y0) * dy) / length2, 0, 1)
        dist = np.hypot(xs - (x0 + t * dx), ys - (y0 + t * dy))
        corners[dist <= radius] = 1


def paint_paths(world):
    for my in range(METAS):
        for mx in range(METAS):
            c = world.path_corners[my:my + 2, mx:mx + 2]
            if c.max() == 0:
                continue
            f = corner_field(world.path_corners, mx, my)
            tile = world.ground[my * META:(my + 1) * META, mx * META:(mx + 1) * META]
            edge = (f > 0.5) & (f <= 0.62)
            inner = f > 0.62
            tile[edge] = G_DIRT_D
            tile[inner] = G_DIRT_M
            tile[inner & (SPECKLE > 0.85)] = G_DIRT_L
            tile[inner & (SPECKLE < 0.06)] = G_DIRT_D
            # A thin dark rim of grass makes the path read as slightly sunken.
            tile[(f > 0.42) & (f <= 0.5)] = G_GRASS_D


def paint_water(world):
    sparkle = np.random.default_rng(9).random((META, META))
    for my in range(METAS):
        for mx in range(METAS):
            c = world.water_corners[my:my + 2, mx:mx + 2]
            if c.max() == 0:
                continue
            f = corner_field(world.water_corners, mx, my)
            tile = world.ground[my * META:(my + 1) * META, mx * META:(mx + 1) * META]
            tile[(f > 0.36) & (f <= 0.46)] = G_DIRT_L
            tile[(f > 0.46) & (f <= 0.52)] = G_FOAM
            water = f > 0.52
            tile[water] = G_WATER_M
            tile[water & (f < 0.62)] = G_WATER_D
            tile[water & (f >= 0.62) & (sparkle > 0.985)] = G_WATER_L
            # Water is solid: mark each 8x8 cell that is mostly water.
            for cy in range(2):
                for cx in range(2):
                    if water[cy * 8:(cy + 1) * 8, cx * 8:(cx + 1) * 8].sum() >= 24:
                        world.solid[my * 2 + cy, mx * 2 + cx] = True


# ---------------------------------------------------------------------------------------------
# Trees
# ---------------------------------------------------------------------------------------------

TREE_W, TREE_H, CANOPY_H = 32, 48, 28


def make_tree_canopy(seed):
    """A round canopy built from overlapping leaf clumps lit from the top left."""
    local = np.random.default_rng(seed)
    canopy = np.zeros((CANOPY_H, TREE_W), dtype=np.uint8)
    clumps = [(16, 14, 13)] + [(local.uniform(8, 24), local.uniform(7, 20), local.uniform(6, 8))
                               for _ in range(7)]
    ys, xs = np.mgrid[0:CANOPY_H, 0:TREE_W]
    inside = np.zeros_like(canopy, dtype=bool)
    shade = np.zeros(canopy.shape)
    for cx, cy, r in clumps:
        d = np.hypot(xs + 0.5 - cx, (ys + 0.5 - cy) * 1.1)
        m = d < r
        # Light from the top left: brighter toward the clump's upper-left.
        light = 1 - np.hypot(xs + 0.5 - (cx - r * 0.45), ys + 0.5 - (cy - r * 0.5)) / (r * 1.6)
        shade = np.where(m, np.maximum(shade, light) if inside.any() else light, shade)
        shade[m & ~inside] = light[m & ~inside]
        inside |= m
    levels = np.digitize(shade, [0.05, 0.3, 0.55, 0.8])
    leaf = [O_LEAF_0, O_LEAF_1, O_LEAF_2, O_LEAF_3, O_LEAF_4]
    for i, value in enumerate(leaf):
        canopy[inside & (levels == i)] = value
    # Dither the boundaries between shades a little.
    dither = local.random(canopy.shape) < 0.12
    canopy[inside & dither & (canopy > O_LEAF_0)] -= 1
    # Outline.
    padded = np.pad(inside, 1)
    edge = inside & ~(padded[:-2, 1:-1] & padded[2:, 1:-1] & padded[1:-1, :-2] & padded[1:-1, 2:])
    canopy[edge] = O_OUTLINE
    return canopy


def make_tree_trunk():
    trunk = np.zeros((TREE_H - CANOPY_H + 4, TREE_W), dtype=np.uint8)
    # trunk rows map to tree rows CANOPY_H-4 .. TREE_H-1
    for y in range(trunk.shape[0]):
        flare = 1 if y > trunk.shape[0] - 5 else 0
        x0, x1 = 12 - flare, 20 + flare
        trunk[y, x0:x1] = G_TRUNK_M
        trunk[y, x0] = G_TRUNK_D
        trunk[y, x1 - 1] = G_TRUNK_D
        trunk[y, x0 + 3] = G_TRUNK_D if y % 5 == 2 else G_TRUNK_M
    # Roots and a shadow on the grass.
    bottom = trunk.shape[0] - 1
    trunk[bottom, 9:23] = G_SHADOW
    trunk[bottom - 1, 10:12] = G_TRUNK_D
    trunk[bottom - 1, 20:22] = G_TRUNK_D
    trunk[bottom, 11:21] = G_TRUNK_D
    return trunk


CANOPIES = [make_tree_canopy(s) for s in (11, 12, 13)]
TRUNK = make_tree_trunk()


def place_tree(world, x, y, variant=0, collide=True):
    """Place a tree with its top-left corner at (x, y); positions are snapped to 8 px."""
    x, y = x // 8 * 8, y // 8 * 8
    canopy = CANOPIES[variant % len(CANOPIES)]
    trunk_y = y + CANOPY_H - 4
    for part, layer, py in ((TRUNK, world.ground, trunk_y), (canopy, world.overhead, y)):
        h, w = part.shape
        y0, y1 = max(py, 0), min(py + h, SIZE)
        x0, x1 = max(x, 0), min(x + w, SIZE)
        if y0 >= y1 or x0 >= x1:
            continue
        src = part[y0 - py:y1 - py, x0 - x:x1 - x]
        dst = layer[y0:y1, x0:x1]
        dst[src != 0] = src[src != 0]
    if collide:
        world.block(x + 8, y + 32, 16, 16)


def place_bush(world, x, y):
    x, y = x // 8 * 8, y // 8 * 8
    ys, xs = np.mgrid[0:16, 0:16]
    d = np.hypot(xs + 0.5 - 8, (ys + 0.5 - 9) * 1.2)
    inside = d < 7.5
    light = 1 - np.hypot(xs - 5, ys - 5) / 12
    tile = world.ground[y:y + 16, x:x + 16]
    tile[inside] = np.where(light[inside] > 0.55, G_GRASS_H, np.where(light[inside] > 0.25, G_GRASS_L, G_GRASS_D))
    padded = np.pad(inside, 1)
    edge = inside & ~(padded[:-2, 1:-1] & padded[2:, 1:-1] & padded[1:-1, :-2] & padded[1:-1, 2:])
    tile[edge] = G_SHADOW
    tile[15, 3:13] = G_SHADOW
    world.block(x + 2, y + 8, 12, 8)


# ---------------------------------------------------------------------------------------------
# Northshire Abbey
# ---------------------------------------------------------------------------------------------

ABBEY_W, ABBEY_H, ABBEY_ROOF_H = 160, 128, 64


def paint_bricks(canvas, x0, y0, w, h, dark, mid, light):
    for y in range(h):
        for x in range(w):
            row = y // 4
            offset = 4 if row % 2 else 0
            if y % 4 == 3:
                c = dark
            elif (x + offset) % 8 == 7:
                c = dark
            elif y % 4 == 0 and (x + offset) % 8 < 3:
                c = light
            else:
                c = mid
            canvas[y0 + y, x0 + x] = c


def place_abbey(world, x, y):
    """Abbey at (x, y), 160x128: slate roof and bell tower overhead, stone walls on the ground."""
    g, o = world.ground, world.overhead
    wall_y = y + ABBEY_ROOF_H
    wall_h = ABBEY_H - ABBEY_ROOF_H
    # Walls.
    paint_bricks(g, x, wall_y, ABBEY_W, wall_h, S_STONE_D, S_STONE_M, S_STONE_L)
    g[wall_y:wall_y + 2, x:x + ABBEY_W] = S_STONE_H
    g[wall_y + wall_h - 2:wall_y + wall_h, x:x + ABBEY_W] = S_STONE_D
    g[wall_y:wall_y + wall_h, x] = S_OUTLINE
    g[wall_y:wall_y + wall_h, x + ABBEY_W - 1] = S_OUTLINE
    # Buttresses.
    for bx in (x + 24, x + 56, x + 96, x + 128):
        g[wall_y:wall_y + wall_h, bx:bx + 8] = S_STONE_L
        g[wall_y:wall_y + wall_h, bx + 7] = S_STONE_D
        g[wall_y:wall_y + wall_h, bx] = S_STONE_H
    # Arched windows.
    for wx in (x + 8, x + 40, x + 108, x + 140):
        g[wall_y + 16:wall_y + 40, wx:wx + 12] = S_OUTLINE
        g[wall_y + 18:wall_y + 38, wx + 2:wx + 10] = S_GLASS_D
        g[wall_y + 22:wall_y + 38, wx + 3:wx + 9:2] = S_GLASS_L
        g[wall_y + 16, wx:wx + 3] = S_STONE_M
        g[wall_y + 16, wx + 9:wx + 12] = S_STONE_M
    # Great door with banners either side.
    dx = x + ABBEY_W // 2 - 12
    g[wall_y + 24:wall_y + wall_h, dx:dx + 24] = S_OUTLINE
    g[wall_y + 26:wall_y + wall_h, dx + 2:dx + 22] = S_WOOD_D
    for px in range(dx + 3, dx + 21, 4):
        g[wall_y + 28:wall_y + wall_h, px:px + 2] = S_WOOD_L
    g[wall_y + 44, dx + 9:dx + 15] = S_GOLD
    g[wall_y + 22:wall_y + 26, dx + 2:dx + 22] = S_STONE_H
    for bx in (dx - 16, dx + 32):
        g[wall_y + 12:wall_y + 44, bx:bx + 8] = S_BANNER
        g[wall_y + 12:wall_y + 44, bx] = S_BANNER_D
        g[wall_y + 12:wall_y + 14, bx:bx + 8] = S_GOLD
        g[wall_y + 24:wall_y + 30, bx + 2:bx + 6] = S_GOLD
        g[wall_y + 44, bx + 1:bx + 7] = S_BANNER
        g[wall_y + 45, bx + 2:bx + 6] = S_BANNER
        g[wall_y + 46, bx + 3:bx + 5] = S_BANNER

    # Roof (overhead): shingles getting lighter toward the ridge, with eaves.
    for ry in range(ABBEY_ROOF_H - 24, ABBEY_ROOF_H):
        for rx in range(ABBEY_W):
            band = (ry // 4) % 2
            c = O_ROOF_M
            if ry % 4 == 3:
                c = O_ROOF_D
            elif (rx + band * 4) % 8 == 0:
                c = O_ROOF_D
            elif ry % 4 == 0:
                c = O_ROOF_L
            o[y + ry, x + rx] = c
    o[y + ABBEY_ROOF_H - 2:y + ABBEY_ROOF_H, x:x + ABBEY_W] = O_OUTLINE
    o[y + ABBEY_ROOF_H - 25, x:x + ABBEY_W] = O_ROOF_H
    o[y + ABBEY_ROOF_H - 24:y + ABBEY_ROOF_H, x] = O_OUTLINE
    o[y + ABBEY_ROOF_H - 24:y + ABBEY_ROOF_H, x + ABBEY_W - 1] = O_OUTLINE
    # Upper roof slope, seen from above.
    for ry in range(8, ABBEY_ROOF_H - 25):
        for rx in range(8, ABBEY_W - 8):
            c = O_ROOF_L if (rx // 8 + ry // 4) % 2 else O_ROOF_M
            if ry % 4 == 3:
                c = O_ROOF_D
            o[y + ry, x + rx] = c
    o[y + 8, x + 8:x + ABBEY_W - 8] = O_ROOF_H
    o[y + 7, x + 8:x + ABBEY_W - 8] = O_OUTLINE
    o[y + 7:y + ABBEY_ROOF_H - 25, x + 7] = O_OUTLINE
    o[y + 7:y + ABBEY_ROOF_H - 25, x + ABBEY_W - 8] = O_OUTLINE
    # Bell tower on the ridge.
    tx = x + ABBEY_W // 2 - 16
    for ry in range(0, 40):
        for rx in range(32):
            c = O_STONE_M
            if ry % 4 == 3 or (rx + (ry // 4 % 2) * 4) % 8 == 7:
                c = O_STONE_D
            elif ry % 4 == 0:
                c = O_STONE_L
            o[y + ry, tx + rx] = c
    o[y:y + 40, tx] = O_OUTLINE
    o[y:y + 40, tx + 31] = O_OUTLINE
    o[y, tx:tx + 32] = O_OUTLINE
    o[y + 1:y + 6, tx + 1:tx + 31] = O_ROOF_D
    o[y + 1, tx + 1:tx + 31] = O_ROOF_H
    o[y + 12:y + 28, tx + 10:tx + 22] = O_OUTLINE
    o[y + 14:y + 28, tx + 12:tx + 20] = O_GLASS
    o[y + 20:y + 25, tx + 14:tx + 18] = O_GOLD

    # Collision: the walls and the lower roof (so the upper roof can hide the player).
    world.block(x, y + 32, ABBEY_W, ABBEY_H - 32)


def place_courtyard(world, x, y, w, h):
    """Paved stone square (ground bank 1), snapped to 8x8 tiles."""
    g = world.ground
    for py in range(h):
        for px in range(w):
            stone = ((px // 8) + (py // 8)) % 2
            c = S_PATH_L if stone else S_STONE_L
            if px % 8 == 7 or py % 8 == 7:
                c = S_PATH_D
            g[y + py, x + px] = c


# ---------------------------------------------------------------------------------------------
# Map layout
# ---------------------------------------------------------------------------------------------

def build():
    world = World()
    paint_grass(world)

    # Paths: from the abbey courtyard south toward Goldshire, with branches to the pond (east)
    # and the wolf meadow (west). Coordinates are in metatile corners.
    set_corners_along(world.path_corners, [(32, 17), (32, 24), (30, 32), (31, 42), (34, 52), (33, 64)], 1.2)
    set_corners_along(world.path_corners, [(31, 28), (38, 29), (44, 31)], 0.9)
    set_corners_along(world.path_corners, [(30, 37), (22, 38), (14, 41)], 0.9)
    paint_paths(world)

    # Pond in the east.
    ys, xs = np.mgrid[0:METAS + 1, 0:METAS + 1]
    pond = ((xs - 50) / 6.5) ** 2 + ((ys - 30) / 4.5) ** 2 <= 1
    pond |= ((xs - 54) / 3.5) ** 2 + ((ys - 34) / 3) ** 2 <= 1
    world.water_corners[pond] = 1
    paint_water(world)

    # Abbey and courtyard.
    abbey_x, abbey_y = 432, 112
    place_abbey(world, abbey_x, abbey_y)
    place_courtyard(world, abbey_x + 48, abbey_y + ABBEY_H, 64, 32)

    # Forest border: staggered rows of trees all around, solid behind the inner row.
    border = 72
    for row, ty in enumerate(range(-24, border - 24, 24)):
        for tx in range(-16 + (row % 2) * 16, SIZE, 32):
            place_tree(world, tx, ty, row + tx // 32, collide=False)
    for row, ty in enumerate(range(SIZE - border - 24, SIZE, 24)):
        for tx in range(-16 + (row % 2) * 16, SIZE, 32):
            if 488 <= tx <= 568 and ty < SIZE - 40:
                continue   # leave the road south open
            place_tree(world, tx, ty, row + tx // 32, collide=False)
    for side_x in (-8, SIZE - 56):
        for row, ty in enumerate(range(border - 24, SIZE - border - 24, 24)):
            for k in range(2):
                place_tree(world, side_x + k * 32 - (row % 2) * 16 * (1 if side_x < 0 else -1) + 8,
                           ty, row + k, collide=False)
    world.block(0, 0, SIZE, border - 8)
    world.block(0, 0, 64, SIZE)
    world.block(SIZE - 64, 0, 64, SIZE)
    world.block(0, SIZE - border + 16, 488, border)
    world.block(600, SIZE - border + 16, SIZE - 600, border)

    # Scattered trees and groves inside the clearing.
    groves = [(160, 160), (192, 184), (144, 216), (720, 152), (760, 184), (688, 200),
              (240, 432), (272, 456), (112, 520), (176, 600), (208, 640), (680, 600),
              (720, 640), (760, 608), (840, 720), (880, 760), (360, 760), (400, 800),
              (120, 840), (624, 824), (880, 280), (904, 320), (320, 280), (600, 300)]
    for i, (gx, gy) in enumerate(groves):
        place_tree(world, gx, gy, i)

    bushes = [(400, 288), (560, 288), (456, 360), (592, 376), (296, 520), (352, 552),
              (600, 520), (640, 560), (240, 720), (712, 760), (520, 700), (440, 640),
              (840, 360), (168, 360), (904, 600), (96, 680)]
    for bx, by in bushes:
        place_bush(world, bx, by)

    return world


def write_collision_header(world):
    rows = []
    for cy in range(CELLS):
        bits = world.solid[cy]
        values = []
        for byte in range(CELLS // 8):
            v = 0
            for b in range(8):
                if bits[byte * 8 + b]:
                    v |= 1 << b
            values.append(f'0x{v:02x}')
        rows.append('        ' + ', '.join(values) + ',')
    text = '\n'.join([
        '// Generated by tools/gen_northshire.py. Do not edit by hand.',
        '#ifndef GW_NORTHSHIRE_COLLISION_H',
        '#define GW_NORTHSHIRE_COLLISION_H',
        '',
        '#include <cstdint>',
        '',
        'namespace gw::northshire',
        '{',
        f'    constexpr int collision_cell_size = {CELL};',
        f'    constexpr int collision_columns = {CELLS};',
        f'    constexpr int collision_rows = {CELLS};',
        '',
        '    // One bit per cell, row-major, least significant bit first. 1 = solid.',
        f'    alignas(4) constexpr uint8_t collision[] = {{',
        *rows,
        '    };',
        '}',
        '',
        '#endif',
        '',
    ])
    (INCLUDE / 'gw_northshire_collision.h').write_text(text)


def main():
    world = build()
    check_tile_banks(world.ground, 'northshire_ground')
    ground_tiles = unique_tile_count(world.ground)
    overhead_tiles = unique_tile_count(world.overhead)
    print(f'ground: {ground_tiles} unique tiles, overhead: {overhead_tiles} unique tiles (limit 1024)')
    if ground_tiles > 1024 or overhead_tiles > 1024:
        raise SystemExit('too many unique tiles')

    save_indexed_bmp(GRAPHICS / 'northshire_ground.bmp', world.ground, GROUND_PALETTE, pad_to_256=True)
    (GRAPHICS / 'northshire_ground.json').write_text(
        '{\n    "type": "regular_bg",\n    "bpp_mode": "bpp_4_manual",\n    "colors_count": 32\n}\n')
    save_indexed_bmp(GRAPHICS / 'northshire_overhead.bmp', world.overhead, OVERHEAD_PALETTE)
    (GRAPHICS / 'northshire_overhead.json').write_text(
        '{\n    "type": "regular_bg",\n    "bpp_mode": "bpp_4_manual"\n}\n')
    write_collision_header(world)

    # Preview: overhead composited on the ground, collision tinted red.
    lut_g = np.array([c for c in GROUND_PALETTE] + [(0, 0, 0)] * (256 - len(GROUND_PALETTE)), dtype=np.uint8)
    lut_o = np.array(OVERHEAD_PALETTE + [(0, 0, 0)] * (256 - len(OVERHEAD_PALETTE)), dtype=np.uint8)
    rgb = lut_g[world.ground]
    over = world.overhead != 0
    rgb[over] = lut_o[world.overhead][over]
    from PIL import Image
    PREVIEW.mkdir(parents=True, exist_ok=True)
    Image.fromarray(rgb, 'RGB').save(PREVIEW / 'northshire.png')
    solid = np.kron(world.solid, np.ones((CELL, CELL), dtype=bool))
    tinted = rgb.copy()
    tinted[solid] = (tinted[solid] * 0.5 + np.array([255, 0, 0]) * 0.5).astype(np.uint8)
    Image.fromarray(tinted, 'RGB').save(PREVIEW / 'northshire_collision.png')


if __name__ == '__main__':
    main()
