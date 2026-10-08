#ifndef GW_SAVE_H
#define GW_SAVE_H

namespace gw
{

// The character is saved to the cartridge's SRAM.

[[nodiscard]] bool has_save();

// Loads the saved character. Returns false (and changes nothing) if there is no valid save.
bool load_game();

void save_game();

void erase_save();

}

#endif
