/**
 * @file system_quiet.h
 * @brief The quiet time panel: shows whether Quiet Time (do not disturb) is on.
 * @ingroup gridlock_mod_system
 */
#pragma once
#include "mosaic/engine/catalog.h"

/**
 * @brief Draws the quiet time panel. An ON / OFF readout with a muted or full volume
 * icon depending on whether Quiet Time is active.
 */
extern const ModuleDef mod_system_quiet_def;
