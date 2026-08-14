/**
 * @file dev_wx.h
 * @brief The weather walk: a tap steps the condition through every token the icon table knows,
 * day then night, so each glyph can be checked against the chrome around it.
 *
 * @ingroup watchface-lcars
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup watchface-lcars
 * @{
 */

/**
 * @brief Seed every store, pinning the weather to the first condition in the walk.
 *
 * @param hour The pinned clock hour (0-23).
 * @param min The pinned clock minute.
 */
void dev_wx_seed_stores(int hour, int min);

/**
 * @brief Paint the first condition and subscribe the tap that advances them.
 *
 * @param apply_theme The face's theme-apply hook.
 */
void dev_wx_init(void (*apply_theme)(void));

/** @brief Drop the tap subscription. */
void dev_wx_deinit(void);

/** @} */
