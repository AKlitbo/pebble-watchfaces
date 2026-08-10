/**
 * @file goal_vibe.c
 * @brief Goal-met celebration buzz. See goal_vibe.h.
 */
#include "mosaic/goal_vibe.h"

#include "io/stores/time_store.h"
#include "io/stores/health_store.h"
#include "settings_schema.h"
#include "system/vibe/vibe.h"

// most segments a pattern can hold. the phone caps its list to match, and the parser drops the rest
#define GOAL_VIBE_MAX_SEGMENTS 40

// the movement goals we watch, in the order the readings and goals are read below
typedef enum
{
    METRIC_STEPS,
    METRIC_CALORIES,
    METRIC_DISTANCE,
    METRIC_ACTIVE,
    METRIC_COUNT
} GoalMetric;

static bool s_met[METRIC_COUNT]; // the goals already cheered today
static bool s_seeded;            // the first look after launch or a day flip takes stock without buzzing
static int  s_yday;              // the day the latches belong to (tm_yday), -1 before the first look

// play a comma list of milliseconds like "100,80,300". reads the numbers into the buffer, clamps
// each to what the SDK allows, and buzzes the rhythm. a list with no numbers stays silent
static void play_pattern_string(const char *s)
{
    // static, not a stack buffer. vibes_enqueue_custom_pattern queues the pattern and keeps
    // reading these durations while it plays, so a stack array would be freed out from under the
    // vibe controller the moment this returns. the SDK's own example passes a static array for
    // the same reason. only one pattern is ever queued at a time (goal_vibe_update buzzes once
    // per pass), so a single shared buffer cannot be overwritten mid-play
    static uint32_t buf[GOAL_VIBE_MAX_SEGMENTS];
    int      count = 0;
    uint32_t value = 0;
    bool     have = false;

    for (const char *p = s;; p++)
    {
        if (*p >= '0' && *p <= '9')
        {
            value = value * 10 + (uint32_t)(*p - '0');
            have = true;
        }
        else
        {
            if (have && count < GOAL_VIBE_MAX_SEGMENTS)
            {
                if (value < 10)
                {
                    value = 10;
                }
                else if (value > 10000) // the SDK's per-segment ceiling
                {
                    value = 10000;
                }
                buf[count++] = value;
            }
            value = 0;
            have = false;
            if (*p == '\0')
            {
                break;
            }
        }
    }

    if (count > 0)
    {
        vibe_custom(buf, (uint32_t)count);
    }
}

// fire the buzz for a Goal Met Vibe pick. a one-letter sentinel is a plain pulse, C means the
// user's own pattern, and anything else is a comma list of milliseconds straight off the dropdown
static void play_goal_vibe(const char *pick)
{
    switch (pick[0])
    {
        case 'S': vibe_pulse(VibePulseShort);  break;
        case 'L': vibe_pulse(VibePulseLong);   break;
        case 'D': vibe_pulse(VibePulseDouble); break;
        case 'C': play_pattern_string(gridlock_goal_vibe_custom()); break;
        default:  play_pattern_string(pick); break;
    }
}

void goal_vibe_init(void)
{
    for (int i = 0; i < METRIC_COUNT; i++)
    {
        s_met[i] = false;
    }
    s_seeded = false;
    s_yday = -1;
}

void goal_vibe_update(void)
{
    const char *pick = gridlock_goal_vibe_pick();
    if (pick[0] == '\0')
    {
        return; // None, the feature is off
    }

    // a new day (or the very first look) forgets yesterday's wins and takes stock afresh below
    int today = time_store_tm()->tm_yday;
    if (today != s_yday)
    {
        for (int i = 0; i < METRIC_COUNT; i++)
        {
            s_met[i] = false;
        }
        s_seeded = false;
        s_yday = today;
    }

    const int reading[METRIC_COUNT] = {
        health_store_steps(),
        health_store_calories(),
        health_store_distance_m(),
        health_store_active_min(),
    };
    const int goal[METRIC_COUNT] = {
        gridlock_goal_steps(),
        gridlock_goal_calories(),
        gridlock_goal_distance_m(),
        gridlock_goal_active_min(),
    };

    bool crossed = false;
    for (int i = 0; i < METRIC_COUNT; i++)
    {
        if (reading[i] < 0)
        {
            continue; // no reading yet, skip this one
        }
        if (reading[i] >= goal[i] && !s_met[i])
        {
            s_met[i] = true;
            crossed = true;
        }
    }

    // the first pass only records where we already stand. it never buzzes, so reopening the face
    // on a goal you already met stays quiet
    if (!s_seeded)
    {
        s_seeded = true;
        return;
    }

    // one buzz even when several goals land at once, the marks above already claimed them all
    if (crossed)
    {
        play_goal_vibe(pick);
    }
}
