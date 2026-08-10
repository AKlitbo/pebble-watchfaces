/**
 * @file persist_keys.h
 * @brief Every persist key this face uses, in one place.
 *
 * Each key is pinned to an explicit number and must never change or be reused once shipped: a
 * watch in the field holds its data under that exact number. Keeping them all here (rather than
 * scattered as literals) is what stops a new one from silently colliding with an existing one.
 *
 * The range is split by convention: settings blobs sit in a low band, store snapshots in a high
 * one, so the two groups grow toward each other with plenty of room between. The lib stores
 * don't know their own key anymore, the face hands it to each one through its config.
 * @ingroup gridlock_settings
 */
#pragma once

// --- settings blobs (low band, owned by settings_schema.c) ---
#define GRIDLOCK_CORE_KEY         1
#define GRIDLOCK_CUSTOM_THEME_KEY 2
#define GRIDLOCK_WEATHER_KEY      3
#define GRIDLOCK_HEALTH_KEY       4
#define GRIDLOCK_CLOCK_KEY        5
#define GRIDLOCK_BLUETOOTH_KEY    6
#define GRIDLOCK_STOCKS_KEY       7
#define GRIDLOCK_CUSTOM_FLAGS_KEY 8
#define GRIDLOCK_ANALOG_KEY       9
#define GRIDLOCK_CALENDAR_KEY     10
#define GRIDLOCK_GOAL_VIBE_KEY    11
// the night layout keeps its own key rather than riding the core blob: core is already 131 bytes
// and a second 128-byte layout would push it past the 256 a persist key holds, which
// persist_write_data reports by writing nothing at all
#define GRIDLOCK_NIGHT_KEY        12

// --- store snapshots (high band, handed to each store's init) ---
// 253 is the location store's slot, left reserved: this face doesn't wire location
#define CALENDAR_STORE_KEY 251
#define HEALTH_STORE_KEY   252
#define STOCK_STORE_KEY    254
#define WEATHER_STORE_KEY  255

// a stray edit that lets the two bands meet breaks the build instead of a watch in the field
_Static_assert(GRIDLOCK_NIGHT_KEY < CALENDAR_STORE_KEY, "settings keys must stay below the store keys");
