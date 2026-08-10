#include "time_yearend.h"
#include "mosaic/draw/stat_panel.h"
#include "io/stores/time_store.h"
#include "clock/date.h"
#include <stdio.h>

static void yearend_value(char *out, size_t n, const char **unit_out)
{
    const struct tm *t = time_store_tm();
    snprintf(out, n, "%d", date_days_left_in_year(t->tm_year + 1900, t->tm_yday));
    *unit_out = "DAYS";
}

static const StatPanel1x2 yearend_desc = {
    .value_1x2 = yearend_value,
    .icon = &ICON_DATE_TIME,
    .font = FONT_TEKO_26,
};

// borrow the Day of Year colour so the two year-count tiles read as one family
STAT_PANEL_1X2(time_yearend, "DAYS LEFT", SZ_1x2, FEATURE_TIME, MOD_TIME_DAYOFYEAR, &yearend_desc);
