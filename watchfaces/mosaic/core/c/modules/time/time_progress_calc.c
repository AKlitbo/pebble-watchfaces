#include "time_progress_calc.h"
#include <stdbool.h>
#include <stdint.h>

#define SECS_PER_HOUR 3600
#define SECS_PER_DAY 86400

// floor of elapsed over total as a percent. int64 because a year of seconds times 100 overflows
// a 32 bit int, and the watch is a 32 bit part
static int pct_of_seconds(int64_t elapsed, int64_t total)
{
    if (total <= 0 || elapsed <= 0)
    {
        return 0;
    }
    int64_t pct = elapsed * 100 / total;
    return pct > 99 ? 99 : (int)pct;
}

static int seconds_into_day(int hour, int min, int sec)
{
    return hour * SECS_PER_HOUR + min * 60 + sec;
}

static int days_in_year(int year)
{
    bool leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    return leap ? 366 : 365;
}

int progress_week_day(int wday, int week_start)
{
    return (wday - week_start + 7) % 7 + 1;
}

int progress_hour_pct(int min, int sec)
{
    return pct_of_seconds(min * 60 + sec, SECS_PER_HOUR);
}

int progress_day_pct(int hour, int min, int sec)
{
    return pct_of_seconds(seconds_into_day(hour, min, sec), SECS_PER_DAY);
}

int progress_week_pct(int wday, int week_start, int hour, int min, int sec)
{
    int64_t elapsed = (int64_t)(progress_week_day(wday, week_start) - 1) * SECS_PER_DAY
                      + seconds_into_day(hour, min, sec);
    return pct_of_seconds(elapsed, (int64_t)7 * SECS_PER_DAY);
}

int progress_year_pct(int year, int yday, int hour, int min, int sec)
{
    int64_t elapsed = (int64_t)yday * SECS_PER_DAY + seconds_into_day(hour, min, sec);
    return pct_of_seconds(elapsed, (int64_t)days_in_year(year) * SECS_PER_DAY);
}
