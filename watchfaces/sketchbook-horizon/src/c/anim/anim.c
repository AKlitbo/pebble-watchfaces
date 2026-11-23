/**
 * @file anim.c
 * @brief The burst's frame counter, the timer that steps it, and the flick, the wrist raise, and the
 * opening that start one.
 *
 * @ingroup watchface-sketchbook
 */
#include "anim/anim.h"

#include "io/stores/system_store.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

static uint8_t s_tick;
static uint8_t s_ticks = ANIM_TICKS;
static AppTimer *s_timer;
static bool s_extras;  ///< The burst running brings the birds and the shooting star

// counts the bursts round to ANIM_EXTRAS_EVERY. the one that starts on 0 brings the birds and the
// shooting star
static uint8_t s_bursts;
static void (*s_repaint)(void);
static uint8_t (*s_length)(void);

#if !DEV_MODE
static AnimTrigger s_trigger;  ///< What is being watched for, besides the opening
static uint8_t s_hold;         ///< Readings in a row a raise needs, from the Raise Delay setting
static bool s_focused;      ///< The face is the one on screen, rather than covered by a notification
static bool s_listening;    ///< Sampling the accelerometer
static bool s_looking;      ///< Counted as looking, until the watch tilts past the wider window
static uint8_t s_held;      ///< Readings in a row towards going in, or towards coming out once looking
static AppTimer *s_resume;  ///< Looks again after Quiet Time or a low battery paused the sampling
#endif

#if DEV_MODE
static bool s_pinned;  ///< A dev walk holds the frame, so no burst may move it
#endif

/**
 * @brief Move the burst on a frame, or end it, and repaint.
 *
 * The next frame is booked before the repaint, so a slow paint eats into the gap rather than
 * stretching the burst. A paint that runs past the gap is not doubled up, since the SDK drops a
 * second repaint request while one is still waiting.
 *
 * @param data Unused.
 */
static void on_frame(void *data)
{
    s_tick++;

    if (s_tick >= s_ticks)
    {
        s_tick = 0;
        s_timer = NULL;
    }
    else
    {
        s_timer = app_timer_register(ANIM_FRAME_MS, on_frame, NULL);
    }

    if (s_repaint)
    {
        s_repaint();
    }
}

/** @brief End any burst where it stands and put the counter back on the still frame. */
static void stop(void)
{
    if (s_timer)
    {
        app_timer_cancel(s_timer);
        s_timer = NULL;
    }

    s_tick = 0;
}

#if !DEV_MODE
/** @brief Cut a running burst short and repaint, so the still frame is what stays on screen. */
static void end_burst(void)
{
    bool running = s_timer || s_tick;

    stop();

    if (running && s_repaint)
    {
        s_repaint();
    }
}
#endif

/**
 * @brief Whether the watch wants the face left still right now.
 *
 * @return True during Quiet Time, and on a low battery that is not charging.
 */
static bool keep_still(void)
{
    // the SDK asks faces to hold back from anything disturbing while Quiet Time is on
    if (quiet_time_is_active())
    {
        return true;
    }

    return system_store_battery() <= ANIM_LOW_BATTERY && !system_store_charging();
}

#if !DEV_MODE
/** @brief Start a burst as long as the weather on screen asks for. */
static void start(void)
{
    anim_burst(s_length ? s_length() : ANIM_TICKS);
}

static void listen(bool on);

/**
 * @brief Whether one averaged reading has the watch tilted between two angles.
 *
 * The screen has to face up, and the tilt along the 12 to 6 line has to sit between the two
 * angles, given as their sines squared in thousandths, such as ANIM_LOOK_MIN_SIN2 and
 * ANIM_LOOK_MAX_SIN2. The tilt is measured against the whole pull of gravity rather than a fixed
 * 1000, so a reading taken mid-swing reads about the same as one held still. Squaring the sines saves a
 * square root, which the watch has no cheap way to do.
 *
 * @param x The reading across the screen, in thousandths of a g.
 * @param y The reading along the 12 to 6 line. It goes negative as 12 rises.
 * @param z The reading out of the screen. It goes negative as the screen faces up.
 * @param min_sin2 The flattest tilt, as its sine squared in thousandths.
 * @param max_sin2 The steepest tilt, the same way.
 * @return True inside the window.
 */
static bool tilted_between(int32_t x, int32_t y, int32_t z, int32_t min_sin2, int32_t max_sin2)
{
    if (z >= 0 || y >= 0)
    {
        return false;
    }

    int64_t all = (int64_t)x * x + (int64_t)y * y + (int64_t)z * z;
    int64_t along = (int64_t)y * y * 1000;

    return along >= all * min_sin2 && along <= all * max_sin2;
}

/**
 * @brief Quiet Time or a low battery paused the sampling. Look again a minute later.
 *
 * @param data Unused.
 */
static void on_resume(void *data)
{
    s_resume = NULL;

    if (s_trigger != ANIM_ON_RAISE)
    {
        return;
    }

    if (keep_still())
    {
        s_resume = app_timer_register(ANIM_LOOK_PAUSE_MS, on_resume, NULL);
        return;
    }

    listen(true);
}

/**
 * @brief Count one reading towards starting a burst or towards leaving the window.
 *
 * Each step needs a few readings in a row rather than one, so a single jolt counts for nothing.
 * Going in takes as many readings inside the window as the Raise Delay asks for. Coming out takes
 * ANIM_LOOK_LEAVE readings past the wider one.
 *
 * @param x The reading across the screen.
 * @param y The reading along the 12 to 6 line.
 * @param z The reading out of the screen.
 */
static void take(int32_t x, int32_t y, int32_t z)
{
    if (s_looking)
    {
        bool still_in = tilted_between(x, y, z, ANIM_LOOK_LEAVE_MIN_SIN2, ANIM_LOOK_LEAVE_MAX_SIN2);

        s_held = still_in ? 0 : s_held + 1;

        if (s_held >= ANIM_LOOK_LEAVE)
        {
            s_looking = false;
            s_held = 0;
        }

        return;
    }

    if (!tilted_between(x, y, z, ANIM_LOOK_MIN_SIN2, ANIM_LOOK_MAX_SIN2))
    {
        s_held = 0;
        return;
    }

    s_held++;

    if (s_held >= s_hold)
    {
        s_looking = true;
        s_held = 0;

        if (s_focused)
        {
            start();
        }
    }
}

/**
 * @brief Accelerometer batch: a raise of the wrist into the viewing window starts a burst.
 *
 * Each reading counts on its own rather than as a batch average, so a raise that settles partway
 * through a batch counts from where it settled rather than needing a whole batch inside. A reading
 * taken while the watch was buzzing breaks the run, so a notification's buzz cannot shake the watch
 * into the window or join two swings into one. The watch has to stay in the window for the Raise Delay's readings before a burst
 * starts, so an arm swinging through it while walking starts nothing. Once it has, the watch has to
 * tilt past a wider window before another can start, so holding it up near either edge plays one
 * burst rather than one per wobble. A raise while a notification covers the face starts nothing.
 *
 * Quiet Time and a low battery keep the face still anyway, so they also stop the sampling and its
 * two wakes a second. on_resume looks again once a minute, which costs far less.
 *
 * @param data The batch.
 * @param count How many samples are in it.
 */
static void on_accel(AccelData *data, uint32_t count)
{
    // a batch already on its way when the sampling stopped is dropped
    if (!s_listening)
    {
        return;
    }

    if (keep_still())
    {
        listen(false);

        if (!s_resume)
        {
            s_resume = app_timer_register(ANIM_LOOK_PAUSE_MS, on_resume, NULL);
        }

        return;
    }

    for (uint32_t i = 0; i < count; i++)
    {
        if (data[i].did_vibrate)
        {
            s_held = 0;
        }
        else
        {
            take(data[i].x, data[i].y, data[i].z);
        }
    }
}

/**
 * @brief Start or stop sampling the accelerometer for the wrist raise.
 *
 * The first batch after it starts never plays a burst, since it counts as already looking. The
 * opening burst covers the moment the face comes into view, so a watch already held up does not
 * get a second one straight after.
 *
 * @param on True to sample.
 */
static void listen(bool on)
{
    if (on == s_listening)
    {
        return;
    }

    s_listening = on;

    if (on)
    {
        s_looking = true;
        s_held = 0;
        accel_data_service_subscribe(ANIM_LOOK_SAMPLES, on_accel);
        accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
    }
    else
    {
        accel_data_service_unsubscribe();
    }
}

/**
 * @brief Accelerometer tap: a flick of the wrist starts a burst while the face is on screen.
 *
 * A flick that lights up a notification covering the face starts nothing, so no burst runs out of
 * sight and then holds off the one the face opening asks for.
 *
 * @param axis Unused.
 * @param direction Unused.
 */
static void on_flick(AccelAxisType axis, int32_t direction)
{
    if (s_focused)
    {
        start();
    }
}

/**
 * @brief Something is about to cover the face, such as a notification. A running burst ends on its
 * still frame, so the face is still when it comes back into view.
 *
 * @param in_focus False when the face is about to be covered.
 */
static void on_will_focus(bool in_focus)
{
    if (!in_focus)
    {
        s_focused = false;
        end_burst();
    }
}

/**
 * @brief The face came into view, at launch or with a notification closing over it.
 *
 * @param in_focus True once the face is the one on screen.
 */
static void on_did_focus(bool in_focus)
{
    if (in_focus)
    {
        s_focused = true;
        start();
    }
}

/**
 * @brief Start or stop watching for s_trigger and for the face coming into view.
 *
 * @param on True to watch.
 */
static void watch(bool on)
{
    if (on)
    {
        // the face is in view when the bursts are turned on, at launch or from the settings page
        s_focused = true;

        if (s_trigger == ANIM_ON_FLICK)
        {
            accel_tap_service_subscribe(on_flick);
        }
        else
        {
            listen(!keep_still());

            if (!s_listening)
            {
                s_resume = app_timer_register(ANIM_LOOK_PAUSE_MS, on_resume, NULL);
            }
        }

        app_focus_service_subscribe_handlers((AppFocusHandlers){.will_focus = on_will_focus,
                                                                .did_focus = on_did_focus});
    }
    else
    {
        if (s_trigger == ANIM_ON_FLICK)
        {
            accel_tap_service_unsubscribe();
        }

        listen(false);

        if (s_resume)
        {
            app_timer_cancel(s_resume);
            s_resume = NULL;
        }

        app_focus_service_unsubscribe();
    }
}
#endif

void anim_init(void (*repaint)(void), uint8_t (*length)(void))
{
    s_repaint = repaint;
    s_length = length;
    // starting the count from the clock means a face reopened often still lands on the birds now
    // and then, rather than always or never on its opening burst
    s_bursts = (uint8_t)(time(NULL) % ANIM_EXTRAS_EVERY);
}

void anim_enable(AnimTrigger trigger, uint8_t raise_delay)
{
#if DEV_MODE
    // the dev walk steps on the same tap and pins its frames itself, so it keeps the face still
    (void)trigger;
    (void)raise_delay;
#else
    // Short, Medium, and Long as readings in a row at 10 a second, so 0.2, 0.3, and 0.5 seconds of
    // holding still. a batch only arrives every half second, so the burst can land up to that much later
    static const uint8_t holds[ANIM_RAISE_DELAY_COUNT] = {2, 3, 5};

    s_hold = holds[raise_delay < ARRAY_LENGTH(holds) ? raise_delay : ARRAY_LENGTH(holds) - 1];

    if (trigger == s_trigger)
    {
        return;
    }

    if (s_trigger != ANIM_OFF)
    {
        watch(false);
    }

    s_trigger = trigger;

    if (trigger != ANIM_OFF)
    {
        watch(true);
    }
    else
    {
        end_burst();
    }
#endif
}

void anim_deinit(void)
{
#if !DEV_MODE
    if (s_trigger != ANIM_OFF)
    {
        watch(false);
        s_trigger = ANIM_OFF;
    }
#endif

    // the window is on its way down, so the burst just stops with no repaint
    stop();
}

void anim_burst(uint8_t ticks)
{
    if (s_timer || ticks < 2 || keep_still())
    {
        return;
    }

#if DEV_MODE
    if (s_pinned)
    {
        return;
    }
#endif

    s_ticks = ticks;
    s_tick = 0;
    s_extras = s_bursts == 0;
    s_bursts = (uint8_t)((s_bursts + 1) % ANIM_EXTRAS_EVERY);
    s_timer = app_timer_register(ANIM_FRAME_MS, on_frame, NULL);
}

AnimPhase anim_phase(void)
{
    return (AnimPhase){.tick = s_tick, .ticks = s_ticks, .extras = s_tick != 0 && s_extras};
}

#if DEV_MODE
void anim_pin(uint8_t tick, uint8_t ticks, bool extras)
{
    if (s_timer)
    {
        app_timer_cancel(s_timer);
        s_timer = NULL;
    }

    s_pinned = true;
    s_ticks = ticks < 2 ? ANIM_TICKS : ticks;
    s_tick = tick < s_ticks ? tick : 0;
    s_extras = extras;
}
#endif

/** @} */
