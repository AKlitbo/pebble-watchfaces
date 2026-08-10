#include "time_weeknum.h"
#include "mosaic/draw/stat_panel.h"
#include "io/stores/time_store.h"
#include "clock/date.h"
#include <stdio.h>

static void weeknum_value(char *out, size_t n, const char **unit_out)
{
    const struct tm *t = time_store_tm();
    int year = t->tm_year + 1900;
    snprintf(out, n, "%d", date_iso_week(year, t->tm_yday, t->tm_wday));

    // trailing total weeks, same "value / total" shape as the Day of Year tile. the total counts
    // the ISO year the week belongs to, which around New Year is the neighbouring one, so week 53
    // of a 53 week year never reads against a 52 week total
    int iso_year = date_iso_week_year(year, t->tm_yday, t->tm_wday);
    *unit_out = date_iso_weeks_in_year(iso_year) == 53 ? "/53" : "/52";
}

static const StatPanel1x2 weeknum_desc = {
    .value_1x2 = weeknum_value,
    .icon = &ICON_DATE_TIME,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(time_weeknum, "WEEK", SZ_1x2, FEATURE_TIME, 0, &weeknum_desc);
