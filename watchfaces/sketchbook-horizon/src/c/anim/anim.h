/**
 * @file anim.h
 * @brief Short bursts of motion: the frame counter every moving part reads, the timer that steps
 * it, and what starts a burst.
 *
 * A burst runs the counter from 1 up to one less than its length at ANIM_FRAME_MS a frame,
 * repainting the scene each time, then puts it back to 0 and repaints once more. At 0 every draw
 * paints exactly the still frame, so the face between bursts, the dev walks, and the settings-page previews all look
 * the way they do with no burst at all.
 *
 * Once turned on, the face coming into view starts a burst, and so does either a flick of the wrist
 * or raising it to read the watch, whichever the wearer picked. A burst ends on its still frame when
 * a notification covers the face, and a flick or a raise while it is covered starts nothing. Turned
 * off, it watches for nothing, so a face with animation off pays nothing for it.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

#include "anim/motion.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/// How long one frame of a burst stays up, which is 10 frames a second
#define ANIM_FRAME_MS 100

/// How many frames an ordinary burst runs, 2.4 seconds
#define ANIM_TICKS 24

/// How many a storm runs, 3 seconds, so the lightning has room to strike three times
#define ANIM_STORM_TICKS 30

/// At or under this charge, and off the charger, a burst is skipped to save what is left
#define ANIM_LOW_BATTERY 20

/// One burst in this many brings the birds and the shooting star, so they stay a treat rather
/// than turning up every time
#define ANIM_EXTRAS_EVERY 3

// TODO: lift the wrist-raise check into the framework if another face ever wants it. It would then
// have to share the one accelerometer subscription an app gets with the HRV store

/// Samples in each accelerometer batch at 10 a second, so the watch wakes twice a second to check
#define ANIM_LOOK_SAMPLES 5

/// sin² of the flattest tilt that counts as looking, 20°, in thousandths
#define ANIM_LOOK_MIN_SIN2 117

/// sin² of the steepest, 60°, in thousandths
#define ANIM_LOOK_MAX_SIN2 750

/// sin² of the flattest tilt that still counts once looking, 15°, in thousandths. The gap below
/// 20° keeps a watch held near the edge from wobbling in and out
#define ANIM_LOOK_LEAVE_MIN_SIN2 67

/// sin² of the steepest that still counts once looking, 70°, in thousandths
#define ANIM_LOOK_LEAVE_MAX_SIN2 883

/// Readings in a row past the wider window before another burst can start, half a second
#define ANIM_LOOK_LEAVE 5

/// How long the sampling waits before looking again once Quiet Time or a low battery pauses it
#define ANIM_LOOK_PAUSE_MS 60000

/** @brief What starts a burst, besides the face coming into view. The values are what the settings page saves. */
typedef enum
{
    ANIM_OFF = 0,       ///< no bursts at all
    ANIM_ON_FLICK = 1,  ///< a flick of the wrist, from the tap service, which costs almost nothing
    ANIM_ON_RAISE = 2,  ///< raising the wrist to read the watch, which samples twice a second
    ANIM_TRIGGER_COUNT,
} AnimTrigger;

/// How many Raise Delay choices there are, Short, Medium, and Long
#define ANIM_RAISE_DELAY_COUNT 3

/**
 * @brief Remember what repaints the scene and how long a burst runs. Starts nothing, and watches
 * for nothing until anim_enable turns it on.
 *
 * @param repaint Repaints the scene, called once a frame during a burst and once as it ends.
 * @param length How many frames a burst runs for the weather on screen.
 */
void anim_init(void (*repaint)(void), uint8_t (*length)(void));

/**
 * @brief Pick what starts a burst, or turn the bursts off.
 *
 * On, it watches for the face coming into view, which includes launch and a notification closing
 * over it, and for the flick or the raise. Off, it stops watching and ends any burst on its still
 * frame, repainting it. A dev build never watches, since the dev walk steps on taps and pins its
 * frames itself.
 *
 * @param trigger What starts a burst.
 * @param raise_delay How long a raise has to settle, 0 for short, 1 for medium, 2 for long. Only
 * ANIM_ON_RAISE reads it, and anything past 2 takes long.
 */
void anim_enable(AnimTrigger trigger, uint8_t raise_delay);

/** @brief Stop any burst that is running, with no repaint, and stop watching for what starts one. */
void anim_deinit(void);

/**
 * @brief Start a burst of the given length, unless one is already running or the watch should be
 * left still.
 *
 * A burst that is running is left to finish rather than started over. Quiet Time keeps the face
 * still, and so does a battery at ANIM_LOW_BATTERY or under while off the charger.
 *
 * One burst in ANIM_EXTRAS_EVERY brings the birds and the shooting star. The count starts from
 * the clock's second at launch, so a face that is opened often still sees them one burst in
 * ANIM_EXTRAS_EVERY rather than always on opening or never.
 *
 * @param ticks How many frames to run, 2 or more.
 */
void anim_burst(uint8_t ticks);

/**
 * @brief Where the burst has got to right now.
 *
 * @return The phase. Its tick is 0 between bursts.
 */
AnimPhase anim_phase(void);

#if DEV_MODE
/**
 * @brief Pin the phase to one frame, for a dev walk shooting a burst a frame at a time.
 *
 * It ends any burst that is running, and once a frame is pinned no burst starts.
 *
 * @param tick The frame to hold, or 0 for still.
 * @param ticks The length of the burst it is a frame of. Under 2 takes ANIM_TICKS.
 * @param extras True for a burst that brings the birds and the shooting star.
 */
void anim_pin(uint8_t tick, uint8_t ticks, bool extras);
#endif

/** @} */
