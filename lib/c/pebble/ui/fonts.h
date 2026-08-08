/**
 * @file fonts.h
 * @brief Font registry: a face loads its custom fonts once, each under a small integer slot id, and
 * the rest of the ui looks them up by id so layout tables can stay static (an id, not a live GFont
 * handle). The registry does not care which fonts they are. The face owns the slots and what they
 * mean (see the face's own draw/fonts.h), this just maps an id to its handle
 *
 * @ingroup lib_ui
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_ui
 * @{
 */

/**
 * @brief A font slot id. The face defines its slots as 0 to FONT_SLOTS_MAX-1 and the registry
 * treats the id as a plain index.
 */
typedef uint8_t FontId;

/// Registry capacity. Sized to the face's slot count so the table wastes no memory. The face's
/// draw/fonts.h checks FONT_COUNT against this at build time, so adding a slot past it fails the
/// build with a clear message. Raise it by the same amount you grow the face
#define FONT_SLOTS_MAX 24

/**
 * @brief Store a loaded font handle under its slot id.
 *
 * This is for a handle the app loaded itself with fonts_load_custom_font, so the registry takes
 * on freeing it in fonts_unload_all. A system font must go in through fonts_register_system
 * instead: the app never owned it, and unloading one is a fault at teardown.
 *
 * You keep the handle. Registering over a slot that already holds one just overwrites it and the
 * old font stays loaded, so a face swapping a font at runtime needs to hang on to the old handle
 * and unload it first. The same handle may safely sit in more than one slot: fonts_unload_all
 * clears every copy when it frees it.
 *
 * A NULL handle goes in like any other and fonts_get then reads that slot as empty and hands back
 * the system font for the rest of the session. fonts_load_custom_font gives you NULL when it cannot
 * load the resource, so passing that straight in quietly parks you on the fallback.
 *
 * @param id The slot id.
 * @param handle The loaded font handle.
 */
void fonts_register(FontId id, GFont handle);

/**
 * @brief Store a system font handle under its slot id, without taking ownership.
 *
 * fonts_get_system_font hands back a handle the firmware owns, so the app must never unload it.
 * Registering one through fonts_register would put it in the queue for fonts_unload_all and fault
 * on the way out. This parks it in the slot and leaves it out of that sweep.
 *
 * @param id The slot id.
 * @param handle The system font handle.
 */
void fonts_register_system(FontId id, GFont handle);

/**
 * @brief Resolve a slot id to its handle. Falls back to the system font on a miss.
 *
 * @param id The slot id.
 * @return The font handle.
 */
GFont fonts_get(FontId id);

/**
 * @brief Unload every font the app owns and clear the registry.
 *
 * Only handles registered through fonts_register are freed. System fonts parked with
 * fonts_register_system are left alone, and a handle sitting in several slots is freed once.
 */
void fonts_unload_all(void);

/** @} */
