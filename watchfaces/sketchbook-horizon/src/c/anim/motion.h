/**
 * @file motion.h
 * @brief Where a burst has got to, and the few shapes of motion every moving part is written with.
 *
 * Each helper returns 0 when the face is still, so a draw that adds one to a position paints
 * exactly the still frame between bursts.
 *
 * A burst draws its ticks from 1 up to one less than its length, and the still frame that follows
 * stands in for the tick equal to the length. Each helper lands on 0, or a whole loop along, at that
 * tick. The last frame drawn is one step short of it, so the end of a burst runs straight into the
 * still frame with no jump.
 *
 * A phase whose tick is 0, or not under its length, reads as still. A helper never divides by a
 * length of 0.
 *
 * @ingroup watchface-sketchbook
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/// How many frames a roll takes to pull out before it heads home. Three reads as a quick slide out,
/// where a single frame reads as a jump
#define ANIM_ROLL_LEAD 3

/** @brief Where a burst has got to. Handed to every draw inside the scene frame. */
typedef struct
{
    uint8_t tick;   ///< The frame, from 1 up to one less than ticks during a burst, and 0 when still
    uint8_t ticks;  ///< How many frames this burst runs. Never 0, so it is safe to divide by
    bool extras;    ///< This burst brings the birds and the shooting star. Never true when still
} AnimPhase;

/**
 * @brief An offset that runs from 0 up through `loops` lengths of `span` over the burst.
 *
 * Used for anything that wraps, such as rain falling through its band. It reaches a whole number of
 * spans along at the still frame after the burst, so the last frame runs straight back into it.
 *
 * @param anim The phase.
 * @param span How far one loop goes.
 * @param loops How many loops the burst makes.
 * @return 0 when still, otherwise how far along it is.
 */
int anim_loop(AnimPhase anim, int span, int loops);

/**
 * @brief Where the first mark of an evenly spaced row starts, as the row slides one gap along over
 * the burst.
 *
 * It starts a gap back, off the left edge, so a mark sliding in across the edge comes in partway
 * rather than popping up whole. Still, the row sits at `offset` with one more mark a gap before it.
 *
 * @param anim The phase.
 * @param offset Where the row's first mark sits when still, from 0 up to one less than `gap`.
 * @param gap How far apart the marks are.
 * @param way 1 to slide right, -1 to slide left.
 * @return Where to start the row, from `-gap` up to just under 0.
 */
int anim_slide(AnimPhase anim, int offset, int gap, int way);

/**
 * @brief How far out an out-and-back motion is, rising smoothly from 0 to `reach` halfway through
 * the burst and settling back to 0 at the still frame after it.
 *
 * @param anim The phase.
 * @param reach How far out it gets at the middle of the burst.
 * @return 0 when still, otherwise how far out it is.
 */
int anim_swell(AnimPhase anim, int reach);

/**
 * @brief How far back from home something that rolls in is. It is pulled out to `reach` over the
 * burst's first `lead` frames, then rolls home for the rest of it, quick at first and slowing as it
 * lands, and is back at 0 at the still frame after the burst.
 *
 * The pull out follows a quarter sine, so it slows into the turn rather than bouncing off it.
 *
 * @param anim The phase.
 * @param reach How far out the roll starts.
 * @param lead How many frames the pull out takes. 1 jumps straight out on the first frame.
 * @return 0 when still, otherwise how far from home it is.
 */
int anim_settle(AnimPhase anim, int reach, int lead);

/**
 * @brief A sway that runs `cycles` whole waves over the burst, starting from where a thing already
 * sits in its wave so neighbours move out of step.
 *
 * It is 0 when still and back to 0 at the still frame after the burst, and it swings both ways
 * around rest. It rounds to the nearest pixel, so a reach of 1 sits a pixel out for most of each
 * wave. A start of 0 is a plain wave. Any other start has its starting height eased off over
 * the burst, so a thing that starts near a crest still swings past rest rather than only one way.
 *
 * @param anim The phase.
 * @param reach How far either way the sway goes.
 * @param cycles How many whole waves the burst makes.
 * @param start Where in the wave this thing starts, as a TRIG angle, so neighbours do not move in step.
 * @return The offset.
 */
int anim_sway(AnimPhase anim, int reach, int cycles, int32_t start);

/**
 * @brief Where something that crosses during the burst is, from just off one side to just off the
 * other.
 *
 * @param anim The phase.
 * @param from Where it is at the start, off the screen.
 * @param to Where it is at the end, off the screen too.
 * @param enter The frame it sets off.
 * @param leave The frame it reaches `to`. One less than the burst's length puts it there on the last
 * frame drawn.
 * @param out Where it is now. Left alone when it is not on its way.
 * @return True while it is on its way, false before and after and when still.
 */
bool anim_cross(AnimPhase anim, int from, int to, int enter, int leave, int *out);

/** @} */
