#!/usr/bin/env python3
"""Reads a cartridge save written by the game and checks the character in it.

Usage: check_save.py SAVE [key=value ...]
  Prints the newest save's version and character, then fails if any key=value doesn't match.
  Keys: version, race, class, level, xp, money, map, minutes (played) and rested (version 4).

Version 4 saves are two slots of chunks (see src/gw_save.cpp); versions 2 and 3 are one struct at the
start of SRAM.
"""

import struct
import sys

MAGIC = b'GBAWOWSV'
SLOT_SIZE = 16 * 1024
HEADER = struct.Struct('<8siIiI')    # magic, version, checksum, size, sequence
RACES = ['Human', 'Dwarf', 'Night Elf']
CLASSES = ['Warrior', 'Mage', 'Hunter']


def fnv1a(data):
    result = 2166136261
    for byte in data:
        result = ((result ^ byte) * 16777619) & 0xFFFFFFFF
    return result


def read_v4(sram):
    best = None
    for slot in range(2):
        offset = slot * SLOT_SIZE
        magic, version, checksum, size, sequence = HEADER.unpack_from(sram, offset)
        if magic != MAGIC or version != 4 or not 0 < size <= SLOT_SIZE - HEADER.size:
            continue
        payload = sram[offset + HEADER.size:offset + HEADER.size + size]
        if fnv1a(payload) == checksum and (best is None or sequence > best[0]):
            best = (sequence, slot, payload)
    if best is None:
        return None
    sequence, slot, payload = best
    position = 0
    while position + 4 <= len(payload):
        tag, length = struct.unpack_from('<HH', payload, position)
        position += 4
        if tag == 0:
            break
        if tag == 1:    # CORE
            race, player_class, level, _, xp, money = struct.unpack_from('<BBBBii', payload, position)
            map_id, = struct.unpack_from('<H', payload, position + 16)
            play_frames, = struct.unpack_from('<I', payload, position + 22)
            rest_xp, = struct.unpack_from('<i', payload, position + 31)
            return {'version': 4, 'slot': slot, 'sequence': sequence, 'race': RACES[race],
                    'class': CLASSES[player_class], 'level': level, 'xp': xp, 'money': money, 'map': map_id,
                    'minutes': play_frames // 3600, 'rested': rest_xp}
        position += length
    return None


def read_legacy(sram):
    magic, version, _ = struct.unpack_from('<8siI', sram, 0)
    if magic != MAGIC or version not in (2, 3):
        return None
    race, player_class, level, _, xp, money = struct.unpack_from('<BBBBii', sram, 16)
    map_id = sram[16 + 16]
    return {'version': version, 'race': RACES[race], 'class': CLASSES[player_class], 'level': level, 'xp': xp,
            'money': money, 'map': map_id}


def main():
    sram = open(sys.argv[1], 'rb').read()
    character = read_v4(sram) or read_legacy(sram)
    if character is None:
        sys.exit('no valid save')
    print(' '.join(f'{key}={value}' for key, value in character.items()))
    failed = False
    for check in sys.argv[2:]:
        key, expected = check.split('=', 1)
        if str(character.get(key)) != expected:
            print(f'expected {key}={expected}, got {character.get(key)}')
            failed = True
    sys.exit(1 if failed else 0)


if __name__ == '__main__':
    main()
