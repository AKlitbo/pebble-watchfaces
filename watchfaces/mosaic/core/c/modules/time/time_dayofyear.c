#include "time_dayofyear.h"
#include "mosaic/draw/stat_panel.h"
#include "io/stores/time_store.h"
#include "clock/date.h"
#include <stdio.h>

static void dayofyear_value(char *out, size_t n, const char **unit_out)
{
    const struct tm *t = time_store_tm();
    snprintf(out, n, "%d", date_day_of_year(t->tm_yday));

    // days gone plus days left is the year's full length
    int total = date_day_of_year(t->tm_yday) + date_days_left_in_year(t->tm_year + 1900, t->tm_yday);
    *unit_out = total == 366 ? "/366" : "/365";
}

static const StatPanel1x2 dayofyear_desc = {
    .value_1x2 = dayofyear_value,
    .icon = &ICON_DATE_TIME,
    .font = FONT_TEKO_26,
};

STAT_PANEL_1X2(time_dayofyear, "DAY OF YEAR", SZ_1x2, FEATURE_TIME, 0, &dayofyear_desc);
