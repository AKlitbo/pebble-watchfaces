/**
 * @file health.c
 * @brief health-service accessors (no-op stubs when PBL_HEALTH is absent)
 */
#include "health.h"

#if defined(PBL_HEALTH)
/**
 * @brief Check if a health metric has data available over a time range.
 *
 * @param metric The health metric to check.
 * @param start The start of the time range.
 * @param end The end of the time range.
 * @return True if data is available, false otherwise.
 */
static bool metric_available(HealthMetric metric, time_t start, time_t end)
{
    HealthServiceAccessibilityMask access = health_service_metric_accessible(metric, start, end);
    return access & HealthServiceAccessibilityMaskAvailable;
}
#endif

void health_init(HealthEventHandler handler)
{
#if defined(PBL_HEALTH)
    health_service_events_subscribe(handler, NULL);
#endif
}

int health_get_current_hr(void)
{
#if defined(PBL_HEALTH)
    time_t now = time(NULL);
    if (metric_available(HealthMetricHeartRateBPM, now, now))
    {
        return (int)health_service_peek_current_value(HealthMetricHeartRateBPM);
    }
#endif
    return 0;
}

int health_get_today_steps(void)
{
#if defined(PBL_HEALTH)
    time_t now = time(NULL);
    if (metric_available(HealthMetricStepCount, time_start_of_today(), now))
    {
        return (int)health_service_sum_today(HealthMetricStepCount);
    }
#endif
    return 0;
}

int health_get_today_distance(void)
{
#if defined(PBL_HEALTH)
    time_t now = time(NULL);
    if (metric_available(HealthMetricWalkedDistanceMeters, time_start_of_today(), now))
    {
        return (int)health_service_sum_today(HealthMetricWalkedDistanceMeters);
    }
#endif
    return 0;
}
