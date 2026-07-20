/**
 * @file health_steps_graph.h
 * @brief The steps trend panel: draws today's steps as one filled bar per elapsed hour.
 * @ingroup gridlock_mod_health
 */
#pragma once
#include "engine/catalog.h"

/**
 * @brief Draws the Steps Graph panel. Hourly step bars from midnight to now with today's
 * total called out and the quietest and busiest hours flanking it.
 */
extern const ModuleDef mod_health_steps_graph_def;
