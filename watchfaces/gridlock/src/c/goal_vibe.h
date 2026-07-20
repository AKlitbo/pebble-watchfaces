/**
 * @file goal_vibe.h
 * @brief Buzzes a little celebration the first time you hit a daily movement goal.
 *
 * Watches steps, calories, distance, and active minutes and fires the buzz the user picked in
 * "Goal Met Vibe" the moment a reading crosses its goal. It only cheers once per goal per day and
 * never on the first look after launch, so reopening the face on a goal you already met stays
 * quiet.
 *
 * The tune itself comes from the phone, not the watch: the pick is a short string set in the
 * config (a plain pulse, or a comma list of milliseconds for a fanfare or your own pattern). So
 * the watch carries no tune tables of its own.
 *
 * @ingroup gridlock_engine
 */
#pragma once

/**
 * @brief Forgets any goals hit today and marks us as not having looked yet. Call once at start
 * up before the first health reading lands.
 */
void goal_vibe_init(void);

/**
 * @brief Checks the movement goals and buzzes once for each one that just got hit. Safe to call
 * as often as the readings change. The first call after launch or a new day only takes stock, it
 * never buzzes.
 */
void goal_vibe_update(void);
