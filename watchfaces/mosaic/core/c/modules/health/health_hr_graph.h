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
