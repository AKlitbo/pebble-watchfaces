/**
 * @file step_hours.c
 * @brief The hour arithmetic behind the step chart's buckets.
 */
#include "health/step_hours.h"

// spelled out here rather than taken from the SDK, since core builds on the host for the specs too
#define SECONDS_PER_STEP_HOUR 3600

int step_hours_settled(int cur_hour, int cur_min)
{
    int settled = (cur_min > 0) ? cur_hour : cur_hour - 1;

    // midnight's first minute would ask for the hour before the day started
    return (settled < 0) ? 0 : settled;
}

int step_hours_bucket(int seconds_into_day)
{
    if (seconds_into_day < 0)
    {
        return -1;
    }

    int hour = seconds_into_day / SECONDS_PER_STEP_HOUR;
    return (hour < STEP_HOURS_PER_DAY) ? hour : -1;
}
