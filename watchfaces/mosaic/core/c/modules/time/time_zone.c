#include "engine/grid_engine.h"
#include "time_zone.h"
#include "mosaic/draw/grid_helpers.h"
#include "io/stores/time_store.h"
#include "settings_schema.h"
#include "dev/dev.h"
#include <pebble.h>

static void time_zone_body(GridCtx *gctx, uint8_t index)
{
    if (gctx->size != MSIZE_1x2)
    {
        return;
    }

    char val[16] = "--:--";
    bool h12 = false;
    bool is_am = false;

    // get the current UTC time
    time_t now = time(NULL);

    // add the configured offset in minutes
    int16_t offset_minutes = gridlock_time_zone_offset_minutes(index);
    now += (offset_minutes * 60);

    // turn it back into tm with gmtime so it holds the other zone's time
    struct tm *t = gmtime(&now);

    if (t)
    {
        dev_force_time(t);
        h12 = gridlock_format_clock(val, sizeof(val), t->tm_hour, t->tm_min);
        is_am = t->tm_hour < 12;
    }

    gh_stat_1x2(gctx, val, h12 ? (is_am ? "AM" : "PM") : NULL, FONT_TEKO_26, &ICON_GLOBE);
}

static void time_zone_1_body(GridCtx *gctx)
{
    time_zone_body(gctx, 0);
}

static char s_tz_label[13];

static const char* time_zone_1_get_label(ModuleSize size)
{
    (void)size;
    const char *name = gridlock_time_zone_name(0);
    // copy up to 12 chars (stopping at comma) and convert to upper case
    int i = 0;
    while (i < 12 && name[i] != '\0' && name[i] != ',')
    {
        char c = name[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        s_tz_label[i] = c;
        i++;
    }
    s_tz_label[i] = '\0';
    return s_tz_label;
}

const ModuleDef mod_time_zone_1_def = {
    .label = "ZONE 1",
    .get_label = time_zone_1_get_label,
    .sizes = SZ_1x2,
    .features = FEATURE_TIME,
    .body = time_zone_1_body
};
