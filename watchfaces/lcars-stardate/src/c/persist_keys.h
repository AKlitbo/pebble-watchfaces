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
 * don't know their own key, the face hands it to each one through its config.
 *
 * @ingroup watchface-lcars
 */
#pragma once

// --- settings blobs (low band, owned by settings_schema.c) ---
#define LCARS_SETTINGS_KEY 5

// --- store snapshots (high band, handed to each store's init) ---
// 251 (calendar) and 254 (stock) are left reserved: this face wires neither. 253 belongs to the
// location store, which owns its key itself
#define HEALTH_STORE_KEY  252
#define WEATHER_STORE_KEY 255

// a stray edit that lets the two bands meet breaks the build instead of a watch in the field
_Static_assert(LCARS_SETTINGS_KEY < HEALTH_STORE_KEY, "settings keys must stay below the store keys");
