#include "gw_sprite_palettes.h"

#include "bn_span.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_palettes.h"

#include "bn_sprite_items_fx_castbar.h"
#include "bn_sprite_items_fx_circle.h"
#include "bn_sprite_items_fx_markers.h"
#include "bn_sprite_items_fx_projectiles.h"
#include "bn_sprite_items_fx_target.h"
#include "bn_sprite_items_fx_text_white.h"

namespace gw::sprite_palettes
{

namespace
{
    constexpr int colors_per_palette = 16;

    [[nodiscard]] bool fits_with(const bn::sprite_palette_item& item, bn::span<const bn::sprite_palette_item> kept)
    {
        int reserve = 0;

        for(const bn::sprite_palette_item& effect : kept)
        {
            if(! effect.find_palette())
            {
                ++reserve;
            }
        }

        return fits(item, reserve);
    }
}

bool fits(const bn::sprite_palette_item& item, int reserve)
{
    if(item.find_palette())
    {
        return true;
    }

    return bn::sprite_palettes::available_colors_count() >= (1 + reserve) * colors_per_palette;
}

bool npc_fits(const bn::sprite_palette_item& item)
{
    // Every floating text sheet shares fx_text_white's palette.
    const bn::sprite_palette_item kept[] = {
        bn::sprite_items::fx_markers.palette_item(),
        bn::sprite_items::fx_target.palette_item(),
        bn::sprite_items::fx_text_white.palette_item(),
    };

    return fits_with(item, kept);
}

bool character_fits(const bn::sprite_palette_item& item)
{
    // The pet bar shares the cast bar's palette.
    const bn::sprite_palette_item kept[] = {
        bn::sprite_items::fx_target.palette_item(),
        bn::sprite_items::fx_text_white.palette_item(),
        bn::sprite_items::fx_projectiles.palette_item(),
        bn::sprite_items::fx_castbar.palette_item(),
        bn::sprite_items::fx_markers.palette_item(),
        bn::sprite_items::fx_circle.palette_item(),
    };

    return fits_with(item, kept);
}

}
