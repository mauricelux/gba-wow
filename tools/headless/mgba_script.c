/*
 * Headless mGBA runner for quick checks without a display.
 *
 * Usage: mgba_script <rom.gba> <command>...
 *   wait:N            run N frames with no keys held
 *   hold:KEYS:N       hold KEYS for N frames (letters: U D L R A B S=start E=select l=L r=R)
 *   shot:FILE.ppm     write the current frame as a binary PPM
 *
 * Build: cc -O2 -o mgba_script mgba_script.c -lmgba
 * (needs libmgba-dev; on Debian/Ubuntu: apt install libmgba-dev)
 */

#include <mgba/core/core.h>
#include <mgba/core/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GBA key bits, in hardware order.
enum { GBA_KEY_A, GBA_KEY_B, GBA_KEY_SELECT, GBA_KEY_START, GBA_KEY_RIGHT, GBA_KEY_LEFT, GBA_KEY_UP,
       GBA_KEY_DOWN, GBA_KEY_R, GBA_KEY_L };

static void quiet_log(struct mLogger* logger, int category, enum mLogLevel level, const char* format, va_list args)
{
    (void) logger; (void) category; (void) level; (void) format; (void) args;
}

static uint32_t parse_keys(const char* keys)
{
    uint32_t mask = 0;
    for(const char* c = keys; *c; ++c)
    {
        switch(*c)
        {
        case 'A': mask |= 1 << GBA_KEY_A; break;
        case 'B': mask |= 1 << GBA_KEY_B; break;
        case 'E': mask |= 1 << GBA_KEY_SELECT; break;
        case 'S': mask |= 1 << GBA_KEY_START; break;
        case 'R': mask |= 1 << GBA_KEY_RIGHT; break;
        case 'L': mask |= 1 << GBA_KEY_LEFT; break;
        case 'U': mask |= 1 << GBA_KEY_UP; break;
        case 'D': mask |= 1 << GBA_KEY_DOWN; break;
        case 'r': mask |= 1 << GBA_KEY_R; break;
        case 'l': mask |= 1 << GBA_KEY_L; break;
        default: fprintf(stderr, "unknown key '%c'\n", *c); exit(2);
        }
    }
    return mask;
}

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        fprintf(stderr, "usage: %s rom.gba command...\n", argv[0]);
        return 2;
    }

    static struct mLogger logger = { .log = quiet_log };
    mLogSetDefaultLogger(&logger);

    struct mCore* core = mCoreFind(argv[1]);
    if(! core || ! core->init(core))
    {
        fprintf(stderr, "cannot create a core for %s\n", argv[1]);
        return 1;
    }

    mCoreInitConfig(core, NULL);
    unsigned width, height;
    core->desiredVideoDimensions(core, &width, &height);
    color_t* buffer = calloc((size_t) width * height, sizeof(color_t));
    core->setVideoBuffer(core, buffer, width);

    if(! mCoreLoadFile(core, argv[1]))
    {
        fprintf(stderr, "cannot load %s\n", argv[1]);
        return 1;
    }

    core->reset(core);

    for(int i = 2; i < argc; ++i)
    {
        char command[256];
        strncpy(command, argv[i], sizeof(command) - 1);
        command[sizeof(command) - 1] = 0;

        if(strncmp(command, "wait:", 5) == 0)
        {
            int frames = atoi(command + 5);
            core->setKeys(core, 0);
            for(int f = 0; f < frames; ++f)
            {
                core->runFrame(core);
            }
        }
        else if(strncmp(command, "hold:", 5) == 0)
        {
            char* keys = command + 5;
            char* colon = strchr(keys, ':');
            if(! colon)
            {
                fprintf(stderr, "bad hold command: %s\n", argv[i]);
                return 2;
            }
            *colon = 0;
            int frames = atoi(colon + 1);
            core->setKeys(core, parse_keys(keys));
            for(int f = 0; f < frames; ++f)
            {
                core->runFrame(core);
            }
            core->setKeys(core, 0);
        }
        else if(strcmp(command, "reset") == 0)
        {
            // Restarts the game; save data in memory survives, like turning the console off and on.
            core->reset(core);
        }
        else if(strncmp(command, "shot:", 5) == 0)
        {
            FILE* file = fopen(command + 5, "wb");
            if(! file)
            {
                perror(command + 5);
                return 1;
            }
            fprintf(file, "P6\n%u %u\n255\n", width, height);
            for(unsigned p = 0; p < width * height; ++p)
            {
                uint32_t c = buffer[p];
                unsigned char rgb[3] = { c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF };
                fwrite(rgb, 1, 3, file);
            }
            fclose(file);
        }
        else
        {
            fprintf(stderr, "unknown command: %s\n", argv[i]);
            return 2;
        }
    }

    core->deinit(core);
    free(buffer);
    return 0;
}
