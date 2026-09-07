/**
 * @file system_alarm.c
 * @brief The next alarm panel, drawn at two sizes from the watch's own alarm.
 *   1x2  the alarm time with a small grey am/pm after it, like the clock, and the bell on the right
 *   2x2  the same time up top, how long is left as the caption, and a bar that fills as the
 *        alarm approaches
 *
 * A smart alarm reports the end of its wake window, so the time shown is the latest it can go off.
 *
 * @ingroup mosaic_mod_system
 */
#include "engine/grid_engine.h"
#include "system_alarm.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/icons.h"
#include "io/stores/system_store.h"
#include "clock/duration.h"
#include "settings_schema.h"

#include <stdio.h>
#include <time.h>

// how far ahead the 2x2 bar spans: a full window out the bar reads empty, filling as the alarm
// nears and topping out right as it goes off
#define ALARM_WINDOW_S (12 * 60 * 60)

static void system_alarm_body(GridCtx *gctx)
{
    time_t alarm = 0;
    bool has_alarm = system_store_next_alarm(&alarm);

    char val[8] = "--:--";
    char caption[16] = "";
    bool meridiem = false;
    bool is_am = false;
    int progress = 0;

    if (has_alarm)
    {
        struct tm lt = *localtime(&alarm);
        meridiem = gridlock_format_clock(val, sizeof(val), lt.tm_hour, lt.tm_min);
        is_am = lt.tm_hour < 12;

        long remaining = (long)(alarm - time(NULL));
        if (remaining < 0)
        {
            remaining = 0;
        }

        char left[12];
        duration_hm_compact(left, sizeof(left), (int)(remaining / 60));
        snprintf(caption, sizeof(caption), "IN %s", left);

        // the bar fills as the alarm approaches: empty a full window out, full as it goes off
        long elapsed = ALARM_WINDOW_S - remaining;
        if (elapsed < 0)
        {
            elapsed = 0;
        }
        progress = (int)((elapsed * 100) / ALARM_WINDOW_S);
    }

    if (gctx->size == MSIZE_1x2)
    {
        gh_stat_1x2(gctx, val, meridiem ? (is_am ? "AM" : "PM") : NULL, FONT_TEKO_26, &ICON_ALARM);
    }
    else if (gctx->size == MSIZE_2x2)
    {
        gh_stat_time_2x2(gctx, val, meridiem, is_am, caption, "", &ICON_ALARM, progress);
    }
}

const ModuleDef mod_system_alarm_def = {
    .label = "NEXT ALARM",
    .sizes = SZ_1x2 | SZ_2x2,
    .features = FEATURE_TIME | FEATURE_SYSTEM,
    .body = system_alarm_body
};
