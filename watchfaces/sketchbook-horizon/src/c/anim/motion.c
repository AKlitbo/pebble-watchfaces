/**
 * @file motion.c
 * @brief The shapes of motion a burst is drawn with. Pure maths on the phase, with no SDK state.
 *
 * @ingroup watchface-sketchbook
 */
#include "anim/motion.h"

#include "math/scale.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

/**
 * @brief Whether a phase is partway through a burst, which is the only time a helper moves anything.
 *
 * @param anim The phase.
 * @return False when still, and for a tick that is not under the burst's length.
 */
static bool moving(AnimPhase anim)
{
    return anim.tick != 0 && anim.tick < anim.ticks;
}

int anim_loop(AnimPhase anim, int span, int loops)
{
    if (!moving(anim))
    {
        return 0;
    }

    return (anim.tick * span * loops) / anim.ticks;
}

int anim_slide(AnimPhase anim, int offset, int gap, int way)
{
    int along = (offset + way * anim_loop(anim, gap, 1)) % gap;
    return (along < 0 ? along + gap : along) - gap;
}

int anim_swell(AnimPhase anim, int reach)
{
    if (!moving(anim))
    {
        return 0;
    }

    // half a sine over the burst, so it leaves and comes home with no jolt at either end
    int32_t angle = (TRIG_MAX_ANGLE / 2) * anim.tick / anim.ticks;
    return (reach * sin_lookup(angle)) / TRIG_MAX_RATIO;
}

int anim_settle(AnimPhase anim, int reach, int lead)
{
    if (!moving(anim))
    {
        return 0;
    }

    // the roll home needs at least a frame of its own. a moving phase has a length of 2 or more
    lead = clamp_int(lead, 1, anim.ticks - 1);

    if (anim.tick <= lead)
    {
        int32_t angle = (TRIG_MAX_ANGLE / 4) * anim.tick / lead;
        return (reach * sin_lookup(angle)) / TRIG_MAX_RATIO;
    }

    // the square of the frames left, so each step home is smaller than the one before
    int left = anim.ticks - anim.tick;
    int home = anim.ticks - lead;
    return (reach * left * left) / (home * home);
}

int anim_sway(AnimPhase anim, int reach, int cycles, int32_t start)
{
    if (!moving(anim))
    {
        return 0;
    }

    // the wave less where it started, which is 0 at both ends. a thing that starts near a crest
    // would only ever swing down from it, so where it started is taken off along a slow wave of one
    // whole cycle instead, which still leaves 0 at both ends but averages out to rest over the burst
    // dividing by 1 plus the starting height keeps the swing within reach, and it rounds to the
    // nearest pixel. cutting the fraction off would leave a reach of 1 at 0 nearly all the time
    int32_t wave = sin_lookup(start + (TRIG_MAX_ANGLE * cycles * anim.tick) / anim.ticks);
    int32_t height = sin_lookup(start);
    int32_t slow = cos_lookup((TRIG_MAX_ANGLE * anim.tick) / anim.ticks);
    int32_t lean = (height * (slow / 2)) / (TRIG_MAX_RATIO / 2);
    int32_t span = TRIG_MAX_RATIO + (height < 0 ? -height : height);
    int32_t sway = reach * (wave - lean);
    return (sway + (sway < 0 ? -span / 2 : span / 2)) / span;
}

bool anim_cross(AnimPhase anim, int from, int to, int enter, int leave, int *out)
{
    if (!moving(anim) || anim.tick < enter || anim.tick > leave || leave <= enter)
    {
        return false;
    }

    *out = from + ((to - from) * (anim.tick - enter)) / (leave - enter);
    return true;
}

/** @} */
