/**
 * @file pct.h
 * @brief Progress towards a goal, as a percent.
 *
 * @ingroup lib_core
 */
#pragma once

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief How far along a goal is, from 0 to 100.
 *
 * Caps at 100 so a beaten goal fills a bar rather than overflowing it, and answers 0 rather than
 * dividing by a goal of zero.
 *
 * @param value How much is done so far.
 * @param goal What is being aimed at.
 * @return The progress from 0 to 100.
 *
 * @note A value below zero is the readouts' no-data sentinel, and this answers 0 for it, same as
 *   it does for a real zero. A caller that needs to tell "nothing yet" apart from "none of it
 *   done" has to check before it asks.
 */
int pct_of(int value, int goal);

/** @} */
