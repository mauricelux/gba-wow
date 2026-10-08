#include "gw_save.h"

#include "bn_sram.h"
#include "bn_string_view.h"

#include "gw_character.h"

namespace gw
{

namespace
{
    // Bump when character_data changes so old saves are ignored instead of misread.
    constexpr int version = 2;
    constexpr char magic[8] = { 'G', 'B', 'A', 'W', 'O', 'W', 'S', 'V' };

    struct save_file
    {
        char magic[8];
        int32_t version;
        uint32_t checksum;
        character_data character;
    };

    [[nodiscard]] uint32_t checksum(const character_data& data)
    {
        auto bytes = reinterpret_cast<const uint8_t*>(&data);
        uint32_t result = 2166136261u;

        for(int index = 0; index < int(sizeof(data)); ++index)
        {
            result = (result ^ bytes[index]) * 16777619u;
        }

        return result;
    }

    [[nodiscard]] bool read_valid(save_file& file)
    {
        bn::sram::read(file);

        if(bn::string_view(file.magic, 8) != bn::string_view(magic, 8) || file.version != version)
        {
            return false;
        }

        return file.checksum == checksum(file.character);
    }
}

bool has_save()
{
    save_file file;
    return read_valid(file);
}

bool load_game()
{
    save_file file;

    if(! read_valid(file))
    {
        return false;
    }

    character() = file.character;
    return true;
}

void save_game()
{
    save_file file;

    for(int index = 0; index < 8; ++index)
    {
        file.magic[index] = magic[index];
    }

    file.version = version;
    file.character = character();
    file.checksum = checksum(file.character);
    bn::sram::write(file);
}

void erase_save()
{
    bn::sram::clear(sizeof(save_file));
}

}
