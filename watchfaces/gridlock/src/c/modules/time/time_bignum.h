/**
 * @file time_bignum.h
 * @brief Big-digit night panels: the hour on its own, the minutes on their own, and the
 * whole clock as one big number. Oversized numbers for reading the time without glasses.
 * @ingroup gridlock_modules
 */
#pragma once
#include "engine/catalog.h"

/** @addtogroup gridlock_modules @{ */

extern const ModuleDef mod_time_hour_big_def;  ///< 2x2, just the hour, biggest font that fits
extern const ModuleDef mod_time_min_big_def;   ///< 2x2, just the minutes
extern const ModuleDef mod_time_bigclock_def;  ///< 2x4, the whole clock as one big number

/** @} */
