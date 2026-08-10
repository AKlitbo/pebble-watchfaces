/**
 * @file timeband.h
 * @brief A stretch of the day as an axis, and the sums that put a moment somewhere along it.
 *
 * A face that draws the day as one line has the same problem everywhere it looks: the line may
 * start at any hour and run past midnight, so every reading it wants to place has to be measured
 * against that start rather than against midnight. Doing that at each drawing site is where the
 * wrapping bugs live, so it is done once here and nothing downstream ever sees a wrap.
 *
 * Everything handed back is an offset from the window's start, counting up. That is what lets the
 * same sums serve a ribbon and a ring: the ribbon asks for a position along its pixels and the ring
 * asks for one along a full turn, and neither ever gets a span that runs backwards.
 *
 * The axis unit is the caller's, so nothing here reaches for a GRect or an angle.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdbool.h>
#include <time.h>

/**
 * @addtogroup lib_core
 * @{
 */

/// Minutes in a day, the modulus every wrap here is taken over
#define TIMEBAND_DAY_MINUTES 1440

/** @brief The stretch of day a band shows: where it starts and how long it runs. */
typedef struct
{
    int start_min;  ///< First minute of the window, 0 to 1439
    int span_min;   ///< How many minutes it covers, 1 to 1440
} TimeBand;

/** @brief A window running midnight to midnight. */
TimeBand timeband_full_day(void);

/**
 * @brief A window that puts a given moment a fixed way into it.
 *
 * @param now_min The moment to place, minutes past midnight.
 * @param span_min How long the window runs, pinned to 1..1440.
 * @param lead_min How far into it the moment sits, pinned to 0..span_min.
 * @return The window, its start wrapped into 0..1439.
 */
TimeBand timeband_rolling(int now_min, int span_min, int lead_min);

/**
 * @brief A window starting at the top of a given hour.
 *
 * The hourly forecast carries its own base hour, so feeding it in here lands the strip's columns on
 * even fractions of the axis instead of somewhere arbitrary along it.
 *
 * @param base_hour The hour the window opens on, wrapped into 0..23.
 * @param span_min How long it runs, pinned to 1..1440.
 * @return The window.
 */
TimeBand timeband_from_hour(int base_hour, int span_min);

/**
 * @brief How far into the window a minute of the day falls.
 *
 * @param band The window.
 * @param minute_of_day The moment, minutes past midnight.
 * @return Minutes past the window's start, 0 to span_min - 1, or -1 when it sits outside. The far
 *   end is outside: a window is half open so two of them laid end to end share no minute.
 */
int timeband_offset(TimeBand band, int minute_of_day);

/**
 * @brief Where a minute of the day lands along an axis of a given length.
 *
 * @param band The window.
 * @param length The axis length in whatever the caller measures it in.
 * @param minute_of_day The moment, minutes past midnight.
 * @return 0 to length - 1, or -1 when the moment sits outside the window or the length is not
 *   positive.
 */
int timeband_pos(TimeBand band, int length, int minute_of_day);

/**
 * @brief The same mapping for a value already measured from the window's start.
 *
 * This is the one the drawing uses, since everything clipped comes back as an offset. It takes the
 * far end too, so a span ending exactly at the window's end reaches the end of the axis rather than
 * stopping a unit short.
 *
 * @param band The window.
 * @param length The axis length.
 * @param offset_min Minutes past the window's start, 0 to span_min.
 * @return 0 to length, or -1 when the offset is outside 0..span_min or the length is not positive.
 */
int timeband_pos_offset(TimeBand band, int length, int offset_min);

/** @brief A span clipped to the window, in minutes past the window's start. */
typedef struct
{
    int  from;          ///< First minute past the window's start, 0 to span_min
    int  to;            ///< First minute past the span. Never below from + 1
    bool clipped_start; ///< The span began before the window opened
    bool clipped_end;   ///< And ran past the end of it
} TimeBandSpan;

/**
 * @brief Clip an absolute span onto the window.
 *
 * Both ends are epochs, which is what an agenda carries, and the window's own first minute is given
 * as one too, so an event from yesterday or from next week falls outside rather than being wrapped
 * onto today.
 *
 * A span with no length still comes back a minute wide, so a moment someone pinned to their
 * calendar still draws instead of vanishing.
 *
 * @param band The window.
 * @param window_epoch The epoch of the window's first minute.
 * @param start The span's start.
 * @param end The span's end. An end before the start is read as having no length.
 * @param out Receives the clipped span. Untouched unless this returns true.
 * @return Whether any of the span landed inside the window.
 */
bool timeband_clip(TimeBand band, time_t window_epoch, time_t start, time_t end,
                   TimeBandSpan *out);

/**
 * @brief Clip a daily span, one that repeats every day like the daylight, onto the window.
 *
 * A window running past midnight can see the same daily span at both of its ends, which is why this
 * hands back pieces rather than one span. A span whose end is at or before its start is read as
 * wrapping midnight itself, so a polar winter's short daylight and a midsummer night's short
 * darkness both work without the caller knowing which it has.
 *
 * @param band The window.
 * @param from_min The daily span's start, minutes past midnight, or -1 for no reading.
 * @param to_min Its end, same. A -1 on either side gives no pieces.
 * @param out Receives the pieces, earliest first.
 * @param max_out How many @p out holds. Two is enough for any window of a day or less.
 * @return How many pieces were written, 0 to 2.
 */
int timeband_clip_daily(TimeBand band, int from_min, int to_min,
                        TimeBandSpan *out, int max_out);

/**
 * @brief The epoch the window's first minute sits at.
 *
 * A window whose start is later in the day than it is now began yesterday, which is where a rolling
 * window lands every time it crosses midnight. Midnight is handed over rather than worked out here,
 * so this needs no localtime and tests off the watch.
 *
 * @param band The window.
 * @param midnight The epoch of today's midnight.
 * @param now_min The clock, minutes past midnight.
 * @return The epoch of the window's first minute.
 */
time_t timeband_window_epoch(TimeBand band, time_t midnight, int now_min);

/** @} */
