/**
 * @file weekday.c
 * @brief Weekday names, host-testable off-device
 */
#include "clock/weekday.h"

const char *weekday_short(int wday)
{
    // indexed like tm_wday and the phone's base_weekday (0 = Sunday)
    static const char *const names[7] = { "SU", "MO", "TU", "WE", "TH", "FR", "SA" };
    if (wday < 0 || wday > 6)
    {
        return "";
    }
    return names[wday];
}
