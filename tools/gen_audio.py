"""Generate placeholder music (ProTracker .mod) and sound effects (.wav) into audio/.

Every tune is original and written here as chords, a melody and a few accompaniment styles; the
samples are tiny synthesized waveforms. Butano turns audio/NAME.mod into bn::music_items::NAME and
audio/NAME.wav into bn::sound_items::NAME.

Run from the tools directory: python3 gen_audio.py
"""
import math
import os
import random
import struct
import wave

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'audio')

# --- samples ----------------------------------------------------------------------------------------
# A looped single-cycle waveform of N samples played at MOD note C-2 (8363 Hz) sounds at 8363 / N Hz:
# N = 32 gives C4. Waveforms of 64 samples sound an octave lower, 16 an octave higher.

RATE = 8363


def clamp8(v):
    return max(-128, min(127, int(round(v))))


def cycle(fn, n, amp=100):
    return [clamp8(fn(i / n) * amp) for i in range(n)]


def square(duty):
    return lambda t: 1.0 if t < duty else -1.0


def triangle(t):
    return 4 * t - 1 if t < 0.5 else 3 - 4 * t


def saw_soft(t):
    # A saw with its corner rounded off by a few harmonics only.
    return sum(math.sin(2 * math.pi * k * t) / k for k in range(1, 6)) * 0.6


def organ(t):
    return (math.sin(2 * math.pi * t) + 0.5 * math.sin(4 * math.pi * t) + 0.25 * math.sin(6 * math.pi * t)) * 0.6


def bell(n_cycles=96, n=32):
    out = []
    total = n_cycles * n
    for i in range(total):
        t = i / n
        env = math.exp(-3.5 * i / total)
        v = math.sin(2 * math.pi * t) + 0.45 * math.sin(2 * math.pi * 2 * t) * math.exp(-6 * i / total) + \
            0.2 * math.sin(2 * math.pi * 3 * t) * math.exp(-9 * i / total)
        out.append(clamp8(v * 70 * env))
    return out


def kick():
    out = []
    phase = 0.0
    length = 1400
    for i in range(length):
        freq = 50 + 120 * math.exp(-i / 180)
        phase += freq / RATE
        out.append(clamp8(math.sin(2 * math.pi * phase) * 120 * math.exp(-i / 500)))
    return out


def snare(seed=7):
    rng = random.Random(seed)
    out = []
    length = 1800
    for i in range(length):
        noise = rng.uniform(-1, 1)
        tone = math.sin(2 * math.pi * 190 * i / RATE)
        out.append(clamp8((noise * 0.8 + tone * 0.35 * math.exp(-i / 200)) * 110 * math.exp(-i / 380)))
    return out


def hat(seed=11):
    rng = random.Random(seed)
    out = []
    prev = 0.0
    for i in range(420):
        noise = rng.uniform(-1, 1)
        high = noise - prev       # a crude high-pass
        prev = noise
        out.append(clamp8(high * 55 * math.exp(-i / 90)))
    return out


def timpani():
    out = []
    phase = 0.0
    for i in range(2600):
        freq = 65 + 25 * math.exp(-i / 300)
        phase += freq / RATE
        v = math.sin(2 * math.pi * phase) + 0.3 * math.sin(4 * math.pi * phase)
        out.append(clamp8(v * 90 * math.exp(-i / 900)))
    return out


# name: (data, looped, default volume, cycle length for pitch)
SAMPLES = {
    'lead': (cycle(square(0.5), 32, 70), True, 40, 32),
    'soft': (cycle(triangle, 32, 110), True, 50, 32),
    'pulse': (cycle(square(0.25), 32, 70), True, 40, 32),
    'bass': (cycle(triangle, 64, 115), True, 56, 64),
    'pad': (cycle(saw_soft, 64, 85), True, 30, 64),
    'organ': (cycle(organ, 32, 100), True, 36, 32),
    'bell': (bell(), False, 46, 32),
    'kick': (kick(), False, 56, 0),
    'snare': (snare(), False, 40, 0),
    'hat': (hat(), False, 28, 0),
    'timpani': (timpani(), False, 54, 0),
}

SAMPLE_ORDER = list(SAMPLES)

# --- notes ------------------------------------------------------------------------------------------

PERIODS = [856, 808, 762, 720, 678, 640, 604, 570, 538, 508, 480, 453,
           428, 404, 381, 360, 339, 320, 302, 285, 269, 254, 240, 226,
           214, 202, 190, 180, 170, 160, 151, 143, 135, 127, 120, 113]

NOTE_NAMES = {'C': 0, 'D': 2, 'E': 4, 'F': 5, 'G': 7, 'A': 9, 'B': 11}


def note_number(name):
    """'C4' -> 48, 'F#3' -> 42, 'Bb2' -> 34 (C0 = 0)."""
    base = NOTE_NAMES[name[0]]
    rest = name[1:]
    if rest.startswith('#'):
        base += 1
        rest = rest[1:]
    elif rest.startswith('b'):
        base -= 1
        rest = rest[1:]
    return base + 12 * int(rest)


def period_for(sample, note):
    """The MOD period that makes the sample sound the musical note."""
    length = SAMPLES[sample][3]
    if not length:
        return 428      # drums play at their own rate (MOD C-2)
    lowest = {16: 4, 32: 3, 64: 2}[length] * 12     # musical note of MOD C-1
    index = note - lowest
    while index < 0:
        index += 12
    while index >= len(PERIODS):
        index -= 12
    return PERIODS[index]


CHORDS = {
    '': [0, 4, 7], 'm': [0, 3, 7], '7': [0, 4, 7, 10], 'm7': [0, 3, 7, 10], 'maj7': [0, 4, 7, 11],
    'sus2': [0, 2, 7], 'sus4': [0, 5, 7], 'dim': [0, 3, 6],
}


def chord_notes(symbol, octave):
    """'Am' in octave 3 -> MIDI-like numbers of A3 C4 E4."""
    root = symbol[0]
    rest = symbol[1:]
    if rest[:1] in ('#', 'b'):
        root += rest[0]
        rest = rest[1:]
    base = note_number(root + str(octave))
    return [base + i for i in CHORDS[rest]]


# --- patterns ---------------------------------------------------------------------------------------

ROWS = 64
BAR = 16


class Cell:
    __slots__ = ('sample', 'note', 'effect', 'param')

    def __init__(self, sample=None, note=None, effect=0, param=0):
        self.sample = sample
        self.note = note
        self.effect = effect
        self.param = param


def empty_pattern():
    return [[Cell() for _ in range(4)] for _ in range(ROWS)]


def put(pattern, row, channel, sample, note, volume=None, decay=0, hold=0):
    """A note with an optional volume and a slide down over the next hold rows."""
    cell = pattern[row][channel]
    cell.sample = sample
    cell.note = note
    if volume is not None:
        cell.effect, cell.param = 0xC, volume
    for r in range(row + 1, min(ROWS, row + 1 + hold)):
        if decay and pattern[r][channel].note is None and pattern[r][channel].effect == 0:
            pattern[r][channel].effect, pattern[r][channel].param = 0xA, decay


def cut(pattern, row, channel):
    cell = pattern[row][channel]
    if cell.note is None:
        cell.effect, cell.param = 0xC, 0


def parse_melody(text):
    """'E5 - - D5 . C5' -> [(step, note or None)], '-' holds, '.' stops the note."""
    tokens = text.split()
    events = []
    for step, token in enumerate(tokens):
        if token == '-':
            continue
        events.append((step, None if token == '.' else note_number(token)))
    return events, len(tokens)


class Song:
    def __init__(self, name, speed, tempo=125):
        self.name = name
        self.speed = speed
        self.tempo = tempo
        self.patterns = []
        self.order = []

    def add(self, pattern, times=1):
        self.patterns.append(pattern)
        index = len(self.patterns) - 1
        self.order.extend([index] * times)
        return index

    def repeat(self, index, times=1):
        self.order.extend([index] * times)


def arrange(chords, melody=None, lead='lead', lead_volume=None, lead_decay=0, harmony='arp', harmony_sample='bell',
            harmony_octave=4, bass='roots', bass_sample='bass', bass_octave=2, drums=None, harmony_volume=None,
            bass_volume=None):
    """One 64-row pattern: four bars of chords with a melody, harmony, bass and drums."""
    p = empty_pattern()
    if melody:
        events, length = parse_melody(melody)
        scale = ROWS // length
        for i, (step, note) in enumerate(events):
            row = step * scale
            next_row = events[i + 1][0] * scale if i + 1 < len(events) else ROWS
            if note is None:
                cut(p, row, 0)
            else:
                put(p, row, 0, lead, note, lead_volume, lead_decay, next_row - row - 1)

    for bar, symbol in enumerate(chords):
        top = bar * BAR
        if symbol == '-':
            continue
        notes = chord_notes(symbol, harmony_octave)
        root = chord_notes(symbol, bass_octave)[0]
        fifth = root + 7

        if harmony == 'arp':
            for step in range(8):
                put(p, top + step * 2, 1, harmony_sample, notes[step % len(notes)], harmony_volume)
        elif harmony == 'arp_up_down':
            seq = notes + notes[-2:0:-1]
            for step in range(8):
                put(p, top + step * 2, 1, harmony_sample, seq[step % len(seq)], harmony_volume)
        elif harmony == 'pad':
            put(p, top, 1, harmony_sample, notes[1], harmony_volume)
        elif harmony == 'stabs':
            for step in (0, 6, 8, 14):
                put(p, top + step, 1, harmony_sample, notes[(step // 6) % len(notes)], harmony_volume, 4, 1)
        elif harmony == 'bells':
            put(p, top, 1, harmony_sample, notes[0] + 12, harmony_volume)
            put(p, top + 8, 1, harmony_sample, notes[2], harmony_volume)

        if bass == 'roots':
            put(p, top, 2, bass_sample, root, bass_volume)
            put(p, top + 8, 2, bass_sample, root, bass_volume)
        elif bass == 'root_fifth':
            for step, note in ((0, root), (4, fifth), (8, root), (12, fifth)):
                put(p, top + step, 2, bass_sample, note, bass_volume, 3, 3)
        elif bass == 'eighths':
            for step in range(0, 16, 2):
                put(p, top + step, 2, bass_sample, root if step != 12 else fifth, bass_volume, 6, 1)
        elif bass == 'whole':
            put(p, top, 2, bass_sample, root, bass_volume)
        elif bass == 'drone':
            put(p, top, 2, bass_sample, root, bass_volume)

    if drums:
        lines = drums.split('|')
        for bar in range(4):
            line = lines[bar % len(lines)].strip()
            for step, ch in enumerate(line[:BAR]):
                sample = {'k': 'kick', 's': 'snare', 'h': 'hat', 't': 'timpani'}.get(ch)
                if sample:
                    put(p, bar * BAR + step, 3, sample, note_number('C4'))
    return p


def set_speed(pattern, speed, tempo=None):
    # The first row of the first pattern: speed (ticks per row) and tempo, on channel 4 if it's free.
    for channel in (3, 2, 1, 0):
        cell = pattern[0][channel]
        if cell.effect == 0:
            cell.effect, cell.param = 0xF, speed
            break
    if tempo:
        for channel in (2, 1, 0, 3):
            cell = pattern[0][channel]
            if cell.effect == 0:
                cell.effect, cell.param = 0xF, tempo
                break


# --- file writers -----------------------------------------------------------------------------------

def write_mod(song):
    set_speed(song.patterns[song.order[0]], song.speed, song.tempo if song.tempo != 125 else None)
    out = bytearray()
    out += song.name.encode('ascii')[:20].ljust(20, b'\0')

    sample_data = []
    for i in range(31):
        if i < len(SAMPLE_ORDER):
            name = SAMPLE_ORDER[i]
            data, looped, volume, _ = SAMPLES[name]
            if len(data) % 2:
                data = data + [0]
            if not looped:
                data = [0, 0] + data[2:]
            words = len(data) // 2
            out += name.encode('ascii').ljust(22, b'\0')
            out += struct.pack('>HBBHH', words, 0, volume, 0, words if looped else 1)
            sample_data.append(bytes((v & 0xFF) for v in data))
        else:
            out += b'\0' * 22 + struct.pack('>HBBHH', 0, 0, 0, 0, 1)

    out += bytes([len(song.order), 127])
    out += bytes(song.order + [0] * (128 - len(song.order)))
    out += b'M.K.'

    for pattern in song.patterns:
        for row in pattern:
            for cell in row:
                period = period_for(cell.sample, cell.note) if cell.note is not None else 0
                number = SAMPLE_ORDER.index(cell.sample) + 1 if cell.sample else 0
                out += bytes([(number & 0xF0) | (period >> 8), period & 0xFF,
                              ((number & 0x0F) << 4) | cell.effect, cell.param])

    for data in sample_data:
        out += data

    with open(os.path.join(OUT, song.name + '.mod'), 'wb') as f:
        f.write(out)


def write_wav(name, samples, rate=11025):
    with wave.open(os.path.join(OUT, name + '.wav'), 'wb') as f:
        f.setnchannels(1)
        f.setsampwidth(1)
        f.setframerate(rate)
        f.writeframes(bytes(max(0, min(255, int(128 + v * 127))) for v in samples))


# --- the tunes --------------------------------------------------------------------------------------

def song_title():
    s = Song('title', 8)
    drums = 't...............|t.......t.......'
    a = arrange(['Dm', 'Bb', 'F', 'C'],
                'D5 - - - - - A4 - D5 - E5 - F5 - - - '
                'F5 - - - E5 - D5 - C5 - - - D5 - - - '
                'A4 - - - - - C5 - F5 - - - E5 - D5 - '
                'C5 - - - - - - - E5 - - - - - - - ',
                lead='lead', lead_volume=36, harmony='pad', harmony_sample='pad', harmony_octave=3,
                bass='whole', drums=drums)
    b = arrange(['Dm', 'Bb', 'C', 'A'],
                'F5 - - - E5 - D5 - E5 - - - A4 - - - '
                'Bb4 - - - C5 - D5 - F5 - - - E5 - - - '
                'E5 - - - D5 - C5 - G5 - - - F5 - E5 - '
                'E5 - - - - - - - C#5 - - - - - - - ',
                lead='lead', lead_volume=36, harmony='pad', harmony_sample='pad', harmony_octave=3,
                bass='whole', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_elwynn():
    s = Song('elwynn', 7)
    drums = '....h.......h...'
    intro = arrange(['G', 'Em', 'C', 'D'], None, harmony='arp_up_down', harmony_sample='bell',
                    bass='roots', bass_volume=40)
    a = arrange(['G', 'Em', 'C', 'D'],
                'B4 - - - D5 - G5 - F#5 - E5 - D5 - - - '
                'E5 - - - G5 - B5 - A5 - G5 - E5 - - - '
                'G5 - - - E5 - C5 - D5 - E5 - G5 - - - '
                'F#5 - - - E5 - D5 - A4 - - - - - - - ',
                lead='soft', lead_volume=44, harmony='arp_up_down', harmony_sample='bell', bass='roots',
                bass_volume=40, drums=drums)
    b = arrange(['C', 'G', 'Am', 'D'],
                'E5 - - - G5 - - - C6 - B5 - A5 - G5 - '
                'G5 - - - D5 - - - B4 - - - G4 - - - '
                'A4 - - - C5 - E5 - A5 - G5 - E5 - - - '
                'F#5 - - - - - - - D5 - - - - - - - ',
                lead='soft', lead_volume=44, harmony='arp_up_down', harmony_sample='bell', bass='roots',
                bass_volume=40, drums=drums)
    s.add(intro)
    s.add(a)
    s.add(b)
    s.repeat(1)
    return s


def song_town():
    s = Song('town', 6)
    drums = 'k...h.h.k...h.h.'
    a = arrange(['F', 'Bb', 'C', 'F'],
                'A4 - C5 - F5 - - - E5 - D5 - C5 - - - '
                'D5 - F5 - Bb5 - - - A5 - G5 - F5 - - - '
                'E5 - G5 - C6 - - - Bb5 - A5 - G5 - - - '
                'A5 - - - F5 - - - C5 - - - - - - - ',
                lead='pulse', lead_volume=34, lead_decay=1, harmony='stabs', harmony_sample='organ',
                bass='root_fifth', drums=drums)
    b = arrange(['Dm', 'Bb', 'C', 'C7'],
                'D5 - F5 - A5 - - - G5 - F5 - D5 - - - '
                'F5 - - - D5 - - - Bb4 - - - D5 - - - '
                'C5 - E5 - G5 - - - F5 - E5 - C5 - - - '
                'E5 - - - G5 - - - Bb5 - - - - - - - ',
                lead='pulse', lead_volume=34, lead_decay=1, harmony='stabs', harmony_sample='organ',
                bass='root_fifth', drums=drums)
    c = arrange(['Bb', 'F', 'Gm', 'C'],
                'D5 - - - C5 - Bb4 - F5 - - - - - - - '
                'C5 - - - A4 - F4 - A4 - C5 - - - - - '
                'Bb4 - - - D5 - G5 - F5 - E5 - D5 - - - '
                'C5 - - - E5 - - - G5 - - - - - - - ',
                lead='soft', lead_volume=40, harmony='arp', harmony_sample='bell', harmony_volume=30,
                bass='roots', drums='k.......h.......')
    s.add(a)
    s.add(b)
    s.repeat(0)
    s.add(c)
    return s


def song_westfall():
    s = Song('westfall', 9)
    drums = 'k.......h.......'
    a = arrange(['Am', 'G', 'F', 'E'],
                'E5 - - - - - D5 - C5 - - - B4 - - - '
                'D5 - - - - - C5 - B4 - - - G4 - - - '
                'C5 - - - - - A4 - F4 - - - A4 - - - '
                'B4 - - - - - - - G#4 - - - - - - - ',
                lead='soft', lead_volume=42, harmony='pad', harmony_sample='pad', harmony_octave=3,
                bass='roots', drums=drums)
    b = arrange(['F', 'C', 'Dm', 'E'],
                'A4 - - - C5 - F5 - E5 - - - C5 - - - '
                'G5 - - - E5 - C5 - G4 - - - - - - - '
                'F5 - - - E5 - D5 - A4 - - - D5 - - - '
                'E5 - - - - - - - B4 - - - - - - - ',
                lead='soft', lead_volume=42, harmony='pad', harmony_sample='pad', harmony_octave=3,
                bass='roots', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_redridge():
    """Lakeshire under the red peaks: a slow march in D."""
    s = Song('redridge', 8)
    drums = 'k.......h...k.h.'
    a = arrange(['D', 'C', 'G', 'D'],
                'A4 - - - D5 - E5 - F#5 - - - E5 - D5 - '
                'E5 - - - C5 - - - G4 - - - C5 - - - '
                'D5 - - - B4 - G4 - B4 - D5 - G5 - - - '
                'F#5 - - - E5 - D5 - A4 - - - - - - - ',
                lead='soft', lead_volume=44, harmony='arp', harmony_sample='bell', harmony_volume=28,
                bass='root_fifth', drums=drums)
    b = arrange(['Bm', 'G', 'A', 'A'],
                'F#5 - - - D5 - B4 - D5 - F#5 - B5 - - - '
                'G5 - - - - - D5 - B4 - - - G4 - - - '
                'A4 - - - C#5 - E5 - A5 - G5 - E5 - - - '
                'C#5 - - - - - - - E5 - - - - - - - ',
                lead='soft', lead_volume=44, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=24, bass='roots', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_duskwood():
    """Duskwood's endless night: a slow, hollow waltz-like lament in D minor."""
    s = Song('duskwood', 11)
    drums = 'k...............|....h.......h...'
    a = arrange(['Dm', 'Bb', 'Gm', 'A'],
                'D5 - - - - - F5 - E5 - D5 - - - - - '
                'F5 - - - - - D5 - Bb4 - - - - - - - '
                'G4 - - - Bb4 - D5 - G5 - F5 - D5 - - - '
                'E5 - - - - - - - C#5 - - - - - - - ',
                lead='soft', lead_volume=40, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=22, bass='roots', drums=drums)
    b = arrange(['Dm', 'C', 'Bb', 'A'],
                'A5 - - - - - G5 - F5 - - - E5 - - - '
                'G5 - - - - - E5 - C5 - - - - - - - '
                'F5 - - - D5 - - - Bb4 - D5 - F5 - - - '
                'E5 - - - - - - - A4 - - - - - - - ',
                lead='soft', lead_volume=40, harmony='arp', harmony_sample='bell', harmony_volume=22,
                bass='root_fifth', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_ironforge():
    """Ironforge: a heavy dwarven march in D minor, organ and timpani under the mountain."""
    s = Song('ironforge', 7)
    drums = 't.......k...t.k.|t.......k.k.t...'
    a = arrange(['Dm', 'Dm', 'Bb', 'A'],
                'D5 - - - D5 - A4 - D5 - F5 - E5 - D5 - '
                'A5 - - - G5 - F5 - E5 - - - D5 - - - '
                'F5 - - - D5 - Bb4 - D5 - F5 - Bb5 - - - '
                'A5 - - - - - - - E5 - - - C#5 - - - ',
                lead='organ', lead_volume=38, harmony='stabs', harmony_sample='pad', harmony_octave=3,
                harmony_volume=26, bass='root_fifth', drums=drums)
    b = arrange(['Gm', 'Dm', 'C', 'A'],
                'G5 - - - Bb5 - - - A5 - G5 - F5 - - - '
                'F5 - - - A5 - - - D5 - - - F5 - - - '
                'E5 - - - G5 - - - C6 - - - E5 - - - '
                'C#5 - - - E5 - - - A5 - - - - - - - ',
                lead='organ', lead_volume=38, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=24, bass='eighths', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_wetlands():
    """The Wetlands: misty and wandering, a lilting tune in A minor over a drone."""
    s = Song('wetlands', 9)
    drums = 'k.......h.......|k.......h...h...'
    a = arrange(['Am', 'G', 'F', 'Em'],
                'E5 - - - A5 - - - G5 - E5 - D5 - - - '
                'D5 - - - G5 - - - B4 - D5 - G5 - - - '
                'F5 - - - C5 - - - A4 - C5 - F5 - - - '
                'E5 - - - - - B4 - G4 - - - B4 - - - ',
                lead='soft', lead_volume=42, harmony='arp_up_down', harmony_sample='bell', harmony_volume=24,
                bass='whole', drums=drums)
    b = arrange(['F', 'C', 'Dm', 'E'],
                'A5 - - - G5 - F5 - C5 - - - F5 - A5 - '
                'G5 - - - - - E5 - C5 - - - - - - - '
                'D5 - - - F5 - A5 - D6 - C6 - A5 - - - '
                'G#5 - - - - - - - E5 - - - B4 - - - ',
                lead='soft', lead_volume=42, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=22, bass='roots', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_hillsbrad():
    """Hillsbrad: green foothills by the sea, a bright folk tune in G major with a lilting step."""
    s = Song('hillsbrad', 8)
    drums = 'k.....h.k.....h.|k.....h.k...h.h.'
    a = arrange(['G', 'C', 'G', 'D'],
                'D5 - - - G5 - - - B5 - A5 - G5 - - - '
                'E5 - - - G5 - - - C6 - B5 - A5 - - - '
                'B5 - - - D6 - B5 - G5 - - - B5 - - - '
                'A5 - - - - - F#5 - D5 - - - - - - - ',
                lead='soft', lead_volume=42, harmony='arp_up_down', harmony_sample='bell', harmony_volume=22,
                bass='root_fifth', drums=drums)
    b = arrange(['Em', 'C', 'Am', 'D'],
                'G5 - - - E5 - G5 - B5 - - - A5 - G5 - '
                'E5 - - - - - G5 - C6 - - - B5 - - - '
                'A5 - - - C6 - A5 - E5 - - - A5 - - - '
                'F#5 - - - A5 - - - D5 - - - - - - - ',
                lead='soft', lead_volume=42, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=22, bass='roots', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_monastery():
    """The Scarlet Monastery: a slow chant in E minor, organ over a low drone and a tolling bell."""
    s = Song('monastery', 12)
    drums = 't...............|................'
    a = arrange(['Em', 'Em', 'C', 'B'],
                'E5 - - - - - G5 - F#5 - - - E5 - - - '
                'B4 - - - - - D5 - E5 - - - - - - - '
                'G5 - - - - - E5 - C5 - - - E5 - - - '
                'D#5 - - - - - - - B4 - - - - - - - ',
                lead='organ', lead_volume=36, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=24, bass='drone', bass_sample='bass', bass_octave=2, bass_volume=30, drums=drums)
    b = arrange(['Am', 'Em', 'C', 'B'],
                'A5 - - - - - C6 - B5 - - - A5 - - - '
                'G5 - - - - - - - E5 - - - - - - - '
                'E5 - - - G5 - - - C6 - - - B5 - A5 - '
                'B5 - - - - - - - F#5 - - - D#5 - - - ',
                lead='bell', lead_volume=40, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=22, bass='drone', bass_sample='bass', bass_octave=2, bass_volume=30, drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_stranglethorn():
    """Stranglethorn Vale: jungle drums under a marimba-like bell tune in D minor pentatonic."""
    s = Song('stranglethorn', 7)
    drums = 'k..t..t.k.t.t...|k..t..t.k.t.t.h.'
    a = arrange(['Dm', 'C', 'Dm', 'Am'],
                'D5 - F5 - A5 - - - G5 - F5 - D5 - - - '
                'C5 - E5 - G5 - - - A5 - G5 - E5 - - - '
                'D5 - F5 - A5 - C6 - D6 - - - C6 - A5 - '
                'A5 - - - G5 - E5 - C5 - - - - - - - ',
                lead='bell', lead_volume=44, harmony='arp_up_down', harmony_sample='soft', harmony_volume=18,
                bass='root_fifth', drums=drums)
    b = arrange(['Gm', 'Dm', 'F', 'A'],
                'G5 - - - A#5 - - - D6 - C6 - A#5 - G5 - '
                'F5 - - - A5 - - - D5 - - - - - - - '
                'F5 - A5 - C6 - - - A5 - G5 - F5 - - - '
                'E5 - - - - - C#5 - A4 - - - - - - - ',
                lead='bell', lead_volume=44, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=20, bass='roots', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_tanaris():
    """Tanaris: hot wind over the dunes, a snaking tune in E Phrygian dominant over hand drums."""
    s = Song('tanaris', 8)
    drums = 'k..t.tk.k..t.t..|k..t.tk.k.t.t.h.'
    a = arrange(['E', 'F', 'E', 'Dm'],
                'E5 - - - F5 - G#5 - A5 - - - G#5 - F5 - '
                'F5 - - - A5 - - - C6 - B5 - A5 - - - '
                'G#5 - - - B5 - - - E5 - - - G#5 - - - '
                'A5 - - - F5 - - - D5 - - - - - - - ',
                lead='soft', lead_volume=42, harmony='arp_up_down', harmony_sample='bell', harmony_volume=18,
                bass='root_fifth', drums=drums)
    b = arrange(['Am', 'Dm', 'F', 'E'],
                'C6 - - - B5 - A5 - E5 - - - A5 - - - '
                'D6 - - - C6 - A5 - F5 - - - D5 - - - '
                'F5 - A5 - C6 - - - B5 - A5 - G#5 - - - '
                'G#5 - - - - - F5 - E5 - - - - - - - ',
                lead='soft', lead_volume=42, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=20, bass='roots', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_feralas():
    """Feralas: an old elven forest, a lilting tune in D Dorian over soft pads, under giant trees."""
    s = Song('feralas', 9)
    drums = 'k.......t.......|k.......t...h...'
    a = arrange(['Dm', 'G', 'Dm', 'C'],
                'D5 - - - E5 - F5 - A5 - - - G5 - - - '
                'B5 - - - A5 - G5 - D5 - - - - - - - '
                'F5 - - - A5 - - - C6 - - - A5 - G5 - '
                'E5 - - - G5 - - - C5 - - - - - - - ',
                lead='bell', lead_volume=40, harmony='arp_up_down', harmony_sample='soft', harmony_volume=16,
                bass='root_fifth', drums=drums)
    b = arrange(['Am', 'G', 'F', 'C'],
                'E5 - - - - - D5 - C5 - - - A4 - - - '
                'B4 - - - D5 - - - G5 - - - - - - - '
                'A5 - - - G5 - F5 - C5 - - - F5 - - - '
                'E5 - - - - - D5 - C5 - - - - - - - ',
                lead='soft', lead_volume=40, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=20, bass='roots', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_steppes():
    """The Burning Steppes: ash and fire under Blackrock Mountain, a grim march in C minor over war drums."""
    s = Song('steppes', 7)
    drums = 'k...k.s.k...k.s.|k.k.k.s.k...s.s.'
    a = arrange(['Cm', 'Ab', 'Cm', 'Bb'],
                'C5 - - - - - G4 - C5 - D5 - Eb5 - - - '
                'Eb5 - - - D5 - C5 - Ab4 - - - - - - - '
                'G5 - - - F5 - Eb5 - D5 - Eb5 - C5 - - - '
                'D5 - - - Bb4 - - - F5 - - - - - - - ',
                lead='organ', lead_volume=38, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=20, bass='eighths', drums=drums)
    b = arrange(['Fm', 'Cm', 'Ab', 'G'],
                'F5 - - - Ab5 - - - C6 - - - Bb5 - Ab5 - '
                'G5 - - - - - Eb5 - C5 - - - - - - - '
                'Ab5 - - - G5 - F5 - Eb5 - - - C5 - - - '
                'B4 - - - D5 - - - G5 - - - - - - - ',
                lead='lead', lead_volume=34, harmony='arp', harmony_sample='organ', harmony_volume=16,
                bass='root_fifth', drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_plaguelands():
    """The Plaguelands: a mournful lament in G minor for a dead land, a lone bell over a cold drone."""
    s = Song('plaguelands', 11)
    drums = 'k...............|k.......t.......'
    a = arrange(['Gm', 'Eb', 'Gm', 'D'],
                'G5 - - - - - A5 - Bb5 - - - A5 - G5 - '
                'G5 - - - F5 - Eb5 - Bb4 - - - - - - - '
                'D5 - - - G5 - - - Bb5 - A5 - G5 - - - '
                'F#5 - - - - - A5 - D5 - - - - - - - ',
                lead='bell', lead_volume=40, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=22, bass='drone', bass_sample='bass', bass_octave=2, bass_volume=30, drums=drums)
    b = arrange(['Cm', 'Gm', 'Eb', 'D'],
                'C6 - - - - - Bb5 - G5 - - - Eb5 - - - '
                'D5 - - - - - G5 - Bb5 - - - - - - - '
                'Eb5 - - - G5 - - - C6 - - - Bb5 - G5 - '
                'A5 - - - - - F#5 - D5 - - - - - - - ',
                lead='organ', lead_volume=34, harmony='arp_up_down', harmony_sample='soft', harmony_volume=16,
                bass='drone', bass_sample='bass', bass_octave=2, bass_volume=30, drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_dungeon():
    s = Song('dungeon', 10)
    drums = 'k...............|k.......k.......'
    a = arrange(['Em', 'F', 'Em', 'D'],
                'E5 - - - - - - - . - - - G5 - F5 - '
                'E5 - - - - - - - . - - - - - - - '
                'B4 - - - - - - - C5 - - - B4 - - - '
                'A4 - - - - - - - . - - - - - - - ',
                lead='bell', lead_volume=44, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=22, bass='drone', bass_sample='bass', bass_octave=2, bass_volume=30, drums=drums)
    b = arrange(['Am', 'Em', 'F', 'B'],
                'C5 - - - - - B4 - A4 - - - - - - - '
                'G4 - - - - - - - E4 - - - - - - - '
                'F4 - - - A4 - - - C5 - - - - - - - '
                'B4 - - - - - - - D#5 - - - - - - - ',
                lead='bell', lead_volume=44, harmony='pad', harmony_sample='pad', harmony_octave=3,
                harmony_volume=22, bass='drone', bass_sample='bass', bass_octave=2, bass_volume=30, drums=drums)
    s.add(a)
    s.add(b)
    return s


def song_boss():
    s = Song('boss', 5)
    drums = 'k.h.s.h.k.k.s.h.'
    a = arrange(['Cm', 'Ab', 'Bb', 'G'],
                'C5 - - - Eb5 - - - G5 - - - F5 - Eb5 - '
                'Eb5 - - - C5 - - - Ab4 - - - C5 - - - '
                'D5 - - - F5 - - - Bb5 - - - Ab5 - G5 - '
                'G5 - - - - - - - B4 - - - D5 - - - ',
                lead='lead', lead_volume=34, harmony='arp', harmony_sample='pulse', harmony_volume=22,
                bass='eighths', drums=drums)
    b = arrange(['Fm', 'Cm', 'Ab', 'G'],
                'F5 - - - Ab5 - - - C6 - - - Ab5 - - - '
                'G5 - - - Eb5 - - - C5 - - - Eb5 - - - '
                'Ab5 - - - G5 - - - F5 - - - Eb5 - - - '
                'D5 - - - B4 - - - G4 - - - - - - - ',
                lead='lead', lead_volume=34, harmony='arp', harmony_sample='pulse', harmony_volume=22,
                bass='eighths', drums=drums)
    c = arrange(['Ab', 'Bb', 'Cm', 'G'],
                'C6 - - - Bb5 - Ab5 - G5 - - - Eb5 - - - '
                'D5 - - - F5 - Bb5 - D6 - - - - - - - '
                'C6 - - - G5 - Eb5 - C5 - - - Eb5 - G5 - '
                'B5 - - - - - - - G5 - - - D5 - B4 - ',
                lead='pulse', lead_volume=34, harmony='arp_up_down', harmony_sample='lead', harmony_volume=20,
                bass='eighths', drums='k.k.s.h.k.k.s.s.')
    s.add(a)
    s.add(b)
    s.repeat(0)
    s.add(c)
    return s


# --- sound effects ----------------------------------------------------------------------------------

def sfx(duration, fn, rate=11025):
    n = int(duration * rate)
    return [fn(i / rate, i / n) for i in range(n)]


def tone(freq_fn, wave_fn=math.sin, env=lambda p: 1 - p):
    phase = [0.0]

    def f(t, p):
        phase[0] += freq_fn(p) / 11025
        return wave_fn(2 * math.pi * phase[0]) * env(p)
    return f


def square_wave(x):
    return 1.0 if math.sin(x) >= 0 else -1.0


def notes_fx(freqs, step, wave_fn=square_wave, volume=0.5):
    def f(t, p):
        index = min(int(t / step), len(freqs) - 1)
        local = (t - index * step) / step
        return wave_fn(2 * math.pi * freqs[index] * t) * volume * (1 - local * 0.6) * (1 - p * 0.3)
    return f


def noise_fx(seed, decay):
    rng = random.Random(seed)
    prev = [0.0]

    def f(t, p):
        prev[0] = prev[0] * 0.55 + rng.uniform(-1, 1) * 0.45   # a crude low-pass
        return prev[0] * math.exp(-p * decay) * 1.4
    return f


def write_sounds():
    write_wav('sfx_hit', sfx(0.09, noise_fx(3, 4)))
    write_wav('sfx_spell', sfx(0.25, tone(lambda p: 300 + 700 * p, env=lambda p: 0.5 * (1 - p))))
    write_wav('sfx_level_up', sfx(0.7, notes_fx([523, 659, 784, 1047], 0.12)))
    write_wav('sfx_quest', sfx(0.45, notes_fx([784, 1175], 0.18, math.sin, 0.6)))
    write_wav('sfx_coin', sfx(0.12, notes_fx([1319, 1760], 0.05, square_wave, 0.35)))
    write_wav('sfx_select', sfx(0.05, tone(lambda p: 880, square_wave, lambda p: 0.3 * (1 - p))))
    write_wav('sfx_death', sfx(0.8, tone(lambda p: 330 - 220 * p, square_wave, lambda p: 0.4 * (1 - p))))


def main():
    os.makedirs(OUT, exist_ok=True)
    for song in (song_title(), song_elwynn(), song_town(), song_westfall(), song_dungeon(), song_boss(),
                 song_redridge(), song_duskwood(), song_ironforge(), song_wetlands(), song_hillsbrad(),
                 song_monastery(), song_stranglethorn(), song_tanaris(),
                 song_feralas(), song_steppes(), song_plaguelands()):
        write_mod(song)
        print(f'{song.name}.mod: {len(song.patterns)} patterns, {len(song.order)} in order')
    write_sounds()
    print('sound effects written')


if __name__ == '__main__':
    main()
