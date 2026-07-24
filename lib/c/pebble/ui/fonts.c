/**
 * @file fonts.c
 * @brief Font registry implementation: a flat id-indexed table of loaded handles
 */
#include "ui/fonts.h"

static GFont s_fonts[FONT_SLOTS_MAX]; // loaded font handles

void fonts_register(FontId id, GFont handle)
{
    if (id >= FONT_SLOTS_MAX)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "fonts_register: id %d out of range", id);
        return;
    }

    s_fonts[id] = handle;
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
        if (s_fonts[i])
        {
            fonts_unload_custom_font(s_fonts[i]);
            s_fonts[i] = NULL;
        }
    }
}
