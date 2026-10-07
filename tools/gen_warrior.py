"""Generate the placeholder Human Warrior sprite sheet (graphics/warrior.bmp).

The sheet is 32 px wide with 12 frames of 32x32 stacked vertically:
  0-3   walk down (frame 0 doubles as idle)
  4-7   walk up
  8-11  walk left (the game mirrors these for right)

Each frame is built from ASCII parts so the art can be tweaked by hand. Front and back views
are written as left halves and mirrored. This is placeholder art until real Aseprite sprites
replace it; it only has to read clearly at 240x160.
"""

import numpy as np

from art_common import GRAPHICS, PREVIEW, save_indexed_bmp, save_preview_png

PALETTE = [
    (255, 0, 255),    # 0 transparent
    (24, 24, 32),     # o outline
    (176, 112, 80),   # s skin shadow
    (232, 168, 128),  # S skin
    (88, 48, 24),     # h hair dark
    (144, 88, 40),    # H hair
    (88, 96, 120),    # a armor dark
    (144, 152, 168),  # A armor mid
    (208, 216, 224),  # L armor light
    (32, 48, 128),    # b tabard dark
    (56, 88, 184),    # B tabard blue
    (232, 184, 64),   # g gold
    (64, 40, 24),     # l leather dark
    (120, 80, 48),    # e leather
    (160, 120, 40),   # d gold shadow
    (248, 248, 248),  # W white
]

KEYS = {'.': 0, 'o': 1, 's': 2, 'S': 3, 'h': 4, 'H': 5, 'a': 6, 'A': 7, 'L': 8,
        'b': 9, 'B': 10, 'g': 11, 'l': 12, 'e': 13, 'd': 14, 'W': 15}


def mirror(rows):
    return [r + r[::-1] for r in rows]


FRONT_TOP = mirror([
    '......oo',
    '....ooHH',
    '...oHHHH',
    '..oHHHHH',
    '..ohHHHh',
    '..ohSSSS',
    '..ohSoSS',
    '...osSSS',
    '....osSS',
    '.ooAAoos',
    'oALLAobB',
    'oAAAaobB',
    '.oAAobBB',
    '.oAaobBg',
    '.oaaobBB',
    '.oSsollg',
    '..oobBBB',
    '...obBbB',
])

BACK_TOP = mirror([
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
])

LEGS_STAND = mirror([
    '...oaAAo',
    '...oaAAo',
    '...oaAAo',
    '...oallo',
    '...oeelo',
    '..oeeelo',
    '..ollllo',
    '...oooo.',
])

SIDE_TOP = [
    '.....oooo.......',
    '....oHHHHo......',
    '...oHHHHHHo.....',
    '..oHHHHHHHHo....',
    '..oSHHHHHhHo....',
    '.oSSSHHhhhHo....',
    '.oSoSSShhhHo....',
    '.oSSSSsShhho....',
    '..osSSsooo......',
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

SIDE_LEGS_STAND = [
    '....oaAAAo......',
    '....oaAAAo......',
    '....oaAAAo......',
    '....oallao......',
    '...oeeello......',
    '..oeeeeelo......',
    '..ollllllo......',
    '...oooooo.......',
]

SIDE_LEGS_STRIDE = [
    '....oaAAAo......',
    '...oaAoaAAo.....',
    '...oaAooaAAo....',
    '..oallo.oallo...',
    '..oeelo..oeelo..',
    '.oeeeeo..oeeeo..',
    '.ollllo..olllo..',
    '..oooo....ooo...',
]

SIDE_LEGS_STRIDE_B = [
    '....oaAAAo......',
    '...oaAAoaAo.....',
    '...oaAAooaAo....',
    '..oallo.oallo...',
    '..oeelo..oeelo..',
    '.oeeeeo..oeeeo..',
    '.ollllo..olllo..',
    '..oooo....ooo...',
]

FRAME = 32
OX, OY = 8, 4          # where the 16x26 figure sits inside the 32x32 frame
LEG_Y = OY + 18


def to_array(rows):
    return np.array([[KEYS[c] for c in row] for row in rows], dtype=np.uint8)


def blit(frame, part, x, y):
    h, w = part.shape
    region = frame[y:y + h, x:x + w]
    mask = part != 0
    region[mask] = part[mask]


def legs_lifted(lift_left, lift_right):
    """Front/back legs with one leg lifted a few pixels to suggest a step."""
    legs = to_array(LEGS_STAND)
    out = np.zeros((legs.shape[0] + 2, legs.shape[1]), dtype=np.uint8)
    halves = ((slice(0, 8), lift_left), (slice(8, 16), lift_right))
    for cols, lift in halves:
        out[2 - lift:2 - lift + legs.shape[0], cols] = legs[:, cols]
    return out


def front_or_back_frames(top_rows):
    top = to_array(top_rows)
    frames = []
    for lift_left, lift_right, bob in ((0, 0, 0), (0, 2, 1), (0, 0, 0), (2, 0, 1)):
        frame = np.zeros((FRAME, FRAME), dtype=np.uint8)
        blit(frame, legs_lifted(lift_left, lift_right), OX, LEG_Y - 2)
        blit(frame, top, OX, OY + bob)
        frames.append(frame)
    return frames


def side_frames():
    top = to_array(SIDE_TOP)
    legs = [SIDE_LEGS_STAND, SIDE_LEGS_STRIDE, SIDE_LEGS_STAND, SIDE_LEGS_STRIDE_B]
    frames = []
    for i, leg_rows in enumerate(legs):
        frame = np.zeros((FRAME, FRAME), dtype=np.uint8)
        blit(frame, to_array(leg_rows), OX, LEG_Y)
        blit(frame, top, OX, OY + (1 if i % 2 else 0))
        frames.append(frame)
    return frames


def main():
    frames = front_or_back_frames(FRONT_TOP) + front_or_back_frames(BACK_TOP) + side_frames()
    sheet = np.concatenate(frames, axis=0)
    save_indexed_bmp(GRAPHICS / 'warrior.bmp', sheet, PALETTE)
    (GRAPHICS / 'warrior.json').write_text('{\n    "type": "sprite",\n    "height": 32\n}\n')
    # Preview: frames side by side, 4x scale.
    row = np.concatenate(frames, axis=1)
    save_preview_png(PREVIEW / 'warrior.png', row, PALETTE, scale=4)
    print(f'warrior.bmp: {len(frames)} frames')


if __name__ == '__main__':
    main()
