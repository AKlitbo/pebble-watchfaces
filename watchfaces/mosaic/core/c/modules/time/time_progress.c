#include "engine/grid_engine.h"
#include "time_progress.h"
#include "time_progress_calc.h"
#include "mosaic/draw/grid_helpers.h"
#include "mosaic/draw/common.h"
#include "mosaic/draw/icons.h"
#include "mosaic/draw/metrics.h"
#include "io/stores/time_store.h"
#include "ui/fonts.h"
#include "clock/date.h"
#include "settings_schema.h"
#include <stdio.h>

// every size shows the percent over a bar. the 2x2 adds two captions saying how far through the
// period that is in its own units, and the 1x4 lays the percent, bar, and icon out in one row
static void progress_draw(GridCtx *gctx, int pct, const char *line1, const char *line2,
                          const IconSpec *icon)
{
    char val[8];

    switch (gctx->size)
    {
    case MSIZE_1x2:
        snprintf(val, sizeof(val), "%d%%", pct);
        gh_bar_1x2(gctx, val, NULL, icon, pct);
        break;
    case MSIZE_2x2:
        snprintf(val, sizeof(val), "%d%%", pct);
        gh_stat_2x2(gctx, val, line1, line2, icon, pct);
        break;
    case MSIZE_1x4:
        snprintf(val, sizeof(val), "%d%%", pct);
        gh_bar_1x4(gctx, val, NULL, icon, pct);
        break;
    default:
        break;
    }
}

static void year_progress_body(GridCtx *gctx)
{
    const struct tm *t = time_store_tm();
    int year = t->tm_year + 1900;
    int pct = progress_year_pct(year, t->tm_yday, t->tm_hour, t->tm_min, t->tm_sec);

    // days gone plus days left is the year's full length
    int day = date_day_of_year(t->tm_yday);
    int total = day + date_days_left_in_year(year, t->tm_yday);

    // the bare fraction with no DAY in front, since the calendar icon is tall and a longer
    // caption runs under it on the 2x2. OF THE YEAR above says what the numbers count
    char line2[24];
    snprintf(line2, sizeof(line2), "%d/%d", day, total);
    progress_draw(gctx, pct, "OF THE YEAR", line2, &ICON_DATE_TIME);
}

static void day_progress_body(GridCtx *gctx)
{
    const struct tm *t = time_store_tm();
    int pct = progress_day_pct(t->tm_hour, t->tm_min, t->tm_sec);

    // the hour we are inside, so 00:30 is hour 1 of 24
    char line2[24];
    snprintf(line2, sizeof(line2), "HOUR %d/24", t->tm_hour + 1);
    progress_draw(gctx, pct, "OF THE DAY", line2, &ICON_CLOCK);
}

static void hour_progress_body(GridCtx *gctx)
{
    const struct tm *t = time_store_tm();
    int pct = progress_hour_pct(t->tm_min, t->tm_sec);

    char line2[24];
    snprintf(line2, sizeof(line2), "%d/60 MIN", t->tm_min);
    progress_draw(gctx, pct, "OF THE HOUR", line2, &ICON_CLOCK);
}

static void week_progress_body(GridCtx *gctx)
{
    const struct tm *t = time_store_tm();
    int first = gridlock_week_start();
    int pct = progress_week_pct(t->tm_wday, first, t->tm_hour, t->tm_min, t->tm_sec);

    char line2[24];
    snprintf(line2, sizeof(line2), "DAY %d/7", progress_week_day(t->tm_wday, first));
    progress_draw(gctx, pct, "OF THE WEEK", line2, &ICON_DATE_TIME);
}

// the year borrows Day of Year's colours and the week borrows Week Number's, so each sits under the
// Year Count or Week Count swatch in the appearance editor. the day owns a colour set of its own and
// the hour borrows it, so the two time of day panels share a Time Progress swatch
const ModuleDef mod_time_year_progress_def = {
    .label = "THIS YEAR",
    .sizes = SZ_1x2 | SZ_2x2 | SZ_1x4,
    .features = FEATURE_TIME,
    .theme_alias = MOD_TIME_DAYOFYEAR,
    .body = year_progress_body,
};

const ModuleDef mod_time_day_progress_def = {
    .label = "TODAY",
    .sizes = SZ_1x2 | SZ_2x2 | SZ_1x4,
    .features = FEATURE_TIME,
    .body = day_progress_body,
};

const ModuleDef mod_time_hour_progress_def = {
    .label = "THIS HOUR",
    .sizes = SZ_1x2 | SZ_2x2 | SZ_1x4,
    .features = FEATURE_TIME,
    .theme_alias = MOD_TIME_DAY_PROGRESS,
    .body = hour_progress_body,
};

const ModuleDef mod_time_week_progress_def = {
    .label = "THIS WEEK",
    .sizes = SZ_1x2 | SZ_2x2 | SZ_1x4,
    .features = FEATURE_TIME,
    .theme_alias = MOD_TIME_WEEKNUM,
    .body = week_progress_body,
};
