/**
 * @file location_store.c
 * @brief The active location store: holds the phone's coordinates and owns the appmessage
 * coords channel.
 */
#include "io/stores/location_store.h"

#include <stdio.h>

#include "io/appmessage/appmessage.h"

static struct
{
    char lat[20];
    char lon[20];
} s_state;

static void (*s_cb)(void);

/**
 * @brief Save the latest coordinates and tell whoever is listening. Doubles as the coords
 * channel handler (its signature matches) and the seed writer.
 *
 * @param lat Latitude string, empty when unavailable.
 * @param lon Longitude string, empty when unavailable.
 */
static void set(const char *lat, const char *lon)
{
    snprintf(s_state.lat, sizeof(s_state.lat), "%s", lat ? lat : "");
    snprintf(s_state.lon, sizeof(s_state.lon), "%s", lon ? lon : "");
    if (s_cb) s_cb();
}

void location_store_subscribe(void (*cb)(void))
{
    s_cb = cb;
}

void location_store_init(LocationConfig cfg, const LocationSeed *seed)
{
    s_state.lat[0] = '\0';
    s_state.lon[0] = '\0';

    if (seed)
    {
        set(seed->lat, seed->lon);  // s_cb is NULL until the face subscribes so no redraw yet
    }

    if (!cfg.enabled)
    {
        return;
    }

    if (cfg.live)
    {
        appmessage_on_coords(set);  // the store owns its channel
    }
}

const char *location_store_lat(void) { return s_state.lat; }
const char *location_store_lon(void) { return s_state.lon; }
