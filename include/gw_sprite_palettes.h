#ifndef GW_SPRITE_PALETTES_H
#define GW_SPRITE_PALETTES_H

#include "bn_sprite_palette_item.h"

namespace gw::sprite_palettes
{

// The GBA has 16 sprite palettes for everything on screen, and Butano stops the game with an error when
// a sprite needs a 17th. Characters take one per look, the hud's icons one, and each kind of effect its
// own, so every sprite made while the world runs checks for room first and waits or goes without.
// Full-screen scenes (the menu, flights, voyages) drop the map's sprites before they make theirs.

// True when a sprite with this palette can be made while `reserve` more palettes stay free: its palette
// is already in use (sprites with the same colors share it), or enough are free.
[[nodiscard]] bool fits(const bn::sprite_palette_item& item, int reserve = 0);

// For NPCs: also keeps a palette free for the quest markers, the target ring and the floating text if
// they aren't on screen yet.
[[nodiscard]] bool npc_fits(const bn::sprite_palette_item& item);

// For enemies, the pet and chests: also keeps a palette free for each effect of a fight (target ring,
// floating text, projectiles, cast and pet bars, quest markers and loot sparkles, area circles) that
// isn't on screen yet, so a crowd of different looks can't leave a fight without them.
[[nodiscard]] bool character_fits(const bn::sprite_palette_item& item);

}

#endif
