/**
 * @file series.h
 * @brief Reducing a run of samples to the few numbers a chart draws from, no SDK.
 *
 * A sparkline or a bar strip keeps its readings in a plain array with a value that stands for "no
 * sample here". Turning that array into a low, a high, and a most-recent, or into the busiest value
 * the bars scale against, is pure work with no watch behind it, so it lives here where it can be
 * tested off the device.
 *
 * @ingroup lib_core
 */
#pragma once
#include <stdint.h>

/**
 * @addtogroup lib_core
 * @{
 */

/**
 * @brief Walk a series of samples for its low, high, and most recent reading.
 *
 * A slot holding the no-sample value is skipped, so a gappy history still reads its real values.
 * The outputs are filled only when there is at least one real sample. With none they are left as
 * the caller set them, so a caller that seeds them and checks the count first cannot read a value
 * that was never there.
 *
 * @param samples The readings, oldest first so the last real one is the most recent.
 * @param count How many slots there are.
 * @param no_sample The value that marks an empty slot.
 * @param lo Receives the lowest real reading.
 * @param hi Receives the highest real reading.
 * @param last Receives the most recent real reading.
 * @return How many real samples there were.
 */
int series_range(const uint8_t *samples, int count, uint8_t no_sample,
                 int *lo, int *hi, int *last);

/**
 * @brief The largest value in a series, for scaling a bar strip against its busiest slot.
 *
 * @param values The readings.
 * @param count How many there are.
 * @return The largest value, or 0 for an empty series.
 */
int series_max_u16(const uint16_t *values, int count);

/** @} */
