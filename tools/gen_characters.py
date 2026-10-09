"""Generate the character sprite sheets and their palettes.

Humanoid sheets are 32x32 frames, 17 per sheet, stacked vertically:
  0-3   walk down (frame 0 doubles as idle)
  4-7   walk up
  8-11  walk left (the game mirrors these for right)
  12    attack down, 13 attack up, 14 attack left
  15    cast (facing down, arms raised)
  16    dead

Creature sheets are side views only, 6 frames: 0-3 walk left (0 = idle), 4 attack, 5 dead.

Mount sheets (the horse, ram and nightsaber of the Mount ability) have 8 frames and their own palette:
  0-3   gallop left (0 = standing), 4-5 trot towards the viewer, 6-7 trot away
The rider is drawn behind the mount, raised so the mount hides the legs.

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
    # A topknot, a heavy brow, a wide jaw and two tusks (the trim color).
    'orc': {
        'front': {1: '.....oHH', 2: '....oHHH', 3: '..ooSSHH', 4: '.oSSSSSH', 5: '.osSSSSS', 6: '.oSSoSSS',
                  7: '.oSmmgmm', 8: '..osmmmm'},
        'back': {1: '.....oHH', 2: '....oHHH', 3: '..ooSSHH', 4: '.oSSSSSH', 5: '.osSSSSS', 6: '.oSSSSSS',
                 7: '.oSSSSSS', 8: '..osSSSS'},
        'side': {0: '.......ooo......', 1: '......oHHHo.....', 2: '....ooHHHHo.....',
                 3: '..ooSSSHHSSo....', 4: '.oSSSSSSSSSo....', 5: 'osSSSSSSSSSo....',
                 6: '.oSoSSSSSSSo....', 7: 'ogmmmmsSSSSo....', 8: '.ommmmsooo......'},
    },
    'dwarf': {
        'front': {7: '...ommmm', 8: '...ommmm', 9: '.ooAommm', 10: 'oALLAomm', 11: 'oAAAaobm'},
        'side': {8: '.ommmmsooo......', 9: '..ommmAooogo....', 10: '..ommLAAobBgo...',
                 11: '..oomAAAobBgo...'},
    },
    # A gnome replaces every row: a big round head with a hair tuft, wide eyes and long ears sticking out
    # sideways over a narrow, short body (rows 11-17; the hands stay on row 15 like everyone else's). The
    # chin is the lower face, so an old gnome gets a beard from the palette.
    'gnome': {
        'front': dict(enumerate([
            '......oo',
            '.....oHH',
            '...ooHHH',
            '..oHHHHH',
            '.ohHoggo',
            'o.ohoddo',
            'SoohSSSS',
            'oSShSoSS',
            '.osSSoSS',
            '..osSSms',
            '...osmmm',
            '..ooAAom',
            '.oALLAob',
            '.oAAaobB',
            '.oaaobBg',
            '.oSsollg',
            '..oobBBB',
            '...obBbB',
        ])),
        'back': dict(enumerate([
            '......oo',
            '.....oHH',
            '...ooHHH',
            '..oHHHHH',
            '.ohHHHHH',
            'o.ohdddd',
            'SohhHHHH',
            'oShhHHHH',
            '.oohhHHH',
            '...ohhhH',
            '....ohhh',
            '..ooAAoo',
            '.oALLAgg',
            '.oAAagBB',
            '.oaagBgB',
            '.oSsogBB',
            '..ooodgB',
            '...obodd',
        ])),
        'side': dict(enumerate([
            '......oo........',
            '.....oHHo.......',
            '....oHHHHoo.....',
            '...oHHHHHHHo....',
            '..oggoHHHHHHo...',
            '..oddoddddhHo...',
            '.oSSSHHhhhhHo...',
            '.oSoSSShSSShoSo.',
            'oSSoSSShsSSSSo..',
            '.osSSSSshSoo....',
            '..ommmmsho......',
            '...ooAAooo......',
            '..oALLAobBo.....',
            '..oAAAAobBo.....',
            '..oaAaaobgo.....',
            '...oSsolldo.....',
            '...obBBBoo......',
            '...obBbBo.......',
        ])),
    },
    # A jungle troll: a tall mohawk standing up from a bald head, long ears sticking out sideways and up, a deep
    # brow and a long jaw (the lower face) with two tusks (the trim color) curving up from it, over the shared body.
    'troll': {
        'front': dict(enumerate([
            '.......H',
            '......hH',
            '......hH',
            '......hH',
            '....ooSh',
            'S..ossSS',
            'SSSSSoSS',
            '..soSSSs',
            '...gSSss',
            '..oogooo',
            'oALLAomm',
            'oAAAaoom',
        ])),
        'back': dict(enumerate([
            '.......H',
            '......hH',
            '......hH',
            '.....ohH',
            '...ooSSH',
            'S..oSSSH',
            'sSSSSSSh',
            '..ssSSSh',
            '....osSS',
            '.ooAAoss',
        ])),
        'side': dict(enumerate([
            '......H.H.......',
            '.....hHhHH......',
            '.....hHHHHh.....',
            '....ohHHHHho....',
            '...ooShhhhSo....',
            '..oSSSSSSSSSo.SS',
            '.ossoSSSSSSSSSs.',
            'oSSSSSSSSSSso...',
            '.ogSsSSSSSSo....',
            '.gmmmoAooogo....',
        ])),
    },
    # A goblin keeps the gnome's short body under a big bald head with long pointed ears sticking out sideways
    # (three columns past the body on each side: 'margin'), a long hooked nose and a wide grin. The grin's teeth
    # are the lower face.
    'goblin': {
        'margin': 3,
        'front': dict(enumerate([
            '........ooo',
            '......ooSSS',
            '.....oSSSSS',
            'S...oSSSSSS',
            'SSS.oShhSSs',
            '.SSSSSSoSoS',
            '..sssSSSSoS',
            '....oSSSSoS',
            '.....oooooS',
            '.....ommmms',
            '......ossss',
            '.....ooAAoo',
            '....oALLAob',
            '....oAAaobB',
            '....oaaobBg',
            '....oSsollg',
            '.....oobBBB',
            '......obBbB',
        ])),
        'back': dict(enumerate([
            '........ooo',
            '......ooSSS',
            '.....oSSSSS',
            'S...oSSSSSS',
            'SSS.oSSSSSS',
            '.SSSSSSSSSS',
            '..sssSSSSSS',
            '....oSSSSSS',
            '.....osSSSS',
            '.....ossSSS',
            '......ossss',
            '.....ooAAoo',
            '....oALLAgg',
            '....oAAagBB',
            '....oaagBgB',
            '....oSsogBB',
            '.....ooodgB',
            '......obodd',
        ])),
        'side': dict(enumerate([
            '......oooo......',
            '....ooSSSSoo....',
            '...oSSSSSSSSo...',
            '..oSSSSSSSSSSo..',
            '..oShhSSSSSSSo.S',
            '.oSSoSSSSSSoSSSs',
            'ooSSSSSSSSSSss..',
            'SSSSsSSSSSSo....',
            'Ssooo.SSSSso....',
            '.o.ommmSSso.....',
            '....ossssoo.....',
            '...ooAAooo......',
            '..oALLAobBo.....',
            '..oAAAAobBo.....',
            '..oaAaaobgo.....',
            '...oSsolldo.....',
            '...obBBBoo......',
            '...obBbBo.......',
        ])),
    },
}

# The gnome's cast pose: the arms raised out to the sides, below the ears.
GNOME_CAST = {
    9: 'oS.sSSms',
    10: 'oAo.osmm',
    11: '.oAoAAom',
    12: '..oALAob',
    13: '...oaobB',
    14: '...oobBg',
    15: '...ollll',
}

# The goblin's: the same raised arms, three columns further in under its wider rows.
GOBLIN_CAST = {
    7: '..oSoSSSSoS',
    8: '..oSAoooooS',
    9: '..oAAommmms',
    10: '...oAAossss',
    11: '....oAAAAoo',
    12: '.....oALAob',
    13: '......oaobB',
    14: '......oobBg',
    15: '......ollll',
}

# Heads that hold the shorter sword.
SHORT_HEADS = ('gnome', 'goblin')
CAST_POSES = {'gnome': GNOME_CAST, 'goblin': GOBLIN_CAST}
# Where the raised hand of a cast pose holds a sword: columns right of the body's left edge, rows below the top.
CAST_HANDS = {'gnome': (1, 9), 'goblin': (0, 7)}


def widen(rows, margin):
    """Front or back half rows with margin more columns on the outside, for heads wider than the body."""
    return ['.' * margin + row for row in rows]


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
SWORD_SHORT = SWORD[:4] + SWORD[6:]         # a gnome's: the blade two pixels shorter

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
        self.leg_extra = {'dwarf': -3, 'elf': 1, 'gnome': -5, 'troll': 1, 'goblin': -5}.get(head, 0)
        patches = HEAD_PATCHES[head]
        self.margin = patches.get('margin', 0)          # the front and back views are this much wider each side
        front = patched(widen(FRONT, self.margin), patches.get('front', {}))
        self.front = to_array(mirror(front), HKEYS)
        self.sword = SWORD_SHORT if head in SHORT_HEADS else SWORD
        self.cast_top = None
        if head in CAST_POSES:
            self.cast_top = to_array(mirror(patched(front, CAST_POSES[head])), HKEYS)
        self.back = to_array(mirror(patched(widen(BACK, self.margin), patches.get('back', {}))), HKEYS)
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
            top_part = self._raise_arms(top_part) if self.cast_top is None else self.cast_top.copy()
        blit(frame, top_part, self.ox + lunge - (self.margin if view != 'side' else 0), top_y + bob)

        if pose == 'walk' and view == 'side':
            self._held_weapon(frame, view, hand_x, off_hand_x, hand_y)
        elif pose == 'attack':
            self._attack_weapon(frame, view, hand_x, hand_y)
        elif pose == 'cast':
            self._cast_weapon(frame, top_y)
        return outline(frame)

    def _held_weapon(self, frame, view, hand_x, off_hand_x, hand_y):
        if self.weapon == 'sword':
            place(frame, stamp(self.sword), SWORD_GRIP, hand_x, hand_y)
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
        rows = self.sword if self.weapon != 'staff' else STAFF[:12]
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
        elif self.weapon == 'sword' and self.cast_top is not None:
            part = stamp(self.sword)[::-1, :]           # the grip in the raised hand
            hand_x, hand_y = CAST_HANDS[self.head]
            blit(frame, part, self.ox + hand_x - 2, top_y + hand_y - (part.shape[0] - 3))
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
    'orc_sword': Humanoid('orc', 'armor', 'sword'),
    'orc_staff': Humanoid('orc', 'robe', 'staff'),
    'gnome_plain': Humanoid('gnome', 'armor', 'none'),
    'gnome_sword': Humanoid('gnome', 'armor', 'sword'),
    'dwarf_plain': Humanoid('dwarf', 'armor', 'none'),
    'fem_bow': Humanoid('long', 'armor', 'bow'),
    'troll_sword': Humanoid('troll', 'armor', 'sword'),
    'troll_staff': Humanoid('troll', 'robe', 'staff'),
    'goblin_plain': Humanoid('goblin', 'armor', 'none'),
    'goblin_sword': Humanoid('goblin', 'armor', 'sword'),
}

# --- creatures --------------------------------------------------------------------------------------

CREATURE_ROLES = ['transparent', 'outline', 'dark', 'main', 'light', 'second_dark', 'second', 'eye',
                  'tooth_dark', 'tooth', 'extra', 'extra_dark', 'flame', 'flame_dark', 'weapon_dark',
                  'weapon']
C = {name: index for index, name in enumerate(CREATURE_ROLES)}

# Keys for creature parts drawn as text (Canvas.sticker); lower case is the darker shade of a pair.
CKEYS = {'.': 0, 'o': 1, 'd': 2, 'm': 3, 'l': 4, 's': 5, 'S': 6, 'e': 7, 't': 8, 'T': 9, 'X': 10, 'x': 11,
         'F': 12, 'f': 13, 'w': 14, 'W': 15}

# Limbs on the far side of the body are drawn a shade darker.
FAR_SHADE = np.arange(16, dtype=np.uint8)
for _near, _far in (('main', 'dark'), ('light', 'main'), ('second', 'second_dark'), ('tooth', 'tooth_dark'),
                    ('extra', 'extra_dark'), ('weapon', 'weapon_dark')):
    FAR_SHADE[C[_near]] = C[_far]


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

    def sticker(self, rows, x, y, shift=None, far=False, edge=True):
        """A part drawn as text (CKEYS) with its top-left at (x, y). shift moves each row sideways (strides,
        swings), far shades it as a limb on the far side, and edge borders it in the outline color over what
        is behind (a number of rows leaves the top without a border, where the part joins the body)."""
        width = max(len(row) for row in rows)
        rows = [row.ljust(width, '.') for row in rows]
        if shift:
            pad = max(abs(s) for s in shift)
            rows = ['.' * (pad + s) + row + '.' * (pad - s) for row, s in zip(rows, shift)]
            x -= pad
        part = to_array(rows, CKEYS)
        if far:
            part = FAR_SHADE[part]
        full = np.zeros((FRAME, FRAME), dtype=np.uint8)
        blit(full, part, x, y)
        mask = full != 0
        if edge:
            ring = neighbors(mask) & ~mask
            if edge is not True:
                ring[:y + edge, :] = False          # no border over the first rows, where the part joins on
            self.img[ring] = C['outline']
        self.img[mask] = full[mask]

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


def water_elemental(step, pose):
    """A Water Elemental: a body of churning water with heavy arms, standing on its own wave."""
    c = Canvas()
    bob = [0, 1, 2, 1][step]
    attack = pose == 'attack'
    c.ellipse(15.5, 26, 9 + (step % 2), 2.5, 'dark', light='main')                    # wave
    c.poly([(10, 26), (12, 14 + bob), (20, 14 + bob), (22, 26)], 'main', dark='dark', light='light')
    c.line(12, 23, 15, 19 + bob, 'second', edge=False)                                # swirls
    c.line(17, 24, 20, 20 + bob, 'second', edge=False)
    c.ellipse(15.5, 13 + bob, 7, 5, 'main', dark='dark', light='light')               # chest
    lift = 4 if attack else 0
    for x in (6, 25):
        c.ellipse(x, 15 + bob - lift, 3, 4, 'main', dark='dark', light='light')     # arms
        c.px(x, 12 + bob - lift, 'second')
    c.ellipse(15.5, 6 + bob, 4, 3.5, 'main', dark='dark', light='light')              # head
    c.line(13, 3 + bob, 18, 3 + bob, 'second', edge=False)                            # foam crest
    c.px(14, 6 + bob, 'eye')
    c.px(17, 6 + bob, 'eye')
    return c.done()


def leg_stride(stride, rows=12, hip=3):
    """Row shifts for a leg drawn as text: the hip stays put, the knee moves half the stride, the rest all of it."""
    return [0] * hip + [stride // 2] * 2 + [stride] * (rows - hip - 2)


def sway(swing, rows):
    """Row shifts for a hanging limb drawn as text: fixed at the top, swinging pixels sideways at the bottom."""
    return [round(swing * i / (rows - 1)) for i in range(rows)]


def glow(c, x, y, r):
    """A glowing orb or flame (spells in hand, staff tops): a bright core in a darker halo."""
    c.ellipse(x, y, r, r, 'flame_dark')
    c.ellipse(x, y, max(r - 1, 0.5), max(r - 1, 0.5), 'flame', edge=False)


WORGEN_HEAD = [
    '.......l...d..',
    '.......ll..dd.',
    '......llm.ddd.',
    '......lmm.ddd.',
    '.....lmmmmmdd.',
    '...llmmmmmmmd.',
    '.llmmmoemmmmm.',
    'ommmmmmmmmmmm.',
    '.mmmmmmmmmmmd.',
    '.ooToSSmmmmdd.',
    '..SSSSSSmmdd..',
    '...ooooodd....',
]
WORGEN_SNARL = [
    '.......l...d..',
    '.......ll..dd.',
    '......llm.ddd.',
    '......lmm.ddd.',
    '.....lmmmmmdd.',
    '.lllmmmmmmmmd.',
    'olmmmmoemmmmm.',
    '.mmmmmmmmmmmm.',
    '.TToTTmmmmmmd.',
    '.o...ommmmmdd.',
    '..TToSSmmmdd..',
    '...SSSSSddd...',
    '....ooooo.....',
]
WORGEN_BODY = [
    '......l........',
    '.....lm..l.....',
    '....lmm.lm..l..',
    '...lmmdlmm.lm..',
    '...mmmmmmdlmml.',
    '..mmmmdmmmmmdm.',
    '.mmmmmmmmdmmmd.',
    'mmmmmmmmmmmdmm.',
    'mmmmmmmmmmmmmd.',
    'lmmmmmmmmmmmd..',
    '.lmmmmmmmmmd...',
    '..lmmmmmmmd....',
    '...lmmmmmmd....',
    '....mmmmmmd....',
    '....xxxxxxx....',
]
WORGEN_BODY_UPRIGHT = [
    '.....l..l....',
    '....lm.lm..l.',
    '...lmmlmm.lm.',
    '..lmmmmmmdmm.',
    '.lmmmmmmmmmdl',
    '.mmmmmmmmmdm.',
    'lmmmmmmmmmmd.',
    'lmmmmmmmmmd..',
    '.lmmmmmmmmd..',
    '..lmmmmmmmd..',
    '..lmmmmmmd...',
    '...mmmmmmd...',
    '...xxxxxxx...',
]
WORGEN_LEG = [
    '..XXXXXX.',
    '..XXXXXXx',
    '.XXXXXXx.',
    'XXXXXxx..',
    'XoXxo....',
    '.mmd.....',
    '..mmd....',
    '...mmd...',
    '....mmd..',
    '....mmd..',
    '.lmmmmd..',
    'Tmmmmmd..',
]
WORGEN_ARM = [
    '......lm.',
    '.....lmm.',
    '.....mmd.',
    '....lmmd.',
    '....mmd..',
    '...lmmd..',
    '...mmd...',
    '..lmmd...',
    '..mmd....',
    '.lmmd....',
    '.mmmd....',
    'mmmmd....',
    'T.T.T....',
]
WORGEN_ARM_BENT = [
    '......lm.',
    '.....lmmd',
    '.....mmmd',
    '.....lmmd',
    '..lmmmmmd',
    '.mmmmmmd.',
    'mmdddddd.',
]
WORGEN_ARM_THRUST = [
    '.........lm.',
    '..lmmmmmmmmm',
    '.lmmmmmmmmmd',
    '.mmdddddddd.',
]
WORGEN_ARM_SWIPE = [
    'T...........',
    '.T.......lm.',
    'T.lmmmmmmmmm',
    '.lmmmmmmmmmd',
    'T.mdddddddd.',
    '.T..........',
]


def worgen(step, pose, caster=False):
    """A worgen: a tall hunched wolf-man on digitigrade legs, with a shaggy mane and long clawed arms.
    The caster stands more upright and holds a glowing orb in both hands."""
    c = Canvas()
    d = STRIDE[step]
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    u = -2 if attack else 0                     # the upper body lunges forward
    c.sticker(WORGEN_LEG, 15, 17 + b, leg_stride(-d), far=True)                       # far leg
    if caster:
        # Upright, the head further back, both hands holding a glowing orb in front of the chest.
        c.sticker(WORGEN_ARM_THRUST if attack else WORGEN_ARM_BENT, 7 if attack else 10, 9 + b, far=True)
        c.sticker(WORGEN_BODY_UPRIGHT, 11 + u // 2, 4 + b)
        c.sticker(WORGEN_LEG, 13, 17 + b, leg_stride(d), edge=2)
        c.sticker(WORGEN_HEAD, 4 + u, 1 + b, edge=False)
        if attack:
            c.sticker(WORGEN_ARM_THRUST, 5, 10 + b)
            glow(c, 3, 11 + b, 3)
            for x, y in ((3, 6), (0, 8), (6, 7), (0, 14), (3, 16), (6, 15)):
                c.px(x, y + b, 'flame')
        else:
            c.sticker(WORGEN_ARM_BENT, 8, 9 + b)
            glow(c, 6, 14 + b, 2.5 if step in (1, 3) else 2)
        return c.done()
    if not attack:
        c.sticker(WORGEN_ARM, 11, 9 + b, sway(d, 13), far=True)                      # far arm
    c.sticker(WORGEN_BODY, 10 + u, 2 + b)
    c.sticker(WORGEN_LEG, 13, 17 + b, leg_stride(d), edge=2)                         # near leg
    c.sticker(WORGEN_SNARL if attack else WORGEN_HEAD, 1 + u, 1 + b, edge=False)
    if attack:
        c.sticker(WORGEN_ARM_SWIPE, 1 + u, 9 + b)
    else:
        c.sticker(WORGEN_ARM, 8, 9 + b, sway(-d, 13))
    return c.done()


SKULL = [
    '..llll..',
    '.lmmmmm.',
    'lmmmmmmd',
    'ooommmmd',
    'oeommmmd',
    '.mmmmmd.',
    'omomomd.',
    '.mmmmd..',
]
RIBCAGE = [
    '.lmmmd.',
    'mmmmmmd',
    'oooooom',
    'mmmmmmd',
    'oooooom',
    '.mmmmmd',
    '..ooomd',
    '.....m.',
]


def sword(c, x, y, dx, dy, length=8):
    """A sword with its grip at (x, y), the blade running along (dx, dy) (one of the 8 directions)."""
    c.line(x - dy, y + dx, x + dy, y - dx, 'weapon_dark')                            # cross-guard
    c.px(x - dx, y - dy, 'weapon_dark')                                               # pommel
    tip = (x + dx * length, y + dy * length)
    c.line(x + dx, y + dy, tip[0], tip[1], 'weapon')
    side = (-dy, dx) if dy >= 0 else (dy, -dx)
    c.line(x + dx + side[0], y + dy + side[1], tip[0] - dx + side[0], tip[1] - dy + side[1], 'weapon_dark',
           edge=False)


def skeleton(step, pose, robe=False):
    """A skeleton warrior with a rusty sword, or (robe) a skeletal mage in a tattered hood with a glowing staff."""
    c = Canvas()
    s = STRIDE[step] // 2
    bob = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    u = -1 if attack else 0
    if robe:
        sx = 8 if not attack else 6
        if not attack:
            c.line(sx, 5 + bob, sx, 28, 'weapon')                                      # staff behind the arm
        c.poly([(12, 11 + bob), (19, 11 + bob), (22, 28), (8, 28)], 'extra', dark='extra_dark')
        for x in range(9, 23, 3):
            c.img[28, x + step % 2] = 0                                                  # tattered hem
        c.img[27, 13 + step % 2] = 0
        c.img[27, 19 + step % 2] = 0
        c.line(17, 19 + bob, 18, 21 + bob, 'outline', edge=False)                       # rips
        c.line(12, 23, 13, 24, 'outline', edge=False)
        if step in (1, 3):
            c.rect(9 - (s > 0), 28, 11 - (s > 0), 28, 'main')                           # a bony foot
        c.ellipse(14.5 + u, 8 + bob, 4.5, 4.5, 'extra', dark='extra_dark')             # hood
        c.poly([(16 + u, 4 + bob), (21 + u, 6 + bob), (19 + u, 10 + bob)], 'extra', dark='extra_dark')
        c.ellipse(12.5 + u, 9 + bob, 2.5, 3, 'main', light='light', edge=False)       # skull in the hood
        c.rect(10 + u, 8 + bob, 11 + u, 9 + bob, 'outline', edge=False)
        c.px(10 + u, 8 + bob, 'eye')
        c.px(11 + u, 11 + bob, 'outline')
        if attack:
            c.line(3, 4 + bob, 12, 27, 'weapon')
            c.line(15, 14 + bob, 8, 15 + bob, 'extra', width=2)                        # sleeve
            c.px(7, 15 + bob, 'main')
            glow(c, 3, 4 + bob, 3)
            for x, y in ((3, 0), (0, 3), (7, 3), (0, 7), (6, 7)):
                c.px(x, y + bob, 'flame')
        else:
            c.line(15, 13 + bob, 10, 16 + bob, 'extra', width=2)                       # sleeve
            c.px(sx, 16 + bob, 'main')
            c.px(sx + 1, 16 + bob, 'main')
            glow(c, sx, 3 + bob, 2 if step in (1, 3) else 1.5)
        return c.done()

    b = bob

    def leg(x, sw, key):
        knee = (x - 1 + sw, 24)
        foot = (x + 2 * sw, 27)
        c.line(x, 20 + b, knee[0], knee[1], key)
        c.line(knee[0], knee[1], foot[0], foot[1], key)
        c.rect(foot[0] - 2, 28, foot[0], 28, key)
        c.px(knee[0], knee[1], 'light' if key == 'main' else 'main')
    leg(17, s, 'dark')
    if not attack:
        c.line(17, 13 + b, 18, 17 + b, 'dark')                                         # far arm
        c.line(18, 17 + b, 18 + s, 20 + b, 'dark')
    c.sticker(RIBCAGE, 11 + u, 12 + b)
    c.rect(14, 19 + b, 18, 20 + b, 'extra', dark='extra_dark')                          # belt
    c.poly([(13, 20 + b), (17, 20 + b), (16, 24 + b), (14, 23 + b)], 'extra', dark='extra_dark')
    leg(15, -s, 'main')
    c.sticker(SKULL, 9 + u, 4 + b)
    c.ellipse(16.5 + u, 12.5 + b, 2, 1.5, 'extra', dark='extra_dark', light='light')   # pauldron
    if attack:
        c.line(16 + u, 14 + b, 13, 13 + b, 'main')
        c.line(13, 13 + b, 11, 11 + b, 'main')
        sword(c, 10, 10 + b, -1, -1, 7)                                                # raised forward
    else:
        hand = (12 - s, 19 + b)
        c.line(16, 13 + b, 14, 17 + b, 'main')
        c.line(14, 17 + b, hand[0], hand[1], 'main')
        sword(c, hand[0] - 1, hand[1] + 1, -1, 1, 6)                                   # held low, pointing ahead
    return c.done()


GHOUL_HEAD = [
    '....lll...',
    '..llmmmml.',
    '.lmmmmmmmd',
    '.oemmmmmmd',
    '.mmmmmmmdd',
    '..ommmmmd.',
    'T.T.Tomd..',
    'mmmmmmmd..',
    '.mmmmmd...',
    '..ddd.....',
]
GHOUL_GAPE = [
    '....lll...',
    '..llmmmml.',
    '.lmmmmmmmd',
    'loemmmmmmd',
    'mmmmmmmmdd',
    'TTToommmd.',
    '.....omd..',
    'T.T.Tomd..',
    'mmmmmmmd..',
    '.mmmmmd...',
    '..ddd.....',
]
GHOUL_BODY = [
    '.......l.l....',
    '.....lmlmlml..',
    '...llmmmmmmmd.',
    '..lmmmSmmmmmmd',
    '.lmmmmmmmmmSmd',
    'lmmmmmmmmmmmmd',
    'mmmlolololommd',
    '.mmlolololomd.',
    '..mlolololomd.',
    '...dmmmmmmmd..',
    '....ddmmmmd...',
]
GHOUL_ARM = [
    '....lmm',
    '....mmd',
    '...lmmd',
    '...mmd.',
    '...mmd.',
    '...mmd.',
    '...mmd.',
    '..lmd..',
    '..mmd..',
    '..mmd..',
    '.lmd...',
    '.mmd...',
    '.mmd...',
    'lmmd...',
    'mmmd...',
    'T.T....',
    '.T.T...',
]
GHOUL_ARM_LUNGE = [
    'T..........',
    '.T.....lmm.',
    'T.lmmmmmmmd',
    '.lmmmmmmdd.',
    'T.mddddd...',
    '.T.........',
]
GHOUL_LEG = [
    '...XXXX',
    '..XXXXx',
    '.mmmXx.',
    'mmmmd..',
    'mmmd...',
    '.mmd...',
    '..mmd..',
    '...mmd.',
    '...mmd.',
    '...mmd.',
    '.lmmmd.',
    'Tmmmmd.',
]


def ghoul(step, pose):
    """A ghoul: a hunched, bald undead with a heavy jaw, showing ribs, its long arms dragging low."""
    c = Canvas()
    d = STRIDE[step]
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    u = -2 if attack else 0
    c.sticker(GHOUL_LEG, 19, 17 + b, leg_stride(-d), far=True)
    if attack:
        c.sticker(GHOUL_ARM_LUNGE, 3, 13 + b, far=True)
    else:
        c.sticker(GHOUL_ARM, 10, 11 + b, sway(d // 2, 17), far=True)
    c.sticker(GHOUL_BODY, 10 + u // 2, 7 + b - (1 if attack else 0))
    c.sticker(GHOUL_LEG, 17, 17 + b, leg_stride(d), edge=3)
    c.sticker(GHOUL_GAPE if attack else GHOUL_HEAD, 3 + u, 8 + b - (2 if attack else 0), edge=False)
    if attack:
        c.sticker(GHOUL_ARM_LUNGE, 1, 15 + b)
    else:
        c.sticker(GHOUL_ARM, 7, 11 + b, sway(-(d // 2), 17))
    return c.done()


OGRE_HEAD = [
    '....S....',
    '...sS....',
    '..lllll..',
    '.lmmmmmmd',
    '.oooommmd',
    '.meeommmd',
    'ommmmmmmd',
    'T.ooommd.',
    'Tmmmmmmd.',
    '.mmmmmd..',
    '..dddd...',
]
OGRE_BODY = [
    '.........llllll.......',
    '.......llmmmmmmll.....',
    '......lmmmmmmmmmmml...',
    '.....lmmmmmmmmmmmmmd..',
    '....lmmmmmmmmmmmmmmmd.',
    '....mmmmmmmmmmmmmmmmd.',
    '...lmmmmmmmmmmmmmmmmmd',
    '...mmmmmmmmmmmmmmmmmmd',
    '..lmmmmdmmmmmmmmmmmmmd',
    '.lmmmmmmdmmmmmmmmmmmdd',
    'lmmmmmmmmdmmmmmmmmmmd.',
    'lllmmmmmmmmmmmmmmmmdd.',
    'llmmmmmmmmmmmmmmmmmd..',
    'lmmmmmmmmmmmmmmmmmdd..',
    'mmmmmmmmmmmmmmmmmmd...',
    'dmmmmmmmmmmmmmmmmdd...',
    '.ddmmmmmmmmmmmmmdd....',
    '..XXXXXXXXXXXXXXX.....',
    '..xXXXXXXXXXXXXXx.....',
]
OGRE_LEG = [
    '.lmmmd',
    '.mmmmd',
    '.mmmmd',
    '.mmmmd',
    '.mmmmd',
    'lmmmmd',
    'mmmmmd',
]
OGRE_ARM_DOWN = [
    '.lmml.',
    'lmmmmd',
    'lmmmmd',
    'mmmmmd',
    'mmmmmd',
    '.mmmmd',
    '.mmmmd',
    '.mmmmd',
    '.lmmmd',
    '.mmmmd',
    'lmmmmd',
    'mmmmmd',
    '.dddd.',
]
OGRE_ARM_SMASH = [
    '...........lm',
    '.........lmmmd',
    '.......lmmmmd.',
    '.....lmmmmmd..',
    '...lmmmmmd....',
    '..mmmmmdd.....',
    '.lmmmd........',
    '.mmmd.........',
]


def club(c, x0, y0, x1, y1):
    """A wooden club from the grip (x0, y0) to its heavy end around (x1, y1)."""
    c.line(x0, y0, x1, y1, 'weapon', width=2)
    c.ellipse(x1, y1, 2.5, 2.5, 'weapon', dark='weapon_dark', light='light')
    c.px(x1, y1, 'weapon_dark')


def ogre(step, pose):
    """An ogre: a huge brute with a belly, a small tusked head and a wooden club."""
    c = Canvas()
    d = STRIDE[step]
    s = d // 2
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    u = -1 if attack else 0
    c.sticker(OGRE_LEG, 17 + d, 22, far=True)
    c.sticker(OGRE_BODY, 5 + u, 3 + b)
    c.poly([(9, 22 + b), (14, 22 + b), (13, 25 + b), (10, 25 + b)], 'extra', dark='extra_dark')    # loincloth
    c.sticker(OGRE_LEG, 9 - d, 22)
    c.sticker(OGRE_HEAD, 3 + u * 2, 2 + b)
    c.ellipse(19 + u, 7 + b, 3.5, 2.5, 'extra', dark='extra_dark', light='light')    # shoulder pad
    if attack:
        club(c, 5, 15 + b, 3, 24)                                                      # smashed down in front
        c.sticker(OGRE_ARM_SMASH, 4, 8 + b)
        for x, y in ((0, 28), (7, 28), (8, 26), (0, 20)):
            c.px(x, y, 'light')
    else:
        club(c, 21, 19 + b, 26 - s, 25)                                                # dragged behind
        c.sticker(OGRE_ARM_DOWN, 17, 7 + b, sway(-s, 13))
    return c.done()


ABOMINATION_BODY = [
    '........lllllll........',
    '......llmmmmmmmll......',
    '.....lmmmmmmmmmmml.....',
    '....lmmmmmmmmmXmmml....',
    '...lmmmmmmmmmXmXmmmd...',
    '..lmmmmmmmmmXmmmXmmmd..',
    '..mmmmmmmmmXmmmmmXmmd..',
    '.lmmmmmmmmXmmmmmmmmmmd.',
    '.mmmmmmmmXXXmmmmmmmmmd.',
    'lmmmmmmmmmXmmmXmXmXmmmd',
    'lmmmmmmmmXXXmXSSSSSXmmd',
    'lmmmmmmmmmXmmmSSSSSmmmd',
    'mmmmmmmmmXXXmXSsSSSXmmd',
    'mmmmmmmmmmXmmmSSSSSmmmd',
    'mmmmmmmmmXXXmmXmXmXmmdd',
    'dmmmmmmmmmXmmmmmmmmmmd.',
    'dmmmmmmmmXXXmmmmmmmmdd.',
    '.dmmmmmmmmmmmmmmmmmdd..',
    '..ddmmmmmmmmmmmmmddd...',
    '....dddddddddddddd.....',
]
ABOMINATION_HEAD = [
    '..lll..',
    '.lmmmd.',
    'lemmmmd',
    'mXXXmmd',
    '.mmmmd.',
]
ABOMINATION_ARM = [
    '.......lmml',
    '......lmmmmd',
    '.....lmmmmmd',
    '....lmmmmmd.',
    '...lmmmmmd..',
    '..lmmmmmd...',
    '..mmmmmd....',
    '.lmmmmd.....',
    '.mmmmmd.....',
    'lmmmmmd.....',
    'mmmmmmd.....',
    '.dddd.......',
]
ABOMINATION_ARM_SWING = [
    '.........lmm',
    '.lmmmmmmmmmmd',
    'lmmmmmmmmmmd.',
    'mmmmddddddd..',
    '.ddd.........',
]
ABOMINATION_FAR_ARM = [
    'SSS....',
    '.SSSS..',
    '...SSS.',
    '....SSs',
    '....SSs',
    '....SSs',
    '...SSs.',
    '...SSs.',
    '..SSSs.',
    '..T.T..',
]
MEAT_HOOK = [
    '...w',
    '...w',
    '...W',
    '...W',
    'W..W',
    'W..W',
    '.WW.',
]
MEAT_HOOK_SWUNG = [
    '.WW.',
    'W..W',
    'W..W',
    '...W',
    '...W',
    '...w',
    '...w',
]


def abomination(step, pose):
    """An abomination: a huge stitched-together flesh golem with a tiny head, a stitched-on patch of other
    flesh, a smaller mismatched arm and a meat hook."""
    c = Canvas()
    d = STRIDE[step]
    s = d // 2
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    u = -1 if attack else 0
    c.rect(18 + d, 23, 22 + d, 28, 'dark')                                              # far leg
    c.sticker(ABOMINATION_FAR_ARM, 24, 9 + b, sway(s, 10))                             # smaller, other flesh
    c.sticker(ABOMINATION_BODY, 5 + u, 4 + b)
    c.rect(10 - d, 24, 14 - d, 28, 'main', dark='dark')                                # near leg
    c.sticker(ABOMINATION_HEAD, 10 + u, 1 + b)
    if attack:
        c.sticker(MEAT_HOOK_SWUNG, 1, 1 + b)
        c.sticker(ABOMINATION_ARM_SWING, 3, 8 + b)
    else:
        c.sticker(MEAT_HOOK, 4 - s, 20 + b)
        c.sticker(ABOMINATION_ARM, 4, 8 + b, sway(-s, 12))
    return c.done()


def ridges(c, xs, key='second_dark'):
    """Spikes along a back: a pixel of key on top of the topmost colored pixel of each column."""
    for x in xs:
        if not 0 <= x < FRAME:
            continue
        col = c.img[:, x]
        rows = np.where((col != 0) & (col != C['outline']))[0]
        if len(rows) and rows[0] > 0:
            c.px(x, rows[0] - 1, key)


def teeth(c, xs, y0, y1, down=True, key='tooth'):
    """Teeth along a jaw: under the lowest (down) or over the highest colored pixel of each column between
    rows y0 and y1."""
    for x in xs:
        col = c.img[y0:y1 + 1, x]
        rows = np.where((col != 0) & (col != C['outline']))[0]
        if len(rows):
            c.px(x, y0 + (rows[-1] + 1 if down else rows[0] - 1), key)


def crocolisk(step, pose):
    """A crocolisk: a long, low crocodile with a flat snout, a ridged back and a heavy tail swaying behind."""
    c = Canvas()
    s = STRIDE[step] // 2
    t = [0, 1, 0, -1][step]
    attack = pose == 'attack'
    u = -1 if attack else 0
    lift = [(0, 0), (1, 0), (0, 0), (0, 1)][step]                                    # diagonal pairs step
    for x, up in ((13 - s, lift[1]), (24 + s, lift[0])):                             # far legs
        c.rect(x, 24, x + 1, 27 - up, 'dark')
        c.rect(x - 2, 28 - up, x + 1, 28 - up, 'dark')
    c.poly([(22, 18.5), (30.6, 23 + t), (30.6, 24.5 + t), (22, 26.6)], 'main', dark='dark', light='light')
    body = c.ellipse_mask(16.5 + u, 22.5, 8.5, 4)
    c.part(body, 'main', dark='dark', light='light')
    c.part(body & (c.ys >= 25), 'second', edge=False)                                # pale belly
    ridges(c, range(11 + u, 31, 2))
    for x, up in ((10 + s, lift[0]), (21 - s, lift[1])):                             # near legs
        c.rect(x, 24, x + 2, 27 - up, 'main', dark='dark')
        c.rect(x - 2, 28 - up, x + 2, 28 - up, 'main')
        c.px(x - 2, 28 - up, 'tooth')
    hx = 9 + u * 2
    if attack:
        c.ellipse(hx + 1, 21.5, 3, 2.5, 'main', light='light')
        c.poly([(hx + 1, 19), (hx + 1, 22.6), (2, 18.6), (2, 16)], 'main', light='light')  # jaws thrown open
        c.poly([(hx + 1, 23.4), (hx + 1, 25.6), (2, 26.6), (2, 25)], 'second', dark='second_dark')
        teeth(c, (3, 5, 7), 12, 22)
        teeth(c, (3, 5, 7), 24, 28, down=False)
    else:
        c.ellipse(hx + 1, 21.5, 3, 2.5, 'main', light='light')
        c.rect(1, 20, hx, 22, 'main', light='light')                                    # upper jaw
        c.rect(2, 24, hx, 24, 'second')                                                 # lower jaw
        c.line(2, 23, hx, 23, 'outline', edge=False)
        for x in (3, 5, 7):
            c.px(x, 23, 'tooth')
        c.px(1, 19, 'main')                                                             # nostril bump
    c.ellipse(hx + 1, 19, 1.5, 1.5, 'main', light='light')                              # eye bump
    c.px(hx + 1, 19, 'eye')
    return c.done()


RAPTOR_LEG = [
    '..lmml..',
    '.lmmmmd.',
    '.mmmmmd.',
    '.mmmmmd.',
    '..mmmd..',
    '...mmd..',
    '....mmd.',
    '....mmd.',
    '...mmd..',
    '..mmdT..',
    '.mmmd...',
    'TmmmmT..',
]


def raptor(step, pose):
    """A raptor: a lean two-legged hunter leaning forward over a long tail, with a feathered crest, striped
    hide, small clawed arms and sickle-clawed feet; it bites with a lunge and dies on its side, legs up."""
    c = Canvas()
    if pose == 'dead':
        legs = RAPTOR_LEG[::-1]
        c.sticker(legs, 18, 10, far=True)
        c.sticker(legs, 12, 11)
        for i in range(3):
            c.line(9 + i, 22 + i, 13 + i * 2, 19 + i * 2, 'second', edge=True)
        c.part(c.poly_mask([(21, 21.5), (30.6, 25), (30.6, 26.6), (21, 27.6)]) | c.ellipse_mask(16, 24.5, 7, 3.5) |
               c.ellipse_mask(8, 25.5, 3, 2.5) | c.rect_mask(2, 25, 7, 27) |
               c.poly_mask([(9, 23), (13, 22), (13, 27.6), (9, 27.6)]), 'main', dark='dark', light='light')
        c.line(3, 26, 6, 26, 'outline', edge=False)
        c.px(7, 24, 'outline')                                                           # eye shut
        return c.done()
    d = STRIDE[step]
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    hx, hy = (8, 11 + b) if attack else (8, 6 + b)
    c.sticker(RAPTOR_LEG, 18, 17, leg_stride(-d), far=True)
    for i in range(3):                                                                 # crest feathers
        c.line(hx + 1 + i, hy - 2 + i, hx + 5 + i * 2, hy - 5 + i * 2, 'second', edge=True)
    tail = c.poly_mask([(20, 10 + b), (30.6, 5 + b), (30.6, 7 + b), (22, 15 + b)])
    body = c.ellipse_mask(17, 14 + b, 6.5, 4)
    neck = c.poly_mask([(hx + 1, hy - 1), (hx + 3, hy - 2), (14, 11 + b), (13, 15 + b), (hx + 1, hy + 2)])
    skull = c.ellipse_mask(hx, hy, 3, 2.5)
    if attack:
        jaw = (c.poly_mask([(hx - 1, hy - 2), (hx - 7, hy - 4), (hx - 7, hy - 2), (hx, hy + 1)]) |
               c.poly_mask([(hx - 1, hy + 1), (hx - 6, hy + 3), (hx - 6, hy + 4.5), (hx + 1, hy + 2.5)]))
    else:
        jaw = c.rect_mask(hx - 6, hy - 1, hx - 1, hy + 1)
    c.part(tail | body | neck | skull | jaw, 'main', dark='dark', light='light')
    for x, y in ((14, 11), (18, 10), (22, 10), (26, 8)):                               # stripes
        c.line(x, y + b, x + 1, y + 2 + b, 'dark', edge=False)
    if attack:
        teeth(c, (hx - 6, hx - 3), hy - 6, hy)
        teeth(c, (hx - 5, hx - 2), hy + 1, hy + 6, down=False)
    else:
        c.line(hx - 5, hy + 1, hx - 1, hy + 1, 'outline', edge=False)                    # mouth
        c.px(hx - 3, hy + 2, 'tooth')
    c.px(hx - 1, hy - 1, 'eye')
    c.px(hx - 6, hy - 1, 'dark')                                                         # nostril
    c.sticker(RAPTOR_LEG, 16, 17, leg_stride(d), edge=2)
    arm_end = (hx - 1, hy + 5) if attack else (10, 17 + b)
    c.line(13, 14 + b, arm_end[0], arm_end[1], 'main', dark='dark')                    # small arm
    c.px(arm_end[0] - 1, arm_end[1], 'tooth')
    c.px(arm_end[0], arm_end[1] + 1, 'tooth')
    return c.done()


def tube(c, points, r0, r1):
    """A mask for a snake-like body: discs along the polyline through points, the radius going from r0 at
    the first point to r1 at the last."""
    mask = np.zeros((FRAME, FRAME), dtype=bool)
    segments = list(zip(points, points[1:]))
    lengths = [float(np.hypot(x1 - x0, y1 - y0)) for (x0, y0), (x1, y1) in segments]
    total = sum(lengths)
    done = 0.0
    for ((x0, y0), (x1, y1)), length in zip(segments, lengths):
        n = max(int(length * 2), 1)
        for i in range(n + 1):
            t = i / n
            r = r0 + (r1 - r0) * (done + length * t) / total
            mask |= c.ellipse_mask(x0 + (x1 - x0) * t, y0 + (y1 - y0) * t, r, r)
        done += length
    return mask


def naga(step, pose, caster=False):
    """A naga: a scaled torso and a finned head over a coiled snake tail that sways as it slithers;
    warriors carry a trident and thrust it, casters hold up glowing hands. It dies stretched out."""
    c = Canvas()
    if pose == 'dead':
        if not caster:
            c.line(5, 21, 23, 21, 'weapon')                                                # the dropped trident
            c.line(5, 19, 5, 23, 'weapon')
            for y in (19, 21, 23):
                c.line(2, y, 4, y, 'weapon')
        body = tube(c, [(9, 25.5), (16, 26), (23, 26), (27, 25), (29.5, 22.5)], 3, 0.8)
        c.part(body, 'main', dark='dark', light='light')
        c.part(body & (c.ys >= 27), 'second', edge=False)
        c.poly([(6, 21), (10, 18), (9, 23)], 'extra', dark='extra_dark')                 # head fin
        c.ellipse(5, 24.5, 3, 3, 'main', light='light', dark='dark')
        c.px(3, 24, 'outline')
        c.rect(9, 23, 12, 25, 'extra', dark='extra_dark', edge=False)                     # belt
        return c.done()
    w = [0, 1, 0, -1][step]
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    path = [(15.5, 15 + b), (14, 19 + b), (13 + w * 0.5, 23.5), (16.5, 26), (22 + w, 26), (26 + w, 24),
            (27.5 + w, 20.5), (26.5 + w * 2, 17.5)]
    body = tube(c, path, 3.2, 0.8)
    c.part(body, 'main', dark='dark', light='light')
    belly = tube(c, [(x - 1.5, y + 1) for x, y in path[:5]], 2.2, 1.6) & body
    c.part(belly & ~tube(c, path[:5], 2.4, 1.2), 'second', edge=False)                # pale underside
    for x, y in ((24 + w, 25), (27 + w, 21), (17, 25)):
        c.px(x, y, 'dark')                                                                 # scales
    if caster and attack:
        c.line(16, 11 + b, 11, 5 + b, 'dark', width=2)                                 # far arm raised
    elif attack:
        c.line(16, 11 + b, 10, 13 + b, 'dark', width=2)
    else:
        c.line(16, 11 + b, 12, 15 + b, 'dark', width=2)
    c.poly([(12, 10 + b), (19, 10 + b), (18, 17 + b), (13, 17 + b)], 'main', dark='dark', light='light')
    c.rect(12, 16 + b, 19, 17 + b, 'extra', dark='extra_dark')                       # belt
    hx, hy = 14, 6 + b
    c.poly([(15, 3 + b), (22, 1 + b), (20, 5 + b), (22, 8 + b), (16, 9 + b)], 'extra', dark='extra_dark')
    c.ellipse(hx, hy, 3, 3.5, 'main', light='light', dark='dark')
    c.poly([(15, 5 + b), (19, 3 + b), (18, 7 + b)], 'extra')                          # fin ear
    c.px(hx - 2, hy - 1, 'eye')
    c.px(hx - 3, hy + 1, 'outline')
    c.ellipse(16, 11 + b, 2, 1.5, 'extra', dark='extra_dark', light='light')         # shoulder
    if caster:
        if attack:
            c.line(15, 11 + b, 9, 4 + b, 'main', width=2, dark='dark')
            glow(c, 8, 3 + b, 2.5)
            for x, y in ((4, 1), (12, 1), (4, 6), (12, 6), (8, -1)):
                c.px(x, y + 2 + b, 'flame')
        else:
            c.line(15, 11 + b, 11, 14 + b, 'main', width=2, dark='dark')
            glow(c, 9, 14 + b, 2 if step in (1, 3) else 1.5)
        return c.done()
    if attack:
        c.line(21, 13 + b, 4, 13 + b, 'weapon')                                         # trident thrust
        c.line(4, 11 + b, 4, 15 + b, 'weapon')
        for y in (11, 13, 15):
            c.line(1, y + b, 3, y + b, 'weapon', edge=True)
        c.line(15, 11 + b, 9, 13 + b, 'main', width=2, dark='dark')
    else:
        c.line(9, 4 + b, 9, 27, 'weapon')                                                 # trident upright
        c.line(7, 4 + b, 11, 4 + b, 'weapon')
        for x in (7, 9, 11):
            c.line(x, 1 + b, x, 3 + b, 'weapon')
        c.line(15, 11 + b, 10, 14 + b, 'main', width=2, dark='dark')
    return c.done()


def turtle(step, pose):
    """A big snapping turtle: a high domed shell over stubby legs, the hook-beaked head out front; it snaps
    with the neck stretched out."""
    c = Canvas()
    s = STRIDE[step] // 2
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    for x in (11 - s, 23 + s):                                                       # far legs
        c.rect(x, 23, x + 2, 27, 'second_dark')
        c.rect(x - 1, 28, x + 2, 28, 'second_dark')
    c.poly([(26, 20), (30.6, 24), (26, 24.6)], 'second', dark='second_dark')        # tail
    dome = c.ellipse_mask(17, 23, 11.5, 11) & (c.ys <= 22)
    c.part(dome, 'main', light='light')
    for cx, cy in ((11, 18), (17, 16), (23, 18), (14, 13), (20, 13)):                # shell plates
        plate = c.ellipse_mask(cx, cy, 2.5, 2) & dome
        ring = neighbors(plate) & ~plate & dome
        c.img[ring] = C['dark']
    c.rect(6, 21, 28, 22, 'extra')                                                   # rim
    for x in range(8, 28, 3):
        c.px(x, 22, 'extra_dark')
    hx, hy = (5, 19) if attack else (6, 20 + b)
    c.part(c.rect_mask(hx + 1, hy - 2, 9, hy + 2) | c.ellipse_mask(hx, hy, 3.5, 3), 'second', dark='second_dark',
           light='light')
    if attack:
        c.poly([(hx - 1, hy - 3), (hx - 4.4, hy - 3), (hx - 4.4, hy - 1), (hx - 1, hy)], 'second', light='light')
        c.poly([(hx - 1, hy + 2), (hx - 4, hy + 3), (hx - 4, hy + 4), (hx, hy + 3.6)], 'second', dark='second_dark')
        c.px(hx - 4, hy - 1, 'tooth_dark')
    else:
        c.rect(hx - 4, hy - 1, hx - 3, hy + 1, 'second', light='light')                 # hooked beak
        c.px(hx - 4, hy + 1, 'tooth_dark')
        c.line(hx - 3, hy + 1, hx, hy + 1, 'outline', edge=False)
    c.px(hx - 1, hy - 1, 'eye')
    c.px(hx, hy - 1, 'outline')
    for x in (8 + s, 20 - s):                                                        # near legs
        c.rect(x, 23, x + 3, 27, 'second', dark='second_dark')
        c.rect(x - 1, 28, x + 3, 28, 'second')
        c.px(x - 1, 28, 'tooth')
    return c.done()


def hydra_head(c, hx, hy, attack, dead=False):
    """One of a hydra's heads facing left, its skull centered on (hx, hy), with a finned crest."""
    c.poly([(hx + 1, hy - 2), (hx + 6, hy - 4), (hx + 4, hy + 1)], 'extra', dark='extra_dark')
    skull = c.ellipse_mask(hx, hy, 3, 2.5)
    if attack:
        upper = c.poly_mask([(hx - 1, hy - 2.5), (hx - 6, hy - 4), (hx - 6, hy - 2), (hx, hy + 0.5)])
        c.part(skull | upper, 'main', light='light', dark='dark')
        c.part(c.poly_mask([(hx - 1, hy + 1), (hx - 5, hy + 2.5), (hx - 5, hy + 4), (hx + 1, hy + 2.5)]), 'second',
               dark='second_dark')
        c.px(hx - 5, hy - 1, 'tooth')
        c.px(hx - 3, hy, 'tooth')
        c.px(hx - 4, hy + 2, 'tooth')
    else:
        c.part(skull | c.rect_mask(hx - 5, hy - 1, hx - 1, hy + 1), 'main', light='light', dark='dark')
        c.line(hx - 4, hy + 1, hx - 1, hy + 1, 'outline', edge=False)
        c.px(hx - 3, hy + 2, 'tooth')
    if dead:
        c.line(hx - 1, hy - 1, hx, hy - 1, 'outline', edge=False)
    else:
        c.px(hx - 1, hy - 1, 'eye')
        c.px(hx, hy - 1, 'outline')


def hydra_neck(c, path):
    neck = tube(c, path, 2.2, 1.3)
    c.part(neck, 'main', dark='dark', light='light')
    c.part(neck & tube(c, [(x - 1.5, y + 1.5) for x, y in path], 1.5, 1) & ~tube(c, path, 1.4, 0.6), 'second',
           edge=False)


def hydra(step, pose):
    """A hydra (Aku'mai): a squat scaled body on stumpy legs with three long necks fanned out; the heads
    bob out of step as it walks and all lunge to bite. It dies with the necks sprawled on the ground."""
    c = Canvas()
    if pose == 'dead':
        c.rect(24, 17, 26, 20, 'dark')
        c.rect(18, 17, 20, 20, 'dark')                                                    # legs in the air
        body = c.ellipse_mask(21.5, 24, 8.5, 4.5) | c.poly_mask([(27, 22), (30.6, 26), (30.6, 27.6), (26, 27.6)])
        c.part(body, 'main', dark='dark', light='light')
        c.part(body & (c.ys <= 21) & (c.xs <= 27), 'second', edge=False)
        necks = [([(16, 21), (13, 18), (10, 17)], (7, 17)), ([(15, 24), (11, 23), (9, 23)], (6, 23)),
                 ([(17, 26), (14, 26.5)], (11, 26))]
        for path, (hx, hy) in necks:
            hydra_neck(c, path + [(hx + 1, hy)])
        for path, (hx, hy) in necks:
            hydra_head(c, hx, hy, False, dead=True)
        return c.done()
    s = STRIDE[step] // 2
    attack = pose == 'attack'
    c.rect(25 + s, 24, 27 + s, 28, 'dark')                                           # far legs
    c.rect(15 - s, 24, 17 - s, 28, 'dark')
    lunge = -1 if attack else 0
    necks = [([(22, 17), (23, 12), (22, 8)], (19, 6)),                                # fanned out, back first
             ([(18, 17), (16, 13), (14, 11)], (11, 9)),
             ([(15, 20), (12, 21), (11, 19)], (8, 17))]
    heads = []
    for k, (path, (hx, hy)) in enumerate(necks):
        dy = 1 if attack else [0, 1, 0, -1][(step + k) % 4]
        hx += lunge
        hy += dy
        path = path[:1] + [(x + lunge * i / 2, y + dy * i / 2) for i, (x, y) in enumerate(path) if i]
        heads.append((path + [(hx + 2, hy)], hx, hy))
    hydra_neck(c, heads[0][0])
    body = c.ellipse_mask(21.5, 21.5, 8.5, 6) | c.poly_mask([(27, 18), (30.6, 24), (30.6, 25.6), (26, 25.6)])
    c.part(body, 'main', dark='dark', light='light')
    c.part(body & (c.ys >= 25) & (c.xs <= 27), 'second', edge=False)
    ridges(c, range(24, 31, 2), 'extra')
    for path, _, _ in heads[1:]:
        hydra_neck(c, path)
    for _, hx, hy in heads:
        hydra_head(c, hx, hy, attack)
    c.rect(19 + s, 24, 22 + s, 28, 'main', dark='dark')                              # near legs
    c.rect(26 - s, 25, 28 - s, 28, 'main', dark='dark')
    return c.done()


def bone(c, x0, y0, x1, y1):
    """A big bone used as a club, knobbed at both ends."""
    c.line(x0, y0, x1, y1, 'weapon', width=2)
    for x, y in ((x0, y0), (x1, y1)):
        c.ellipse(x, y, 1.6, 1.6, 'weapon', dark='weapon_dark')


def trogg(step, pose):
    """A trogg: a hunched, stocky cave brute with stony skin and lumps along its back, a small head with a
    jutting jaw and long heavy arms; it swings a bone club and dies face down."""
    c = Canvas()
    if pose == 'dead':
        bone(c, 2, 21, 9, 19)
        for y in (24, 26):
            c.rect(23, y, 29, y + 1, 'dark' if y == 24 else 'main', dark='dark')        # legs
        c.ellipse(17, 24, 8, 4.5, 'main', dark='dark', light='light')
        for x, y, r in ((13, 20, 1.5), (18, 19.5, 2), (23, 21, 1.5)):
            c.ellipse(x, y, r, r, 'second', dark='second_dark')
        c.ellipse(7, 25.5, 3, 2.5, 'main', dark='dark', light='light')
        c.line(5, 25, 6, 25, 'outline', edge=False)                                    # eyes shut
        c.line(12, 26, 4, 28, 'main', width=2, dark='dark')                             # arm flung out
        return c.done()
    d = STRIDE[step] // 2
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    u = -1 if attack else 0
    c.rect(19 + d, 22, 22 + d, 27, 'dark')                                           # far leg
    c.rect(18 + d, 28, 22 + d, 28, 'dark')
    if not attack:
        c.line(22, 12 + b, 21, 18 + b, 'dark', width=3)                                # far arm
        c.line(21, 18 + b, 19 - d, 23 + b, 'dark', width=2)
    c.ellipse(19 + u, 15 + b, 7.5, 7, 'main', dark='dark', light='light')
    for x, y, r in ((16, 9, 1.5), (21, 9, 2), (25, 12, 1.5), (26, 17, 1.2)):        # stony lumps
        c.ellipse(x + u, y + b, r, r, 'second', dark='second_dark')
    c.poly([(14, 19 + b), (22, 19 + b), (21, 24 + b), (15, 24 + b)], 'extra', dark='extra_dark')
    c.rect(14 - d, 21, 17 - d, 27, 'main', dark='dark')                              # near leg
    c.rect(13 - d, 28, 17 - d, 28, 'main')
    hx, hy = (9, 16) if attack else (10, 14 + b)
    c.ellipse(hx, hy, 3, 3, 'main', dark='dark', light='light')
    c.rect(hx - 3, hy + 1, hx + 1, hy + 3, 'main', dark='dark')                       # jutting jaw
    c.px(hx - 3, hy, 'tooth')
    c.px(hx - 1, hy, 'tooth')
    c.line(hx - 3, hy - 2, hx, hy - 2, 'dark', edge=False)                             # brow
    c.px(hx - 2, hy - 1, 'eye')
    if attack:
        bone(c, 5, 11, 2, 22)                                                            # smashed down in front
        c.line(16, 10, 6, 10, 'main', width=3, dark='dark')
        c.ellipse(5, 11, 1.8, 1.8, 'main', dark='dark')
        for x, y in ((1, 28), (5, 28), (6, 26), (1, 25)):
            c.px(x, y, 'light')
    else:
        bone(c, 11, 23 + b, 6 + d, 27)                                                   # dragged along
        c.line(16, 11 + b, 14, 17 + b, 'main', width=3, dark='dark')
        c.line(14, 17 + b, 12, 22 + b, 'main', width=2, dark='dark')
        c.ellipse(11.5, 23 + b, 1.8, 1.8, 'main', dark='dark')                           # fist
    return c.done()


def ooze(step, pose):
    """An ooze: a blob of slime that squashes and stretches as it creeps, bubbles drifting inside and two
    eyes up front; it rears up to strike and dies as a puddle."""
    c = Canvas()
    if pose == 'dead':
        mask = c.ellipse_mask(16, 28.5, 14, 3) & (c.ys <= 28)
        c.part(mask, 'main', light='light')
        for x in (8, 15, 22):
            c.px(x, 27, 'second')
        return c.done()
    if pose == 'attack':
        mask = (c.ellipse_mask(18, 28.5, 10, 7) | c.ellipse_mask(14, 17, 5.5, 9) |
                c.ellipse_mask(9, 10, 5.5, 4)) & (c.ys <= 28)
        c.part(mask, 'main', dark='dark', light='light')
        for x, y in ((6, 14), (9, 15), (5, 15)):
            c.px(x, y, 'main')                                                           # drips
        bubbles = [(15, 18, 1.5), (19, 24, 1.2), (12, 10, 1)]
        eyes = [(6, 10), (9, 10)]
    else:
        w, h, x = [(11, 9, 16), (9, 12, 15), (11, 9, 15), (12.5, 8, 16)][step]
        mask = (c.ellipse_mask(x, 28.5, w, h) | c.ellipse_mask(x - w + 2, 27, 3, 2)) & (c.ys <= 28)
        c.part(mask, 'main', dark='dark', light='light')
        top = 28 - h
        bubbles = [(x + 3, int(top) + 5 + step % 2, 1.5), (x - 1, int(top) + 7 - step % 2, 1), (x + 6, 26, 1)]
        eyes = [(int(x - w) + 4, int(top) + 4), (int(x - w) + 7, int(top) + 3)]
    for bx, by, r in bubbles:
        ring = c.ellipse_mask(bx, by, r, r)
        c.img[ring & mask] = C['second']
        c.px(int(bx - r / 2), int(by - r / 2), 'light')
    for ex, ey in eyes:
        c.px(ex, ey, 'eye')
        c.px(ex, ey + 1, 'outline')
    return c.done()


def robot(step, pose):
    """A gnomish mechanical walker: a riveted box on two piston legs with a glowing lamp in front and a
    smoke stack, swinging a claw arm; it dies as a wreck slumped on the ground."""
    c = Canvas()
    if pose == 'dead':
        c.line(22, 26, 29, 21, 'second_dark', width=2)                                  # a leg sticking out
        c.rect(26, 19, 30, 20, 'second_dark')
        c.rect(7, 18, 24, 28, 'main', dark='dark', light='light')
        c.rect(19, 13, 21, 17, 'second', dark='second_dark')
        c.ellipse(11, 22, 2, 2, 'dark')                                                    # the lamp is out
        for x, y in ((9, 18), (22, 18), (9, 27), (22, 27)):
            c.px(x, y, 'extra')
        c.line(5, 26, 2, 27, 'second', width=2)
        c.rect(1, 28, 3, 28, 'weapon')
        for x, y in ((20, 10), (22, 8), (19, 6), (23, 4)):
            c.px(x, y, 'light')                                                           # smoke
        c.px(4, 20, 'flame')
        c.px(6, 17, 'flame')
        return c.done()
    d = STRIDE[step]
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'

    def leg(x, far):
        key, dark = ('second_dark', None) if far else ('second', 'second_dark')
        c.rect(x - 1, 19 + b, x + 2, 23, key, dark=dark)
        c.rect(x, 24, x + 1, 26, 'weapon_dark' if far else 'weapon')
        c.rect(x - 2, 27, x + 3, 28, key, dark=dark)
    leg(20 - d, True)
    c.rect(19, 3 + b, 21, 8 + b, 'second', dark='second_dark', light='light')       # smoke stack
    c.rect(18, 2 + b, 22, 3 + b, 'second_dark')
    c.rect(9, 8 + b, 24, 19 + b, 'main', dark='dark', light='light')
    c.line(17, 10 + b, 17, 17 + b, 'dark', edge=False)                                # panel seam
    for x, y in ((11, 9), (22, 9), (22, 17), (19, 17)):
        c.px(x, y + b, 'extra')
    c.ellipse(11.5, 12.5 + b, 2.5, 2.5, 'extra', dark='extra_dark')                   # lamp housing
    glow(c, 11.5, 12.5 + b, 1.5)
    leg(14 + d, False)
    c.ellipse(13, 16 + b, 1.5, 1.5, 'second', dark='second_dark')                    # shoulder
    if attack:
        c.line(12, 16 + b, 5, 16 + b, 'second', width=2, dark='second_dark')
        c.rect(4, 15 + b, 5, 17 + b, 'weapon_dark')                                       # claw wide open
        c.line(4, 15 + b, 2, 11 + b, 'weapon')
        c.line(2, 11 + b, 1, 13 + b, 'weapon')
        c.line(4, 17 + b, 2, 21 + b, 'weapon')
        c.line(2, 21 + b, 1, 19 + b, 'weapon')
        for x, y in ((1, 16), (3, 14), (2, 18)):
            c.px(x, y + b, 'flame')
    else:
        c.line(12, 16 + b, 7, 20 + b, 'second', width=2, dark='second_dark')
        c.line(6, 19 + b, 3, 18 + b, 'weapon')
        c.line(6, 22 + b, 3, 22 + b, 'weapon')
        c.px(2, 19 + b, 'weapon')
        c.px(2, 21 + b, 'weapon')
    return c.done()


def bomb(step, pose):
    """A walking bomb: a small round black bomb toddling on two tiny legs, its fuse fizzing on top; it hops
    with the fuse flaring to attack and dies tipped over with the fuse out."""
    c = Canvas()
    if pose == 'dead':
        c.line(19, 22, 22, 19, 'weapon')                                                   # legs in the air
        c.line(20, 25, 23, 23, 'weapon')
        c.ellipse(15, 24, 4.5, 4.5, 'main', dark='dark', light='light')
        c.rect(9, 23, 10, 25, 'second', dark='second_dark')
        c.line(8, 24, 6, 26, 'extra')
        return c.done()
    d = [0, 1, 0, -1][step]
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    lift = 2 if attack else 0
    c.line(14, 25 - lift, 13 + d, 27 - lift, 'weapon_dark')
    c.px(12 + d, 28 - lift, 'weapon_dark')
    c.ellipse(16, 21 + b - lift, 4.5, 4.5, 'main', dark='dark', light='light')
    c.px(14, 18 + b - lift, 'light')
    c.px(13, 19 + b - lift, 'light')
    c.rect(15, 16 + b - lift, 17, 16 + b - lift, 'second', dark='second_dark')
    c.line(17, 15 + b - lift, 18, 13 + b - lift, 'extra')
    c.line(18, 25 - lift, 19 - d, 27 - lift, 'weapon')
    c.px(18 - d, 28 - lift, 'weapon')
    fy = 12 + b - lift
    if attack:
        glow(c, 19, fy - 1, 2)
        for x, y in ((16, fy - 3), (22, fy - 3), (22, fy + 1), (19, fy - 5)):
            c.px(x, y, 'flame')
    else:
        c.px(19, fy, 'flame')
        c.px(19 + (step % 2), fy - 1, 'flame_dark')
        c.px(18 - (step % 2), fy - 1, 'flame')
    return c.done()


def paw(c, x, top, key, dark=None, width=3, claws=True):
    """A thick beast leg from row top down to the feet line, ending in a paw a pixel longer in front, with
    claws (tooth) at its toes."""
    c.rect(x, top, x + width - 1, 27, key, dark=dark)
    c.rect(x - 1, 28, x + width - 1, 28, key)
    if claws:
        c.px(x - 1, 28, 'tooth_dark' if key == 'dark' else 'tooth')


def bear(step, pose):
    """A bear: a heavy body on thick legs with a hump over the shoulders, a big round head carried low with
    a pale muzzle and small round ears; it rears up on its hind legs to swipe with a clawed forepaw."""
    c = Canvas()
    if pose == 'attack':
        paw(c, 19, 21, 'dark')                                                       # far hind leg
        c.line(13, 9, 4, 3, 'dark', width=3)                                          # far forepaw raised
        for x, y in ((1, 1), (1, 3), (2, 5)):
            c.line(x + 1, y, x, y, 'tooth_dark', edge=False)
        c.ellipse(26, 21, 1.5, 1.5, 'main', dark='dark')                              # tail
        body = tube(c, [(21, 21), (17, 15), (14, 10)], 6.5, 5)
        c.part(body, 'main', dark='dark', light='light')
        paw(c, 21, 21, 'main', dark='dark', width=4)                                   # near hind leg
        hx, hy = 9, 7
        c.ellipse(hx + 1, hy, 4.5, 4, 'main', dark='dark', light='light')
        c.ellipse(hx + 3, hy - 4, 1.6, 1.6, 'main', light='light')                    # ear
        c.px(hx + 3, hy - 4, 'dark')
        c.rect(hx - 5, hy - 1, hx - 2, hy, 'second', light='light')                   # snout, roaring
        c.rect(hx - 4, hy + 2, hx - 2, hy + 3, 'second', dark='second_dark')
        c.line(hx - 4, hy + 1, hx - 2, hy + 1, 'outline', edge=False)
        c.px(hx - 5, hy - 1, 'outline')                                               # nose
        c.px(hx - 4, hy, 'tooth')
        c.px(hx - 3, hy + 2, 'tooth')
        c.px(hx - 1, hy - 2, 'eye')
        c.line(14, 13, 6, 15, 'main', width=3, dark='dark')                           # near forepaw swiping
        c.ellipse(5, 15, 2, 2, 'main', dark='dark')
        for x, y in ((2, 13), (2, 15), (3, 17)):
            c.line(x + 1, y, x, y, 'tooth', edge=False)
        return c.done()
    s = STRIDE[step] // 2
    b = 1 if step in (1, 3) else 0
    paw(c, 20 - s, 22, 'dark')                                                       # far legs
    paw(c, 10 + s, 22, 'dark')
    c.ellipse(27.5, 15 + b, 1.5, 1.5, 'main', dark='dark')                           # tail stub
    body = c.ellipse_mask(18.5, 18 + b, 9, 6.5) | c.ellipse_mask(14, 14 + b, 6.5, 6)  # the hump over the shoulders
    c.part(body, 'main', dark='dark', light='light')
    for x in (12, 15, 18):
        c.px(x, 10 + b + (x % 2), 'light')                                           # fur on the hump
    paw(c, 22 + s, 22, 'main', dark='dark', width=4)                                 # near legs
    paw(c, 11 - s, 21, 'main', dark='dark', width=4)
    hx, hy = 6, 17 + b
    c.ellipse(hx + 1.5, hy, 4.5, 4.5, 'main', dark='dark', light='light')             # big round head
    c.ellipse(hx + 3.5, hy - 5, 1.6, 1.6, 'main', light='light')                      # ear
    c.px(hx + 3, hy - 4, 'dark')
    c.rect(hx - 4, hy, hx - 1, hy + 2, 'second', dark='second_dark', light='light')   # blunt muzzle
    c.rect(hx - 4, hy, hx - 4, hy, 'outline', edge=False)                             # nose
    c.px(hx, hy - 2, 'eye')
    return c.done()


def cat(step, pose, stripes=False):
    """A big cat (a mountain lion; panthers recolor it): a long, low body on slender legs, a small head with round
    ears, a pale belly and muzzle (second) and a long tail hanging in a curve with a dark tip. It pounces with the
    body stretched out, forepaws thrown ahead and claws bared. A tiger (stripes) has dark stripes (extra) across
    its back, flanks, head and tail."""
    if stripes:
        return tiger_stripes(cat(step, pose), step, pose)
    c = Canvas()
    if pose == 'attack':
        c.line(22, 22, 27, 25, 'dark', width=2)                                      # far hind leg pushing off
        c.line(27, 25, 30, 27, 'dark', width=2)
        c.line(10, 15, 3, 9, 'dark', width=2)                                        # far foreleg reaching
        tail = tube(c, [(24, 18), (28, 16), (31, 13)], 1.3, 0.9)
        c.part(tail, 'main', dark='dark')
        c.px(31, 12, 'dark')
        body = tube(c, [(23, 20), (17, 17.5), (11, 15)], 4, 4.2)
        c.part(body, 'main', dark='dark', light='light')
        c.part(body & tube(c, [(22, 23), (11, 18)], 2.2, 2) & ~tube(c, [(23, 19.5), (11, 14)], 3.6, 3.8), 'second',
               edge=False)                                                            # pale belly
        c.line(21, 22, 25, 26, 'main', width=2, dark='dark')                          # near hind leg
        c.line(25, 26, 28, 27, 'main', width=2, dark='dark')
        hx, hy = 6, 11
        c.ellipse(hx + 2, hy - 3, 1.3, 1.3, 'main', dark='dark')                      # ear
        c.ellipse(hx + 1, hy, 3.5, 3, 'main', dark='dark', light='light')
        c.rect(hx - 4, hy - 1, hx - 2, hy, 'second', light='light')                   # jaws wide open
        c.rect(hx - 3, hy + 2, hx - 1, hy + 3, 'second', dark='second_dark')
        c.line(hx - 3, hy + 1, hx - 1, hy + 1, 'outline', edge=False)
        c.px(hx - 4, hy - 1, 'outline')
        c.px(hx - 3, hy, 'tooth')
        c.px(hx - 2, hy + 2, 'tooth')
        c.px(hx, hy - 1, 'eye')
        c.line(12, 16, 4, 14, 'main', width=2, light='light')                         # near foreleg reaching
        for x, y in ((2, 12), (2, 14), (2, 16)):
            c.px(x, y, 'tooth')
        c.px(3, 8, 'tooth_dark')
        c.px(2, 10, 'tooth_dark')
        return c.done()
    s = STRIDE[step]
    b = 1 if step in (1, 3) else 0
    w = [0, 1, 0, -1][step]
    paw(c, 20 - s, 22, 'dark', width=2)                                              # far legs
    paw(c, 9 + s, 22, 'dark', width=2)
    tail = tube(c, [(23, 17 + b), (27, 19 + b), (29, 23 + w), (29.5 + w * 0.5, 26), (31, 25)], 1.3, 0.9)
    c.part(tail, 'main', dark='dark')
    c.part(tail & (c.ys >= 25), 'dark', edge=False)                                  # dark tail tip
    body = c.ellipse_mask(16.5, 19 + b, 9.5, 4) | c.ellipse_mask(10, 19 + b, 4.5, 4.5)
    c.part(body, 'main', dark='dark', light='light')
    c.part(body & (c.ys >= 21 + b), 'second', edge=False)                             # pale belly
    c.px(11, 15 + b, 'light')                                                        # shoulder blade
    c.px(12, 15 + b, 'light')
    paw(c, 22 + s, 22, 'main', dark='dark', width=2)                                 # near legs
    paw(c, 11 - s, 22, 'main', dark='dark', width=2)
    hx, hy = 5, 15 + b
    c.poly([(hx + 1, hy - 2), (hx + 2.5, hy - 5.5), (hx + 4.5, hy - 2)], 'main', light='light')   # ear
    c.ellipse(hx + 1.5, hy, 3.5, 3, 'main', dark='dark', light='light')               # head
    c.px(hx + 2, hy - 3, 'dark')
    c.rect(hx - 3, hy, hx - 1, hy + 1, 'second', edge=True)                           # muzzle
    c.px(hx - 3, hy, 'outline')                                                       # nose
    c.px(hx - 1, hy + 2, 'second_dark')                                               # chin
    c.px(hx, hy - 1, 'eye')
    return c.done()


def tiger_stripes(img, step, pose):
    """Paint stripes (extra) on a drawn cat: short bands down from its back over the fur (main, light and dark,
    never the pale belly), rings on the tail and a mark on the brow."""
    c = Canvas()
    c.img = img.copy()
    fur = (C['main'], C['light'])
    if pose == 'attack':
        bands = [((24, 16), (23, 19)), ((21, 15), (20, 18)), ((18, 14), (17, 17)), ((15, 13), (14, 16)),
                 ((12, 12), (12, 14)), ((27, 16), (27, 17)), ((29, 14), (30, 15))]
        brow = [(6, 9), (7, 9)]
    else:
        b = 1 if step in (1, 3) else 0
        bands = [((23, 15 + b), (22, 18 + b)), ((20, 15 + b), (19, 18 + b)), ((17, 15 + b), (16, 18 + b)),
                 ((14, 15 + b), (13, 17 + b)), ((28, 20 + b), (29, 20 + b)), ((28, 23), (30, 23))]
        brow = [(6, 13 + b), (7, 13 + b)]
    for (x0, y0), (x1, y1) in bands:
        band = c.line_mask(x0, y0, x1, y1)
        c.img[band & np.isin(c.img, fur)] = C['extra']
    for x, y in brow:
        if c.img[y, x] in fur:
            c.px(x, y, 'extra')
    return c.img


def shaggy(c, mask, phase=0, sides=False):
    """A fur fringe around a mask: on every other column one pixel more hangs below its lowest edge, and
    (sides) on every other row one pixel more sticks out to the left and right."""
    out = mask.copy()
    out[1:, :] |= mask[:-1, :] & ((c.xs[1:, :] + phase) % 2 == 0)
    if sides:
        rows = (c.ys + phase) % 2 == 0
        out[:, 1:] |= mask[:, :-1] & rows[:, 1:]
        out[:, :-1] |= mask[:, 1:] & rows[:, :-1]
    return out


YETI_HORN = ['.XX',
             'Xx.',
             'X..']


def yeti(step, pose):
    """A yeti: a broad, shaggy ape standing upright with hunched shoulders and long arms hanging to its
    knuckles, a bare face (second) under a fringe of fur and two small horns; it smashes both fists down
    in front."""
    c = Canvas()
    d = STRIDE[step] // 2
    b = 1 if step in (1, 3) else 0
    attack = pose == 'attack'
    c.rect(19 + d, 21, 23 + d, 27, 'dark')                                            # far leg
    c.rect(18 + d, 28, 23 + d, 28, 'second_dark')
    if attack:
        arm = shaggy(c, tube(c, [(19, 11 + b), (14, 17), (10, 24)], 2.6, 2.2), phase=1)
        c.part(arm, 'dark')                                                           # far arm smashing
        c.ellipse(9, 26, 2.5, 2.2, 'second_dark')
    else:
        arm = shaggy(c, tube(c, [(22, 11 + b), (23, 17 + b), (21 - d, 22 + b)], 2.6, 2), phase=1)
        c.part(arm, 'dark')                                                           # far arm
        c.ellipse(20.5 - d, 23.5 + b, 2, 2, 'second_dark')
    sx, sy = (13, 12 + b) if attack else (16, 10 + b)
    body = shaggy(c, c.ellipse_mask(18, 16 + b, 6.5, 7.5) | c.ellipse_mask(sx, sy, 6.5, 4.5), sides=True)
    c.part(body, 'main', dark='dark', light='light')
    for x, y in ((21, 13), (19, 17), (22, 19), (17, 21)):
        c.line(x, y + b, x + 1, y + 1 + b, 'dark', edge=False)                        # shaggy locks
    c.rect(13 - d, 21, 17 - d, 27, 'main', dark='dark')                               # near leg
    c.rect(12 - d, 28, 17 - d, 28, 'second', dark='second_dark')
    hx, hy = (8, 13 + b) if attack else (10, 7 + b)
    c.sticker(YETI_HORN, hx + 3, hy - 6, far=True)                                    # far horn
    head = shaggy(c, c.ellipse_mask(hx + 0.5, hy, 4.5, 4.5), phase=1)
    c.part(head, 'main', dark='dark', light='light')
    c.sticker(YETI_HORN, hx - 1, hy - 6)                                              # near horn
    face = c.ellipse_mask(hx + 0.5, hy, 4.5, 4.5) & (c.xs <= hx) & (c.ys >= hy - 1)
    c.part(face, 'second', dark='second_dark', edge=False)                            # bare face
    c.line(hx - 4, hy - 2, hx, hy - 2, 'light', edge=False)                           # fringe of fur
    c.px(hx - 2, hy - 1, 'eye')
    if attack:
        c.rect(hx - 3, hy + 2, hx - 1, hy + 3, 'outline', edge=False)                 # roaring
        c.px(hx - 3, hy + 2, 'tooth')
        c.px(hx - 1, hy + 2, 'tooth')
        for x, y in ((0, 28), (11, 28), (12, 26), (0, 23), (2, 21), (9, 21)):
            c.px(x, y, 'light')                                                       # snow thrown up
        arm = shaggy(c, tube(c, [(15, 13 + b), (10, 19), (5, 24)], 2.6, 2.2), phase=1)
        c.part(arm, 'main', dark='dark', light='light')                               # near arm smashing
        c.ellipse(4, 26, 2.5, 2.2, 'second', dark='second_dark')
        return c.done()
    arm = shaggy(c, tube(c, [(14, 11 + b), (12, 17 + b), (10 + d, 22 + b)], 2.8, 2.2), phase=1)
    c.part(arm, 'main', dark='dark', light='light')                                   # near arm
    c.ellipse(9.5 + d, 23.5 + b, 2.2, 2, 'second', dark='second_dark')                # fist
    return c.done()


GORILLA_HEAD = [
    '...llm..',
    '..lmmmml',
    '.lmmmmmd',
    'ddddmmmd',
    'SeSSmmmd',
    'SSSSSmmd',
    'oSSSSmd.',
    '.ssSmmd.',
    '..ssdd..',
]
GORILLA_ROAR = [
    '...llm..',
    '..lmmmml',
    '.lmmmmmd',
    'ddddmmmd',
    'SeSSmmmd',
    'SSSSSmmd',
    'ooooSmd.',
    'TToSSmd.',
    '.ooSmmd.',
    'TSSsdd..',
    '.ss.....',
]
GORILLA_BODY = [
    '....llllll...........',
    '..llllllllll.........',
    '.lllmmmmmmmlll.......',
    'llmmmmmmmmmmmmll.....',
    'lmmmmmmmmmmmmmmmll...',
    'mmmmmmmmmmmmmmmmmmml.',
    'mmmmmmmmmmmmmmmmmmmmd',
    'mmmmmmmmmmmmmmmmmmmmd',
    'SSmmmmmmmmmmmmmmmmmmd',
    'SSSmmmmmmmmmmmmmmmmmd',
    'sSSmmmmmmmmmmmmmmmmdd',
    '.sSSmmmmmmmmmmmmmmdd.',
    '..ssdddmmmmmmmmmddd..',
    '.......ddddddddd.....',
]
GORILLA_BODY_UPRIGHT = [
    '...llllll.....',
    '.llllllllll...',
    'lllmmmmmmmll..',
    'lmmmmmmmmmmml.',
    'mmmmmmmmmmmmmd',
    'SSSmmmmmmmmmmd',
    'SSSSmmmmmmmmmd',
    'SSSSmmmmmmmmmd',
    'SSSSmmmmmmmmmd',
    'sSSSmmmmmmmmmd',
    '.ssmmmmmmmmmd.',
    '..mmmmmmmmmmd.',
    '..dmmmmmmmmdd.',
    '...dddddddd...',
]
GORILLA_ARM = [
    '.lmmmd.',
    'lmmmmmd',
    'mmmmmmd',
    'mmmmmmd',
    'mmmmmmd',
    '.mmmmmd',
    '.mmmmd.',
    '.lmmmd.',
    '.mmmmd.',
    '.mmmmd.',
    '.mmmmd.',
    'lmmmmd.',
    'mmmmmd.',
    'mmmmmd.',
    'mmmmdd.',
    'SSSSs..',
    'SSSSs..',
    'ssss...',
]
GORILLA_ARM_RAISED = [
    '.SSS...',
    'SSSSs..',
    'sSSSs..',
    'lmmmd..',
    'lmmmmd.',
    '.lmmmd.',
    '.lmmmmd',
    '..mmmmd',
    '..lmmmd',
    '..mmmmd',
    '..mmmmd',
    '.mmmmmd',
]
GORILLA_LEG = [
    'lmmmmd',
    'mmmmmd',
    'mmmmd.',
    '.mmmd.',
    '.mmmd.',
    '.mmmd.',
    'smmmdd',
    'SSSSSs',
]
GORILLA_DEAD = [
    '.....llllllll...........',
    '...lllllllllllll........',
    '..lmmmmmmmmmmmmmlll.....',
    '.lmmmmmmmmmmmmmmmmmll...',
    'lmmmmmmmmmmmmmmmmmmmmld.',
    'mmmmmmmmmmmmmmmmmmmmmmmd',
    'mmmmmmmmmmmmmmmmmmddmmmd',
    'dddddddddddddddddd.ddddd',
]
GORILLA_ARM_FLUNG = [
    '....lllllll.',
    'SSSSmmmmmmmd',
    'SSSsmmmmmddd',
    'sss.dddd....',
]


def gorilla(step, pose):
    """A gorilla: huge shoulders and long, thick arms over short legs, walking on its knuckles with the back
    sloping down from the shoulders and a small head with a heavy brow carried low in front; the chest, face,
    knuckles and soles are bare (second), the back a lighter saddle (light). It rears up on its legs with both
    fists raised over its head to smash down, roaring, and dies sprawled on its belly."""
    c = Canvas()
    if pose == 'dead':
        c.sticker(GORILLA_DEAD, 7, 21)
        c.sticker(['SSs', 'sSS'], 29, 26)                                                  # a sole turned up
        c.sticker(GORILLA_ARM_FLUNG, 0, 25)
        c.sticker([row.replace('e', 'S') for row in GORILLA_HEAD], 4, 20)                 # head on the arm
        c.line(4, 24, 5, 24, 'outline', edge=False)                                       # eye shut
        return c.done()
    if pose == 'attack':
        c.sticker(GORILLA_LEG, 21, 21, far=True)
        c.sticker(GORILLA_ARM_RAISED, 18, 1, far=True)                                    # both fists raised
        c.sticker(GORILLA_BODY_UPRIGHT, 12, 9)
        c.sticker(GORILLA_LEG, 15, 21, edge=2)
        c.sticker(GORILLA_ARM_RAISED, 12, 2, edge=False)
        c.sticker(GORILLA_ROAR, 5, 8)
        for x, y in ((9, 2), (7, 5), (27, 2), (28, 5)):
            c.px(x, y, 'light')                                                           # fury
        return c.done()
    d = STRIDE[step]
    b = 1 if step in (1, 3) else 0
    c.sticker(GORILLA_LEG, 23, 21, leg_stride(-d // 2, 8, 2), far=True)                 # far leg
    c.sticker(GORILLA_ARM[b:], 14, 11 + b, sway(d, 18 - b), far=True)                   # far arm
    c.sticker(GORILLA_BODY, 8, 6 + b)
    c.sticker(GORILLA_LEG, 19, 21, leg_stride(d // 2, 8, 2), edge=2)                     # near leg
    c.sticker(GORILLA_ARM[b:], 9, 11 + b, sway(-d, 18 - b), edge=3)                     # near arm
    c.sticker(GORILLA_HEAD, 3, 9 + b)
    return c.done()


def wisps(img, region, keep=0):
    """Thin out the pixels of img inside region in a checker pattern, so that part of a spirit fades into
    the air: the outline goes and every other colored pixel with it."""
    ys, xs = np.mgrid[0:FRAME, 0:FRAME]
    img[region & (img == C['outline'])] = 0
    img[region & ((xs + ys) % 2 == keep)] = 0
    return img


def ghost_claws(c, x, y, key, fan):
    """Long fingers fanning out ahead (to the left) of a spirit's hand at (x, y); fan lists where each
    fingertip ends up or down."""
    for dy in fan:
        c.line(x, y, x - 3, y + dy, key, edge=True)


def spirit(step, pose):
    """A spirit: a hooded, wispy ghost floating over the ground on a trailing tail that fades into the air,
    glowing eyes in the dark of the hood and clawed hands reaching ahead. It bobs instead of walking, the
    tail swaying; it lunges with the claws spread, and dies as a fading wisp on the ground."""
    c = Canvas()
    ys, xs = c.ys, c.xs
    if pose == 'dead':
        robe = c.ellipse_mask(15, 28, 8.5, 2.6) & (ys <= 28)
        c.part(robe, 'main', dark='dark', light='light')                              # the robe, flat on the ground
        c.part(robe & (ys == 27) & (xs >= 12) & (xs <= 19), 'second', edge=False)
        c.ellipse(8.5, 25.5, 4.5, 3.4, 'main', dark='dark', light='light')             # the hood, empty
        c.ellipse(6, 25.5, 2, 2, 'outline', edge=False)
        c.px(5, 25, 'second_dark')                                                    # a last, dim glint
        mist = tube(c, [(22, 27.5), (26, 27), (30, 26)], 1.6, 1) & (ys <= 28)
        c.part(mist, 'main', light='light')
        rising = (tube(c, [(11, 20), (12, 17), (11, 15), (12, 13)], 0.8, 0.5) |
                  tube(c, [(16, 23), (17, 20), (16, 18)], 0.8, 0.5) | tube(c, [(21, 24), (22, 22), (21, 20)], 0.8, 0.5))
        c.part(rising, 'second', edge=False)                                          # wisps rising
        img = c.done()
        return wisps(img, neighbors(rising) | rising | (xs >= 23))
    bob = [0, -1, -2, -1][step]
    w = [0, 1, 0, -1][step]
    attack = pose == 'attack'
    u = -2 if attack else 0
    y0 = 2 + bob
    if attack:
        path = [(15 + u, y0 + 11), (16, y0 + 15), (20, y0 + 18), (24, y0 + 19), (28, y0 + 18), (31, y0 + 15)]
    else:
        path = [(15, y0 + 11), (16, y0 + 16), (19, y0 + 20), (23, y0 + 22 + w), (27, y0 + 21 + w * 2),
                (30, y0 + 18 + w * 2)]
    tail = tube(c, path, 5.5, 0.8)
    if attack:
        c.line(16 + u, y0 + 10, 7 + u, y0 + 9, 'dark', width=2)                      # far arm thrust out
        ghost_claws(c, 6 + u, y0 + 9, 'second_dark', (-3, -1, 1))
    else:
        c.line(16, y0 + 11, 9, y0 + 14 + w, 'dark', width=2)                          # far arm reaching
    c.part(tail, 'main', dark='dark', light='light')
    glow_path = [(x - 1.5, y + 2.5) for x, y in path[:4]]
    c.part(tail & tube(c, glow_path, 2, 1) & ~tube(c, path[:4], 3.5, 1), 'second', edge=False)
    hood = (c.ellipse_mask(13 + u, y0 + 6, 5, 5) |
            c.poly_mask([(14 + u, y0 + 1), (22 + u, y0 + 3 + w), (18 + u, y0 + 10)]))
    c.part(hood, 'main', dark='dark', light='light')
    c.ellipse(11 + u, y0 + 7, 2.5, 3.5, 'outline', edge=False)                       # the dark inside the hood
    for x in (10, 12):
        c.px(x + u, y0 + 6, 'eye')
        if attack:
            c.px(x + u, y0 + 7, 'eye')                                                # the eyes flare
    c.line(13 + u, y0 + 2, 15 + u, y0 + 10, 'dark', edge=False)                      # hood edge
    if attack:
        c.line(15 + u, y0 + 12, 8 + u, y0 + 13, 'main', width=2, light='light')       # near arm lunging
        ghost_claws(c, 7 + u, y0 + 13, 'second', (-1, 1, 3))
    else:
        c.line(15, y0 + 12, 7, y0 + 15 - w, 'main', width=2, light='light')           # near arm reaching
        ghost_claws(c, 6, y0 + 15 - w, 'second', (-1, 1, 3))
    img = c.done()
    return wisps(img, (xs >= 26) | (ys >= y0 + 23), keep=step % 2)


CREATURES = {
    'water_elemental': water_elemental,
    'wolf': wolf,
    'boar': boar,
    'spider': spider,
    'kobold': lambda step, pose: small_humanoid(step, pose, 'kobold'),
    'kobold_candle': lambda step, pose: kobold_candle(small_humanoid(step, pose, 'kobold'), step, pose),
    'murloc': lambda step, pose: small_humanoid(step, pose, 'murloc'),
    'goblin': lambda step, pose: small_humanoid(step, pose, 'goblin'),
    'gnoll': gnoll,
    'watcher': watcher,
    'worgen': worgen,
    'worgen_caster': lambda step, pose: worgen(step, pose, caster=True),
    'skeleton': skeleton,
    'skeleton_mage': lambda step, pose: skeleton(step, pose, robe=True),
    'ghoul': ghoul,
    'ogre': ogre,
    'abomination': abomination,
    'crocolisk': crocolisk,
    'raptor': raptor,
    'naga': naga,
    'naga_caster': lambda step, pose: naga(step, pose, caster=True),
    'turtle': turtle,
    'hydra': hydra,
    'trogg': trogg,
    'ooze': ooze,
    'robot': robot,
    'bomb': bomb,
    'bear': bear,
    'cat': cat,
    'yeti': yeti,
    'spirit': spirit,
    'gorilla': gorilla,
    'tiger': lambda step, pose: cat(step, pose, stripes=True),         # the cat's own sheet stays unstriped
}


# Upright creatures fall on their back instead of turning over like beasts.
LYING_DEAD = {'worgen', 'worgen_caster', 'skeleton', 'skeleton_mage', 'yeti'}

# Creatures that draw their own dead frame (pose 'dead'): an ooze dies as a puddle, a robot as a wreck, a spirit
# as a fading wisp, a gorilla sprawled on its belly.
OWN_DEAD = {'raptor', 'naga', 'naga_caster', 'hydra', 'trogg', 'ooze', 'robot', 'bomb', 'spirit', 'gorilla'}


def creature_sheet(draw, lying=False, own_dead=False):
    frames = [draw(step, 'walk') for step in range(4)]
    frames.append(draw(0, 'attack'))
    stand = frames[0]
    if own_dead:
        frames.append(draw(0, 'dead'))
        return frames
    if lying:
        frames.append(dead_frame(stand))
        return frames
    dead = np.zeros_like(stand)
    flipped = stand[::-1, :]
    rows = np.where(flipped.any(axis=1))[0]
    part = flipped[rows[0]:rows[-1] + 1, :]
    dead[FEET_Y + 1 - part.shape[0]:FEET_Y + 1, :] = part
    frames.append(dead)
    return frames


def creature_sheet_options(name):
    return {'lying': name in LYING_DEAD, 'own_dead': name in OWN_DEAD}


# --- mounts -----------------------------------------------------------------------------------------

def mount_legs(c, xs, top, bottom, key, dark=None, hoof='second_dark'):
    for x in xs:
        c.rect(x, top, x + 1, bottom - 1, key, dark=dark)
        c.rect(x, bottom, x + 1, bottom, hoof, edge=False)


def horse(view, step):
    c = Canvas()
    if view == 'side':
        s = STRIDE[step]
        b = 1 if step in (1, 3) else 0
        c.line(25, 13 + b, 29, 21 + b, 'second', width=2, dark='second_dark')          # tail
        mount_legs(c, [9 - s, 21 + s], 21, 28, 'dark')                                  # far legs
        c.ellipse(16, 17 + b, 10, 5, 'main', dark='dark', light='light')                # body
        c.rect(15, 12 + b, 22, 17 + b, 'extra', dark='extra_dark', light='flame')       # blanket
        mount_legs(c, [11 + s, 19 - s], 21, 28, 'main', dark='dark')                    # near legs
        c.poly([(4, 6 + b), (9, 4 + b), (13, 14 + b), (8, 17 + b)], 'main', dark='dark', light='light')
        c.ellipse(5, 7 + b, 3, 2.5, 'main', light='light')                              # head
        c.rect(1, 7 + b, 4, 10 + b, 'main', dark='dark')                                # muzzle
        c.line(8, 3 + b, 12, 12 + b, 'second', width=2, edge=False)                     # mane
        c.poly([(6, 4 + b), (7, 0 + b), (9, 4 + b)], 'second_dark')                     # ear
        c.px(4, 6 + b, 'eye')
        c.px(1, 9 + b, 'outline')
        return c.done()
    b = step
    if view == 'down':
        mount_legs(c, [10, 20], 22, 28 - b, 'dark')
        c.ellipse(15.5, 17, 7, 5, 'main', dark='dark', light='light')                  # chest
        c.rect(7, 14, 9, 20, 'extra', dark='extra_dark', light='flame')                 # blanket sides
        c.rect(22, 14, 24, 20, 'extra', dark='extra_dark', light='flame')
        c.ellipse(15.5, 23 + b, 3, 5, 'main', dark='dark', light='light')              # long face
        c.rect(14, 18 + b, 17, 27 + b, 'tooth', edge=False)                            # blaze
        c.rect(15, 18 + b, 16, 26 + b, 'main', edge=False)
        c.poly([(12, 19 + b), (12, 15 + b), (14, 18 + b)], 'second_dark')              # ears
        c.poly([(19, 19 + b), (19, 15 + b), (17, 18 + b)], 'second_dark')
        c.px(13, 21 + b, 'eye')
        c.px(18, 21 + b, 'eye')
        c.px(14, 27 + b, 'outline')
        c.px(17, 27 + b, 'outline')
        return c.done()
    # Seen from behind, the head hides behind the rider.
    mount_legs(c, [10, 20], 22, 28 - b, 'dark')
    c.ellipse(15.5, 19, 7.5, 6, 'main', dark='dark', light='light')
    c.rect(7, 14, 9, 20, 'extra', dark='extra_dark', light='flame')
    c.rect(22, 14, 24, 20, 'extra', dark='extra_dark', light='flame')
    c.line(15.5, 19, 15.5 + (1 if b else 0), 27, 'second', width=3, dark='second_dark')
    return c.done()


def ram(view, step):
    c = Canvas()
    if view == 'side':
        s = STRIDE[step] // 2 * 2
        b = 1 if step in (1, 3) else 0
        c.ellipse(27, 14 + b, 2, 2, 'light', dark='main')                              # tail
        mount_legs(c, [9 - s, 21 + s], 22, 28, 'dark')
        c.ellipse(16, 17 + b, 11, 6, 'main', dark='dark', light='light')                # woolly body
        for x in range(8, 26, 3):
            c.px(x, 13 + b + (x % 2), 'dark')
            c.px(x + 1, 20 + b - (x % 2), 'dark')
        c.rect(15, 11 + b, 22, 16 + b, 'extra', dark='extra_dark', light='flame')       # blanket
        mount_legs(c, [11 + s, 19 - s], 22, 28, 'second', dark='second_dark')
        c.poly([(3, 11 + b), (8, 9 + b), (11, 16 + b), (6, 18 + b)], 'main', dark='dark', light='light')
        c.ellipse(5, 12 + b, 3.5, 3, 'second', light='light', dark='second_dark')       # face
        c.rect(1, 12 + b, 4, 15 + b, 'second', dark='second_dark')                     # muzzle
        c.ellipse(8, 11 + b, 2.5, 2.5, 'weapon', dark='weapon_dark')                    # curled horn
        c.px(8, 11 + b, 'weapon_dark')
        c.px(9, 12 + b, 'weapon_dark')
        c.rect(3, 16 + b, 4, 18 + b, 'light', edge=False)                               # beard
        c.px(4, 11 + b, 'eye')
        c.px(1, 14 + b, 'outline')
        return c.done()
    b = step
    if view == 'down':
        mount_legs(c, [10, 20], 23, 28 - b, 'second', dark='second_dark')
        c.ellipse(15.5, 18, 8, 5.5, 'main', dark='dark', light='light')
        c.rect(6, 15, 8, 20, 'extra', dark='extra_dark', light='flame')
        c.rect(23, 15, 25, 20, 'extra', dark='extra_dark', light='flame')
        c.ellipse(15.5, 23 + b, 3.5, 4, 'second', dark='second_dark', light='light')
        c.ellipse(10.5, 20 + b, 2.5, 2.5, 'weapon', dark='weapon_dark')                # horns
        c.ellipse(20.5, 20 + b, 2.5, 2.5, 'weapon', dark='weapon_dark')
        c.px(10, 20 + b, 'weapon_dark')
        c.px(21, 20 + b, 'weapon_dark')
        c.px(14, 22 + b, 'eye')
        c.px(17, 22 + b, 'eye')
        c.rect(14, 26 + b, 17, 27 + b, 'light', edge=False)
        return c.done()
    mount_legs(c, [10, 20], 23, 28 - b, 'second', dark='second_dark')
    c.ellipse(15.5, 19, 8.5, 6.5, 'main', dark='dark', light='light')
    c.rect(6, 15, 8, 20, 'extra', dark='extra_dark', light='flame')
    c.rect(23, 15, 25, 20, 'extra', dark='extra_dark', light='flame')
    c.ellipse(15.5, 22 + b, 2, 2, 'light', dark='main')
    return c.done()


def saber(view, step):
    c = Canvas()
    if view == 'side':
        s = STRIDE[step]
        b = 1 if step in (1, 3) else 0
        c.line(25, 17 + b, 29, 10 + b, 'main', width=2, dark='dark')                    # tail
        c.px(29, 9 + b, 'second_dark')
        mount_legs(c, [8 - s, 22 + s], 22, 28, 'dark', hoof='dark')
        c.ellipse(16, 18 + b, 10, 4.5, 'main', dark='dark', light='light')              # long body
        for x in (10, 13, 22, 25):
            c.line(x, 14 + b, x - 1, 17 + b, 'second', edge=False)                      # stripes
        c.rect(15, 13 + b, 21, 17 + b, 'extra', dark='extra_dark', light='flame')       # saddle
        mount_legs(c, [10 + s, 20 - s], 22, 28, 'main', dark='dark', hoof='dark')
        c.ellipse(6, 13 + b, 4.5, 4, 'main', light='light', dark='dark')                # head
        c.rect(1, 14 + b, 4, 16 + b, 'light', dark='main')                              # muzzle
        c.poly([(6, 10 + b), (7, 6 + b), (9, 10 + b)], 'second_dark')                   # ear
        c.px(4, 12 + b, 'eye')
        c.px(1, 15 + b, 'outline')
        return c.done()
    b = step
    if view == 'down':
        mount_legs(c, [10, 20], 23, 28 - b, 'main', dark='dark', hoof='dark')
        c.ellipse(15.5, 18, 7, 5, 'main', dark='dark', light='light')
        c.rect(7, 15, 9, 20, 'extra', dark='extra_dark', light='flame')
        c.rect(22, 15, 24, 20, 'extra', dark='extra_dark', light='flame')
        c.ellipse(15.5, 23 + b, 4.5, 4, 'main', dark='dark', light='light')             # face
        c.poly([(11, 21 + b), (11, 17 + b), (14, 20 + b)], 'second_dark')               # ears
        c.poly([(20, 21 + b), (20, 17 + b), (17, 20 + b)], 'second_dark')
        c.line(15.5, 19 + b, 15.5, 21 + b, 'second', edge=False)
        c.px(13, 22 + b, 'eye')
        c.px(18, 22 + b, 'eye')
        c.rect(14, 25 + b, 17, 26 + b, 'light', edge=False)
        c.px(15, 25 + b, 'outline')
        return c.done()
    mount_legs(c, [10, 20], 23, 28 - b, 'main', dark='dark', hoof='dark')
    c.ellipse(15.5, 19, 7.5, 6, 'main', dark='dark', light='light')
    for y in (15, 18, 21):
        c.line(11, y, 20, y, 'second', edge=False)
    c.rect(7, 14, 9, 20, 'extra', dark='extra_dark', light='flame')
    c.rect(22, 14, 24, 20, 'extra', dark='extra_dark', light='flame')
    c.line(15.5, 22, 15.5 + (2 if b else -2), 29, 'main', width=2, dark='dark')         # tail
    return c.done()


MOUNT_VIEWS = [('side', step) for step in range(4)] + [('down', 0), ('down', 1), ('up', 0), ('up', 1)]


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
                     weapon=(120, 120, 128), tooth=(240, 236, 216), dark=None, light=None, flame_dark=None):
    def shade(c, f):
        return tuple(max(0, min(255, int(v * f))) for v in c)
    return [
        (255, 0, 255), (24, 20, 28), dark or shade(main, 0.62), main, light or shade(main, 1.3),
        shade(second, 0.7), second, eye, shade(tooth, 0.7), tooth, extra, shade(extra, 0.6),
        flame, flame_dark or (232, 128, 40), shade(weapon, 0.6), weapon,
    ]


SKIN = (232, 168, 128)
SKIN_TAN = (200, 136, 96)
SKIN_DARK = (152, 96, 64)
SKIN_ELF = (176, 152, 224)
SKIN_DWARF = (232, 160, 128)
SKIN_ORC = (104, 152, 72)
TUSK = (232, 224, 200)


SKIN_GNOME = (240, 184, 152)
SKIN_DARK_IRON = (140, 140, 160)
SKIN_FORSAKEN = (136, 152, 128)
SKIN_TROLL = (88, 140, 168)
SKIN_GOBLIN = (120, 176, 72)
GOBLIN_TEETH = (240, 232, 200)


def orc_palette(armor, tabard, hair=(40, 32, 32), leather=(96, 64, 40), skin=SKIN_ORC, trim=TUSK):
    return humanoid_palette(skin=skin, hair=hair, armor=armor, tabard=tabard, trim=trim, leather=leather,
                            lower_face=(skin[0] - 8, skin[1] - 8, skin[2]))


def dwarf_palette(hair, armor, tabard, trim=(232, 184, 64), skin=SKIN_DWARF, beard=None, **kw):
    """A dwarf: the beard is the lower face, the hair color unless it is given."""
    return humanoid_palette(skin=skin, hair=hair, armor=armor, tabard=tabard, trim=trim,
                            lower_face=beard or hair, **kw)


def gnome_palette(hair, armor, tabard, trim=(232, 184, 64), skin=SKIN_GNOME, beard=None, **kw):
    """A gnome: the trim also colors the goggles pushed up on the forehead, and beard (the chin) defaults
    to the skin, so only old gnomes have one."""
    return humanoid_palette(skin=skin, hair=hair, armor=armor, tabard=tabard, trim=trim,
                            lower_face=beard or (skin[0], skin[1], min(255, skin[2] + 8)), **kw)


def troll_palette(hair, armor, tabard, trim=TUSK, skin=SKIN_TROLL, **kw):
    """A troll: the mohawk is the hair, the tusks the trim and the long jaw the lower face, a shade off the skin."""
    return humanoid_palette(skin=skin, hair=hair, armor=armor, tabard=tabard, trim=trim,
                            lower_face=(max(0, skin[0] - 12), max(0, skin[1] - 12), max(0, skin[2] - 4)), **kw)


def goblin_palette(armor, tabard, trim=(232, 184, 64), skin=SKIN_GOBLIN, teeth=GOBLIN_TEETH, hair=None, **kw):
    """A goblin: bald, so the hair only shows as the brows (a darker skin unless it is given), and the grin's
    teeth are the lower face."""
    hair = hair or (int(skin[0] * 0.55), int(skin[1] * 0.55), int(skin[2] * 0.5))
    return humanoid_palette(skin=skin, hair=hair, armor=armor, tabard=tabard, trim=trim, lower_face=teeth, **kw)


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
    # Stormwind
    'stormwind_guard': ('hum_sword', humanoid_palette(hair=(96, 104, 120), armor=(176, 176, 192),
                                                      tabard=(32, 56, 160), trim=(232, 192, 72),
                                                      hair_dark=(64, 72, 88))),
    'bolvar': ('hum_sword', humanoid_palette(hair=(208, 176, 96), armor=(216, 184, 96),
                                             tabard=(40, 64, 152), trim=(248, 232, 160),
                                             armor_light=(248, 224, 144))),
    'archbishop': ('hum_robe', humanoid_palette(hair=(232, 232, 232), armor=(240, 236, 224),
                                                tabard=(232, 192, 72), trim=(176, 40, 40),
                                                armor_light=(248, 248, 248))),
    'explorer': ('dwarf_bow', humanoid_palette(skin=SKIN_DWARF, hair=(176, 88, 40), armor=(152, 112, 64),
                                               tabard=(72, 96, 136), trim=(216, 184, 96),
                                               lower_face=(176, 88, 40))),
    'innkeeper_f': ('fem_robe', humanoid_palette(hair=(200, 120, 56), armor=(176, 112, 64),
                                                 tabard=(232, 224, 200), trim=(120, 80, 48))),
    'warden': ('hum_sword', humanoid_palette(hair=(152, 152, 160), armor=(112, 116, 132),
                                             tabard=(32, 40, 88), trim=(176, 176, 184),
                                             hair_dark=(104, 104, 112))),
    # the Stockade
    'defias_prisoner': ('hum_plain', humanoid_palette(skin=SKIN_TAN, hair=(96, 72, 48),
                                                      armor=(152, 144, 128), tabard=(120, 112, 96),
                                                      trim=(168, 40, 40), lower_face=(168, 40, 40))),
    'defias_convict': ('hum_sword', humanoid_palette(hair=(56, 48, 40), armor=(136, 128, 112),
                                                     tabard=(96, 88, 72), trim=(176, 44, 40),
                                                     lower_face=(176, 44, 40))),
    'defias_insurgent': ('hum_sword', humanoid_palette(hair=(152, 36, 40), armor=(64, 56, 56),
                                                       tabard=(112, 32, 40), trim=(152, 36, 40),
                                                       lower_face=(152, 36, 40))),
    'targorr': ('hum_sword', humanoid_palette(skin=SKIN_DARK, hair=SKIN_DARK, armor=(120, 88, 56),
                                              tabard=(88, 64, 40), trim=(144, 144, 152),
                                              hair_dark=(112, 72, 48))),
    'kam_deepfury': ('dwarf_sword', humanoid_palette(skin=(136, 136, 152), hair=(40, 40, 48),
                                                     armor=(80, 80, 92), tabard=(144, 48, 32),
                                                     trim=(200, 104, 40), lower_face=(40, 40, 48))),
    'bazil_thredd': ('hum_sword', humanoid_palette(hair=(32, 28, 36), armor=(56, 40, 64),
                                                   tabard=(152, 32, 40), trim=(232, 184, 64),
                                                   lower_face=(152, 32, 40), armor_light=(104, 80, 112))),
    # travel
    'gryphon_master': ('dwarf_sword', humanoid_palette(skin=SKIN_DWARF, hair=(200, 120, 56), armor=(120, 96, 64),
                                                       tabard=(48, 96, 136), trim=(232, 184, 64),
                                                       lower_face=(200, 120, 56))),
    'riding_trainer': ('hum_plain', humanoid_palette(hair=(96, 64, 32), armor=(136, 96, 56), tabard=(56, 104, 64),
                                                     trim=(200, 168, 96))),
    'tram_conductor': ('dwarf_sword', humanoid_palette(skin=SKIN_DWARF, hair=(160, 160, 168), armor=(56, 64, 104),
                                                       tabard=(176, 136, 56), trim=(232, 200, 96),
                                                       lower_face=(160, 160, 168))),
    # Lakeshire
    'magistrate': ('hum_robe', humanoid_palette(hair=(184, 184, 192), armor=(56, 56, 88), tabard=(152, 32, 40),
                                                trim=(232, 184, 64))),
    'foreman': ('hum_plain', humanoid_palette(hair=(120, 72, 40), armor=(144, 104, 64), tabard=(96, 96, 104),
                                              trim=(184, 152, 96), leather=(104, 72, 48))),
    'dockmaster': ('hum_plain', humanoid_palette(skin=SKIN_TAN, hair=(80, 56, 40), armor=(64, 88, 136),
                                                 tabard=(216, 208, 184), trim=(184, 152, 96))),
    'chef_f': ('fem_robe', humanoid_palette(hair=(176, 104, 56), armor=(232, 228, 216), tabard=(200, 64, 56),
                                            trim=(232, 184, 64))),
    'fisherman': ('hum_plain', humanoid_palette(hair=(176, 176, 168), armor=(88, 112, 72), tabard=(136, 104, 64),
                                                trim=(200, 168, 96), lower_face=(176, 176, 168))),
    'gryphon_master_f': ('fem_robe', humanoid_palette(hair=(224, 200, 120), armor=(120, 96, 64),
                                                      tabard=(48, 96, 136), trim=(232, 184, 64))),
    # The Blackrock orcs
    'blackrock_outrunner': ('orc_sword', orc_palette(armor=(136, 96, 64), tabard=(152, 40, 32))),
    'blackrock_grunt': ('orc_sword', orc_palette(armor=(120, 120, 136), tabard=(136, 32, 32))),
    'blackrock_shadowcaster': ('orc_staff', orc_palette(armor=(80, 56, 104), tabard=(40, 32, 48))),
    'blackrock_renegade': ('orc_sword', orc_palette(armor=(88, 80, 80), tabard=(96, 24, 24),
                                                    skin=(96, 136, 64))),
    'blackrock_summoner': ('orc_staff', orc_palette(armor=(144, 40, 32), tabard=(56, 40, 40))),
    'gath_ilzogg': ('orc_sword', orc_palette(armor=(64, 64, 72), tabard=(176, 40, 32), hair=(24, 20, 24),
                                             skin=(88, 128, 56))),
    # Duskwood: Darkshire and the Night Watch
    'ello_ebonlocke': ('hum_sword', humanoid_palette(hair=(96, 60, 32), armor=(88, 92, 112), tabard=(136, 36, 56),
                                                     trim=(216, 176, 72), armor_light=(136, 140, 160))),
    'althea_ebonlocke': ('fem_sword', humanoid_palette(hair=(40, 36, 44), armor=(136, 140, 152),
                                                       tabard=(48, 96, 56), trim=(184, 184, 192),
                                                       hair_dark=(24, 20, 28))),
    'night_watch': ('hum_sword', humanoid_palette(hair=(104, 84, 64), armor=(128, 136, 152), tabard=(48, 96, 56),
                                                  trim=(184, 184, 192))),
    'madame_eva': ('fem_robe', humanoid_palette(hair=(176, 176, 184), armor=(112, 56, 144), tabard=(80, 40, 104),
                                                trim=(232, 192, 72), armor_light=(152, 96, 184))),
    'sirra_von_indi': ('fem_robe', humanoid_palette(hair=(40, 36, 44), armor=(48, 64, 152), tabard=(32, 40, 104),
                                                    trim=(192, 200, 232), hair_dark=(24, 20, 28))),
    'sven_yorgen': ('hum_plain', humanoid_palette(hair=(208, 176, 104), armor=(128, 88, 56), tabard=(136, 40, 40),
                                                  trim=(184, 152, 96))),
    'calor': ('hum_plain', humanoid_palette(hair=(56, 40, 32), armor=(152, 136, 112), tabard=(104, 68, 40),
                                            trim=(160, 160, 168), leather=(104, 68, 40))),
    'abercrombie': ('hum_robe', humanoid_palette(hair=(224, 224, 224), armor=(112, 88, 64), tabard=(88, 72, 56),
                                                 trim=(152, 136, 104), lower_face=(224, 224, 224))),
    'trelayne': ('hum_robe', humanoid_palette(hair=(112, 72, 40), armor=(184, 40, 40), tabard=(144, 32, 32),
                                              trim=(232, 184, 64))),
    'felicia_maline': ('fem_robe', humanoid_palette(hair=(224, 176, 88), armor=(136, 96, 56), tabard=(104, 72, 48),
                                                    trim=(200, 168, 96))),
    'ranger_valdan': ('elf_bow', humanoid_palette(skin=(232, 184, 152), hair=(232, 200, 96),
                                                  armor=(64, 112, 56), tabard=(40, 80, 40), trim=(184, 160, 104),
                                                  armor_light=(104, 152, 88))),
    'stalvan_mistmantle': ('hum_robe', humanoid_palette(skin=(176, 200, 216), hair=(208, 224, 232),
                                                        armor=(112, 128, 160), tabard=(88, 100, 128),
                                                        trim=(184, 200, 216))),
    'morbent_fel': ('hum_robe', humanoid_palette(skin=(208, 200, 192), hair=(32, 28, 36), armor=(56, 52, 64),
                                                 tabard=(104, 40, 136), trim=(160, 136, 192),
                                                 armor_light=(88, 84, 100))),
    # Shadowfang Keep
    'haunted_servitor': ('hum_plain', humanoid_palette(skin=(168, 208, 200), hair=(184, 216, 208),
                                                       armor=(120, 160, 160), tabard=(96, 136, 136),
                                                       trim=(176, 208, 200), leather=(96, 128, 128))),
    'wailing_guardsman': ('hum_sword', humanoid_palette(skin=(168, 192, 216), hair=(184, 200, 224),
                                                        armor=(168, 184, 208), tabard=(96, 112, 152),
                                                        trim=(200, 216, 232), leather=(112, 128, 152))),
    'razorclaw_the_butcher': ('hum_sword', humanoid_palette(hair=SKIN, hair_dark=(176, 128, 96),
                                                            armor=(224, 220, 208), tabard=(152, 24, 32),
                                                            trim=(120, 80, 48))),
    'baron_silverlaine': ('hum_sword', humanoid_palette(skin=(208, 216, 224), hair=(232, 236, 240),
                                                        armor=(200, 208, 224), tabard=(40, 48, 104),
                                                        trim=(232, 232, 240), leather=(128, 136, 160))),
    'commander_springvale': ('hum_sword', humanoid_palette(hair=(32, 28, 32), armor=(48, 46, 56),
                                                           tabard=(72, 20, 28), tabard_dark=(32, 16, 20),
                                                           trim=(216, 176, 72), armor_light=(104, 104, 120))),
    'archmage_arugal': ('hum_staff', humanoid_palette(hair=(232, 232, 232), armor=(112, 56, 152),
                                                      tabard=(72, 32, 104), trim=(200, 200, 216),
                                                      armor_light=(160, 104, 200))),
    # Ironforge and Dun Morogh
    'magni_bronzebeard': ('dwarf_sword', dwarf_palette(hair=(144, 60, 32), armor=(216, 168, 64),
                                                       tabard=(168, 32, 32), trim=(248, 232, 144),
                                                       armor_light=(248, 216, 120), hair_dark=(96, 36, 24))),
    'ironforge_guard': ('dwarf_sword', dwarf_palette(hair=(120, 76, 40), armor=(160, 164, 180),
                                                     tabard=(40, 72, 168), trim=(216, 184, 96))),
    'dwarf_innkeeper': ('dwarf_plain', dwarf_palette(hair=(184, 64, 32), armor=(136, 92, 52),
                                                     tabard=(200, 168, 120), trim=(112, 72, 40),
                                                     leather=(104, 68, 40))),
    'dwarf_merchant': ('dwarf_plain', dwarf_palette(hair=(168, 168, 172), armor=(72, 128, 72),
                                                    tabard=(48, 100, 56), trim=(232, 184, 64))),
    'dwarf_smith': ('dwarf_plain', dwarf_palette(hair=(40, 36, 40), armor=(144, 136, 128),
                                                 tabard=(88, 60, 40), trim=(168, 168, 176),
                                                 hair_dark=(24, 20, 24), leather=(80, 56, 40))),
    'captain_stoutfist': ('dwarf_sword', dwarf_palette(hair=(232, 232, 236), armor=(184, 192, 212),
                                                       tabard=(40, 64, 168), trim=(216, 224, 240),
                                                       hair_dark=(160, 160, 176))),
    'ormer_ironbraid': ('dwarf_bow', dwarf_palette(hair=(216, 112, 40), armor=(96, 124, 64),
                                                   tabard=(64, 96, 48), trim=(200, 168, 96),
                                                   armor_light=(136, 168, 96))),
    'prospector_whelgar': ('dwarf_plain', dwarf_palette(hair=(224, 192, 64), beard=(120, 76, 44),
                                                        armor=(140, 100, 60), tabard=(112, 80, 48),
                                                        trim=(232, 200, 72), hair_dark=(168, 128, 40))),
    'dark_iron_dwarf': ('dwarf_sword', dwarf_palette(skin=SKIN_DARK_IRON, hair=(36, 32, 40), armor=(76, 76, 88),
                                                     tabard=(152, 40, 32), trim=(208, 112, 40))),
    'dark_iron_saboteur': ('dwarf_plain', dwarf_palette(skin=SKIN_DARK_IRON, hair=(184, 56, 32),
                                                        armor=(108, 82, 56), tabard=(80, 60, 44),
                                                        trim=(208, 112, 40))),
    'balgaras_the_foul': ('dwarf_sword', dwarf_palette(skin=(120, 120, 140), hair=(28, 24, 32), armor=(52, 48, 58),
                                                       tabard=(152, 24, 28), trim=(200, 56, 40),
                                                       armor_light=(104, 96, 112))),
    'dark_iron_agent': ('dwarf_sword', dwarf_palette(skin=SKIN_DARK_IRON, hair=(64, 56, 64), armor=(68, 68, 80),
                                                     tabard=(104, 48, 136), trim=(176, 176, 188))),
    # Wetlands: the Dragonmaw orcs and the Twilight's Hammer
    'dragonmaw_grunt': ('orc_sword', orc_palette(armor=(60, 56, 64), tabard=(160, 32, 32))),
    'dragonmaw_shadowwarder': ('orc_staff', orc_palette(armor=(96, 52, 120), tabard=(60, 36, 76))),
    'nek_rosh': ('orc_sword', orc_palette(armor=(48, 44, 52), tabard=(168, 32, 32), trim=(232, 208, 136),
                                          skin=(96, 140, 64))),
    'twilight_acolyte': ('hum_robe', humanoid_palette(hair=(48, 40, 48), armor=(72, 44, 92), tabard=(40, 30, 50),
                                                      trim=(232, 128, 40), armor_light=(112, 76, 136))),
    'twilight_reaver': ('hum_sword', humanoid_palette(hair=(40, 36, 40), armor=(84, 84, 96), tabard=(104, 48, 136),
                                                      trim=(232, 128, 40))),
    'twilight_lord_kelris': ('hum_staff', humanoid_palette(skin=SKIN_ELF, hair=(232, 232, 240), armor=(52, 44, 60),
                                                           tabard=(104, 52, 144), trim=(232, 128, 40),
                                                           armor_light=(96, 84, 108))),
    # Menethil Harbor
    'menethil_guard': ('hum_sword', humanoid_palette(hair=(96, 72, 48), armor=(168, 172, 186), tabard=(32, 120, 136),
                                                     trim=(216, 184, 96))),
    'james_halloran': ('hum_plain', humanoid_palette(hair=(104, 72, 40), armor=(132, 96, 56), tabard=(72, 108, 56),
                                                     trim=(184, 152, 96))),
    # Darkshore and Auberdine
    'sentinel': ('fem_bow', humanoid_palette(skin=SKIN_ELF, hair=(64, 168, 160), armor=(72, 72, 140),
                                             tabard=(88, 48, 128), trim=(208, 208, 232),
                                             armor_light=(112, 112, 188))),
    'dawnwatcher_shaedlass': ('fem_robe', humanoid_palette(skin=SKIN_ELF, hair=(208, 212, 228), armor=(232, 232, 240),
                                                           tabard=(208, 208, 220), trim=(176, 152, 232),
                                                           armor_light=(248, 248, 248))),
    'argent_guard_thaelrid': ('elf_sword', humanoid_palette(skin=SKIN_ELF, hair=(208, 212, 228), armor=(192, 196, 212),
                                                            tabard=(236, 236, 236), trim=(232, 192, 72))),
    'hippogryph_master': ('fem_robe', humanoid_palette(skin=SKIN_ELF, hair=(40, 100, 64), armor=(80, 144, 88),
                                                       tabard=(56, 112, 64), trim=(208, 192, 128))),
    'night_elf_innkeeper': ('fem_robe', humanoid_palette(skin=SKIN_ELF, hair=(64, 96, 192), armor=(132, 76, 164),
                                                         tabard=(96, 56, 128), trim=(216, 200, 144))),
    # Gnomes: Gnomeregan's survivors and its ruin
    'mekkatorque': ('gnome_plain', gnome_palette(hair=(236, 236, 240), beard=(236, 236, 240), armor=(56, 84, 192),
                                                 tabard=(40, 56, 144), trim=(240, 200, 72),
                                                 hair_dark=(176, 176, 192))),
    'tinker_gnome': ('gnome_plain', gnome_palette(hair=(240, 120, 176), armor=(136, 96, 56), tabard=(104, 72, 44),
                                                  trim=(120, 208, 224))),
    'ozzie_togglevolt': ('gnome_plain', gnome_palette(hair=(240, 136, 40), armor=(88, 152, 80), tabard=(56, 120, 64),
                                                      trim=(200, 200, 208))),
    'leper_gnome': ('gnome_sword', gnome_palette(skin=(168, 184, 148), hair=(216, 216, 208), armor=(112, 92, 68),
                                                 tabard=(92, 80, 60), trim=(136, 128, 112), leather=(88, 64, 48))),
    'mekgineer_thermaplugg': ('gnome_sword', gnome_palette(hair=(40, 36, 44), armor=(84, 52, 112),
                                                           tabard=(56, 36, 76), trim=(232, 192, 72),
                                                           armor_light=(132, 92, 160))),
    # Hillsbrad: Southshore and the Syndicate
    'magistrate_maleb': ('hum_plain', humanoid_palette(hair=(168, 168, 176), armor=(48, 44, 52), tabard=(36, 32, 40),
                                                       trim=(232, 192, 72), leather=(72, 56, 40),
                                                       armor_light=(96, 92, 104))),
    'raleigh_the_devout': ('hum_sword', humanoid_palette(hair=(232, 200, 96), armor=(224, 224, 232),
                                                         tabard=(240, 236, 224), trim=(232, 184, 64),
                                                         armor_light=(248, 248, 248), hair_dark=(176, 136, 56))),
    'loremaster_dibbs': ('hum_robe', humanoid_palette(hair=(232, 232, 228), armor=(120, 88, 56), tabard=(64, 112, 64),
                                                      trim=(200, 176, 104), lower_face=(232, 232, 228))),
    'darren_malvew': ('hum_sword', humanoid_palette(hair=(112, 72, 40), armor=(128, 92, 56), tabard=(56, 112, 56),
                                                    trim=(184, 152, 96), leather=(104, 68, 40))),
    'syndicate_footpad': ('hum_sword', humanoid_palette(hair=(40, 36, 44), armor=(48, 44, 52), tabard=(104, 48, 136),
                                                        trim=(104, 48, 136), lower_face=(104, 48, 136),
                                                        leather=(64, 52, 60))),
    'syndicate_thief': ('hum_plain', humanoid_palette(hair=(64, 52, 44), armor=(80, 80, 88), tabard=(64, 64, 72),
                                                      trim=(136, 72, 168), leather=(72, 60, 64))),
    'syndicate_shadow_mage': ('hum_robe', humanoid_palette(hair=(40, 32, 44), armor=(88, 44, 120), tabard=(36, 28, 44),
                                                           trim=(168, 104, 208), armor_light=(128, 80, 160))),
    'syndicate_enforcer': ('hum_sword', humanoid_palette(hair=(56, 48, 40), armor=(88, 88, 100), tabard=(104, 48, 136),
                                                         trim=(152, 152, 168), armor_light=(136, 136, 152))),
    'gravis_slipknot': ('hum_sword', humanoid_palette(hair=(184, 64, 32), armor=(40, 36, 44), tabard=(28, 24, 32),
                                                      trim=(232, 192, 72), lower_face=(184, 64, 32),
                                                      armor_light=(88, 80, 96), leather=(56, 44, 40))),
    # the Forsaken
    'forsaken_thug': ('hum_sword', humanoid_palette(skin=SKIN_FORSAKEN, hair=(48, 44, 40), armor=(104, 80, 56),
                                                    tabard=(44, 40, 40), trim=(120, 112, 96),
                                                    lower_face=(120, 132, 112), leather=(72, 56, 44))),
    'forsaken_herbalist': ('hum_robe', humanoid_palette(skin=SKIN_FORSAKEN, hair=(96, 88, 72), armor=(88, 120, 64),
                                                        tabard=(64, 92, 52), trim=(160, 136, 88),
                                                        lower_face=(120, 132, 112))),
    'forsaken_courier': ('hum_plain', humanoid_palette(skin=SKIN_FORSAKEN, hair=(40, 36, 44), armor=(48, 44, 52),
                                                       tabard=(96, 52, 120), trim=(152, 144, 160),
                                                       lower_face=(120, 132, 112))),
    # the Scarlet Crusade
    'scarlet_convert': ('hum_plain', humanoid_palette(hair=(136, 96, 56), armor=(216, 212, 200), tabard=(176, 32, 32),
                                                      trim=(232, 228, 216), leather=(112, 80, 56))),
    'scarlet_scout': ('fem_bow', humanoid_palette(hair=(192, 128, 64), armor=(168, 40, 36), tabard=(224, 220, 208),
                                                  trim=(176, 32, 32), leather=(120, 84, 56),
                                                  armor_light=(208, 72, 64))),
    'scarlet_torturer': ('hum_sword', humanoid_palette(hair=(152, 28, 28), armor=(48, 44, 48), tabard=(152, 28, 28),
                                                       trim=(120, 116, 120), lower_face=(152, 28, 28),
                                                       hair_dark=(96, 20, 20), leather=(56, 44, 40))),
    'interrogator_vishas': ('hum_plain', humanoid_palette(hair=(56, 40, 36), armor=(36, 32, 36), tabard=(144, 24, 32),
                                                          trim=(184, 40, 48), hair_dark=(176, 120, 88),
                                                          armor_light=(80, 72, 80), leather=(48, 36, 36))),
    'bloodmage_thalnos': ('hum_robe', humanoid_palette(skin=(168, 168, 160), hair=(56, 52, 56), armor=(152, 24, 32),
                                                       tabard=(112, 20, 28), trim=(232, 192, 72),
                                                       armor_light=(200, 56, 56))),
    'scarlet_gallant': ('hum_sword', humanoid_palette(hair=(184, 136, 72), armor=(216, 216, 224), tabard=(176, 32, 32),
                                                      trim=(232, 192, 72), armor_light=(244, 244, 248))),
    'scarlet_adept': ('hum_robe', humanoid_palette(hair=(104, 72, 40), armor=(176, 32, 32), tabard=(228, 224, 212),
                                                   trim=(176, 32, 32), armor_light=(216, 72, 64))),
    'scarlet_monk': ('hum_plain', humanoid_palette(hair=(36, 32, 32), armor=(184, 36, 36), tabard=(232, 228, 216),
                                                   trim=(184, 36, 36), leather=(232, 228, 216),
                                                   armor_light=(224, 80, 72), hair_dark=(20, 18, 20))),
    'scarlet_beastmaster': ('fem_bow', humanoid_palette(hair=(120, 64, 40), armor=(120, 36, 32), tabard=(88, 28, 28),
                                                        trim=(200, 168, 120), leather=(96, 64, 44),
                                                        armor_light=(160, 64, 56))),
    'scarlet_chaplain': ('fem_robe', humanoid_palette(hair=(216, 184, 120), armor=(232, 228, 220), tabard=(176, 32, 32),
                                                      trim=(232, 192, 72), armor_light=(248, 248, 244))),
    'scarlet_diviner': ('hum_staff', humanoid_palette(hair=(152, 120, 88), armor=(168, 32, 36), tabard=(128, 24, 28),
                                                      trim=(232, 192, 72), armor_light=(208, 64, 64))),
    'houndmaster_loksey': ('hum_sword', humanoid_palette(hair=(36, 30, 30), armor=(128, 88, 56), tabard=(176, 32, 32),
                                                         trim=(216, 184, 104), leather=(96, 64, 40),
                                                         lower_face=(36, 30, 30), hair_dark=(20, 18, 20))),
    'arcanist_doan': ('hum_staff', humanoid_palette(hair=(236, 236, 236), armor=(160, 24, 40), tabard=(112, 16, 28),
                                                    trim=(240, 200, 72), lower_face=(236, 236, 236),
                                                    armor_light=(208, 64, 72))),
    # Southshore's inn and the Argent Dawn
    'southshore_innkeeper': ('hum_robe', humanoid_palette(hair=(120, 80, 48), armor=(72, 120, 72), tabard=(136, 96, 56),
                                                          trim=(200, 168, 104), armor_light=(112, 160, 104))),
    'argent_scout': ('elf_bow', humanoid_palette(skin=SKIN_ELF, hair=(208, 212, 228), armor=(176, 180, 196),
                                                 tabard=(104, 56, 144), trim=(232, 232, 240),
                                                 armor_light=(220, 224, 236))),
    # Stranglethorn: the Rebel Camp and Nesingwary's Expedition
    'lieutenant_doren': ('hum_sword', humanoid_palette(hair=(56, 40, 32), armor=(84, 104, 64), tabard=(112, 80, 48),
                                                       trim=(224, 184, 72), lower_face=(76, 56, 44),
                                                       leather=(96, 68, 44), armor_light=(124, 144, 96))),
    'rebel_soldier': ('hum_sword', humanoid_palette(hair=(120, 84, 48), armor=(96, 112, 68), tabard=(124, 92, 56),
                                                    trim=(168, 144, 104), leather=(104, 72, 44))),
    'corporal_bluth': ('hum_plain', humanoid_palette(hair=(200, 104, 40), armor=(132, 96, 60), tabard=(80, 112, 64),
                                                     trim=(192, 168, 112), leather=(96, 68, 44))),
    'hemet_nesingwary': ('dwarf_bow', dwarf_palette(hair=(236, 236, 232), beard=(212, 212, 208),
                                                    armor=(184, 160, 112), tabard=(48, 88, 48), trim=(216, 192, 128),
                                                    hair_dark=(168, 168, 168), leather=(120, 88, 56))),
    'ajeck_rouack': ('dwarf_bow', dwarf_palette(hair=(36, 32, 36), beard=(52, 44, 44), armor=(128, 88, 52),
                                                tabard=(160, 40, 36), trim=(200, 168, 104), hair_dark=(20, 18, 22),
                                                leather=(96, 64, 40))),
    'sir_s_j_erlgadin': ('hum_sword', humanoid_palette(hair=(168, 168, 172), armor=(184, 164, 116),
                                                       tabard=(160, 36, 40), trim=(224, 200, 136),
                                                       lower_face=(188, 188, 192), leather=(112, 80, 52))),
    'barnil_stonepot': ('dwarf_plain', dwarf_palette(hair=(128, 80, 44), beard=(108, 64, 36),
                                                     armor=(184, 152, 104), tabard=(56, 88, 152),
                                                     trim=(208, 184, 120), leather=(104, 72, 44))),
    # Booty Bay's goblins
    'baron_revilgaz': ('goblin_plain', goblin_palette(armor=(112, 56, 144), tabard=(80, 36, 104), trim=(240, 200, 72),
                                                      armor_light=(160, 104, 192))),
    'fleet_master_seahorn': ('goblin_sword', goblin_palette(armor=(48, 60, 120), tabard=(32, 40, 84),
                                                            trim=(232, 192, 72), armor_light=(88, 104, 168))),
    'kebok': ('goblin_plain', goblin_palette(armor=(112, 80, 52), tabard=(124, 36, 40), trim=(184, 160, 112),
                                             leather=(88, 60, 40))),
    'innkeeper_skindle': ('goblin_plain', goblin_palette(armor=(48, 104, 96), tabard=(232, 228, 216),
                                                         trim=(184, 152, 96), armor_light=(88, 148, 136))),
    'gyll': ('goblin_plain', goblin_palette(armor=(128, 92, 56), tabard=(96, 160, 216), trim=(216, 184, 104),
                                            leather=(96, 64, 40))),
    'booty_bay_bruiser': ('goblin_sword', goblin_palette(armor=(168, 40, 36), tabard=(40, 36, 40), trim=(200, 196, 200),
                                                         armor_light=(216, 80, 64), leather=(56, 44, 40))),
    'booty_bay_vendor': ('goblin_plain', goblin_palette(armor=(216, 128, 48), tabard=(120, 80, 48),
                                                        trim=(240, 216, 120), leather=(104, 68, 40))),
    'booty_bay_smith': ('goblin_plain', goblin_palette(armor=(84, 84, 92), tabard=(120, 84, 52), trim=(176, 176, 184),
                                                       leather=(96, 64, 40), armor_light=(128, 128, 140))),
    # Stranglethorn's enemies: the Bloodscalp trolls and the Bloodsail Buccaneers
    'bloodscalp_warrior': ('troll_sword', troll_palette(hair=(200, 40, 32), armor=(136, 96, 60), tabard=(160, 36, 32),
                                                        trim=(236, 228, 204), armor_light=(220, 208, 180),
                                                        leather=(96, 64, 40))),
    'bloodscalp_shaman': ('troll_staff', troll_palette(hair=(208, 44, 36), armor=(168, 40, 36), tabard=(220, 208, 180),
                                                       trim=(236, 228, 204), armor_light=(208, 80, 64))),
    'bloodscalp_headhunter': ('troll_sword', troll_palette(hair=(232, 96, 40), armor=(88, 64, 48), tabard=(56, 44, 40),
                                                           trim=(224, 216, 188), armor_light=(200, 188, 160),
                                                           leather=(120, 84, 52))),
    'mogh_the_undying': ('troll_staff', troll_palette(skin=(136, 156, 136), hair=(236, 236, 232), armor=(96, 52, 120),
                                                      tabard=(36, 28, 40), trim=(208, 200, 176),
                                                      armor_light=(140, 92, 168))),
    'bloodsail_swashbuckler': ('hum_sword', humanoid_palette(skin=SKIN_TAN, hair=(184, 36, 32), armor=(224, 220, 208),
                                                             tabard=(176, 32, 32), trim=(40, 36, 40),
                                                             hair_dark=(120, 24, 24), leather=(80, 56, 40))),
    'bloodsail_mage': ('hum_staff', humanoid_palette(hair=(176, 32, 32), armor=(112, 28, 32), tabard=(40, 32, 36),
                                                     trim=(208, 176, 96), hair_dark=(112, 20, 24),
                                                     armor_light=(160, 56, 56))),
    'bloodsail_sea_dog': ('hum_sword', humanoid_palette(skin=SKIN_TAN, hair=(128, 28, 28), armor=(228, 228, 220),
                                                        tabard=(40, 52, 104), trim=(176, 36, 36),
                                                        leather=(88, 64, 44))),
    'fleet_master_firallon': ('hum_sword', humanoid_palette(hair=(28, 24, 28), armor=(152, 24, 36), tabard=(56, 44, 52),
                                                            trim=(232, 192, 72), lower_face=(44, 36, 36),
                                                            hair_dark=(16, 14, 18), armor_light=(200, 56, 64),
                                                            leather=(72, 52, 40))),
    # The Scarlet Monastery: the Armory
    'scarlet_soldier': ('hum_sword', humanoid_palette(hair=(120, 84, 52), armor=(168, 164, 168), tabard=(176, 32, 32),
                                                      trim=(224, 220, 208), leather=(104, 76, 52))),
    'scarlet_myrmidon': ('hum_sword', humanoid_palette(hair=(88, 60, 40), armor=(168, 36, 36), tabard=(228, 224, 212),
                                                       trim=(176, 32, 32), armor_light=(216, 80, 72),
                                                       leather=(112, 80, 52))),
    'scarlet_defender': ('hum_sword', humanoid_palette(hair=(152, 112, 64), armor=(208, 212, 224), tabard=(176, 32, 32),
                                                       trim=(232, 228, 216), armor_light=(244, 244, 248))),
    'scarlet_trainee': ('hum_plain', humanoid_palette(hair=(168, 120, 72), armor=(224, 220, 208), tabard=(184, 40, 40),
                                                      trim=(232, 228, 216), leather=(128, 92, 60))),
    'herod': ('hum_sword', humanoid_palette(hair=(40, 32, 32), armor=(136, 20, 28), tabard=(48, 40, 44),
                                            trim=(232, 192, 72), lower_face=(56, 44, 40), hair_dark=(24, 20, 22),
                                            armor_light=(192, 48, 52), leather=(76, 56, 44))),
    # The Scarlet Monastery: the Cathedral
    'scarlet_champion': ('hum_sword', humanoid_palette(hair=(176, 132, 80), armor=(228, 228, 220), tabard=(176, 32, 32),
                                                       trim=(232, 192, 72), armor_light=(248, 248, 240))),
    'scarlet_abbot': ('hum_robe', humanoid_palette(hair=(176, 176, 176), armor=(176, 32, 32), tabard=(232, 228, 220),
                                                   trim=(232, 192, 72), armor_light=(216, 72, 64))),
    'scarlet_wizard': ('hum_staff', humanoid_palette(hair=(140, 96, 56), armor=(220, 216, 204), tabard=(168, 28, 36),
                                                     trim=(240, 236, 228), armor_light=(244, 244, 236))),
    'scarlet_centurion': ('hum_sword', humanoid_palette(hair=(64, 48, 40), armor=(120, 28, 32), tabard=(36, 32, 36),
                                                        trim=(184, 40, 44), armor_light=(168, 56, 56),
                                                        leather=(56, 44, 40))),
    'high_inquisitor_fairbanks': ('hum_robe', humanoid_palette(skin=(208, 196, 184), hair=(232, 232, 228),
                                                               armor=(152, 92, 96), tabard=(116, 72, 76),
                                                               trim=(216, 184, 96), armor_light=(184, 136, 136))),
    'scarlet_commander_mograine': ('hum_sword', humanoid_palette(hair=(232, 204, 112), armor=(208, 212, 228),
                                                                 tabard=(184, 32, 36), trim=(240, 200, 72),
                                                                 armor_light=(248, 248, 255),
                                                                 hair_dark=(176, 140, 64))),
    'high_inquisitor_whitemane': ('fem_robe', humanoid_palette(hair=(240, 240, 236), armor=(232, 228, 220),
                                                               tabard=(176, 32, 36), trim=(232, 192, 72),
                                                               armor_light=(248, 248, 244), hair_dark=(184, 184, 192))),
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
    'water_elemental': ('water_elemental', creature_palette((64, 136, 216), (216, 240, 248),
                                                            eye=(240, 252, 255), light=(136, 200, 248))),
    # Redridge
    'redridge_mongrel': ('gnoll', creature_palette((176, 104, 64), (104, 56, 40), eye=(232, 48, 32),
                                                   extra=(112, 80, 56), weapon=(136, 96, 56))),
    'shadowhide_gnoll': ('gnoll', creature_palette((96, 96, 120), (48, 48, 64), eye=(232, 200, 64),
                                                   extra=(72, 56, 48), weapon=(152, 152, 160))),
    'shadowhide_mystic': ('gnoll', creature_palette((104, 88, 128), (56, 40, 72), eye=(160, 232, 248),
                                                    extra=(120, 48, 136), weapon=(136, 96, 56))),
    'ribchaser': ('gnoll', creature_palette((208, 200, 184), (136, 120, 104), eye=(232, 32, 32),
                                            extra=(152, 40, 40), weapon=(184, 184, 192))),
    'murloc_flesheater': ('murloc', creature_palette((168, 104, 72), (216, 176, 136), eye=(24, 24, 24),
                                                     extra=(136, 48, 40))),
    'great_goretusk': ('boar', creature_palette((96, 64, 56), (168, 128, 112), eye=(232, 64, 32))),
    'bellygrub': ('boar', creature_palette((216, 152, 144), (240, 200, 184), eye=(40, 24, 24))),
    'tarantula': ('spider', creature_palette((120, 80, 48), (216, 136, 48), eye=(232, 48, 32))),
    # Duskwood
    'dire_wolf': ('wolf', creature_palette((80, 72, 68), (144, 136, 128), eye=(248, 200, 64))),
    'rabid_dire_wolf': ('wolf', creature_palette((104, 76, 64), (168, 128, 112), eye=(248, 40, 24))),
    'venom_web_spider': ('spider', creature_palette((40, 56, 40), (120, 176, 56), eye=(152, 240, 72),
                                                    light=(72, 96, 64))),
    'bleak_worg': ('wolf', creature_palette((60, 60, 84), (120, 120, 152), eye=(176, 224, 248))),
    'nightbane_dark_runner': ('worgen', creature_palette((96, 84, 72), (136, 124, 108), eye=(248, 208, 64),
                                                         extra=(72, 80, 104))),
    'nightbane_shadow_weaver': ('worgen_caster', creature_palette((72, 60, 80), (112, 100, 120),
                                                                  eye=(224, 168, 248), extra=(88, 56, 64),
                                                                  flame=(232, 200, 255), flame_dark=(144, 72, 216))),
    'nightbane_tainted_one': ('worgen', creature_palette((52, 48, 56), (88, 80, 92), eye=(248, 40, 24),
                                                         extra=(96, 72, 56), light=(96, 88, 104))),
    'shadowfang_moonwalker': ('worgen', creature_palette((120, 116, 128), (168, 164, 176), eye=(224, 236, 248),
                                                         extra=(64, 64, 88))),
    'shadowfang_darkcaster': ('worgen_caster', creature_palette((56, 48, 64), (96, 88, 104), eye=(176, 248, 176),
                                                                extra=(72, 64, 96), light=(96, 88, 112),
                                                                flame=(232, 255, 224), flame_dark=(96, 200, 104))),
    'shadowfang_wolfguard': ('worgen', creature_palette((104, 76, 56), (152, 120, 96), eye=(248, 208, 64),
                                                        extra=(128, 132, 144))),
    'rethilgore': ('worgen', creature_palette((120, 72, 52), (168, 120, 96), eye=(248, 40, 24),
                                              extra=(64, 56, 56))),
    'odo_the_blindwatcher': ('worgen', creature_palette((104, 104, 112), (152, 152, 160), eye=(232, 232, 224),
                                                        extra=(88, 64, 48))),
    'skeletal_warrior': ('skeleton', creature_palette((216, 208, 184), (176, 168, 144), eye=(184, 40, 32),
                                                      extra=(112, 80, 56), weapon=(176, 140, 112))),
    'skeletal_mage': ('skeleton_mage', creature_palette((216, 208, 184), (176, 168, 144), eye=(200, 160, 255),
                                                        extra=(80, 48, 104), weapon=(112, 80, 56),
                                                        flame=(224, 216, 255), flame_dark=(136, 104, 232))),
    'skeletal_servant': ('skeleton', creature_palette((176, 164, 128), (144, 132, 104), eye=(184, 40, 32),
                                                      extra=(96, 88, 72), weapon=(144, 104, 72))),
    'mor_ladim': ('skeleton', creature_palette((176, 184, 200), (136, 144, 160), eye=(96, 200, 255),
                                               extra=(72, 76, 88), weapon=(152, 160, 176))),
    'rotting_ghoul': ('ghoul', creature_palette((120, 136, 104), (112, 64, 80), eye=(232, 216, 64),
                                                extra=(96, 80, 64))),
    'plague_spreader': ('ghoul', creature_palette((144, 140, 88), (152, 232, 72), eye=(216, 248, 96),
                                                  extra=(88, 72, 56))),
    'splinter_fist_ogre': ('ogre', creature_palette((168, 128, 96), (64, 48, 40), eye=(240, 216, 96),
                                                    extra=(88, 60, 40), weapon=(120, 84, 48))),
    'splinter_fist_taskmaster': ('ogre', creature_palette((136, 104, 80), (48, 40, 36), eye=(248, 72, 40),
                                                          extra=(128, 132, 144), weapon=(104, 72, 44))),
    'stitches': ('abomination', creature_palette((192, 176, 168), (152, 160, 136), eye=(232, 216, 64),
                                                 extra=(136, 32, 40), weapon=(144, 144, 152))),
    # Wetlands
    'young_crocolisk': ('crocolisk', creature_palette((112, 164, 80), (208, 208, 144), eye=(232, 200, 64))),
    'giant_crocolisk': ('crocolisk', creature_palette((76, 96, 56), (152, 140, 96), eye=(232, 72, 40),
                                                      light=(112, 128, 72))),
    'mottled_raptor': ('raptor', creature_palette((104, 148, 72), (208, 176, 96), eye=(232, 200, 64))),
    'mottled_screecher': ('raptor', creature_palette((112, 128, 156), (192, 196, 212), eye=(232, 72, 40))),
    'sarltooth': ('raptor', creature_palette((140, 40, 32), (224, 176, 96), eye=(248, 216, 64),
                                             light=(192, 72, 56))),
    'mosshide_gnoll': ('gnoll', creature_palette((124, 128, 72), (72, 80, 48), eye=(232, 48, 32),
                                                 extra=(104, 80, 56), weapon=(136, 96, 56))),
    'mosshide_mystic': ('gnoll', creature_palette((104, 136, 72), (176, 192, 120), eye=(200, 240, 120),
                                                  extra=(96, 64, 112), weapon=(120, 88, 56))),
    'bluegill_murloc': ('murloc', creature_palette((64, 120, 188), (176, 208, 232), eye=(24, 24, 24),
                                                   extra=(96, 200, 208))),
    'gelihast': ('murloc', creature_palette((40, 112, 112), (152, 200, 184), eye=(248, 216, 64),
                                            extra=(208, 48, 40))),
    'blindlight_murloc': ('murloc', creature_palette((184, 200, 216), (228, 234, 242), eye=(168, 192, 216),
                                                     extra=(128, 152, 188), light=(220, 230, 244))),
    # Blackfathom Deeps
    'blackfathom_myrmidon': ('naga', creature_palette((56, 144, 136), (192, 216, 168), eye=(248, 232, 96),
                                                      extra=(152, 72, 136), weapon=(232, 192, 72))),
    'blackfathom_tide_priestess': ('naga_caster', creature_palette((136, 168, 216), (220, 220, 244),
                                                                   eye=(248, 248, 255), extra=(152, 120, 200),
                                                                   flame=(216, 248, 255), flame_dark=(96, 176, 232))),
    'lady_sarevess': ('naga_caster', creature_palette((168, 88, 168), (232, 184, 216), eye=(248, 232, 96),
                                                      extra=(232, 184, 64), flame=(255, 224, 248),
                                                      flame_dark=(200, 96, 216))),
    'aku_mai_snapjaw': ('turtle', creature_palette((88, 128, 64), (136, 152, 104), eye=(232, 200, 64),
                                                   extra=(192, 176, 112))),
    'ghamoo_ra': ('turtle', creature_palette((72, 84, 116), (120, 136, 120), eye=(232, 64, 40),
                                             extra=(152, 144, 128))),
    'aku_mai_servant': ('hydra', creature_palette((56, 136, 136), (168, 200, 168), eye=(248, 216, 64),
                                                  extra=(40, 88, 112))),
    'aku_mai': ('hydra', creature_palette((72, 64, 152), (136, 152, 200), eye=(248, 40, 32), extra=(152, 56, 104))),
    # Gnomeregan
    'irradiated_pillager': ('trogg', creature_palette((136, 136, 128), (152, 232, 72), eye=(184, 248, 96),
                                                      extra=(96, 80, 64), weapon=(208, 200, 176))),
    'caverndeep_burrower': ('trogg', creature_palette((136, 120, 100), (104, 96, 88), eye=(232, 200, 64),
                                                      extra=(112, 72, 48), weapon=(208, 200, 176))),
    'grubbis': ('trogg', creature_palette((80, 88, 104), (124, 132, 148), eye=(248, 64, 40), extra=(136, 40, 40),
                                          weapon=(216, 208, 184))),
    'irradiated_slime': ('ooze', creature_palette((120, 216, 72), (200, 248, 136), eye=(24, 48, 16),
                                                  light=(200, 255, 152))),
    'viscous_fallout': ('ooze', creature_palette((72, 136, 48), (232, 232, 72), eye=(248, 248, 120),
                                                 light=(216, 232, 80))),
    'mechano_tank': ('robot', creature_palette((144, 148, 160), (96, 100, 112), extra=(200, 168, 96),
                                               weapon=(184, 188, 200), flame=(255, 120, 96),
                                               flame_dark=(200, 32, 24))),
    'arcane_nullifier': ('robot', creature_palette((104, 88, 168), (72, 72, 112), extra=(176, 160, 224),
                                                   weapon=(168, 176, 208), flame=(232, 200, 255),
                                                   flame_dark=(144, 80, 232))),
    'electrocutioner_6000': ('robot', creature_palette((200, 128, 64), (152, 112, 56), extra=(240, 208, 96),
                                                       weapon=(200, 200, 208), flame=(216, 248, 255),
                                                       flame_dark=(64, 160, 248))),
    'crowd_pummeler': ('robot', creature_palette((176, 48, 40), (88, 88, 96), extra=(200, 200, 208),
                                                 weapon=(136, 140, 152), flame=(255, 232, 120))),
    'walking_bomb': ('bomb', creature_palette((56, 56, 64), (176, 144, 72), extra=(200, 176, 120),
                                              weapon=(120, 120, 128), light=(120, 120, 136),
                                              flame=(255, 232, 96), flame_dark=(240, 128, 32))),
    # Alterac Mountains, Hillsbrad and the Scarlet Monastery
    'gray_bear': ('bear', creature_palette((128, 112, 100), (176, 160, 140), eye=(216, 152, 56),
                                           light=(164, 148, 136))),
    'mountain_lion': ('cat', creature_palette((192, 140, 80), (236, 216, 176), eye=(232, 200, 64),
                                              dark=(128, 88, 48), light=(224, 176, 112))),
    'mountain_yeti': ('yeti', creature_palette((216, 220, 232), (120, 124, 144), eye=(96, 184, 248),
                                               extra=(168, 152, 128), dark=(144, 148, 168), light=(248, 248, 255))),
    'bloodfang': ('yeti', creature_palette((176, 92, 64), (104, 76, 76), eye=(248, 40, 24), extra=(208, 192, 160),
                                           dark=(112, 52, 40), light=(216, 144, 112))),
    'crushridge_ogre': ('ogre', creature_palette((120, 136, 168), (56, 48, 56), eye=(240, 216, 96),
                                                 extra=(128, 84, 48), weapon=(120, 84, 48))),
    'crushridge_mage': ('ogre', creature_palette((136, 120, 160), (48, 40, 56), eye=(200, 168, 255),
                                                 extra=(64, 44, 88), weapon=(96, 72, 104))),
    'crushridge_enforcer': ('ogre', creature_palette((88, 100, 128), (40, 36, 44), eye=(248, 72, 40),
                                                     extra=(144, 148, 160), weapon=(160, 164, 176))),
    'torn_fin_tidehunter': ('murloc', creature_palette((104, 104, 152), (176, 176, 208), eye=(24, 24, 24),
                                                       extra=(136, 96, 152))),
    'torn_fin_oracle': ('murloc', creature_palette((48, 144, 144), (176, 224, 208), eye=(24, 24, 24),
                                                   extra=(248, 200, 56))),
    'haunting_phantasm': ('spirit', creature_palette((136, 96, 192), (208, 184, 240), eye=(248, 240, 160),
                                                     light=(184, 152, 232))),
    'unfettered_spirit': ('spirit', creature_palette((168, 200, 228), (232, 244, 252), eye=(96, 184, 248),
                                                     dark=(112, 144, 184), light=(220, 236, 248))),
    'azshir_the_sleepless': ('spirit', creature_palette((64, 44, 88), (120, 88, 152), eye=(136, 255, 96),
                                                        dark=(36, 28, 52), light=(104, 76, 136))),
    'ironspine': ('skeleton', creature_palette((200, 208, 128), (152, 160, 96), eye=(168, 248, 64),
                                               extra=(88, 92, 72), weapon=(136, 144, 152))),
    'scarlet_tracking_hound': ('wolf', creature_palette((144, 72, 48), (200, 152, 120), eye=(248, 208, 64))),
    # Stranglethorn Vale
    'stranglethorn_tiger': ('tiger', creature_palette((224, 128, 40), (240, 224, 192), eye=(232, 216, 64),
                                                      extra=(40, 28, 24), dark=(160, 80, 28), light=(240, 168, 80))),
    'shadowmaw_panther': ('cat', creature_palette((52, 44, 64), (88, 68, 112), eye=(200, 240, 64),
                                                  dark=(36, 30, 46), light=(96, 72, 132))),
    'king_bangalash': ('tiger', creature_palette((228, 228, 224), (244, 240, 224), eye=(96, 176, 248),
                                                 extra=(128, 132, 148), dark=(176, 176, 188), light=(255, 255, 255))),
    'stranglethorn_raptor': ('raptor', creature_palette((48, 128, 72), (240, 212, 72), eye=(232, 72, 40),
                                                        light=(88, 176, 96))),
    'lashtail_raptor': ('raptor', creature_palette((200, 80, 40), (216, 184, 136), eye=(248, 216, 64),
                                                   light=(232, 120, 72))),
    'elder_mistvale_gorilla': ('gorilla', creature_palette((56, 56, 64), (168, 168, 176), eye=(232, 72, 40),
                                                           light=(136, 136, 148))),
    'mistvale_gorilla': ('gorilla', creature_palette((84, 60, 44), (184, 152, 112), eye=(232, 72, 40),
                                                     light=(120, 92, 68))),
}


# By race: the Mount ability's horse (humans), ram (dwarves) and nightsaber (night elves).
MOUNTS = {
    'horse': (horse, creature_palette((136, 88, 48), (56, 36, 24), eye=(24, 20, 28), extra=(48, 72, 168),
                                      flame=(232, 184, 64), tooth=(232, 228, 216))),
    'ram': (ram, creature_palette((208, 200, 184), (112, 80, 56), eye=(24, 20, 28), extra=(160, 48, 40),
                                  flame=(232, 184, 64), weapon=(184, 160, 120))),
    'saber': (saber, creature_palette((96, 80, 136), (48, 40, 80), eye=(232, 232, 120), extra=(72, 112, 72),
                                      flame=(208, 208, 232))),
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
            'enum class look_id : uint16_t', '{']
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
        sheets[name] = write_sheet(name, creature_sheet(draw, **creature_sheet_options(name)), base_creature)
    write_palettes()
    write_looks()
    mount_frames = []
    for name, (draw, palette) in MOUNTS.items():
        frames = [draw(view, step) for view, step in MOUNT_VIEWS]
        save_indexed_bmp(GRAPHICS / f'mount_{name}.bmp', np.concatenate(frames, axis=0), unique(palette))
        (GRAPHICS / f'mount_{name}.json').write_text('{\n    "type": "sprite",\n    "height": 32\n}\n')
        mount_frames.append(frames)
    preview(mount_frames, [palette for _, palette in MOUNTS.values()], PREVIEW / 'mounts.png')

    looks = list(HUMANOID_LOOKS.items()) + list(CREATURE_LOOKS.items())
    preview([sheets[sheet] for _, (sheet, _) in looks], [p for _, (_, p) in looks],
            PREVIEW / 'characters.png')
    print(f'{len(sheets)} sheets, {len(looks)} looks')


if __name__ == '__main__':
    main()
