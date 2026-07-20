#include "time_weeksleft.h"
#include "draw/stat_panel.h"
#include "io/stores/time_store.h"
#include "clock/date.h"
#include <stdio.h>

static void weeksleft_value(char *out, size_t n, const char **unit_out)
{
    const struct tm *t = time_store_tm();
    // whole weeks left, worked out from the days remaining in the year
    snprintf(out, n, "%d", date_days_left_in_year(t->tm_year + 1900, t->tm_yday) / 7);
    *unit_out = "WKS";
}

static const StatPanel1x2 weeksleft_desc = {
    .value_1x2 = weeksleft_value,
    .icon = &ICON_DATE_TIME,
    .font = FONT_TEKO_26,
};

// borrow the Week colour so the two week tiles read as one family
STAT_PANEL_1X2(time_weeksleft, "WEEKS LEFT", SZ_1x2, FEATURE_TIME, MOD_TIME_WEEKNUM, &weeksleft_desc);
