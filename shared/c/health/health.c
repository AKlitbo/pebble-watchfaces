// health-service reads: heart rate, today's steps, today's distance
#include "health.h"

#if defined(PBL_HEALTH)
// true when metric has data available over [start, end]
static bool metric_available(HealthMetric metric, time_t start, time_t end)
{
    HealthServiceAccessibilityMask access = health_service_metric_accessible(metric, start, end);
    return access & HealthServiceAccessibilityMaskAvailable;
}
#endif

// subscribe to health events (no-op without PBL_HEALTH)
void health_init(HealthEventHandler handler)
{
#if defined(PBL_HEALTH)
    health_service_events_subscribe(handler, NULL);
#endif
}

// current heart rate in bpm, or 0 if unavailable
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

// today's step count, or 0 if unavailable
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

// today's walked distance in meters, or 0 if unavailable
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
