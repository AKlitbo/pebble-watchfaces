/**
 * @file fonts.c
 * @brief Font registry implementation: a flat id-indexed table of loaded handles
 */
#include "ui/fonts.h"

static GFont s_fonts[FONT_SLOTS_MAX]; // loaded font handles
// whether the app loaded the handle in that slot and so has to free it. a system font is parked
// with fonts_register_system and stays false, because unloading one the firmware owns faults
static bool s_owned[FONT_SLOTS_MAX];

void fonts_register(FontId id, GFont handle)
{
    if (id >= FONT_SLOTS_MAX)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "fonts_register: id %d out of range", id);
        return;
    }

    s_fonts[id] = handle;
    s_owned[id] = true;
}

void fonts_register_system(FontId id, GFont handle)
{
    if (id >= FONT_SLOTS_MAX)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "fonts_register_system: id %d out of range", id);
        return;
    }

    s_fonts[id] = handle;
    s_owned[id] = false;
}

GFont fonts_get(FontId id)
{
    if (id < FONT_SLOTS_MAX && s_fonts[id])
    {
        return s_fonts[id];
    }

    // a mis-wired zone resolves the same id every tick so log the miss once per
    // id rather than spamming the log. out-of-range is a build-time bug so log it plainly
    if (id < FONT_SLOTS_MAX)
    {
        static bool s_warned[FONT_SLOTS_MAX];
        if (!s_warned[id])
        {
            s_warned[id] = true;
            APP_LOG(APP_LOG_LEVEL_ERROR, "fonts_get: id %d unregistered", id);
        }
    }
    else
    {
        // out-of-range can't index s_warned[FONT_SLOTS_MAX] so dedup with its own flag
        static bool s_oor_warned;
        if (!s_oor_warned)
        {
            s_oor_warned = true;
            APP_LOG(APP_LOG_LEVEL_ERROR, "fonts_get: id %d out of range", id);
        }
    }

    return fonts_get_system_font(FONT_KEY_GOTHIC_24);
}

void fonts_unload_all(void)
{
    for (int i = 0; i < FONT_SLOTS_MAX; i++)
    {
        if (s_fonts[i] && s_owned[i])
        {
            GFont handle = s_fonts[i];
            fonts_unload_custom_font(handle);

            // one handle can sit in several slots, so clear every copy of it before moving on
            // or the next slot holding it would unload it a second time
            for (int j = i; j < FONT_SLOTS_MAX; j++)
            {
                if (s_fonts[j] == handle)
                {
                    s_fonts[j] = NULL;
                    s_owned[j] = false;
                }
            }
        }

        s_fonts[i] = NULL;
        s_owned[i] = false;
    }
}
