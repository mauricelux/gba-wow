#include "gw_save.h"

#include <cstddef>

#include "bn_math.h"
#include "bn_span.h"
#include "bn_sram.h"
#include "bn_string_view.h"

#include "gw_character.h"
#include "gw_quest_ids.h"

namespace gw
{

namespace
{
    // Saves from version 4 on are a list of chunks, each a tag, a length and the data, so new fields
    // and longer lists load from old saves (missing data keeps its default) without a new version.
    // They alternate between two slots, and the newest complete one wins: a save cut short by the
    // power going off leaves the one before it intact.
    constexpr int version = 4;
    constexpr char magic[8] = { 'G', 'B', 'A', 'W', 'O', 'W', 'S', 'V' };
    constexpr int slot_count = 2;
    constexpr int slot_size = bn::sram::size() / slot_count;

    struct header
    {
        char magic[8];
        int32_t version;
        uint32_t checksum;      // of the payload
        int32_t size;           // payload bytes after the header
        uint32_t sequence;      // grows with every save
    };

    constexpr int max_payload = slot_size - int(sizeof(header));

    // Append only: saves store these values.
    enum class chunk : uint16_t
    {
        END,
        CORE,
        BAGS,
        EQUIPMENT,
        ABILITIES,
        ACTION_BAR,
        TALENTS,        // three trees of eight, from before subclasses: only read
        QUESTS,
        STORY_FLAGS,
        CHESTS,
        TALENT_TREE     // the subclass's tree
    };

    BN_DATA_EWRAM_BSS uint8_t payload[max_payload];

    // For characters from before subclasses: the subclass whose tree had the most talent points.
    subclass_id suggestion = subclass_id::NONE;

    void suggest_subclass(const uint8_t* old_talents)
    {
        // Three trees of eight talents, in the order of the class's subclasses.
        int best = -1;
        int best_points = 0;

        for(int tree = 0; tree < subclasses_per_class; ++tree)
        {
            int points = 0;

            for(int index = 0; index < 8; ++index)
            {
                points += old_talents[tree * 8 + index];
            }

            if(points > best_points)
            {
                best = tree;
                best_points = points;
            }
        }

        // Without talents, the subclass closest to how the class played before: Fireball for mages.
        class_id player_class = character().player_class;

        if(best < 0)
        {
            best = player_class == class_id::MAGE ? 1 : player_class == class_id::HUNTER ? 1 : 0;
        }

        suggestion = class_subclass(player_class, best);
    }

    [[nodiscard]] uint32_t checksum(const uint8_t* bytes, int size)
    {
        uint32_t result = 2166136261u;

        for(int index = 0; index < size; ++index)
        {
            result = (result ^ bytes[index]) * 16777619u;
        }

        return result;
    }

    // Builds the payload, little-endian.
    class writer
    {

    public:
        [[nodiscard]] int size() const
        {
            return _size;
        }

        void begin(chunk tag)
        {
            put16(int(tag));
            _length_at = _size;
            put16(0);
        }

        void end()
        {
            int length = _size - _length_at - 2;
            payload[_length_at] = uint8_t(length);
            payload[_length_at + 1] = uint8_t(length >> 8);
        }

        void put8(int value)
        {
            BN_ASSERT(_size < max_payload, "Save too large");
            payload[_size++] = uint8_t(value);
        }

        void put16(int value)
        {
            put8(value);
            put8(value >> 8);
        }

        void put32(uint32_t value)
        {
            put16(int(value & 0xFFFF));
            put16(int(value >> 16));
        }

    private:
        int _size = 0;
        int _length_at = 0;
    };

    // Reads one chunk; past its end every value reads as zero.
    class reader
    {

    public:
        reader(const uint8_t* data, int size) :
            _data(data),
            _size(size)
        {
        }

        [[nodiscard]] int remaining() const
        {
            return _size - _position;
        }

        [[nodiscard]] int get8()
        {
            return _position < _size ? _data[_position++] : 0;
        }

        [[nodiscard]] int get16()
        {
            int low = get8();
            return low | (get8() << 8);
        }

        [[nodiscard]] int16_t get16_signed()
        {
            return int16_t(get16());
        }

        [[nodiscard]] uint32_t get32()
        {
            uint32_t low = uint32_t(get16());
            return low | (uint32_t(get16()) << 16);
        }

    private:
        const uint8_t* _data;
        int _size;
        int _position = 0;
    };

    void write_payload(writer& out)
    {
        const character_data& data = character();

        // New fields go at the end of their chunk.
        out.begin(chunk::CORE);
        out.put8(int(data.race));
        out.put8(int(data.player_class));
        out.put8(data.level);
        out.put8(data.talent_points_spent);
        out.put32(uint32_t(data.xp));
        out.put32(uint32_t(data.money));
        out.put16(data.health);
        out.put16(data.power);
        out.put16(int(data.map));
        out.put16(data.x);
        out.put16(data.y);
        out.put32(data.play_frames);
        out.put8(data.home);
        out.put32(data.hearthstone_ready);
        out.put32(uint32_t(data.rest_xp));
        out.put32(data.last_rest);
        out.put8(int(data.subclass));
        out.end();

        out.begin(chunk::BAGS);

        for(const item_stack& slot : data.bags)
        {
            out.put16(int(slot.item));
            out.put16(slot.count);
        }

        out.end();

        out.begin(chunk::EQUIPMENT);

        for(item_id item : data.equipment)
        {
            out.put16(int(item));
        }

        out.end();

        out.begin(chunk::ABILITIES);

        for(uint8_t rank : data.ability_ranks)
        {
            out.put8(rank);
        }

        out.end();

        out.begin(chunk::ACTION_BAR);

        for(ability_id ability : data.action_bar)
        {
            out.put16(int(ability));
        }

        out.end();

        out.begin(chunk::TALENT_TREE);

        for(uint8_t rank : data.talents)
        {
            out.put8(rank);
        }

        out.end();

        out.begin(chunk::QUESTS);

        for(int index = 0; index < int(quest_id::COUNT); ++index)
        {
            const quest_progress& progress = data.quests[index];
            out.put8(int(progress.status));

            for(uint8_t count : progress.counts)
            {
                out.put8(count);
            }
        }

        out.end();

        out.begin(chunk::STORY_FLAGS);

        for(uint32_t bits : data.flags)
        {
            out.put32(bits);
        }

        out.end();

        out.begin(chunk::CHESTS);

        for(uint32_t bits : data.chests_opened)
        {
            out.put32(bits);
        }

        out.end();

        out.begin(chunk::END);
        out.end();
    }

    void read_chunk(chunk tag, reader& in)
    {
        character_data& data = character();

        switch(tag)
        {

        case chunk::CORE:
            data.race = race_id(bn::min(in.get8(), int(race_id::COUNT) - 1));
            data.player_class = class_id(bn::min(in.get8(), int(class_id::COUNT) - 1));
            data.level = uint8_t(bn::clamp(in.get8(), 1, max_level));
            data.talent_points_spent = uint8_t(in.get8());
            data.xp = int32_t(in.get32());
            data.money = int32_t(in.get32());
            data.health = in.get16_signed();
            data.power = in.get16_signed();
            data.map = map_id(in.get16());
            data.x = in.get16_signed();
            data.y = in.get16_signed();
            data.play_frames = in.get32();
            data.home = uint8_t(in.get8());
            data.hearthstone_ready = in.get32();
            data.rest_xp = int32_t(in.get32());
            data.last_rest = in.get32();
            data.subclass = subclass_id(in.get8());

            if(data.subclass >= subclass_id::COUNT || (data.subclass != subclass_id::NONE &&
                                                        subclass_class(data.subclass) != data.player_class))
            {
                data.subclass = subclass_id::NONE;
            }
            break;

        case chunk::BAGS:
            for(item_stack& slot : data.bags)
            {
                slot.item = item_id(in.get16());
                slot.count = uint8_t(in.get16());
            }
            break;

        case chunk::EQUIPMENT:
            for(item_id& item : data.equipment)
            {
                item = item_id(in.get16());
            }
            break;

        case chunk::ABILITIES:
            for(int index = 0; index < ability_count; ++index)
            {
                data.ability_ranks[index] = uint8_t(bn::min(in.get8(), rank_count(ability_id(index))));
            }
            break;

        case chunk::ACTION_BAR:
            for(ability_id& ability : data.action_bar)
            {
                ability = ability_id(in.get16());
            }
            break;

        case chunk::TALENTS:
        {
            uint8_t old_talents[24];

            for(uint8_t& rank : old_talents)
            {
                rank = uint8_t(in.get8());
            }

            suggest_subclass(old_talents);
            break;
        }

        case chunk::TALENT_TREE:
            for(uint8_t& rank : data.talents)
            {
                rank = uint8_t(in.get8());
            }
            break;

        case chunk::QUESTS:
            for(quest_progress& progress : data.quests)
            {
                if(in.remaining() <= 0)
                {
                    break;
                }

                progress.status = quest_status(in.get8());

                for(uint8_t& count : progress.counts)
                {
                    count = uint8_t(in.get8());
                }
            }
            break;

        case chunk::STORY_FLAGS:
            for(uint32_t& bits : data.flags)
            {
                bits = in.get32();
            }
            break;

        case chunk::CHESTS:
            for(uint32_t& bits : data.chests_opened)
            {
                bits = in.get32();
            }
            break;

        default:
            // A chunk from a newer version of the game.
            break;
        }
    }

    // Reads the slot's payload into the buffer. Returns false if the slot has no complete save.
    [[nodiscard]] bool read_slot(int slot, header& head)
    {
        int offset = slot * slot_size;
        bn::sram::read_offset(head, offset);

        if(bn::string_view(head.magic, 8) != bn::string_view(magic, 8) || head.version != version ||
           head.size <= 0 || head.size > max_payload)
        {
            return false;
        }

        bn::span<uint8_t> bytes(payload, head.size);
        bn::sram::read_span_offset(bytes, offset + int(sizeof(header)));
        return head.checksum == checksum(payload, head.size);
    }

    // The slot holding the newest complete save, or -1. Its payload is left in the buffer.
    [[nodiscard]] int newest_slot(uint32_t& sequence)
    {
        int result = -1;
        header head;

        for(int slot = 0; slot < slot_count; ++slot)
        {
            if(read_slot(slot, head) && (result < 0 || head.sequence > sequence))
            {
                result = slot;
                sequence = head.sequence;
            }
        }

        if(result >= 0 && result != slot_count - 1)
        {
            // The last slot read is in the buffer: read the newest again.
            (void) read_slot(result, head);
        }

        return result;
    }

    void parse_payload(int size)
    {
        character() = character_data();
        suggestion = subclass_id::NONE;
        int position = 0;

        while(position + 4 <= size)
        {
            auto tag = chunk(payload[position] | (payload[position + 1] << 8));
            int length = payload[position + 2] | (payload[position + 3] << 8);
            position += 4;

            if(tag == chunk::END || position + length > size)
            {
                break;
            }

            reader in(payload + position, length);
            read_chunk(tag, in);
            position += length;
        }
    }

    // Versions 2 and 3 stored a copy of the character struct of the time at the start of SRAM, with
    // 8-bit ids. Version 2 had no opened chests at the end.
    namespace legacy
    {
        constexpr int version_3 = 3;
        constexpr int version_2 = 2;

        struct item_stack
        {
            uint8_t item;
            uint8_t count;
        };

        struct quest_progress
        {
            uint8_t status;
            uint8_t counts[3];
        };

        struct character_data
        {
            uint8_t race;
            uint8_t player_class;
            uint8_t level;
            uint8_t talent_points_spent;
            int32_t xp;
            int32_t money;
            int16_t health;
            int16_t power;
            uint8_t map;
            int16_t x;
            int16_t y;
            item_stack bags[16];
            uint8_t equipment[8];
            uint32_t known_abilities;
            uint8_t action_bar[7];
            uint8_t talents[24];
            quest_progress quests[32];
            uint32_t flags;
            uint32_t play_frames;
            uint8_t home;
            uint32_t hearthstone_ready;
            uint32_t chests_opened[2];
        };

        static_assert(sizeof(character_data) == 252);
        static_assert(offsetof(character_data, chests_opened) == 244);

        struct save_file
        {
            char magic[8];
            int32_t version;
            uint32_t checksum;
            character_data character;
        };

        [[nodiscard]] bool read(save_file& file)
        {
            bn::sram::read(file);

            if(bn::string_view(file.magic, 8) != bn::string_view(magic, 8))
            {
                return false;
            }

            auto bytes = reinterpret_cast<const uint8_t*>(&file.character);

            if(file.version == version_2)
            {
                if(file.checksum != checksum(bytes, int(offsetof(character_data, chests_opened))))
                {
                    return false;
                }

                file.character.chests_opened[0] = 0;
                file.character.chests_opened[1] = 0;
                return true;
            }

            return file.version == version_3 && file.checksum == checksum(bytes, int(sizeof(character_data)));
        }

        void migrate(const character_data& old)
        {
            gw::character_data& data = character();
            data = gw::character_data();
            data.race = race_id(old.race);
            data.player_class = class_id(old.player_class);
            data.level = old.level;
            data.talent_points_spent = old.talent_points_spent;
            data.xp = old.xp;
            data.money = old.money;
            data.health = old.health;
            data.power = old.power;
            data.map = map_id(old.map);
            data.x = old.x;
            data.y = old.y;

            for(int index = 0; index < 16; ++index)
            {
                data.bags[index].item = item_id(old.bags[index].item);
                data.bags[index].count = old.bags[index].count;
            }

            for(int index = 0; index < 8; ++index)
            {
                data.equipment[index] = item_id(old.equipment[index]);
            }

            for(int index = 0; index < 32; ++index)
            {
                if(old.known_abilities & (1u << index))
                {
                    data.ability_ranks[index] = 1;
                }
            }

            for(int index = 0; index < 7; ++index)
            {
                data.action_bar[index] = ability_id(old.action_bar[index]);
            }

            suggest_subclass(old.talents);

            for(int index = 0; index < 32; ++index)
            {
                data.quests[index].status = quest_status(old.quests[index].status);

                for(int objective = 0; objective < 3; ++objective)
                {
                    data.quests[index].counts[objective] = old.quests[index].counts[objective];
                }
            }

            data.flags[0] = old.flags;
            data.play_frames = old.play_frames;
            data.home = old.home;
            data.hearthstone_ready = old.hearthstone_ready;
            data.chests_opened[0] = old.chests_opened[0];
            data.chests_opened[1] = old.chests_opened[1];

            // Rested experience starts building from here.
            data.last_rest = old.play_frames;
        }
    }

    static_assert(bag_slots >= 16 && int(equip_slot::COUNT) >= 8 && action_slots >= 7 && ability_count >= 32 &&
                  max_quests >= 32 && max_chests >= 64, "legacy::migrate copies the old lists whole");
}

bool has_save()
{
    uint32_t sequence = 0;

    if(newest_slot(sequence) >= 0)
    {
        return true;
    }

    legacy::save_file file;
    return legacy::read(file);
}

bool load_game()
{
    uint32_t sequence = 0;
    int slot = newest_slot(sequence);

    if(slot >= 0)
    {
        header head;
        bn::sram::read_offset(head, slot * slot_size);
        parse_payload(head.size);
        return true;
    }

    legacy::save_file file;

    if(! legacy::read(file))
    {
        return false;
    }

    // The old save stays in the first slot until the second save after this one replaces it.
    legacy::migrate(file.character);
    return true;
}

void save_game()
{
    uint32_t sequence = 0;
    int newest = newest_slot(sequence);
    int slot = newest >= 0 ? (newest + 1) % slot_count : slot_count - 1;

    writer out;
    write_payload(out);

    header head;

    for(int index = 0; index < 8; ++index)
    {
        head.magic[index] = magic[index];
    }

    head.version = version;
    head.size = out.size();
    head.checksum = checksum(payload, out.size());
    head.sequence = newest >= 0 ? sequence + 1 : 1;

    // The payload first and the header last, so a save cut short never looks complete.
    int offset = slot * slot_size;
    bn::span<const uint8_t> bytes(payload, out.size());
    bn::sram::write_span_offset(bytes, offset + int(sizeof(header)));
    bn::sram::write_offset(head, offset);
}

subclass_id suggested_subclass()
{
    return suggestion;
}

void erase_save()
{
    bn::sram::clear(bn::sram::size());
}

}
