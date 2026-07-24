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
#define FONT_SLOTS_MAX 16

/**
 * @brief Store a loaded font handle under its slot id.
 *
 * You keep the handle. Registering over a slot that already holds one just overwrites it and the
 * old font stays loaded, so a face swapping a font at runtime needs to hang on to the old handle
 * and unload it first. fonts_unload_all is the only place the registry unloads anything, which is
 * what tidies up whatever is left in a slot at the end. So keep it to one handle per slot. Put the
 * same handle under two ids and fonts_unload_all tries to unload it twice.
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
 * @brief Resolve a slot id to its handle. Falls back to the system font on a miss.
 *
 * @param id The slot id.
 * @return The font handle.
 */
GFont fonts_get(FontId id);

/**
 * @brief Unload every registered font and clear the registry.
 */
void fonts_unload_all(void);

/** @} */
