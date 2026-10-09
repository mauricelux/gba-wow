#ifndef GW_SAVE_H
#define GW_SAVE_H

#include "gw_ids.h"

namespace gw
{

// The character is saved to the cartridge's SRAM.

[[nodiscard]] bool has_save();

// Loads the saved character. Returns false (and changes nothing) if there is no valid save.
bool load_game();

void save_game();

// For a character from before subclasses (loaded with subclass NONE): the subclass to preselect, the
// one whose talent tree had the most points.
[[nodiscard]] subclass_id suggested_subclass();

void erase_save();

}

#endif
