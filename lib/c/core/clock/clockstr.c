/**
 * @file clockstr.c
 * @brief Reads the "HH:MM" clock strings the phone sends.
 */
#include "clock/clockstr.h"

#include <string.h>

// an hour is one or two digits and never more
// the bound is what keeps h from running away: without it a long enough run of digits overflows
// the int before the 0-23 test below ever gets to refuse it, and a signed overflow is undefined
// rather than merely wrong
#define HOUR_DIGITS_MAX 2

bool clockstr_parse(const char *src, int *hour_out, int *minute_out)
{
    if (!src)
    {
        return false;
    }

    const char *colon = strchr(src, ':');
    if (!colon || colon == src || colon - src > HOUR_DIGITS_MAX)
    {
        return false;
    }

    // read the hour digits sitting before the colon
    int h = 0;
    for (const char *p = src; p < colon; p++)
    {
        if (*p < '0' || *p > '9')
        {
            return false;
        }
        h = h * 10 + (*p - '0');
    }

    // the minute needs two real digits right after the colon
    const char *mp = colon + 1;
    if (mp[0] < '0' || mp[0] > '9' || mp[1] < '0' || mp[1] > '9')
    {
        return false;
    }
    int m = (mp[0] - '0') * 10 + (mp[1] - '0');

    if (h > 23 || m > 59)
    {
        return false;
    }

    *hour_out = h;
    *minute_out = m;
    return true;
}

int clockstr_minutes(const char *src)
{
    int hour, minute;
    if (!clockstr_parse(src, &hour, &minute))
    {
        return -1;
    }
    return hour * 60 + minute;
}
