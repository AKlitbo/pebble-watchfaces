/**
 * @file weather_wire.c
 * @brief The forecast strips as the phone packs them, and the readers that unpack them.
 */
#include "wire/weather_wire.h"

#include "io/bytes_le.h"

// both readers here fill the caller's strip as they go rather than building a copy and handing it
// over at the end, which is what the agenda and the watchlist do. the difference is deliberate and
// comes down to the shape of the message: a column is fixed width, so one check up top settles the
// whole run and nothing after it can fail. the agenda and the watchlist carry a length per field,
// so they only learn a message is short partway through filling one, and a copy is what stops a
// half filled strip reaching the caller. no such moment exists here, and a copy of each strip both
// ways costs more than the two of them are worth

bool weather_hourly_decode(const uint8_t *buf, uint16_t len, WeatherHourly *out)
{
    if (!buf || !out || len < 3)
    {
        return false;
    }

    uint8_t count = buf[0];
    if (count > WEATHER_FORECAST_COLS)
    {
        // the phone's word for how many, pinned to how many there is room for
        count = WEATHER_FORECAST_COLS;
    }

    // three header bytes then three bytes per column. every read below sits inside this
    if (len < (uint16_t)(3 + count * 3))
    {
        return false;
    }

    out->count = count;
    out->base_hour = buf[1];
    out->step_hours = buf[2];
    for (uint8_t i = 0; i < count; i++)
    {
        const uint8_t *col = buf + 3 + i * 3;
        out->col[i].code = col[0];
        out->col[i].temp = read_i16_le(col + 1);
    }

    return true;
}

bool weather_daily_decode(const uint8_t *buf, uint16_t len, WeatherDaily *out)
{
    if (!buf || !out || len < 2)
    {
        return false;
    }

    uint8_t count = buf[0];
    if (count > WEATHER_FORECAST_COLS)
    {
        count = WEATHER_FORECAST_COLS;
    }

    // two header bytes then five bytes per column, fixed width the same way
    if (len < (uint16_t)(2 + count * 5))
    {
        return false;
    }

    out->count = count;
    out->base_weekday = buf[1];
    for (uint8_t i = 0; i < count; i++)
    {
        const uint8_t *col = buf + 2 + i * 5;
        out->col[i].code = col[0];
        out->col[i].temp_max = read_i16_le(col + 1);
        out->col[i].temp_min = read_i16_le(col + 3);
    }

    return true;
}
