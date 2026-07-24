/**
 * @file calendar_store.h
 * @brief Active calendar store. It owns the appmessage calendar channel and its own poll timer,
 * so a face just hands it the rules (enabled / live / poll interval), optionally seeds it, then
 * reads the events and subscribes for repaints. The phone data flows straight in and nobody
 * wires it.
 *
 * @ingroup lib_stores
 */
#pragma once
#include <pebble.h>

#include "wire/calendar_wire.h"

/**
 * @addtogroup lib_stores
 * @{
 */

/**
 * @brief The rules a face hands the store, built from its own settings so the store names no
 * SETTING_*.
 */
typedef struct
{
    bool     enabled;     ///< False makes the store do nothing (no channel and no timer)
    bool     live;        ///< True subscribes the channel and polls. False just keeps the fake data for screenshots
    int      poll_min;    ///< Minutes between calendar requests
    uint32_t persist_key; ///< Slot for the last good strip so the face owns the key instead of the store
} CalendarConfig;

/**
 * @brief Optional prefill: shown until the first real reading lands, or pinned when live = false.
 */
typedef struct
{
    const CalendarStrip *strip; ///< The events to show, or NULL for none
} CalendarSeed;

/**
 * @brief Start the store with its rules. Pass seed = NULL for normal use.
 *
 * @param cfg The rules (enabled / live / poll interval).
 * @param seed Optional prefill, or NULL.
 */
void calendar_store_init(CalendarConfig cfg, const CalendarSeed *seed);

/**
 * @brief Re-apply the rules (e.g. the poll interval changed). Re-arms the timer.
 *
 * @param cfg The new rules.
 */
void calendar_store_reconfigure(CalendarConfig cfg);

/** @brief Hand it the function to call when the agenda changes (the screen redraw). */
void calendar_store_subscribe(void (*cb)(void));

/** @brief The whole agenda strip. count is 0 until a reading lands. */
const CalendarStrip *calendar_store_strip(void);

/**
 * @brief One event by index, or NULL when the index is past what is filled.
 *
 * @param index Which event to read (0 based).
 * @return The event, or NULL.
 */
const CalendarEvent *calendar_store_event(uint8_t index);

/** @brief How many seconds since the last reading turned up, or -1 if we have none. */
int calendar_store_age_s(void);

/** @} */
