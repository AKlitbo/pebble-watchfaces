/**
 * @file health_hrv.h
 * @brief The heart rate variability panels: the reading, its trend, heart rate beside it, last
 * night, the last two hours, still against walking, today by the hour, and the tuning numbers.
 * @ingroup mosaic_mod_health
 */
#pragma once
#include "mosaic/engine/catalog.h"

/** @brief The latest HRV reading. */
extern const ModuleDef mod_health_hrv_def;

/** @brief The latest HRV reading with an arrow against today's average. */
extern const ModuleDef mod_health_hrv_trend_def;

/** @brief Heart rate and HRV together. */
extern const ModuleDef mod_health_heart_def;

/** @brief Last night's average HRV, from the readings taken asleep. */
extern const ModuleDef mod_health_hrv_night_def;

/** @brief The last two hours of HRV readings, resting and walking drawn apart. */
extern const ModuleDef mod_health_hrv_history_def;

/** @brief The latest still, busy, and walking reading, each with its age. */
extern const ModuleDef mod_health_hrv_states_def;

/** @brief Today's HRV by the hour, with the low, average, and high on the wider size. */
extern const ModuleDef mod_health_hrv_day_def;

/** @brief The numbers a wearer tunes the HRV settings from: the burst being measured and the latest ones. */
extern const ModuleDef mod_health_hrv_tuning_def;
