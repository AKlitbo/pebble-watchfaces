/**
 * @file icons.c
 * @brief The Gridlock icon draw helpers: GridCtx-aware placement and the tinted blit, built
 * on top of the shared icon cache (ui/icon_cache.h owns the load-once store, the tint, and
 * the auto-trim margins). Plus the named specs and the wind-direction lookup.
 * @ingroup gridlock_draw
 */
#include "draw/icons.h"
#include <string.h>

// --- named specs ---
// dx/dy is each icon's fix for its own empty space off the standard 5px and text
// bottom placement. hand-tuned where known and 0 where not so tune those in the emulator.
// dy is the icon's own bottom padding and dx its sides.
const IconSpec ICON_HEART           = { RESOURCE_ID_ICON_HEALTH_HEART, -1, 0 };
const IconSpec ICON_UV              = { RESOURCE_ID_ICON_WEATHER_UV, 0, 0 };
const IconSpec ICON_RAIN            = { RESOURCE_ID_ICON_WEATHER_NOW_RAIN, 0, 0 };
const IconSpec ICON_FEET            = { RESOURCE_ID_ICON_HEALTH_STEPS, 0, -1 };
const IconSpec ICON_DISTANCE        = { RESOURCE_ID_ICON_HEALTH_DISTANCE, 0, 0 };
const IconSpec ICON_FIRE            = { RESOURCE_ID_ICON_HEALTH_CALORIES, 0, -1 };
const IconSpec ICON_SNOOZE          = { RESOURCE_ID_ICON_SYSTEM_SNOOZE, -1, 0 };
const IconSpec ICON_TIME_LATE       = { RESOURCE_ID_ICON_TIME_LATE, -1, 0 };
const IconSpec ICON_THERMOMETER     = { RESOURCE_ID_ICON_WEATHER_THERMOMETER, 1, 0 };
const IconSpec ICON_CLOCK           = { RESOURCE_ID_ICON_TIME_CLOCK, -1, -1 };
const IconSpec ICON_DATE_TIME       = { RESOURCE_ID_ICON_DATE_TIME, 0, 0 };
const IconSpec ICON_GLOBE           = { RESOURCE_ID_ICON_SYSTEM_GLOBE, -1, -1 };
const IconSpec ICON_HUMIDITY        = { RESOURCE_ID_ICON_WEATHER_HUMIDITY, 4, 0 };   // about 4px of empty space on the side
const IconSpec ICON_SUNRISE         = { RESOURCE_ID_ICON_WEATHER_SUNRISE, 1, 4 };
const IconSpec ICON_SUNSET          = { RESOURCE_ID_ICON_WEATHER_SUNSET, 1, 0 };
const IconSpec ICON_WIND            = { RESOURCE_ID_ICON_WEATHER_WIND, 6, 0 };       // lots of empty space built in
const IconSpec ICON_BLUETOOTH       = { RESOURCE_ID_ICON_CONNECTION_BLUETOOTH_ON, 0, 0 };
const IconSpec ICON_BLUETOOTH_SLASH = { RESOURCE_ID_ICON_CONNECTION_BLUETOOTH_OFF, 0, 0 };

void blit_tinted(GridCtx *gctx, GBitmap *bmp, GRect dst)
{
    if (!bmp)
    {
        return;
    }

    icon_tint(bmp, gctx->color_icon);
    graphics_context_set_compositing_mode(gctx->ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(gctx->ctx, bmp, dst);
}

void icon_draw_res(GridCtx *gctx, uint32_t res, GAlign align, int dx, int dy)
{
    GBitmap *bmp = icon_get(res);
    if (!bmp)
    {
        return;
    }

    int trim_dx = 0, trim_dy = 0;
    icon_align_trim(align, icon_margins(res), &trim_dx, &trim_dy);

    blit_tinted(gctx, bmp, grid_anchor(gctx, gbitmap_get_bounds(bmp).size,
                                       align, dx + trim_dx, dy + trim_dy));
}

void icon_draw(GridCtx *gctx, const IconSpec *spec, GAlign align, int dx, int dy)
{
    icon_draw_res(gctx, spec->res, align, dx + ICON_DX(spec), dy + ICON_DY(spec));
}

void icon_draw_rect(GridCtx *gctx, uint32_t res, GRect dst)
{
    blit_tinted(gctx, icon_get(res), dst);
}

int icon_visible_width(uint32_t res)
{
    IconMargins m = icon_margins(res);
    return icon_size(res).w - m.e - m.w;
}

uint32_t icon_wind_dir(const char *dir)
{
    if (strcmp(dir, "N") == 0)  return RESOURCE_ID_ICON_WEATHER_WIND_N;
    if (strcmp(dir, "NE") == 0) return RESOURCE_ID_ICON_WEATHER_WIND_NE;
    if (strcmp(dir, "E") == 0)  return RESOURCE_ID_ICON_WEATHER_WIND_E;
    if (strcmp(dir, "SE") == 0) return RESOURCE_ID_ICON_WEATHER_WIND_SE;
    if (strcmp(dir, "S") == 0)  return RESOURCE_ID_ICON_WEATHER_WIND_S;
    if (strcmp(dir, "SW") == 0) return RESOURCE_ID_ICON_WEATHER_WIND_SW;
    if (strcmp(dir, "W") == 0)  return RESOURCE_ID_ICON_WEATHER_WIND_W;
    if (strcmp(dir, "NW") == 0) return RESOURCE_ID_ICON_WEATHER_WIND_NW;
    return RESOURCE_ID_ICON_WEATHER_WIND;
}

IconSpec icon_wind_spec(const char *dir)
{
    return (IconSpec){ icon_wind_dir(dir), ICON_WIND.dx, ICON_WIND.dy };
}
