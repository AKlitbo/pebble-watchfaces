/**
 * @file duration.c
 * @brief A minute count turned into a short human duration.
 */
#include "clock/duration.h"

#include <stdio.h>

void duration_hm(char *out, size_t n, int minutes)
{
    if (minutes < 0)
    {
        minutes = 0;
    }
    snprintf(out, n, "%d:%02d", minutes / 60, minutes % 60);
}

void duration_hm_compact(char *out, size_t n, int minutes)
{
    if (minutes < 0)
    {
        minutes = 0;
    }

    int hours = minutes / 60;
    int mins = minutes % 60;
    if (hours > 0)
    {
        snprintf(out, n, "%dh %dm", hours, mins);
    }
    else
    {
        snprintf(out, n, "%dm", mins);
    }
}
