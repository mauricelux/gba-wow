"""Toolkit for generating the game's maps: background layers, collision and map metadata.

A map has two background layers and a collision grid:
  ground    drawn under characters (terrain, walls, floors)
  overhead  drawn over characters (tree tops, roofs), mostly transparent
  solid     one flag per 8x8 cell

Palettes are described by named roles grouped in 16-color banks. Every 8x8 tile of a 4bpp
background must use a single bank, so objects drawn over grass only use the terrain bank,
and everything using other banks (buildings, fields, camps) is snapped to whole tiles.

Patterns such as bricks, shingles and cobbles are computed from absolute coordinates, so the
same tiles repeat everywhere and the number of unique tiles stays under the GBA's limits.

Maps also collect metadata (warps, NPCs, enemy spawns, named areas, points) which is written
to a C++ header next to the collision grid.
"""

import numpy as np
from PIL import Image

from art_common import (GRAPHICS, INCLUDE, PREVIEW, check_tile_banks, save_indexed_bmp,
                        unique_tile_count)

META = 16
CELL = 8

# ---------------------------------------------------------------------------------------------
# Palettes
# ---------------------------------------------------------------------------------------------


class Palette:
    """Named colors in 16-color banks. Index 0 of each bank is transparent."""

    def __init__(self, banks):
        self.colors = []
        self.index = {}
        for b, bank in enumerate(banks):
            if len(bank) > 15:
                raise ValueError(f'bank {b} has more than 15 colors')
            self.colors.append((255, 0, 255) if b == 0 else (0, 255 - b * 8, 255))
            for role, rgb in bank:
                if role in self.index:
                    raise ValueError(f'duplicate role {role}')
                self.index[role] = len(self.colors)
                self.colors.append(rgb)
            while len(self.colors) % 16:
                n = len(self.colors)
                self.colors.append(((n * 8) & 0xF8, 240, (b * 24 + 8) & 0xF8))

    def __getitem__(self, role):
        return self.index[role]

    @property
    def banks(self):
        return len(self.colors) // 16


TERRAIN_ELWYNN = [
    ('shadow', (32, 64, 40)), ('grass_d', (56, 104, 48)), ('grass_m', (80, 136, 56)),
    ('grass_l', (112, 168, 72)), ('grass_h', (160, 200, 96)),
    ('dirt_d', (120, 88, 56)), ('dirt_m', (160, 120, 72)), ('dirt_l', (192, 152, 96)),
    ('water_d', (40, 72, 144)), ('water_m', (64, 112, 184)), ('water_l', (112, 160, 216)),
    ('foam', (224, 232, 232)), ('trunk_d', (72, 48, 32)), ('trunk_m', (112, 80, 48)),
    ('flower', (240, 208, 72)),
]

TERRAIN_WESTFALL = [
    ('shadow', (88, 72, 40)), ('grass_d', (136, 120, 56)), ('grass_m', (176, 160, 80)),
    ('grass_l', (200, 184, 104)), ('grass_h', (224, 208, 136)),
    ('dirt_d', (120, 88, 56)), ('dirt_m', (160, 120, 72)), ('dirt_l', (192, 152, 96)),
    ('water_d', (40, 80, 136)), ('water_m', (64, 120, 176)), ('water_l', (112, 168, 208)),
    ('foam', (232, 232, 216)), ('trunk_d', (72, 48, 32)), ('trunk_m', (112, 80, 48)),
    ('flower', (192, 80, 48)),
]

BUILDINGS = [
    ('outline', (32, 32, 48)), ('stone_d', (88, 88, 104)), ('stone_m', (128, 128, 144)),
    ('stone_l', (168, 168, 176)), ('stone_h', (208, 208, 200)),
    ('wood_d', (88, 56, 32)), ('wood_l', (144, 96, 56)), ('glass_d', (40, 48, 80)),
    ('glass_l', (240, 200, 96)), ('banner_d', (32, 48, 128)), ('banner', (56, 88, 184)),
    ('gold', (232, 184, 64)), ('cobble_d', (112, 104, 96)), ('cobble_l', (184, 176, 160)),
    ('plaster', (232, 224, 200)),
]

FARM = [
    ('soil_d', (88, 64, 40)), ('soil_m', (128, 96, 56)), ('wheat_d', (176, 136, 48)),
    ('wheat_m', (216, 176, 72)), ('wheat_l', (240, 216, 120)), ('crop', (96, 144, 56)),
    ('cloth_r', (176, 48, 48)), ('cloth_rd', (112, 32, 40)), ('canvas', (216, 200, 160)),
    ('canvas_d', (160, 144, 112)), ('iron_d', (56, 56, 64)), ('iron_l', (120, 120, 136)),
    ('fire', (232, 112, 40)), ('fire_l', (248, 216, 88)), ('camp_dirt', (144, 112, 72)),
]

OVERHEAD_LEAVES = [
    ('outline', (16, 32, 32)), ('leaf_0', (24, 56, 48)), ('leaf_1', (40, 88, 56)),
    ('leaf_2', (64, 120, 64)), ('leaf_3', (96, 152, 72)), ('leaf_4', (144, 192, 96)),
    ('roof_d', (48, 56, 96)), ('roof_m', (72, 88, 136)), ('roof_l', (104, 128, 176)),
    ('stone_d', (88, 88, 104)), ('stone_m', (128, 128, 144)), ('stone_l', (168, 168, 176)),
    ('gold', (232, 184, 64)), ('glass', (40, 48, 80)), ('roof_h', (144, 168, 208)),
]

OVERHEAD_ROOFS = [
    ('red_d', (104, 40, 40)), ('red_m', (152, 64, 56)), ('red_l', (192, 96, 80)),
    ('thatch_d', (128, 96, 48)), ('thatch_m', (176, 136, 72)), ('thatch_l', (208, 176, 104)),
    ('o2', (24, 24, 32)), ('timber', (88, 56, 32)), ('plaster', (232, 224, 200)),
    ('canvas', (216, 200, 160)), ('canvas_d', (160, 144, 112)), ('dead_0', (88, 72, 48)),
    ('dead_1', (128, 104, 64)), ('dead_2', (168, 136, 80)), ('dead_3', (200, 168, 104)),
]

# ---------------------------------------------------------------------------------------------
# Map
# ---------------------------------------------------------------------------------------------


def tile_hash(x, y, salt=0):
    h = (x * 73856093) ^ (y * 19349663) ^ (salt * 83492791)
    return (h ^ (h >> 13)) & 0xFFFF


class Map:
    def __init__(self, name, width, height, ground_palette, overhead_palette):
        self.name = name
        self.width, self.height = width, height
        self.gp, self.op = ground_palette, overhead_palette
        self.ground = np.zeros((height, width), dtype=np.uint8)
        self.overhead = np.zeros((height, width), dtype=np.uint8)
        self.solid = np.zeros((height // CELL, width // CELL), dtype=bool)
        self.metas_x, self.metas_y = width // META, height // META
        self.corners = {}
        self.warps, self.npcs, self.spawns, self.areas, self.points = [], [], [], [], {}
        self.music = 'NONE'

    # --- helpers -------------------------------------------------------------------------------

    def block(self, x, y, w, h):
        """Mark a pixel rectangle as solid (rounded out to whole cells)."""
        x0, y0 = max(0, x // CELL), max(0, y // CELL)
        x1, y1 = (x + w + CELL - 1) // CELL, (y + h + CELL - 1) // CELL
        self.solid[y0:y1, x0:x1] = True

    def unblock(self, x, y, w, h):
        x0, y0 = max(0, x // CELL), max(0, y // CELL)
        self.solid[y0:(y + h + CELL - 1) // CELL, x0:(x + w + CELL - 1) // CELL] = False

    def g(self, role):
        return self.gp[role]

    def o(self, role):
        return self.op[role]

    def stamp(self, layer, part, x, y):
        """Copy the non-transparent pixels of part onto a layer, clipped to the map."""
        h, w = part.shape
        y0, y1 = max(y, 0), min(y + h, self.height)
        x0, x1 = max(x, 0), min(x + w, self.width)
        if y0 >= y1 or x0 >= x1:
            return
        src = part[y0 - y:y1 - y, x0 - x:x1 - x]
        dst = layer[y0:y1, x0:x1]
        dst[src != 0] = src[src != 0]

    # --- metadata ------------------------------------------------------------------------------

    def warp(self, x, y, w, h, target_map, target_point):
        self.warps.append((x, y, w, h, target_map, target_point))

    def npc(self, npc_id, x, y):
        self.npcs.append((npc_id, x, y))

    def spawn(self, enemy_id, x, y):
        self.spawns.append((enemy_id, x, y))

    def spawn_group(self, enemy_id, cx, cy, count, radius, seed=0):
        local = np.random.default_rng(seed + len(self.spawns) * 31)
        placed = 0
        tries = 0
        while placed < count and tries < 400:
            tries += 1
            a = local.uniform(0, 2 * np.pi)
            r = local.uniform(0, radius)
            x, y = int(cx + np.cos(a) * r), int(cy + np.sin(a) * r)
            if self.area_free(x - 8, y - 8, 16, 12):
                self.spawn(enemy_id, x, y)
                placed += 1

    def area(self, x, y, w, h, name, area_id='NONE'):
        self.areas.append((x, y, w, h, name, area_id))

    def point(self, name, x, y):
        self.points[name] = (x, y)

    def area_free(self, x, y, w, h):
        x0, y0 = x // CELL, y // CELL
        x1, y1 = (x + w) // CELL, (y + h) // CELL
        if x0 < 0 or y0 < 0 or x1 >= self.solid.shape[1] or y1 >= self.solid.shape[0]:
            return False
        return not self.solid[y0:y1 + 1, x0:x1 + 1].any()

    # --- output --------------------------------------------------------------------------------

    def save(self, max_ground_tiles=1024):
        """Write the background images and previews. The header is written by write_header."""
        check_tile_banks(self.ground, f'{self.name}_ground')
        check_tile_banks(self.overhead, f'{self.name}_overhead')
        ground_tiles = unique_tile_count(self.ground)
        overhead_tiles = unique_tile_count(self.overhead)
        print(f'{self.name}: {self.width}x{self.height}, ground {ground_tiles} tiles, '
              f'overhead {overhead_tiles} tiles')
        if ground_tiles > max_ground_tiles or overhead_tiles > 1024:
            raise SystemExit(f'{self.name}: too many unique tiles')

        save_indexed_bmp(GRAPHICS / f'map_{self.name}_ground.bmp', self.ground, self.gp.colors,
                         pad_to_256=self.gp.banks > 1)
        (GRAPHICS / f'map_{self.name}_ground.json').write_text(
            '{\n    "type": "regular_bg",\n    "bpp_mode": "bpp_4_manual",\n'
            f'    "colors_count": {self.gp.banks * 16}\n}}\n')
        save_indexed_bmp(GRAPHICS / f'map_{self.name}_overhead.bmp', self.overhead, self.op.colors,
                         pad_to_256=self.op.banks > 1)
        (GRAPHICS / f'map_{self.name}_overhead.json').write_text(
            '{\n    "type": "regular_bg",\n    "bpp_mode": "bpp_4_manual",\n'
            f'    "colors_count": {self.op.banks * 16}\n}}\n')
        self._write_preview()

    def write_header(self, maps):
        """Write the C++ header; maps (name -> Map) resolves warp target points."""
        cols, rows = self.solid.shape[1], self.solid.shape[0]
        lines = []
        for cy in range(rows):
            values = []
            for byte in range(cols // 8):
                v = 0
                for b in range(8):
                    if self.solid[cy, byte * 8 + b]:
                        v |= 1 << b
                values.append(f'0x{v:02x}')
            lines.append('        ' + ','.join(values) + ',')
        n = self.name
        out = [
            f'// Generated by tools/gen_world.py ({n}). Do not edit by hand.',
            f'#ifndef GW_MAP_{n.upper()}_H',
            f'#define GW_MAP_{n.upper()}_H',
            '',
            '#include "gw_map_types.h"',
            '',
            f'namespace gw::map_data::{n}',
            '{',
            f'    constexpr int width = {self.width};',
            f'    constexpr int height = {self.height};',
            f'    constexpr int collision_columns = {cols};',
            f'    constexpr int collision_rows = {rows};',
            f'    constexpr music_id music = music_id::{self.music};',
            '',
            '    alignas(4) constexpr uint8_t collision[] = {',
            *lines,
            '    };',
            '',
        ]
        for name, (x, y) in sorted(self.points.items()):
            out.append(f'    constexpr point_def {name} = {{ {x}, {y} }};')
        out.append('')
        out.append('    constexpr warp_def warps[] = {')
        for x, y, w, h, target, tp in self.warps:
            tx, ty = maps[target].points[tp]
            out.append(f'        {{ {x}, {y}, {w}, {h}, map_id::{target.upper()}, {tx}, {ty} }},')
        if not self.warps:
            out.append('        { 0, 0, 0, 0, map_id::NONE, 0, 0 },')
        out.append('    };')
        out.append('')
        out.append('    constexpr npc_def npcs[] = {')
        for npc_id, x, y in self.npcs:
            out.append(f'        {{ npc_id::{npc_id}, {x}, {y} }},')
        if not self.npcs:
            out.append('        { npc_id::NONE, 0, 0 },')
        out.append('    };')
        out.append('')
        out.append('    constexpr spawn_def spawns[] = {')
        for enemy_id, x, y in self.spawns:
            out.append(f'        {{ enemy_id::{enemy_id}, {x}, {y} }},')
        if not self.spawns:
            out.append('        { enemy_id::NONE, 0, 0 },')
        out.append('    };')
        out.append('')
        out.append('    constexpr area_def areas[] = {')
        for x, y, w, h, name, area_id in self.areas:
            out.append(f'        {{ {x}, {y}, {w}, {h}, area_id::{area_id}, "{name}" }},')
        if not self.areas:
            out.append('        { 0, 0, 0, 0, area_id::NONE, "" },')
        out.append('    };')
        out += ['}', '', '#endif', '']
        (INCLUDE / f'gw_map_{n}.h').write_text('\n'.join(out))

    def _write_preview(self):
        lut_g = np.array(self.gp.colors + [(0, 0, 0)] * (256 - len(self.gp.colors)), dtype=np.uint8)
        lut_o = np.array(self.op.colors + [(0, 0, 0)] * (256 - len(self.op.colors)), dtype=np.uint8)
        rgb = lut_g[self.ground]
        over = self.overhead != 0
        rgb[over] = lut_o[self.overhead][over]
        PREVIEW.mkdir(parents=True, exist_ok=True)
        Image.fromarray(rgb, 'RGB').save(PREVIEW / f'{self.name}.png')
        solid = np.kron(self.solid, np.ones((CELL, CELL), dtype=bool))
        tinted = rgb.copy()
        tinted[solid] = (tinted[solid] * 0.55 + np.array([255, 0, 0]) * 0.45).astype(np.uint8)
        for x, y, w, h, *_ in self.warps:
            tinted[y:y + h, x:x + w] = (tinted[y:y + h, x:x + w] * 0.4 + np.array([0, 0, 255]) * 0.6).astype(np.uint8)
        for _, x, y in self.spawns:
            tinted[max(0, y - 2):y + 2, max(0, x - 2):x + 2] = (255, 0, 255)
        for _, x, y in self.npcs:
            tinted[max(0, y - 3):y + 3, max(0, x - 3):x + 3] = (0, 255, 255)
        Image.fromarray(tinted, 'RGB').save(PREVIEW / f'{self.name}_debug.png')


# ---------------------------------------------------------------------------------------------
# Grass and ground fills
# ---------------------------------------------------------------------------------------------

def grass_variants(m):
    variants = []
    for v in range(6):
        local = np.random.default_rng(100 + v)
        tile = np.full((META, META), m.g('grass_m'), dtype=np.uint8)
        noise = local.random((META, META))
        tile[noise < 0.07] = m.g('grass_d')
        tile[noise > 0.95] = m.g('grass_l')
        for _ in range(2 + v % 3):
            x, y = local.integers(1, META - 1), local.integers(2, META)
            tile[y - 1, x] = m.g('grass_h')
            tile[y, x] = m.g('grass_d')
            if x + 1 < META:
                tile[y - 1, x + 1] = m.g('grass_l')
        if v == 4:
            for _ in range(3):
                x, y = local.integers(2, META - 2), local.integers(2, META - 2)
                tile[y, x] = m.g('flower')
                tile[y + 1, x] = m.g('grass_d')
        if v == 5:
            for _ in range(3):
                x, y = local.integers(2, META - 2), local.integers(2, META - 2)
                tile[y, x] = m.g('foam')
                tile[y + 1, x] = m.g('grass_d')
        variants.append(tile)
    return variants


def fill_grass(m, x=0, y=0, w=None, h=None):
    variants = grass_variants(m)
    w = m.width if w is None else w
    h = m.height if h is None else h
    for my in range(y // META, (y + h) // META):
        for mx in range(x // META, (x + w) // META):
            r = tile_hash(mx, my) % 100
            v = 0 if r < 30 else 1 if r < 55 else 2 if r < 75 else 3 if r < 90 else 4 if r < 95 else 5
            m.ground[my * META:(my + 1) * META, mx * META:(mx + 1) * META] = variants[v]
    m.plain_grass = variants[0]


def plain_grass_under(m, x, y, w, h):
    """Repaint whole metatiles under a rectangle with the plain grass variant (saves tiles)."""
    for my in range(max(0, y // META), min(m.metas_y, (y + h + META - 1) // META)):
        for mx in range(max(0, x // META), min(m.metas_x, (x + w + META - 1) // META)):
            m.ground[my * META:(my + 1) * META, mx * META:(mx + 1) * META] = m.plain_grass


# ---------------------------------------------------------------------------------------------
# Dual-grid terrain (paths, water)
# ---------------------------------------------------------------------------------------------

EDGE_NOISE = np.random.default_rng(7).random((META, META)) * 0.24 - 0.12
SPECKLE = np.random.default_rng(8).random((META, META))
SPARKLE = np.random.default_rng(9).random((META, META))


def corners(m, kind):
    if kind not in m.corners:
        m.corners[kind] = np.zeros((m.metas_y + 1, m.metas_x + 1), dtype=float)
    return m.corners[kind]


def corners_along(m, kind, points, radius):
    """Set dual-grid corners within radius (metatiles) of a polyline given in pixels."""
    c = corners(m, kind)
    ys, xs = np.mgrid[0:c.shape[0], 0:c.shape[1]]
    pts = [(x / META, y / META) for x, y in points]
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        dx, dy = x1 - x0, y1 - y0
        length2 = max(dx * dx + dy * dy, 1e-6)
        t = np.clip(((xs - x0) * dx + (ys - y0) * dy) / length2, 0, 1)
        dist = np.hypot(xs - (x0 + t * dx), ys - (y0 + t * dy))
        c[dist <= radius] = 1


def corners_ellipse(m, kind, cx, cy, rx, ry):
    c = corners(m, kind)
    ys, xs = np.mgrid[0:c.shape[0], 0:c.shape[1]]
    c[((xs - cx / META) / (rx / META)) ** 2 + ((ys - cy / META) / (ry / META)) ** 2 <= 1] = 1


def corners_rect(m, kind, x, y, w, h, value=1):
    c = corners(m, kind)
    c[y // META:(y + h) // META + 1, x // META:(x + w) // META + 1] = value


def corner_field(c, mx, my):
    tl, tr = c[my, mx], c[my, mx + 1]
    bl, br = c[my + 1, mx], c[my + 1, mx + 1]
    u = (np.arange(META) + 0.5) / META
    uu, vv = np.meshgrid(u, u)
    return tl * (1 - uu) * (1 - vv) + tr * uu * (1 - vv) + bl * (1 - uu) * vv + br * uu * vv + EDGE_NOISE


def paint_paths(m, kind='path'):
    c = corners(m, kind)
    for my in range(m.metas_y):
        for mx in range(m.metas_x):
            if c[my:my + 2, mx:mx + 2].max() == 0:
                continue
            tile = m.ground[my * META:(my + 1) * META, mx * META:(mx + 1) * META]
            tile[:] = m.plain_grass
            if c[my:my + 2, mx:mx + 2].min() == 1:
                f = np.ones((META, META))
            else:
                f = corner_field(c, mx, my)
            inner = f > 0.62
            tile[(f > 0.42) & (f <= 0.5)] = m.g('grass_d')
            tile[(f > 0.5) & (f <= 0.62)] = m.g('dirt_d')
            tile[inner] = m.g('dirt_m')
            tile[inner & (SPECKLE > 0.85)] = m.g('dirt_l')
            tile[inner & (SPECKLE < 0.06)] = m.g('dirt_d')


def paint_water(m, kind='water'):
    c = corners(m, kind)
    for my in range(m.metas_y):
        for mx in range(m.metas_x):
            if c[my:my + 2, mx:mx + 2].max() == 0:
                continue
            tile = m.ground[my * META:(my + 1) * META, mx * META:(mx + 1) * META]
            full = c[my:my + 2, mx:mx + 2].min() == 1
            if not full:
                tile[:] = m.plain_grass
            f = np.ones((META, META)) if full else corner_field(c, mx, my)
            tile[(f > 0.36) & (f <= 0.46)] = m.g('dirt_l')
            tile[(f > 0.46) & (f <= 0.52)] = m.g('foam')
            water = f > 0.52
            tile[water] = m.g('water_m')
            if not full:
                tile[water & (f < 0.62)] = m.g('water_d')
            tile[water & (f >= 0.62) & (SPARKLE > 0.985)] = m.g('water_l')
            for cy in range(2):
                for cx in range(2):
                    if water[cy * 8:(cy + 1) * 8, cx * 8:(cx + 1) * 8].sum() >= 24:
                        m.solid[my * 2 + cy, mx * 2 + cx] = True


# ---------------------------------------------------------------------------------------------
# Trees and small props (terrain bank only)
# ---------------------------------------------------------------------------------------------

TREE_W, TREE_H, CANOPY_H = 32, 48, 28


def make_canopy(op, seed, palette=('outline', 'leaf_0', 'leaf_1', 'leaf_2', 'leaf_3', 'leaf_4')):
    local = np.random.default_rng(seed)
    canopy = np.zeros((CANOPY_H, TREE_W), dtype=np.uint8)
    clumps = [(16, 14, 13)] + [(local.uniform(8, 24), local.uniform(7, 20), local.uniform(6, 8))
                               for _ in range(7)]
    ys, xs = np.mgrid[0:CANOPY_H, 0:TREE_W]
    inside = np.zeros_like(canopy, dtype=bool)
    shade = np.zeros(canopy.shape)
    for cx, cy, r in clumps:
        d = np.hypot(xs + 0.5 - cx, (ys + 0.5 - cy) * 1.1)
        mask = d < r
        light = 1 - np.hypot(xs + 0.5 - (cx - r * 0.45), ys + 0.5 - (cy - r * 0.5)) / (r * 1.6)
        shade = np.where(mask & inside, np.maximum(shade, light), shade)
        shade[mask & ~inside] = light[mask & ~inside]
        inside |= mask
    levels = np.digitize(shade, [0.05, 0.3, 0.55, 0.8])
    leaf = [op[p] for p in palette[1:]]
    for i, value in enumerate(leaf):
        canopy[inside & (levels == i)] = value
    dither = local.random(canopy.shape) < 0.12
    for i in range(1, len(leaf)):
        canopy[inside & dither & (canopy == leaf[i])] = leaf[i - 1]
    padded = np.pad(inside, 1)
    edge = inside & ~(padded[:-2, 1:-1] & padded[2:, 1:-1] & padded[1:-1, :-2] & padded[1:-1, 2:])
    canopy[edge] = op[palette[0]]
    return canopy


def make_trunk(m):
    trunk = np.zeros((TREE_H - CANOPY_H + 4, TREE_W), dtype=np.uint8)
    for y in range(trunk.shape[0]):
        flare = 1 if y > trunk.shape[0] - 5 else 0
        x0, x1 = 12 - flare, 20 + flare
        trunk[y, x0:x1] = m.g('trunk_m')
        trunk[y, x0] = m.g('trunk_d')
        trunk[y, x1 - 1] = m.g('trunk_d')
        trunk[y, x0 + 3] = m.g('trunk_d') if y % 5 == 2 else m.g('trunk_m')
    bottom = trunk.shape[0] - 1
    trunk[bottom, 9:23] = m.g('shadow')
    trunk[bottom - 1, 10:12] = m.g('trunk_d')
    trunk[bottom - 1, 20:22] = m.g('trunk_d')
    trunk[bottom, 11:21] = m.g('trunk_d')
    return trunk


class Trees:
    def __init__(self, m, dead=False):
        palette = ('o2', 'dead_0', 'dead_1', 'dead_2', 'dead_3', 'dead_3') if dead else \
            ('outline', 'leaf_0', 'leaf_1', 'leaf_2', 'leaf_3', 'leaf_4')
        self.canopies = [make_canopy(m.op, s, palette) for s in (11, 12, 13)]
        self.trunk = make_trunk(m)


def tree(m, trees, x, y, variant=0, collide=True):
    """Tree with its top-left at (x, y), snapped to 8 px."""
    x, y = x // 8 * 8, y // 8 * 8
    plain_grass_under(m, x + 8, y + CANOPY_H - 4, 16, TREE_H - CANOPY_H + 4)
    m.stamp(m.ground, trees.trunk, x, y + CANOPY_H - 4)
    m.stamp(m.overhead, trees.canopies[variant % len(trees.canopies)], x, y)
    if collide:
        m.block(x + 8, y + 32, 16, 16)


def forest(m, trees, x, y, w, h, seed=0, solid=True, dense=True):
    """Fill a rectangle with staggered rows of trees and (optionally) make it solid."""
    step_y = 24 if dense else 40
    for row, ty in enumerate(range(y - 24, y + h - 24, step_y)):
        for tx in range(x - 16 + (row % 2) * 16, x + w, 32):
            tree(m, trees, tx, ty, row + tx // 32 + seed, collide=False)
    if solid:
        m.block(x, y, w, h)


def grove(m, trees, positions, seed=0):
    for i, (x, y) in enumerate(positions):
        tree(m, trees, x, y, i + seed)


def bush(m, x, y):
    x, y = x // 8 * 8, y // 8 * 8
    plain_grass_under(m, x, y, 16, 16)
    ys, xs = np.mgrid[0:16, 0:16]
    d = np.hypot(xs + 0.5 - 8, (ys + 0.5 - 9) * 1.2)
    inside = d < 7.5
    light = 1 - np.hypot(xs - 5, ys - 5) / 12
    tile = m.ground[y:y + 16, x:x + 16]
    tile[inside] = np.where(light[inside] > 0.55, m.g('grass_h'),
                            np.where(light[inside] > 0.25, m.g('grass_l'), m.g('grass_d')))
    padded = np.pad(inside, 1)
    edge = inside & ~(padded[:-2, 1:-1] & padded[2:, 1:-1] & padded[1:-1, :-2] & padded[1:-1, 2:])
    tile[edge] = m.g('shadow')
    tile[15, 3:13] = m.g('shadow')
    m.block(x + 2, y + 8, 12, 8)


def stump(m, x, y):
    x, y = x // 8 * 8, y // 8 * 8
    plain_grass_under(m, x, y, 16, 16)
    t = m.ground[y:y + 16, x:x + 16]
    t[6:14, 3:13] = m.g('trunk_m')
    t[6:8, 3:13] = m.g('dirt_l')
    t[6:14, 3] = m.g('trunk_d')
    t[6:14, 12] = m.g('trunk_d')
    t[14, 2:14] = m.g('shadow')
    m.block(x + 2, y + 8, 12, 6)


def fence(m, x, y, length, vertical=False):
    """Wooden fence in terrain colors. Horizontal fences are 8 px tall rows of posts and rails."""
    x, y = x // 8 * 8, y // 8 * 8
    if vertical:
        plain_grass_under(m, x, y, 8, length)
        for py in range(y, y + length):
            m.ground[py, x + 3:x + 5] = m.g('trunk_m')
            m.ground[py, x + 5] = m.g('trunk_d')
        for py in range(y, y + length, 16):
            m.ground[py:py + 3, x + 2:x + 6] = m.g('trunk_d')
        m.block(x, y, 8, length)
    else:
        plain_grass_under(m, x, y, length, 8)
        for px in range(x, x + length):
            m.ground[y + 2, px] = m.g('trunk_m')
            m.ground[y + 5, px] = m.g('trunk_m')
            m.ground[y + 6, px] = m.g('trunk_d')
        for px in range(x, x + length, 16):
            m.ground[y:y + 8, px + 2:px + 5] = m.g('trunk_m')
            m.ground[y:y + 8, px + 4] = m.g('trunk_d')
        m.block(x, y + 2, length, 6)


def bridge(m, x, y, w, h):
    """Wooden plank bridge over water (vertical planks for a horizontal crossing)."""
    for py in range(y, y + h):
        for px in range(x, x + w):
            c = m.g('trunk_m') if (px % 8) not in (0, 7) else m.g('trunk_d')
            if py in (y, y + h - 1):
                c = m.g('trunk_d')
            m.ground[py, px] = c
    m.unblock(x, y + 4, w, h - 8)
    m.block(x, y, w, 4)
    m.block(x, y + h - 4, w, 4)


# ---------------------------------------------------------------------------------------------
# Buildings and tile-aligned features (other banks)
# ---------------------------------------------------------------------------------------------

def bricks(m, layer, x, y, w, h, dark, mid, light, palette=None):
    """Brick pattern from absolute coordinates, so walls everywhere share tiles."""
    pal = palette or m.g
    ys, xs = np.mgrid[y:y + h, x:x + w]
    row = ys // 4
    offset = (row % 2) * 4
    pattern = np.full((h, w), pal(mid), dtype=np.uint8)
    pattern[(xs + offset) % 8 == 7] = pal(dark)
    pattern[(ys % 4 == 0) & ((xs + offset) % 8 < 3)] = pal(light)
    pattern[ys % 4 == 3] = pal(dark)
    layer[y:y + h, x:x + w] = pattern


def shingles(m, x, y, w, h, dark, mid, light):
    ys, xs = np.mgrid[y:y + h, x:x + w]
    band = (ys // 4) % 2
    pattern = np.full((h, w), m.o(mid), dtype=np.uint8)
    pattern[(xs + band * 4) % 8 == 0] = m.o(dark)
    pattern[ys % 4 == 0] = m.o(light)
    pattern[ys % 4 == 3] = m.o(dark)
    m.overhead[y:y + h, x:x + w] = pattern


def roof(m, x, y, w, h, colors=('roof_d', 'roof_m', 'roof_l'), ridge='roof_h', outline='outline'):
    """A roof seen from the front-top: a lower slope with eaves and a flatter upper slope."""
    lower = min(24, h // 2)
    shingles(m, x, y + h - lower, w, lower, *colors)
    upper_y, upper_h = y + 8, h - lower - 8
    if upper_h > 0:
        ys, xs = np.mgrid[upper_y:upper_y + upper_h, x + 8:x + w - 8]
        pattern = np.where(((xs // 8 + ys // 4) % 2) == 1, m.o(colors[2]), m.o(colors[1])).astype(np.uint8)
        pattern[ys % 4 == 3] = m.o(colors[0])
        m.overhead[upper_y:upper_y + upper_h, x + 8:x + w - 8] = pattern
        m.overhead[upper_y, x + 8:x + w - 8] = m.o(ridge)
        m.overhead[upper_y - 1, x + 8:x + w - 8] = m.o(outline)
        m.overhead[upper_y - 1:upper_y + upper_h, x + 7] = m.o(outline)
        m.overhead[upper_y - 1:upper_y + upper_h, x + w - 8] = m.o(outline)
        # Gable ends: fill the corners beside the upper slope with lower-slope shingles.
        shingles(m, x, upper_y + upper_h - 8 if upper_h > 8 else upper_y, 8, 8, *colors)
        shingles(m, x + w - 8, upper_y + upper_h - 8 if upper_h > 8 else upper_y, 8, 8, *colors)
    m.overhead[y + h - lower - 1, x:x + w] = m.o(ridge)
    m.overhead[y + h - 2:y + h, x:x + w] = m.o(outline)
    m.overhead[y + h - lower:y + h, x] = m.o(outline)
    m.overhead[y + h - lower:y + h, x + w - 1] = m.o(outline)


def window(m, x, y):
    g = m.ground
    g[y:y + 16, x:x + 8] = m.g('outline')
    g[y + 2:y + 14, x + 1:x + 7] = m.g('glass_d')
    g[y + 5:y + 14, x + 2:x + 6:2] = m.g('glass_l')


def door(m, x, y, h=24):
    g = m.ground
    g[y:y + h, x:x + 16] = m.g('outline')
    g[y + 2:y + h, x + 2:x + 14] = m.g('wood_d')
    for px in range(x + 3, x + 13, 4):
        g[y + 4:y + h, px:px + 2] = m.g('wood_l')
    g[y + h // 2, x + 10:x + 12] = m.g('gold')


def house(m, x, y, w=96, h=96, style='timber', roof_colors=('red_d', 'red_m', 'red_l'),
          roof_ridge='red_l', roof_outline='o2', door_x=None, windows=True):
    """A house with walls on the ground layer and a roof on the overhead layer.

    The lower half is wall, the upper half roof. Returns the door's bottom-center point.
    x, y, w, h must be multiples of 8.
    """
    wall_h = h // 2
    wall_y = y + h - wall_h
    g = m.ground
    if style == 'stone':
        bricks(m, g, x, wall_y, w, wall_h, 'stone_d', 'stone_m', 'stone_l')
    else:
        g[wall_y:wall_y + wall_h, x:x + w] = m.g('plaster')
        for px in range(x, x + w, 24):
            g[wall_y:wall_y + wall_h, px:px + 3] = m.g('wood_d')
        g[wall_y:wall_y + 3, x:x + w] = m.g('wood_d')
        g[wall_y + wall_h - 6:wall_y + wall_h, x:x + w] = m.g('stone_m')
        g[wall_y + wall_h - 6, x:x + w] = m.g('stone_l')
    g[wall_y + wall_h - 1, x:x + w] = m.g('stone_d')
    g[wall_y:wall_y + wall_h, x] = m.g('outline')
    g[wall_y:wall_y + wall_h, x + w - 1] = m.g('outline')
    dx = x + (w // 2 - 8 if door_x is None else door_x)
    dx = dx // 8 * 8
    door(m, dx, wall_y + wall_h - 24, 24)
    if windows:
        for wx in range(x + 8, x + w - 8, 32):
            if abs(wx - dx) >= 16 and abs(wx + 8 - dx) >= 16:
                window(m, wx, wall_y + 8)
    roof(m, x, y, w, h - wall_h, roof_colors, roof_ridge, roof_outline)
    m.block(x, y + 16, w, h - 16)
    return (dx + 8, y + h + 6)


def cobbles(m, x, y, w, h):
    ys, xs = np.mgrid[y:y + h, x:x + w]
    stone = ((xs // 8) + (ys // 8)) % 2
    pattern = np.where(stone == 1, m.g('cobble_l'), m.g('stone_l')).astype(np.uint8)
    pattern[(xs % 8 == 7) | (ys % 8 == 7)] = m.g('cobble_d')
    m.ground[y:y + h, x:x + w] = pattern


def wheat_field(m, x, y, w, h, solid=False):
    ys, xs = np.mgrid[y:y + h, x:x + w]
    pattern = np.full((h, w), m.g('soil_m'), dtype=np.uint8)
    row = ys % 8
    pattern[row == 7] = m.g('soil_d')
    stalk = (xs % 4 == 1) & (row < 6)
    pattern[stalk] = m.g('wheat_m')
    pattern[stalk & (row < 2)] = m.g('wheat_l')
    pattern[(xs % 4 == 2) & (row > 2) & (row < 6)] = m.g('wheat_d')
    m.ground[y:y + h, x:x + w] = pattern
    if solid:
        m.block(x, y, w, h)


def crop_field(m, x, y, w, h):
    ys, xs = np.mgrid[y:y + h, x:x + w]
    pattern = np.full((h, w), m.g('soil_m'), dtype=np.uint8)
    pattern[ys % 8 >= 6] = m.g('soil_d')
    leaf = (ys % 8 < 4) & (xs % 8 >= 2) & (xs % 8 < 6)
    pattern[leaf] = m.g('crop')
    m.ground[y:y + h, x:x + w] = pattern


def camp(m, x, y, w, h, tents, fire=None):
    """Bandit camp: a dirt clearing (farm bank) with tents and a campfire. Snapped to tiles."""
    ys, xs = np.mgrid[y:y + h, x:x + w]
    pattern = np.full((h, w), m.g('camp_dirt'), dtype=np.uint8)
    noise = (tile_hash(0, 0) + xs * 7 + ys * 13) % 11 == 0
    pattern[noise] = m.g('soil_m')
    m.ground[y:y + h, x:x + w] = pattern
    for tx, ty in tents:
        tent(m, tx, ty)
    if fire:
        fx, fy = fire
        g = m.ground
        g[fy + 10:fy + 14, fx + 2:fx + 14] = m.g('soil_d')
        g[fy + 4:fy + 12, fx + 5:fx + 11] = m.g('fire')
        g[fy + 6:fy + 11, fx + 6:fx + 10] = m.g('fire_l')
        g[fy + 2:fy + 5, fx + 7:fx + 9] = m.g('fire')
        m.block(fx + 2, fy + 6, 12, 8)


def tent(m, x, y):
    """32x32 tent on camp ground."""
    g = m.ground
    for py in range(32):
        half = min(16, 2 + py * 15 // 24)
        x0, x1 = x + 16 - half, x + 16 + half
        if py < 26:
            g[y + py, x0:x1] = m.g('cloth_r')
            g[y + py, x0:x0 + 1] = m.g('cloth_rd')
            g[y + py, x1 - 1:x1] = m.g('cloth_rd')
            g[y + py, x + 16:x1] = np.where(np.arange(x + 16, x1) % 2 == 0, m.g('cloth_rd'), m.g('cloth_r'))
        else:
            g[y + py, x + 2:x + 30] = m.g('soil_d')
    g[y + 12:y + 26, x + 13:x + 19] = m.g('iron_d')
    m.block(x + 2, y + 8, 28, 22)


def crates(m, x, y):
    """Stack of crates on camp or dirt ground (farm bank)."""
    g = m.ground
    for cx, cy in ((x, y + 8), (x + 16, y + 8), (x + 8, y)):
        g[cy:cy + 16, cx:cx + 16] = m.g('canvas_d')
        g[cy + 1:cy + 15, cx + 1:cx + 15] = m.g('canvas')
        g[cy + 7:cy + 9, cx + 1:cx + 15] = m.g('canvas_d')
        g[cy, cx:cx + 16] = m.g('soil_d')
    m.block(x, y + 4, 32, 20)


def well(m, x, y):
    """Stone well, 32x32 on a cobble square (building bank)."""
    g = m.ground
    cobbles(m, x, y, 32, 32)
    g[y + 6:y + 30, x + 4:x + 28] = m.g('stone_d')
    g[y + 8:y + 28, x + 6:x + 26] = m.g('stone_m')
    g[y + 12:y + 24, x + 10:x + 22] = m.g('banner_d')
    g[y + 14:y + 22, x + 12:x + 20] = m.g('glass_d')
    g[y + 2:y + 8, x + 4:x + 28] = m.g('wood_d')
    g[y + 2:y + 14, x + 4:x + 6] = m.g('wood_l')
    g[y + 2:y + 14, x + 26:x + 28] = m.g('wood_l')
    m.block(x + 4, y + 8, 24, 22)


def anvil(m, x, y):
    """Smithy anvil on cobbles, 16x16."""
    g = m.ground
    g[y + 6:y + 10, x + 1:x + 15] = m.g('stone_d')
    g[y + 4:y + 7, x + 3:x + 15] = m.g('stone_h')
    g[y + 10:y + 15, x + 5:x + 11] = m.g('stone_d')
    g[y + 15, x + 3:x + 13] = m.g('outline')
    m.block(x + 1, y + 6, 14, 10)


def mine_entrance(m, x, y, w=64, h=48):
    """Rock face with a timber-framed dark opening (building bank). Returns the opening's center."""
    g = m.ground
    bricks(m, g, x, y, w, h, 'stone_d', 'stone_m', 'stone_l')
    g[y:y + 3, x:x + w] = m.g('stone_h')
    ox, ow = x + w // 2 - 16, 32
    g[y + h - 32:y + h, ox:ox + ow] = m.g('outline')
    g[y + h - 34:y + h - 30, ox - 4:ox + ow + 4] = m.g('wood_l')
    g[y + h - 34:y + h, ox - 4:ox] = m.g('wood_d')
    g[y + h - 34:y + h, ox + ow:ox + ow + 4] = m.g('wood_d')
    m.block(x, y, w, h)
    m.unblock(ox, y + h - 24, ow, 24)
    return (ox + ow // 2, y + h - 8)


def tower(m, x, y, w=64, h=128):
    """Sentinel Hill style watchtower: stone walls (ground) and a slate roof (overhead)."""
    g = m.ground
    top = (h // 3) // 8 * 8
    wall_y = y + top
    bricks(m, g, x, wall_y, w, h - top, 'stone_d', 'stone_m', 'stone_l')
    g[wall_y:y + h, x] = m.g('outline')
    g[wall_y:y + h, x + w - 1] = m.g('outline')
    window(m, x + w // 2 - 4, wall_y + 16)
    door(m, x + w // 2 - 8, y + h - 24)
    # crenellated top as part of the overhead roof
    roof(m, x - 8, y, w + 16, top + 8)
    m.block(x, y + 24, w, h - 24)
    return (x + w // 2, y + h + 6)


def abbey(m, x, y):
    """Northshire Abbey, 160x128. Returns the door's bottom-center point."""
    W, H, ROOF = 160, 128, 64
    g, o = m.ground, m.overhead
    wall_y = y + ROOF
    wall_h = H - ROOF
    bricks(m, g, x, wall_y, W, wall_h, 'stone_d', 'stone_m', 'stone_l')
    g[wall_y:wall_y + 2, x:x + W] = m.g('stone_h')
    g[wall_y + wall_h - 2:wall_y + wall_h, x:x + W] = m.g('stone_d')
    g[wall_y:wall_y + wall_h, x] = m.g('outline')
    g[wall_y:wall_y + wall_h, x + W - 1] = m.g('outline')
    for bx in (x + 24, x + 56, x + 96, x + 128):
        g[wall_y:wall_y + wall_h, bx:bx + 8] = m.g('stone_l')
        g[wall_y:wall_y + wall_h, bx + 7] = m.g('stone_d')
        g[wall_y:wall_y + wall_h, bx] = m.g('stone_h')
    for wx in (x + 8, x + 40, x + 104, x + 136):
        g[wall_y + 16:wall_y + 40, wx:wx + 16] = m.g('outline')
        g[wall_y + 18:wall_y + 38, wx + 2:wx + 14] = m.g('glass_d')
        g[wall_y + 22:wall_y + 38, wx + 3:wx + 13:2] = m.g('glass_l')
    dx = x + W // 2 - 12
    g[wall_y + 24:wall_y + wall_h, dx:dx + 24] = m.g('outline')
    g[wall_y + 26:wall_y + wall_h, dx + 2:dx + 22] = m.g('wood_d')
    for px in range(dx + 3, dx + 21, 4):
        g[wall_y + 28:wall_y + wall_h, px:px + 2] = m.g('wood_l')
    g[wall_y + 44, dx + 9:dx + 15] = m.g('gold')
    g[wall_y + 22:wall_y + 26, dx + 2:dx + 22] = m.g('stone_h')
    for bx in (dx - 16, dx + 32):
        g[wall_y + 12:wall_y + 44, bx:bx + 8] = m.g('banner')
        g[wall_y + 12:wall_y + 44, bx] = m.g('banner_d')
        g[wall_y + 12:wall_y + 14, bx:bx + 8] = m.g('gold')
        g[wall_y + 24:wall_y + 30, bx + 2:bx + 6] = m.g('gold')
    roof(m, x, y + 8, W, ROOF - 8)
    tx = x + W // 2 - 16
    ys, xs = np.mgrid[y:y + 40, tx:tx + 32]
    pattern = np.full((40, 32), m.o('stone_m'), dtype=np.uint8)
    pattern[(ys % 4 == 3) | ((xs + (ys // 4 % 2) * 4) % 8 == 7)] = m.o('stone_d')
    pattern[(ys % 4 == 0) & ((xs + (ys // 4 % 2) * 4) % 8 < 7)] = m.o('stone_l')
    o[y:y + 40, tx:tx + 32] = pattern
    o[y:y + 40, tx] = m.o('outline')
    o[y:y + 40, tx + 31] = m.o('outline')
    o[y, tx:tx + 32] = m.o('outline')
    o[y + 1:y + 6, tx + 1:tx + 31] = m.o('roof_d')
    o[y + 12:y + 28, tx + 8:tx + 24] = m.o('outline')
    o[y + 14:y + 28, tx + 10:tx + 22] = m.o('glass')
    o[y + 20:y + 25, tx + 13:tx + 19] = m.o('gold')
    m.block(x, y + 32, W, H - 32)
    return (x + W // 2, y + H + 6)
