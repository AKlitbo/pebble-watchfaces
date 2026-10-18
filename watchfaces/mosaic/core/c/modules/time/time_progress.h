/**
 * @file time_progress.h
 * @brief The four progress panels: how far through the year, day, hour and week it is. Each
 * reads as a percent on the small tile, and as a percent over a progress bar on the big one.
 * @ingroup mosaic_mod_time
 */
#pragma once
#include "mosaic/engine/catalog.h"

/** @brief The year progress panel that the grid can drop in. */
extern const ModuleDef mod_time_year_progress_def;

/** @brief The day progress panel that the grid can drop in. */
extern const ModuleDef mod_time_day_progress_def;

/** @brief The hour progress panel that the grid can drop in. */
extern const ModuleDef mod_time_hour_progress_def;

/** @brief The week progress panel that the grid can drop in. */
extern const ModuleDef mod_time_week_progress_def;
