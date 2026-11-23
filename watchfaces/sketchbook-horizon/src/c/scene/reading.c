/**
 * @file reading.c
 * @brief Writes out the temperature a round scene's holder carries, and measures it.
 *
 * @ingroup watchface-sketchbook
 */
#include "scene/reading.h"

#include "io/stores/weather_store.h"
#include "ui/readouts.h"
#include "draw/letter.h"

/**
 * @addtogroup watchface-sketchbook
 * @{
 */

bool reading_temp(char *out, size_t n, int *width)
{
    if (weather_store_temp() == WEATHER_NO_TEMP)
    {
        return false;
    }

    readout_weather_temp(out, n);
    *width = letter_width(out);
    return true;
}

/** @} */
