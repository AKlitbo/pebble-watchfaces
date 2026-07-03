/**
 * @file health_store.c
 * @brief The active health store: reads the watch's health service and holds the numbers.
 * It owns the service accessors too so it is the single point of truth for health.
 */
#include "io/stores/health_store.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>

static struct
{
    int     hr;
    uint8_t hr_history[60];
    int     steps;
    int     calories;
    int     sleep_min;
    int     active_min;
    int     distance_m;
} s_state;

static void (*s_cb)(void);

// --- health service reads (no-op stubs without PBL_HEALTH) ---

#if defined(PBL_HEALTH)
static bool metric_available(HealthMetric metric, time_t start, time_t end)
{
    HealthServiceAccessibilityMask access = health_service_metric_accessible(metric, start, end);
    return access & HealthServiceAccessibilityMaskAvailable;
}
#endif

static int read_hr(void)
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

static int read_steps(void)
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

static int read_distance(void)
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

// the readouts below return -1 (not 0) when the metric is unavailable so a panel can tell
// "no data yet" apart from a real zero and show a placeholder

static int read_calories(void)
{
#if defined(PBL_HEALTH)
    time_t now = time(NULL);
    if (metric_available(HealthMetricActiveKCalories, time_start_of_today(), now))
    {
        return (int)health_service_sum_today(HealthMetricActiveKCalories);
    }
#endif
    return -1;
}

static int read_sleep_seconds(void)
{
#if defined(PBL_HEALTH)
    time_t now = time(NULL);
    if (metric_available(HealthMetricSleepSeconds, time_start_of_today(), now))
    {
        return (int)health_service_sum_today(HealthMetricSleepSeconds);
    }
#endif
    return -1;
}

static int read_active_minutes(void)
{
#if defined(PBL_HEALTH)
    time_t now = time(NULL);
    if (metric_available(HealthMetricActiveSeconds, time_start_of_today(), now))
    {
        return (int)(health_service_sum_today(HealthMetricActiveSeconds) / 60);
    }
#endif
    return -1;
}

static void read_hr_history(uint8_t *history_out, int max_records)
{
#if defined(PBL_HEALTH)
    if (max_records <= 0 || !history_out) return;

    memset(history_out, 0, max_records);
    HealthMinuteData *minute_data = malloc(max_records * sizeof(HealthMinuteData));
    if (minute_data)
    {
        time_t end = time(NULL);
        time_t start = end - (max_records * SECONDS_PER_MINUTE);

        uint32_t records_read = health_service_get_minute_history(minute_data, max_records, &start, &end);

        int offset = max_records - (int)records_read;
        for (uint32_t i = 0; i < records_read; i++)
        {
            history_out[offset + i] = minute_data[i].is_invalid ? 0 : minute_data[i].heart_rate_bpm;
        }

        free(minute_data);
    }
#endif
}

// --- state + poller ---

/**
 * @brief Read the current health picture off the service into the store, then notify.
 *
 * Reads the hr history too, then the scalars. A heart rate is never really 0, so 0 is kept
 * as "no reading yet".
 */
static void push(void)
{
    read_hr_history(s_state.hr_history, 60);

    // sleep comes in seconds. keep the -1 that means no data rather than dividing it to 0
    int sleep_sec = read_sleep_seconds();
    int sleep_min = sleep_sec < 0 ? -1 : sleep_sec / 60;

    int hr = read_hr();
    s_state.hr = hr > 0 ? hr : -1;
    s_state.steps = read_steps();
    s_state.calories = read_calories();
    s_state.sleep_min = sleep_min;
    s_state.active_min = read_active_minutes();
    s_state.distance_m = read_distance();

    if (s_cb) s_cb();
}

/**
 * @brief Health event handler: refresh on the reading-relevant events.
 *
 * @param event The health event type.
 * @param context Context (unused).
 */
static void on_health_event(HealthEventType event, void *context)
{
    if (event == HealthEventHeartRateUpdate || event == HealthEventMovementUpdate ||
        event == HealthEventSignificantUpdate)
    {
        push();
    }
}

// --- public API ---

void health_store_subscribe(void (*cb)(void))
{
    s_cb = cb;
}

void health_store_init(HealthConfig cfg, const HealthSeed *seed)
{
    // -1 means no reading yet so a panel shows a placeholder until data turns up
    s_state.hr = -1;
    s_state.steps = -1;
    s_state.calories = -1;
    s_state.sleep_min = -1;
    s_state.active_min = -1;
    s_state.distance_m = 0;
    memset(s_state.hr_history, 0, sizeof(s_state.hr_history));

    if (seed)
    {
        s_state.hr = seed->hr > 0 ? seed->hr : -1;
        s_state.steps = seed->steps;
        s_state.calories = seed->calories;
        s_state.sleep_min = seed->sleep_min;
        s_state.active_min = seed->active_min;
        s_state.distance_m = seed->distance_m;
    }

    if (!cfg.enabled)
    {
        return;
    }

    if (cfg.live)
    {
#if defined(PBL_HEALTH)
        health_service_events_subscribe(on_health_event, NULL);
#endif
        push();  // seed the first reading before any event lands
    }
}

uint8_t *health_store_hr_history(void) { return s_state.hr_history; }

int health_store_hr(void)         { return s_state.hr; }
int health_store_steps(void)      { return s_state.steps; }
int health_store_calories(void)   { return s_state.calories; }
int health_store_sleep_min(void)  { return s_state.sleep_min; }
int health_store_active_min(void) { return s_state.active_min; }
int health_store_distance_m(void) { return s_state.distance_m; }
