/**
 * @file health_hr_graph.h
 * @brief The heart-rate trend panel: draws the last hour of bpm readings as a sparkline.
 * @ingroup mosaic_mod_health
 */
#pragma once
#include "mosaic/engine/catalog.h"

/**
 * @brief Draws the HR graph panel. A sparkline of the 60-minute history with the
 * current bpm called out.
 */
extern const ModuleDef mod_health_hr_graph_def;

/**
 * @brief Draws today's heart rate by the hour as bars. The 2x4 adds the day's low, average, and
 * high and the live reading across the top.
 */
extern const ModuleDef mod_health_hr_day_def;
