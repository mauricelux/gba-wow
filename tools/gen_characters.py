"""Generate the character sprite sheets and their palettes.

Humanoid sheets are 32x32 frames, 17 per sheet, stacked vertically:
  0-3   walk down (frame 0 doubles as idle)
  4-7   walk up
  8-11  walk left (the game mirrors these for right)
  12    attack down, 13 attack up, 14 attack left
  15    cast (facing down, arms raised)
  16    dead

Creature sheets are side views only, 6 frames: 0-3 walk left (0 = idle), 4 attack, 5 dead.

Every humanoid sheet uses the same palette layout (HUMANOID_ROLES), and so does every creature sheet
(CREATURE_ROLES). A look is a sheet plus a palette, so guards, bandits and the player share art and
differ only in colors. Palettes are written to include/gw_palettes.h.

This is placeholder art until real sprites replace it; it only has to read clearly at 240x160.
"""

import numpy as np

from art_common import GRAPHICS, INCLUDE, PREVIEW, gba_color, save_indexed_bmp, save_preview_png

FRAME = 32
FEET_Y = 29

# --- humanoids --------------------------------------------------------------------------------------

HUMANOID_ROLES = ['transparent', 'outline', 'skin_dark', 'skin', 'hair_dark', 'hair', 'armor_dark',
                  'armor', 'armor_light', 'tabard_dark', 'tabard', 'trim', 'leather_dark', 'leather',
                  'trim_dark', 'lower_face']

HKEYS = {'.': 0, 'o': 1, 's': 2, 'S': 3, 'h': 4, 'H': 5, 'a': 6, 'A': 7, 'L': 8,
         'b': 9, 'B': 10, 'g': 11, 'l': 12, 'e': 13, 'd': 14, 'm': 15}


def mirror(rows):
    return [r + r[::-1] for r in rows]


FRONT = [
    '......oo',
    '....ooHH',
    '...oHHHH',
    '..oHHHHH',
    '..ohHHHh',
    '..ohSSSS',
    '..ohSoSS',
    '...osmmm',
    '....osmm',
    '.ooAAoos',
    'oALLAobB',
    'oAAAaobB',
    '.oAAobBB',
    '.oAaobBg',
    '.oaaobBB',
    '.oSsollg',
    '..oobBBB',
    '...obBbB',
]

BACK = [
    '......oo',
    '....ooHH',
    '...oHHHH',
    '..oHHHHH',
    '..oHHHHH',
    '..ohHHHH',
    '..ohhHHH',
    '...ohhhH',
    '....oohh',
    '.ooAAooo',
    'oALLAogg',
    'oAAAagBB',
    '.oAAgBgB',
    '.oAagBBg',
    '.oaagBBB',
    '.oSsogBB',
    '..ooodgB',
    '...obodd',
]

SIDE = [
    '.....oooo.......',
    '....oHHHHo......',
    '...oHHHHHHo.....',
    '..oHHHHHHHHo....',
    '..oSHHHHHhHo....',
    '.oSSSHHhhhHo....',
    '.oSoSSShhhHo....',
    '.ommmmsShhho....',
    '..ommmsooo......',
    '...ooAAooogo....',
    '..oALLAAobBgo...',
    '..oALLAAobBgo...',
    '..oAAAAAobBgo...',
    '..oaAAaaobBgo...',
    '...oaaaobBdgo...',
    '...oSsollldo....',
    '...obBBBBoo.....',
    '...obBbBbo......',
]

# Row patches per head style. Front and back patches are left halves (mirrored), side rows are full.
HEAD_PATCHES = {
    'short': {},
    'long': {
        'front': {7: '..ohsmmm', 8: '..ohosmm', 9: '.ohhAoos', 10: 'oAhhAobB'},
        'back': {7: '..ohhhhH', 8: '..ohhhhh', 9: '.ooAohhh', 10: 'oALLAohh', 11: 'oAAAaohh',
                 12: '.oAAgBoh'},
        'side': {7: '.ommmmsShhhho...', 8: '..ommmsohhho....', 9: '...ooAAohhho....',
                 10: '..oALLAAohhgo...'},
    },
    'elf': {
        'front': {3: 'o.oHHHHH', 4: 'oSohHHHh', 5: '.oShSSSS', 6: '..ohSoSS', 7: '..ohsmmm',
                  8: '..ohosmm', 9: '.ohhAoos', 10: 'oAhhAobB'},
        'back': {3: 'o.oHHHHH', 4: 'oSoHHHHH', 5: '.oShHHHH', 7: '..ohhhhH', 8: '..ohhhhh',
                 9: '.ooAohhh', 10: 'oALLAohh', 11: 'oAAAaohh', 12: '.oAAgBoh'},
        'side': {3: '..oHHHHHHHHo.o..', 4: '..oSHHHHHhHoSo..', 5: '.oSSSHHhhhHSo...',
                 7: '.ommmmsShhhho...', 8: '..ommmsohhho....', 9: '...ooAAohhho....',
                 10: '..oALLAAohhgo...'},
    },
    'dwarf': {
        'front': {7: '...ommmm', 8: '...ommmm', 9: '.ooAommm', 10: 'oALLAomm', 11: 'oAAAaobm'},
        'side': {8: '.ommmmsooo......', 9: '..ommmAooogo....', 10: '..ommLAAobBgo...',
                 11: '..oomAAAobBgo...'},
    },
}

LEGS_ARMOR = [
    '...oaAAo',
    '...oaAAo',
    '...oaAAo',
    '...oallo',
    '...oeelo',
    '..oeeelo',
    '..ollllo',
    '...oooo.',
]

LEGS_ROBE = [
    '...obBBB',
    '...obBBB',
    '..obBBBB',
    '..obBBBB',
    '..obBBBB',
    '.obBBBBB',
    '.obbbbbb',
    '..oooooo',
]

ROBE_STEP_FOOT = ['.obbbbbb', '..oelooo']

SIDE_LEGS_ARMOR = [
    ['....oaAAAo......',
     '....oaAAAo......',
     '....oaAAAo......',
     '....oallao......',
     '...oeeello......',
     '..oeeeeelo......',
     '..ollllllo......',
     '...oooooo.......'],
    ['....oaAAAo......',
     '...oaAoaAAo.....',
     '...oaAooaAAo....',
     '..oallo.oallo...',
     '..oeelo..oeelo..',
     '.oeeeeo..oeeeo..',
     '.ollllo..olllo..',
     '..oooo....ooo...'],
    ['....oaAAAo......',
     '...oaAAoaAo.....',
     '...oaAAooaAo....',
     '..oallo.oallo...',
     '..oeelo..oeelo..',
     '.oeeeeo..oeeeo..',
     '.ollllo..olllo..',
     '..oooo....ooo...'],
]

SIDE_LEGS_ROBE = [
    ['....obBBBo......',
     '....obBBBo......',
     '...obBBBBo......',
     '...obBBBBBo.....',
     '..obBBBBBBo.....',
     '..obBBBBBBo.....',
     '..obbbbbbbo.....',
     '...ooooooo......'],
    ['....obBBBo......',
     '....obBBBo......',
     '...obBBBBo......',
     '..obBBBBBBo.....',
     '.obBBBBBBBo.....',
     '.obBBBBBBBBo....',
     '.obbbbbbbbbo....',
     '.oleoooooleo....'],
]

# Weapons are fill-only stamps; outline() adds the dark border. The anchor is the grip.
SWORD = ['.g.',
         '.e.',
         'gdg',
         '.L.',
         '.L.',
         '.L.',
         '.L.',
         '.L.',
         '.A.']
SWORD_GRIP = (1, 1)

STAFF = ['.g.',
         'gLg',
         '.d.'] + ['.e.'] * 19 + ['.l.']
STAFF_GRIP = (1, 9)


def to_array(rows, keys):
    return np.array([[keys[c] for c in row] for row in rows], dtype=np.uint8)


def blit(frame, part, x, y):
    """Copy the non-transparent pixels of part with its top-left at (x, y), clipped to the frame."""
    h, w = part.shape
    for py in range(h):
        fy = y + py
        if not 0 <= fy < frame.shape[0]:
            continue
        for px in range(w):
            fx = x + px
            if 0 <= fx < frame.shape[1] and part[py, px]:
                frame[fy, fx] = part[py, px]


def neighbors(mask):
    out = np.zeros_like(mask)
    out[1:, :] |= mask[:-1, :]
    out[:-1, :] |= mask[1:, :]
    out[:, 1:] |= mask[:, :-1]
    out[:, :-1] |= mask[:, 1:]
    return out


def outline(img, outline_index=1):
    """Give every colored region a dark border on the transparent pixels around it."""
    colored = (img != 0) & (img != outline_index)
    ring = neighbors(colored) & (img == 0)
    img[ring] = outline_index
    return img


def stamp(rows, keys=HKEYS):
    """A weapon stamp with a one-pixel outline added around it."""
    part = to_array(rows, keys)
    padded = np.zeros((part.shape[0] + 2, part.shape[1] + 2), dtype=np.uint8)
    padded[1:-1, 1:-1] = part
    return outline(padded)


def bow_stamp(height=15, flip=False):
    """A bow seen from the side: a leather arc with a straight string."""
    w = 4
    part = np.zeros((height, w), dtype=np.uint8)
    mid = (height - 1) / 2
    for y in range(height):
        t = (y - mid) / mid
        x = int(round((1 - t * t) * (w - 1)))
        part[y, x] = HKEYS['e']
        if 0 < y < height - 1:
            part[y, 0] = part[y, 0] or HKEYS['L']
    if flip:
        part = part[:, ::-1]
    padded = np.zeros((height + 2, w + 2), dtype=np.uint8)
    padded[1:-1, 1:-1] = part
    return outline(padded)


def place(frame, part, grip, x, y):
    """Blit a stamp so its grip (in unpadded stamp coordinates) lands on (x, y)."""
    blit(frame, part, x - grip[0] - 1, y - grip[1] - 1)


def patched(rows, patches):
    rows = list(rows)
    for index, row in patches.items():
        rows[index] = row
    return rows


def leg_rows(rows, extra):
    """Shorten (negative) or lengthen (positive) legs by removing or repeating the thigh row."""
    if extra < 0:
        return rows[-extra:]
    return [rows[0]] * extra + rows


class Humanoid:
    def __init__(self, head, body, weapon):
        self.head = head
        self.body = body
        self.weapon = weapon
        self.leg_extra = {'dwarf': -3, 'elf': 1}.get(head, 0)
        patches = HEAD_PATCHES[head]
        self.front = to_array(mirror(patched(FRONT, patches.get('front', {}))), HKEYS)
        self.back = to_array(mirror(patched(BACK, patches.get('back', {}))), HKEYS)
        self.side = to_array(patched(SIDE, patches.get('side', {})), HKEYS)
        self.ox = 8

    # Vertical layout: legs end on FEET_Y, the 18-row top sits right above them.
    def _leg_top(self, legs_height):
        return FEET_Y - legs_height + 1

    def _front_legs(self, step, back_view=False):
        if self.body == 'robe':
            rows = leg_rows(LEGS_ROBE, self.leg_extra)
            left = list(rows)
            right = list(rows)
            if step == 1:
                right[-2:] = ROBE_STEP_FOOT
            elif step == 3:
                left[-2:] = ROBE_STEP_FOOT
            legs = to_array([l + r[::-1] for l, r in zip(left, right)], HKEYS)
            return legs, 0
        rows = leg_rows(LEGS_ARMOR, self.leg_extra)
        legs = to_array(mirror(rows), HKEYS)
        lift = {1: (0, 2), 3: (2, 0)}.get(step, (0, 0))
        out = np.zeros((legs.shape[0] + 2, legs.shape[1]), dtype=np.uint8)
        for cols, amount in ((slice(0, 8), lift[0]), (slice(8, 16), lift[1])):
            out[2 - amount:2 - amount + legs.shape[0], cols] = legs[:, cols]
        return out, 2

    def _side_legs(self, step):
        if self.body == 'robe':
            options = SIDE_LEGS_ROBE
            rows = options[1] if step in (1, 3) else options[0]
            if step == 3:
                rows = [r[1:] + '.' for r in rows]
        else:
            rows = SIDE_LEGS_ARMOR[{1: 1, 3: 2}.get(step, 0)]
        return to_array(leg_rows(rows, self.leg_extra), HKEYS)

    def frame(self, view, step=0, pose='walk'):
        frame = np.zeros((FRAME, FRAME), dtype=np.uint8)
        bob = 1 if step in (1, 3) else 0
        lunge = 0

        if view == 'side':
            legs = self._side_legs(step)
            top_y = self._leg_top(legs.shape[0]) - 18
            if pose == 'attack':
                lunge = -2
            blit(frame, legs, self.ox, self._leg_top(legs.shape[0]))
            top = self.side
        else:
            legs, pad = self._front_legs(step, view == 'back')
            leg_y = self._leg_top(legs.shape[0] - pad) - pad
            top_y = leg_y + pad - 18
            blit(frame, legs, self.ox, leg_y)
            top = self.front if view == 'front' else self.back

        hand_y = top_y + bob + 15
        if view == 'front':
            hand_x = self.ox + 2
            off_hand_x = self.ox + 13
        elif view == 'back':
            hand_x = self.ox + 13
            off_hand_x = self.ox + 2
        else:
            hand_x = self.ox + 4 + lunge
            off_hand_x = hand_x

        # Weapons held at the side are drawn behind the arm, so the hand covers the grip.
        if pose == 'walk' and view != 'side':
            self._held_weapon(frame, view, hand_x, off_hand_x, hand_y)

        top_part = top.copy()
        if pose == 'cast':
            top_part = self._raise_arms(top_part)
        blit(frame, top_part, self.ox + lunge, top_y + bob)

        if pose == 'walk' and view == 'side':
            self._held_weapon(frame, view, hand_x, off_hand_x, hand_y)
        elif pose == 'attack':
            self._attack_weapon(frame, view, hand_x, hand_y)
        elif pose == 'cast':
            self._cast_weapon(frame, top_y)
        return outline(frame)

    def _held_weapon(self, frame, view, hand_x, off_hand_x, hand_y):
        if self.weapon == 'sword':
            place(frame, stamp(SWORD), SWORD_GRIP, hand_x, hand_y)
        elif self.weapon == 'staff':
            place(frame, stamp(STAFF), STAFF_GRIP, hand_x, hand_y)
        elif self.weapon == 'bow':
            bow = bow_stamp(15, flip=(view == 'front'))
            x = off_hand_x - (1 if view == 'front' else 3)
            if view == 'side':
                x = hand_x - 4
            blit(frame, bow, x, hand_y - 9)

    def _attack_weapon(self, frame, view, hand_x, hand_y):
        if self.weapon == 'bow':
            bow = bow_stamp(15)
            if view == 'side':
                blit(frame, bow, hand_x - 6, hand_y - 11)
            elif view == 'front':
                blit(frame, bow_stamp(15, flip=True), 13, hand_y - 12)
            else:
                blit(frame, bow, 13, hand_y - 16)
            return
        rows = SWORD if self.weapon != 'staff' else STAFF[:12]
        grip = SWORD_GRIP if self.weapon != 'staff' else STAFF_GRIP
        if self.weapon == 'none':
            return
        if view == 'side':
            part = np.rot90(stamp(rows), k=-1)          # blade points left
            blit(frame, part, hand_x - part.shape[1] + grip[1] + 2, hand_y - grip[0] - 1)
        elif view == 'front':
            part = np.rot90(stamp(rows), k=1)           # blade points right, across the legs
            blit(frame, part, hand_x - grip[1] - 1, hand_y + 2 - grip[0] - 1)
        else:
            part = stamp(rows)[::-1, :]                 # blade raised above the head
            blit(frame, part, hand_x - grip[0] - 1, hand_y - part.shape[0] + grip[1] + 2)

    def _raise_arms(self, top):
        top = top.copy()
        # Clear the lowered arms (columns 0-2 and 13-15 below the shoulders) and draw raised ones.
        top[11:17, 0:3] = 0
        top[11:17, 13:16] = 0
        for x0 in (0, 13):
            top[3:11, x0:x0 + 3] = np.where(top[3:11, x0:x0 + 3] == 0, 0, top[3:11, x0:x0 + 3])
            top[5:11, x0 + 1] = HKEYS['A']
            top[5:11, x0 + 2 if x0 == 0 else x0] = HKEYS['a']
            top[3:5, x0 + 1] = HKEYS['S']
        return top

    def _cast_weapon(self, frame, top_y):
        if self.weapon == 'staff':
            place(frame, stamp(STAFF), STAFF_GRIP, self.ox + 1, top_y + 5)
        elif self.weapon == 'sword':
            part = stamp(SWORD)[::-1, :]
            blit(frame, part, self.ox - 1, top_y - 6)

    def sheet(self):
        frames = []
        for view in ('front', 'back', 'side'):
            frames += [self.frame(view, step) for step in range(4)]
        frames.append(self.frame('front', 0, 'attack'))
        frames.append(self.frame('back', 0, 'attack'))
        frames.append(self.frame('side', 0, 'attack'))
        frames.append(self.frame('front', 0, 'cast'))
        frames.append(dead_frame(self.frame('front', 0)))
        return frames


def dead_frame(stand):
    """The idle frame lying on its side, resting on the feet line."""
    lying = np.rot90(stand, k=-1)
    rows = np.where(lying.any(axis=1))[0]
    cols = np.where(lying.any(axis=0))[0]
    part = lying[rows[0]:rows[-1] + 1, cols[0]:cols[-1] + 1]
    frame = np.zeros((FRAME, FRAME), dtype=np.uint8)
    x = (FRAME - part.shape[1]) // 2
    y = FEET_Y + 1 - part.shape[0]
    blit(frame, part, x, y)
    return frame


HUMANOID_SHEETS = {
    'hum_sword': Humanoid('short', 'armor', 'sword'),
    'hum_staff': Humanoid('short', 'robe', 'staff'),
    'hum_robe': Humanoid('short', 'robe', 'none'),
    'hum_plain': Humanoid('short', 'armor', 'none'),
    'fem_robe': Humanoid('long', 'robe', 'none'),
    'fem_sword': Humanoid('long', 'armor', 'sword'),
    'dwarf_sword': Humanoid('dwarf', 'armor', 'sword'),
    'dwarf_bow': Humanoid('dwarf', 'armor', 'bow'),
    'elf_sword': Humanoid('elf', 'armor', 'sword'),
    'elf_bow': Humanoid('elf', 'armor', 'bow'),
}

# --- creatures --------------------------------------------------------------------------------------

CREATURE_ROLES = ['transparent', 'outline', 'dark', 'main', 'light', 'second_dark', 'second', 'eye',
                  'tooth_dark', 'tooth', 'extra', 'extra_dark', 'flame', 'flame_dark', 'weapon_dark',
                  'weapon']
C = {name: index for index, name in enumerate(CREATURE_ROLES)}


class Canvas:
    """Draws creatures out of simple shaded shapes. Later parts are drawn in front."""

    def __init__(self):
        self.img = np.zeros((FRAME, FRAME), dtype=np.uint8)
        ys, xs = np.mgrid[0:FRAME, 0:FRAME]
        self.xs = xs
        self.ys = ys

    def part(self, mask, main, dark=None, light=None, edge=True):
        if edge:
            ring = neighbors(mask) & ~mask
            self.img[ring] = C['outline']
        self.img[mask] = C[main]
        if light:
            top = mask & ~np.roll(mask, 1, axis=0)
            self.img[top] = C[light]
        if dark:
            below1 = ~np.roll(mask, -1, axis=0)
            below2 = ~np.roll(mask, -2, axis=0)
            self.img[mask & (below1 | below2)] = C[dark]

    def ellipse_mask(self, cx, cy, rx, ry):
        return ((self.xs - cx) / max(rx, 0.5)) ** 2 + ((self.ys - cy) / max(ry, 0.5)) ** 2 <= 1.0

    def rect_mask(self, x0, y0, x1, y1):
        return (self.xs >= x0) & (self.xs <= x1) & (self.ys >= y0) & (self.ys <= y1)

    def line_mask(self, x0, y0, x1, y1, width=1):
        mask = np.zeros((FRAME, FRAME), dtype=bool)
        steps = max(abs(x1 - x0), abs(y1 - y0), 1)
        r = (width - 1) / 2
        for i in range(steps + 1):
            x = x0 + (x1 - x0) * i / steps
            y = y0 + (y1 - y0) * i / steps
            mask |= (np.abs(self.xs - x) <= r + 0.5) & (np.abs(self.ys - y) <= r + 0.5)
        return mask

    def poly_mask(self, points):
        """Filled polygon (even-odd rule) sampled at pixel centers."""
        mask = np.zeros((FRAME, FRAME), dtype=bool)
        px = self.xs + 0.5
        py = self.ys + 0.5
        n = len(points)
        for i in range(n):
            x0, y0 = points[i]
            x1, y1 = points[(i + 1) % n]
            cond = ((y0 > py) != (y1 > py))
            with np.errstate(divide='ignore', invalid='ignore'):
                xint = (x1 - x0) * (py - y0) / (y1 - y0) + x0
            mask ^= cond & (px < xint)
        return mask

    def ellipse(self, cx, cy, rx, ry, main, dark=None, light=None, edge=True):
        self.part(self.ellipse_mask(cx, cy, rx, ry), main, dark, light, edge)

    def rect(self, x0, y0, x1, y1, main, dark=None, light=None, edge=True):
        self.part(self.rect_mask(x0, y0, x1, y1), main, dark, light, edge)

    def line(self, x0, y0, x1, y1, main, width=1, dark=None, light=None, edge=True):
        self.part(self.line_mask(x0, y0, x1, y1, width), main, dark, light, edge)

    def poly(self, points, main, dark=None, light=None, edge=True):
        self.part(self.poly_mask(points), main, dark, light, edge)

    def px(self, x, y, key):
        if 0 <= x < FRAME and 0 <= y < FRAME:
            self.img[y, x] = C[key]

    def done(self, dx=0, dy=0):
        img = np.roll(np.roll(self.img, dx, axis=1), dy, axis=0)
        return outline(img)


STRIDE = [0, 2, 0, -2]


def wolf(step, pose):
    c = Canvas()
    s = STRIDE[step]
    bob = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    c.line(23, 16 + bob, 29, 11 + bob, 'main', width=2, dark='dark')                # tail
    c.rect(19 - s, 21, 20 - s, 27, 'dark', edge=True)                               # far legs
    c.rect(9 + s, 21, 10 + s, 27, 'dark', edge=True)
    c.ellipse(16, 18 + bob, 9, 5, 'main', dark='second', light='light')             # body
    c.rect(21 + s, 21, 22 + s, 27, 'main', dark='dark')                             # near legs
    c.rect(11 - s, 21, 12 - s, 27, 'main', dark='dark')
    hx = 6 - (2 if attack else 0)
    hy = 14 + bob + (1 if attack else 0)
    c.poly([(hx + 1, hy - 3), (hx + 2, hy - 7), (hx + 4, hy - 3)], 'dark')           # ear
    c.ellipse(hx + 1, hy, 4, 4, 'main', light='light', dark='dark')                 # head
    c.rect(hx - 5, hy, hx, hy + 2, 'second', dark='second_dark')                     # snout
    c.px(hx - 5, hy, 'outline')
    c.px(hx - 1, hy - 2, 'eye')
    if attack:
        c.rect(hx - 5, hy + 3, hx - 1, hy + 3, 'tooth')
        c.px(hx - 5, hy + 4, 'outline')
    return c.done()


def boar(step, pose):
    c = Canvas()
    s = STRIDE[step] // 2
    bob = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    c.rect(22 - s, 23, 23 - s, 28, 'dark')
    c.rect(10 + s, 23, 11 + s, 28, 'dark')
    c.ellipse(17, 19 + bob, 10, 6, 'main', dark='dark', light='light')
    for x in range(10, 26, 2):                                                      # bristles
        c.px(x, 12 + bob, 'second_dark')
        c.px(x + 1, 13 + bob, 'second_dark')
    c.px(27, 17 + bob, 'dark')
    c.px(28, 16 + bob, 'dark')
    c.rect(24 + s, 23, 25 + s, 28, 'main', dark='dark')
    c.rect(12 - s, 23, 13 - s, 28, 'main', dark='dark')
    hx = 7 - (2 if attack else 0)
    hy = 20 + bob + (1 if attack else 0)
    c.ellipse(hx, hy, 5, 5, 'main', dark='dark', light='light')
    c.poly([(hx + 2, hy - 4), (hx + 4, hy - 8), (hx + 5, hy - 3)], 'second_dark')    # ear
    c.rect(hx - 6, hy - 1, hx - 3, hy + 3, 'second', dark='second_dark')            # snout
    c.px(hx - 6, hy, 'outline')
    c.px(hx - 6, hy + 2, 'outline')
    c.px(hx - 1, hy - 2, 'eye')
    c.px(hx - 3, hy + 4, 'tooth')                                                   # tusk
    c.px(hx - 4, hy + 3, 'tooth')
    c.px(hx - 4, hy + 2, 'tooth')
    return c.done()


def spider(step, pose):
    c = Canvas()
    s = 2 if step in (1, 3) else 0
    if step == 3:
        s = -2
    attack = pose == 'attack'
    far = [((14, 20), (10, 13), (4 - s, 27)), ((16, 20), (14, 12), (10 + s, 28)),
           ((18, 20), (22, 12), (21 - s, 28)), ((20, 20), (26, 13), (28 + s, 27))]
    near = [((13, 21), (8, 15), (1 + s, 28)), ((15, 21), (12, 14), (7 - s, 28)),
            ((19, 21), (24, 14), (24 + s, 28)), ((21, 21), (28, 15), (30 - s, 28))]
    for (x0, y0), (kx, ky), (tx, ty) in far:
        c.line(x0, y0, kx, ky, 'dark', edge=False)
        c.line(kx, ky, tx, ty, 'dark', edge=False)
    c.ellipse(20, 17, 7, 6, 'main', dark='dark', light='light')                     # abdomen
    c.poly([(20, 12), (22, 15), (20, 18), (18, 15)], 'second', edge=False)          # marking
    hx = 11 - (2 if attack else 0)
    c.ellipse(hx, 21, 4, 3, 'main', dark='dark', light='light')
    for (x0, y0), (kx, ky), (tx, ty) in near:
        c.line(x0, y0, kx, ky, 'main', edge=False)
        c.line(kx, ky, tx, ty, 'main', edge=False)
    c.px(hx - 3, 20, 'eye')
    c.px(hx - 2, 20, 'eye')
    c.px(hx - 3, 24, 'tooth_dark')
    c.px(hx - 1, 24, 'tooth_dark')
    return c.done()


def small_humanoid(step, pose, kind):
    """Kobolds, murlocs and goblins: short, big-headed, side view."""
    c = Canvas()
    s = STRIDE[step] // 2
    bob = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    lunge = -2 if attack else 0
    c.rect(17 + s, 23, 18 + s, 28, 'dark')                                          # far leg
    c.rect(15, 15 + bob, 16, 20 + bob, 'dark')                                      # far arm
    c.ellipse(16, 20 + bob, 4, 5, 'second' if kind != 'murloc' else 'main',
              dark='second_dark' if kind != 'murloc' else 'dark', light='light')
    if kind == 'murloc':
        c.ellipse(15, 21 + bob, 2, 3, 'second', edge=False)                         # belly
    c.rect(14 - s, 23, 15 - s, 28, 'main', dark='dark')                             # near leg
    hx = 13 + lunge
    hy = 13 + bob
    if kind == 'kobold':
        c.ellipse(hx + 4, hy - 4, 2, 3, 'light')                                    # ear
        c.ellipse(hx, hy, 5, 4, 'main', dark='dark', light='light')
        c.rect(hx - 7, hy, hx - 3, hy + 2, 'light', dark='second')                  # snout
        c.px(hx - 7, hy, 'outline')
        c.px(hx - 2, hy - 1, 'eye')
        c.px(hx - 5, hy + 3, 'tooth')
    elif kind == 'goblin':
        c.poly([(hx + 3, hy - 2), (hx + 10, hy - 5), (hx + 4, hy + 2)], 'main', dark='dark')
        c.ellipse(hx, hy, 5, 4, 'main', dark='dark', light='light')
        c.rect(hx - 7, hy, hx - 4, hy + 2, 'light', dark='main')                    # nose
        c.rect(hx - 4, hy - 4, hx + 2, hy - 3, 'extra', dark='extra_dark')          # goggles
        c.px(hx - 2, hy - 1, 'eye')
        c.rect(hx - 4, hy + 3, hx - 2, hy + 3, 'tooth')
    else:   # murloc
        c.poly([(hx - 1, hy - 5), (hx + 2, hy - 9), (hx + 4, hy - 4), (hx + 8, hy - 3),
                (hx + 8, hy + 6), (hx + 4, hy + 2)], 'extra', dark='extra_dark')    # fin
        c.ellipse(hx, hy, 6, 5, 'main', dark='dark', light='light')
        c.rect(hx - 4, hy - 3, hx - 2, hy - 1, 'tooth')
        c.px(hx - 3, hy - 2, 'eye')
        c.line(hx - 6, hy + 2, hx - 1, hy + 3, 'outline', edge=False)
        c.px(hx - 5, hy + 3, 'tooth')
        c.px(hx - 3, hy + 4, 'tooth')
    arm_end = (hx - 4, hy + 7) if attack else (hx, hy + 10)
    if kind != 'murloc':
        # A short blade or wrench held low, pointing forward when attacking.
        tip = (arm_end[0] - 5, arm_end[1] - 1) if attack else (arm_end[0] - 1, arm_end[1] + 5)
        c.line(arm_end[0], arm_end[1], tip[0], tip[1], 'weapon', width=1)
    c.line(14 + lunge, 17 + bob, arm_end[0], arm_end[1], 'main', width=2, dark='dark')
    return c.done()


def kobold_candle(img, step, pose):
    """A candle stuck on the kobold's head (tunnelers carry them)."""
    c = Canvas()
    c.img = img.copy()
    bob = 1 if step in (1, 3) else 0
    hx = 13 + (-2 if pose == 'attack' else 0)
    c.rect(hx, 3 + bob, hx + 1, 8 + bob, 'extra', dark='extra_dark')
    c.px(hx, 1 + bob, 'flame')
    c.px(hx, 2 + bob, 'flame')
    c.px(hx + 1, 2 + bob, 'flame_dark')
    return outline(c.img)


def gnoll(step, pose):
    c = Canvas()
    s = STRIDE[step] // 2
    bob = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    lunge = -2 if attack else 0
    c.rect(18 + s, 21, 19 + s, 28, 'dark')
    c.ellipse(17, 16 + bob, 5, 7, 'main', dark='dark', light='light')
    c.rect(13, 16 + bob, 21, 20 + bob, 'extra', dark='extra_dark')                  # armor
    c.line(19, 7 + bob, 22, 14 + bob, 'second_dark', width=2, edge=False)           # mane
    c.rect(15 - s, 21, 16 - s, 28, 'main', dark='dark')
    hx = 12 + lunge
    hy = 9 + bob
    c.poly([(hx + 2, hy - 2), (hx + 4, hy - 7), (hx + 5, hy - 1)], 'second_dark')   # ear
    c.ellipse(hx, hy, 4, 3, 'main', dark='dark', light='light')
    c.rect(hx - 7, hy, hx - 2, hy + 2, 'light', dark='second')                      # snout
    c.px(hx - 7, hy, 'outline')
    c.px(hx - 2, hy - 1, 'eye')
    c.px(hx - 5, hy + 3, 'tooth')
    c.px(hx - 3, hy + 3, 'tooth')
    if attack:
        c.line(7 + lunge, 15 + bob, 1 + lunge, 13 + bob, 'weapon', width=2, dark='weapon_dark')
        c.line(14 + lunge, 13 + bob, 7 + lunge, 15 + bob, 'main', width=2, dark='dark')
    else:
        c.line(11, 19 + bob, 8, 26 + bob, 'weapon', width=2, dark='weapon_dark')
        c.line(14, 13 + bob, 11, 19 + bob, 'main', width=2, dark='dark')
    return c.done()


def watcher(step, pose):
    """A harvest watcher: a scarecrow-like golem of wood and burlap."""
    c = Canvas()
    s = STRIDE[step] // 2
    bob = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    c.rect(18 + s, 20, 19 + s, 28, 'dark')
    c.rect(13 - s, 20, 14 - s, 28, 'main', dark='dark')
    c.rect(10, 11 + bob, 21, 21 + bob, 'extra', dark='extra_dark', light='light')
    arm_y = 12 + bob + (2 if attack else 0)
    c.rect(3 - (2 if attack else 0), arm_y, 28, arm_y + 1, 'main', dark='dark')
    for x in (2 - (2 if attack else 0), 28):
        c.rect(x, arm_y - 1, x + 1, arm_y + 3, 'light', edge=False)
    c.ellipse(15, 6 + bob, 5, 5, 'second', dark='second_dark', light='light')
    c.poly([(10, 3 + bob), (15, -3 + bob), (20, 3 + bob)], 'second_dark')           # hat
    c.px(13, 6 + bob, 'eye')
    c.px(16, 6 + bob, 'eye')
    c.line(13, 9 + bob, 17, 9 + bob, 'outline', edge=False)
    return c.done()


CREATURES = {
    'wolf': wolf,
    'boar': boar,
    'spider': spider,
    'kobold': lambda step, pose: small_humanoid(step, pose, 'kobold'),
    'kobold_candle': lambda step, pose: kobold_candle(small_humanoid(step, pose, 'kobold'), step, pose),
    'murloc': lambda step, pose: small_humanoid(step, pose, 'murloc'),
    'goblin': lambda step, pose: small_humanoid(step, pose, 'goblin'),
    'gnoll': gnoll,
    'watcher': watcher,
}


def creature_sheet(draw):
    frames = [draw(step, 'walk') for step in range(4)]
    frames.append(draw(0, 'attack'))
    stand = frames[0]
    dead = np.zeros_like(stand)
    flipped = stand[::-1, :]
    rows = np.where(flipped.any(axis=1))[0]
    part = flipped[rows[0]:rows[-1] + 1, :]
    dead[FEET_Y + 1 - part.shape[0]:FEET_Y + 1, :] = part
    frames.append(dead)
    return frames


# --- palettes ---------------------------------------------------------------------------------------

def humanoid_palette(skin=(232, 168, 128), hair=(144, 88, 40), armor=(144, 152, 168),
                     tabard=(56, 88, 184), trim=(232, 184, 64), leather=(120, 80, 48),
                     lower_face=None, armor_light=None, hair_dark=None, tabard_dark=None):
    def shade(c, f):
        return tuple(max(0, min(255, int(v * f))) for v in c)
    return [
        (255, 0, 255), (24, 24, 32), shade(skin, 0.76), skin,
        hair_dark or shade(hair, 0.6), hair, shade(armor, 0.62), armor,
        armor_light or shade(armor, 1.32), tabard_dark or shade(tabard, 0.6), tabard, trim,
        shade(leather, 0.55), leather, shade(trim, 0.7),
        lower_face or (skin[0], skin[1], min(255, skin[2] + 8)),
    ]


def creature_palette(main, second, eye=(232, 40, 24), extra=(120, 80, 48), flame=(248, 224, 96),
                     weapon=(120, 120, 128), tooth=(240, 236, 216), dark=None, light=None):
    def shade(c, f):
        return tuple(max(0, min(255, int(v * f))) for v in c)
    return [
        (255, 0, 255), (24, 20, 28), dark or shade(main, 0.62), main, light or shade(main, 1.3),
        shade(second, 0.7), second, eye, shade(tooth, 0.7), tooth, extra, shade(extra, 0.6),
        flame, (232, 128, 40), shade(weapon, 0.6), weapon,
    ]


SKIN = (232, 168, 128)
SKIN_TAN = (200, 136, 96)
SKIN_DARK = (152, 96, 64)
SKIN_ELF = (176, 152, 224)
SKIN_DWARF = (232, 160, 128)

# Looks used by the player (race_class) and by NPCs and humanoid enemies. Each is a sheet + palette.
HUMANOID_LOOKS = {
    # player characters
    'human_warrior': ('hum_sword', humanoid_palette()),
    'human_mage': ('hum_staff', humanoid_palette(hair=(200, 160, 72), armor=(120, 64, 168),
                                                 tabard=(88, 40, 136), trim=(232, 200, 96),
                                                 armor_light=(176, 120, 216))),
    'dwarf_warrior': ('dwarf_sword', humanoid_palette(skin=SKIN_DWARF, hair=(168, 72, 32),
                                                      armor=(152, 152, 160), tabard=(160, 48, 40),
                                                      lower_face=(168, 72, 32))),
    'dwarf_hunter': ('dwarf_bow', humanoid_palette(skin=SKIN_DWARF, hair=(96, 56, 32),
                                                   armor=(136, 104, 64), tabard=(64, 112, 56),
                                                   trim=(200, 168, 96), lower_face=(96, 56, 32),
                                                   armor_light=(184, 152, 104))),
    'elf_warrior': ('elf_sword', humanoid_palette(skin=SKIN_ELF, hair=(72, 168, 152),
                                                  armor=(136, 144, 176), tabard=(88, 48, 136),
                                                  trim=(208, 208, 232))),
    'elf_hunter': ('elf_bow', humanoid_palette(skin=SKIN_ELF, hair=(224, 224, 240),
                                               armor=(80, 112, 64), tabard=(48, 80, 48),
                                               trim=(184, 160, 104), armor_light=(128, 160, 96))),
    # Northshire and Goldshire people
    'guard': ('hum_sword', humanoid_palette(hair=(112, 120, 136), armor=(144, 152, 168),
                                            tabard=(48, 72, 168), hair_dark=(72, 80, 96))),
    'marshal': ('hum_sword', humanoid_palette(hair=(216, 216, 224), armor=(168, 168, 184),
                                              tabard=(40, 64, 152), trim=(248, 208, 72))),
    'trainer_warrior': ('hum_sword', humanoid_palette(hair=(64, 40, 24), armor=(152, 152, 168),
                                                      tabard=(168, 40, 40))),
    'trainer_warrior_f': ('fem_sword', humanoid_palette(hair=(232, 200, 104), armor=(168, 168, 184),
                                                        tabard=(168, 40, 40))),
    'priest': ('hum_robe', humanoid_palette(hair=(176, 176, 176), armor=(224, 224, 216),
                                            tabard=(200, 200, 192), trim=(232, 192, 72),
                                            armor_light=(248, 248, 240))),
    'mage_trainer': ('hum_staff', humanoid_palette(hair=(224, 224, 232), armor=(64, 72, 160),
                                                   tabard=(40, 48, 120), trim=(200, 176, 248),
                                                   armor_light=(112, 120, 208))),
    'hunter_trainer': ('dwarf_bow', humanoid_palette(skin=SKIN_DWARF, hair=(216, 216, 216),
                                                     armor=(128, 96, 56), tabard=(96, 72, 40),
                                                     lower_face=(216, 216, 216))),
    'innkeeper': ('hum_robe', humanoid_palette(hair=(96, 56, 24), armor=(176, 112, 64),
                                               tabard=(232, 224, 200), trim=(120, 80, 48),
                                               armor_light=(208, 152, 104))),
    'merchant': ('hum_robe', humanoid_palette(hair=(48, 32, 24), armor=(56, 128, 72),
                                              tabard=(40, 96, 56), trim=(232, 184, 64))),
    'smith': ('fem_robe', humanoid_palette(hair=(152, 56, 32), armor=(120, 80, 48),
                                           tabard=(96, 64, 40), trim=(160, 160, 168))),
    'farmer_f': ('fem_robe', humanoid_palette(hair=(136, 128, 120), armor=(168, 96, 72),
                                              tabard=(200, 168, 120), trim=(120, 80, 48))),
    'farmer_f2': ('fem_robe', humanoid_palette(hair=(184, 120, 48), armor=(96, 120, 160),
                                               tabard=(216, 200, 160), trim=(120, 80, 48))),
    'militia': ('hum_sword', humanoid_palette(hair=(168, 120, 48), armor=(144, 104, 64),
                                              tabard=(176, 152, 72), trim=(120, 80, 48),
                                              armor_light=(192, 152, 104))),
    'peasant': ('hum_plain', humanoid_palette(hair=(120, 72, 32), armor=(152, 112, 72),
                                              tabard=(192, 176, 128), trim=(120, 80, 48))),
    # the Defias Brotherhood
    'defias_thug': ('hum_sword', humanoid_palette(hair=(168, 40, 40), armor=(104, 80, 64),
                                                  tabard=(88, 64, 48), trim=(168, 40, 40),
                                                  lower_face=(168, 40, 40))),
    'defias_trapper': ('hum_sword', humanoid_palette(hair=(152, 40, 40), armor=(88, 104, 64),
                                                     tabard=(72, 80, 48), trim=(152, 40, 40),
                                                     lower_face=(152, 40, 40))),
    'defias_smuggler': ('hum_sword', humanoid_palette(skin=SKIN_TAN, hair=(176, 48, 40),
                                                      armor=(80, 72, 96), tabard=(64, 56, 80),
                                                      trim=(176, 48, 40), lower_face=(176, 48, 40))),
    'defias_miner': ('hum_sword', humanoid_palette(hair=(216, 184, 64), armor=(112, 96, 80),
                                                   tabard=(96, 80, 64), trim=(168, 40, 40),
                                                   lower_face=(168, 40, 40), hair_dark=(152, 120, 40))),
    'defias_pirate': ('hum_sword', humanoid_palette(skin=SKIN_TAN, hair=(40, 40, 48),
                                                    armor=(200, 200, 192), tabard=(48, 64, 128),
                                                    trim=(168, 40, 40), lower_face=(168, 40, 40))),
    'defias_blackguard': ('hum_sword', humanoid_palette(hair=(40, 32, 40), armor=(72, 64, 80),
                                                        tabard=(48, 40, 56), trim=(152, 32, 40),
                                                        lower_face=(40, 32, 40))),
    'vancleef': ('hum_sword', humanoid_palette(hair=(48, 32, 40), armor=(96, 32, 40),
                                               tabard=(40, 32, 40), trim=(232, 184, 64),
                                               lower_face=(176, 32, 40), armor_light=(152, 56, 64))),
}

CREATURE_LOOKS = {
    'young_wolf': ('wolf', creature_palette((136, 120, 104), (200, 184, 160))),
    'timber_wolf': ('wolf', creature_palette((96, 96, 104), (176, 176, 184), eye=(248, 200, 64))),
    'boar': ('boar', creature_palette((120, 80, 56), (200, 136, 128))),
    'princess': ('boar', creature_palette((192, 120, 120), (232, 176, 168), eye=(40, 24, 24))),
    'forest_spider': ('spider', creature_palette((72, 64, 72), (176, 48, 40), eye=(232, 48, 32))),
    'kobold_vermin': ('kobold', creature_palette((152, 104, 64), (112, 72, 48), eye=(248, 208, 64),
                                                 light=(200, 160, 112))),
    'kobold_tunneler': ('kobold_candle', creature_palette((128, 96, 80), (88, 88, 104),
                                                          eye=(248, 208, 64), extra=(232, 224, 200),
                                                          light=(184, 152, 120))),
    'murloc': ('murloc', creature_palette((72, 152, 96), (184, 208, 152), eye=(24, 24, 24),
                                          extra=(216, 104, 56))),
    'riverpaw_gnoll': ('gnoll', creature_palette((184, 144, 80), (120, 72, 40), eye=(232, 48, 32),
                                                 extra=(120, 80, 48), weapon=(136, 96, 56))),
    'hogger': ('gnoll', creature_palette((112, 96, 88), (64, 48, 40), eye=(248, 64, 32),
                                         extra=(152, 48, 40), weapon=(152, 152, 160))),
    'gnoll_brute': ('gnoll', creature_palette((152, 136, 104), (88, 72, 56), eye=(232, 48, 32),
                                              extra=(88, 88, 104), weapon=(152, 152, 160))),
    'harvest_watcher': ('watcher', creature_palette((136, 96, 56), (200, 176, 120), eye=(248, 216, 64),
                                                    extra=(96, 120, 64), light=(232, 200, 96))),
    'goblin_engineer': ('goblin', creature_palette((104, 168, 72), (120, 88, 64), eye=(24, 24, 24),
                                                   extra=(200, 176, 72), weapon=(160, 160, 168))),
    'sneed': ('goblin', creature_palette((88, 152, 64), (152, 48, 40), eye=(24, 24, 24),
                                         extra=(232, 184, 64), weapon=(184, 184, 192))),
}


# --- output -----------------------------------------------------------------------------------------

def unique(palette):
    """Butano's tool rejects nothing, but the BMP helper wants distinct colors; nudge duplicates."""
    out = []
    for color in palette:
        color = gba_color(*color)
        while color in out:
            color = (color[0], color[1], (color[2] + 8) % 256)
        out.append(color)
    return out


def write_sheet(name, frames, palette):
    sheet = np.concatenate(frames, axis=0)
    save_indexed_bmp(GRAPHICS / f'char_{name}.bmp', sheet, unique(palette))
    (GRAPHICS / f'char_{name}.json').write_text('{\n    "type": "sprite",\n    "height": 32\n}\n')
    return frames


def bn_color(rgb):
    r, g, b = gba_color(*rgb)
    return f'bn::color({r >> 3}, {g >> 3}, {b >> 3})'


def write_palettes():
    lines = ['// Generated by tools/gen_characters.py. Do not edit by hand.',
             '#ifndef GW_PALETTES_H', '#define GW_PALETTES_H', '',
             '#include "bn_color.h"', '#include "bn_sprite_palette_item.h"', '',
             'namespace gw::palettes', '{']
    for name, (_, palette) in list(HUMANOID_LOOKS.items()) + list(CREATURE_LOOKS.items()):
        colors = ', '.join(bn_color(c) for c in palette)
        lines.append(f'    constexpr bn::color {name}_colors[] = {{ {colors} }};')
        lines.append(f'    constexpr bn::sprite_palette_item {name}({name}_colors, bn::bpp_mode::BPP_4);')
        lines.append('')
    lines += ['}', '', '#endif', '']
    (INCLUDE / 'gw_palettes.h').write_text('\n'.join(lines))


def write_looks():
    """include/gw_looks_data.h: look_id values and the sheet + palette behind each look."""
    looks = list(HUMANOID_LOOKS.items()) + list(CREATURE_LOOKS.items())
    sheets = sorted({sheet for _, (sheet, _) in looks})
    lines = ['// Generated by tools/gen_characters.py. Do not edit by hand.',
             '// Include only from gw_looks.cpp: it pulls in every character sheet.',
             '#ifndef GW_LOOKS_DATA_H', '#define GW_LOOKS_DATA_H', '']
    lines += [f'#include "bn_sprite_items_char_{sheet}.h"' for sheet in sheets]
    lines += ['', '#include "gw_looks.h"', '#include "gw_palettes.h"', '', 'namespace gw', '{', '',
              'constexpr look_def look_table[] = {']
    for name, (sheet, _) in looks:
        creature = 'true' if name in CREATURE_LOOKS else 'false'
        lines.append(f'    {{ bn::sprite_items::char_{sheet}, palettes::{name}, {creature} }},')
    lines += ['};', '', 'static_assert(sizeof(look_table) / sizeof(look_table[0]) == int(look_id::COUNT));', '',
              '}', '', '#endif', '']
    (INCLUDE / 'gw_looks_data.h').write_text('\n'.join(lines))

    enum = ['// Generated by tools/gen_characters.py. Do not edit by hand.', '#ifndef GW_LOOK_IDS_H',
            '#define GW_LOOK_IDS_H', '', '#include <cstdint>', '', 'namespace gw', '{', '',
            'enum class look_id : uint8_t', '{']
    enum += [f'    {name.upper()},' for name, _ in looks]
    enum += ['    COUNT', '};', '', '}', '', '#endif', '']
    (INCLUDE / 'gw_look_ids.h').write_text('\n'.join(enum))


def preview(rows_of_frames, palettes, path):
    """Each row: the frames of one look, drawn with its palette."""
    strips = []
    for frames, palette in zip(rows_of_frames, palettes):
        lut = np.array([gba_color(*c) for c in palette], dtype=np.uint8)
        lut[0] = (96, 160, 96)
        strips.append(lut[np.concatenate(frames, axis=1)])
    width = max(s.shape[1] for s in strips)
    strips = [np.pad(s, ((0, 0), (0, width - s.shape[1]), (0, 0))) for s in strips]
    from PIL import Image
    image = Image.fromarray(np.concatenate(strips, axis=0), 'RGB')
    image = image.resize((image.width * 3, image.height * 3), Image.NEAREST)
    PREVIEW.mkdir(parents=True, exist_ok=True)
    image.save(path)


def main():
    sheets = {}
    base_palette = HUMANOID_LOOKS['human_warrior'][1]
    for name, humanoid in HUMANOID_SHEETS.items():
        sheets[name] = write_sheet(name, humanoid.sheet(), base_palette)
    base_creature = CREATURE_LOOKS['young_wolf'][1]
    for name, draw in CREATURES.items():
        sheets[name] = write_sheet(name, creature_sheet(draw), base_creature)
    write_palettes()
    write_looks()

    looks = list(HUMANOID_LOOKS.items()) + list(CREATURE_LOOKS.items())
    preview([sheets[sheet] for _, (sheet, _) in looks], [p for _, (_, p) in looks],
            PREVIEW / 'characters.png')
    print(f'{len(sheets)} sheets, {len(looks)} looks')


if __name__ == '__main__':
    main()
