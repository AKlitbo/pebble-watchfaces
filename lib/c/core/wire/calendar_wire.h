/**
 * @file calendar_wire.h
 * @brief The agenda as the phone packs it, and the reader that unpacks it.
 *
 * The phone sends the agenda as one run of bytes with the events laid end to end, each carrying
 * its own lengths. Nothing about it is fixed width, so the reader walks it and checks every step
 * against the length it was handed. The bytes come off a phone, so none of it is trusted: the
 * lengths are the phone's word for how long things are and the room is ours.
 *
 * The packer on the other side is lib/ts/pkjs/wire.ts, and the two have to agree exactly.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/**
 * @addtogroup lib_core
 * @{
 */

/// How many events the strip can hold. Matches CALENDAR_MAX_SLOTS on the phone
#define CALENDAR_MAX_SLOTS 6

/// Room for a title (up to 24) plus the NUL. Matches CALENDAR_TITLE_MAX on the phone
#define CAL_TITLE_LEN 25

/// Room for a location (up to 16) plus the NUL. Matches CALENDAR_LOC_MAX on the phone
#define CAL_LOC_LEN 17

/**
 * @brief One event. start and end are absolute epochs so the readouts stay fresh against the
 * watch's own clock. The phone fills end with a sensible default when the feed omits DTEND.
 */
typedef struct
{
    time_t start;                 ///< Absolute epoch of the event start
    time_t end;                   ///< Absolute epoch of the event end so we can show how long it runs
    bool   all_day;               ///< True shows a weekday or date instead of a time
    char   title[CAL_TITLE_LEN];  ///< The event summary, or empty when there is none
    char   location[CAL_LOC_LEN]; ///< The place, or empty when there is none
} CalendarEvent;

/** @brief The agenda strip. count is 0 until a reading lands. */
typedef struct
{
    uint8_t       count;                   ///< How many events are filled (0 means none yet)
    CalendarEvent event[CALENDAR_MAX_SLOTS];
} CalendarStrip;

/**
 * @brief Unpacks an agenda off the wire.
 *
 * Layout is [count] then per event [start int32 LE][end int32 LE][flags bit0=allDay][titleLen]
 * [title bytes][locLen][loc bytes]. A count past what the strip holds is pinned to it rather than
 * walking off the end.
 *
 * Nothing is written to @p out until the whole run reads clean, so a message that stops halfway
 * leaves the caller's last good agenda alone rather than half replacing it.
 *
 * @param buf The raw wire bytes.
 * @param len How many bytes there are.
 * @param out Receives the agenda. Untouched unless this returns true.
 * @return Whether the run read clean.
 */
bool calendar_wire_decode(const uint8_t *buf, uint16_t len, CalendarStrip *out);

/** @} */
