#ifndef GW_NPC_DATA_H
#define GW_NPC_DATA_H

#include "gw_ids.h"
#include "gw_look_ids.h"

namespace gw
{

namespace npc_flag
{
    constexpr uint8_t VENDOR = 1;       // sells goods and buys anything
    constexpr uint8_t TRAINER = 2;      // teaches abilities of trainer_class
    constexpr uint8_t INNKEEPER = 4;
}

struct npc_info
{
    const char* name;
    const char* subtitle;   // "<Warrior Trainer>" style role, or empty
    look_id look;
    uint8_t flags;
    class_id trainer_class;
    uint8_t vendor;         // stock list index (gw_vendors), for vendors
    const char* gossip;     // what they say when spoken to
};

[[nodiscard]] const npc_info& get_npc_info(npc_id npc);

}

#endif
