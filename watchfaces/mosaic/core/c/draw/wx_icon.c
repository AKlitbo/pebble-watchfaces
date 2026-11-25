#include "mosaic/draw/wx_icon.h"
#include "io/stores/weather_store.h"
#include "ui/weather/icons.h"
#include <string.h>

/**
 * One cached picture and the condition it was loaded for. cached is sized to match the weather
 * store's cond[32] so a long condition is not cut short here, which would make the strcmp miss
 * and reload on every repaint.
 */
typedef struct
{
    GBitmap *bmp;
    IconMargins margin;
    char cached[32];
} WxIconSlot;

// each set owns its one picture rather than going through the shared icon cache. each glyph the
// cache loads stays there until a full cache pushes it out, so a day of changing weather would
// hold several at once
static WxIconSlot s_now;
static WxIconSlot s_big;

// each 24px glyph and its 40px twin. the framework's table picks the 24px one for a condition, so
// the night glyphs and the fallback for an unknown token stay its rules and only the size is ours
static const uint32_t s_big_for_now[][2] = {
    { RESOURCE_ID_ICON_WEATHER_NOW_CLEAR, RESOURCE_ID_ICON_WEATHER_BIG_CLEAR },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_CLEAR, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_CLEAR },
    { RESOURCE_ID_ICON_WEATHER_NOW_CLOUDY, RESOURCE_ID_ICON_WEATHER_BIG_CLOUDY },
    { RESOURCE_ID_ICON_WEATHER_NOW_FOG, RESOURCE_ID_ICON_WEATHER_BIG_FOG },
    { RESOURCE_ID_ICON_WEATHER_NOW_DRIZZLE, RESOURCE_ID_ICON_WEATHER_BIG_DRIZZLE },
    { RESOURCE_ID_ICON_WEATHER_NOW_RAIN, RESOURCE_ID_ICON_WEATHER_BIG_RAIN },
    { RESOURCE_ID_ICON_WEATHER_NOW_SLEET, RESOURCE_ID_ICON_WEATHER_BIG_SLEET },
    { RESOURCE_ID_ICON_WEATHER_NOW_SNOW, RESOURCE_ID_ICON_WEATHER_BIG_SNOW },
    { RESOURCE_ID_ICON_WEATHER_NOW_SHOWERS, RESOURCE_ID_ICON_WEATHER_BIG_SHOWERS },
    { RESOURCE_ID_ICON_WEATHER_NOW_SNOW_WIND, RESOURCE_ID_ICON_WEATHER_BIG_SNOW_WIND },
    { RESOURCE_ID_ICON_WEATHER_NOW_THUNDERSTORM, RESOURCE_ID_ICON_WEATHER_BIG_THUNDERSTORM },
    { RESOURCE_ID_ICON_WEATHER_NOW_NA, RESOURCE_ID_ICON_WEATHER_BIG_NA },
    { RESOURCE_ID_ICON_WEATHER_NOW_PARTLY_CLOUDY, RESOURCE_ID_ICON_WEATHER_BIG_PARTLY_CLOUDY },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_PARTLY_CLOUDY, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_PARTLY_CLOUDY },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_CLOUDY, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_CLOUDY },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_FOG, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_FOG },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_DRIZZLE, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_DRIZZLE },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_SLEET, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_SLEET },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_RAIN, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_RAIN },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_SNOW, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_SNOW },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_SHOWERS, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_SHOWERS },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_SNOW_WIND, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_SNOW_WIND },
    { RESOURCE_ID_ICON_WEATHER_NOW_NIGHT_THUNDERSTORM, RESOURCE_ID_ICON_WEATHER_BIG_NIGHT_THUNDERSTORM },
};

static uint32_t big_resource_for(const char *cond)
{
    uint32_t now = wx_resource_for(cond);

    for (size_t i = 0; i < ARRAY_LENGTH(s_big_for_now); i++)
    {
        if (s_big_for_now[i][0] == now)
        {
            return s_big_for_now[i][1];
        }
    }

    // a glyph the framework adds before this table learns its twin still draws, just at 24px
    return now;
}

static void slot_clear(WxIconSlot *slot)
{
    if (slot->bmp)
    {
        gbitmap_destroy(slot->bmp);
        slot->bmp = NULL;
    }
    slot->cached[0] = '\0';
    slot->margin = (IconMargins){ 0, 0, 0, 0 };
}

static GBitmap *slot_get(WxIconSlot *slot, uint32_t (*resource_for)(const char *), IconMargins *margin)
{
    const char *cond = weather_store_cond();
    if (!cond[0])
    {
        // no real condition (e.g. a failed fetch cleared the hub). drop any stale picture
        // so a module doesn't draw the last weather's icon next to a "--" readout
        slot_clear(slot);
        *margin = slot->margin;
        return NULL;
    }

    if (strcmp(cond, slot->cached) != 0)
    {
        slot_clear(slot);
        slot->bmp = gbitmap_create_with_resource(resource_for(cond));
        // only remember the condition once the picture actually loaded, so a transient
        // allocation failure is retried on the next repaint instead of cached as a blank
        if (slot->bmp)
        {
            slot->margin = icon_margins_of(slot->bmp);
            strncpy(slot->cached, cond, sizeof(slot->cached) - 1);
            slot->cached[sizeof(slot->cached) - 1] = '\0';
        }
    }

    *margin = slot->margin;
    return slot->bmp;
}

GBitmap *wx_icon_get(IconMargins *margin)
{
    return slot_get(&s_now, wx_resource_for, margin);
}

GBitmap *wx_icon_get_big(IconMargins *margin)
{
    return slot_get(&s_big, big_resource_for, margin);
}

void wx_icon_cleanup(void)
{
    slot_clear(&s_now);
    slot_clear(&s_big);
}
