/**
 * @file location_store.c
 * @brief The active location store: holds the phone's coordinates and owns the appmessage
 * coords channel.
 */
#include "io/stores/location_store.h"

#include <stdio.h>
#include <string.h>

#include "io/appmessage/appmessage.h"

// persist slot for the last good fix so a relaunch (e.g. after the timeline) shows the coords
// straight away instead of blanking to "--". distinct from the weather store's key (255) and
// clear of the settings keys which sit in a low band (1 / 5)
#define LOCATION_STORE_PERSIST_KEY 254

static struct
{
    char lat[20];
    char lon[20];
} s_state;

static void (*s_cb)(void);
static bool s_live;  // true = a live face, so the cache is worth reading and writing

/**
 * @brief Save the latest coordinates and tell whoever is listening. Doubles as the coords
 * channel handler (its signature matches) and the seed writer.
 *
 * @param lat Latitude string, empty when unavailable.
 * @param lon Longitude string, empty when unavailable.
 */
// stash the coords so a relaunch can restore them. only a live face writes, and only a real
// fix (a coordinate always carries digits, unlike the "NO LOCK" placeholder or an empty no-fix)
// so a dropped fix can't stomp the last good location
static void persist_save(void)
{
    if (s_live && strpbrk(s_state.lat, "0123456789"))
    {
        persist_write_data(LOCATION_STORE_PERSIST_KEY, &s_state, sizeof(s_state));
    }
}

static void set(const char *lat, const char *lon)
{
    snprintf(s_state.lat, sizeof(s_state.lat), "%s", lat ? lat : "");
    snprintf(s_state.lon, sizeof(s_state.lon), "%s", lon ? lon : "");
    persist_save();
    if (s_cb) s_cb();
}

void location_store_subscribe(void (*cb)(void))
{
    s_cb = cb;
}

void location_store_init(LocationConfig cfg, const LocationSeed *seed)
{
    s_live = cfg.live;  // set before any set() so persist_save knows whether to write
    s_state.lat[0] = '\0';
    s_state.lon[0] = '\0';

    if (seed)
    {
        set(seed->lat, seed->lon);  // s_cb is NULL until the face subscribes so no redraw yet
    }
    else if (cfg.live
             && persist_exists(LOCATION_STORE_PERSIST_KEY)
             && persist_get_size(LOCATION_STORE_PERSIST_KEY) == (int)sizeof(s_state))
    {
        // restore the last good fix so a relaunch shows it right away. s_cb is still NULL so no
        // redraw fires here, but the first paint (window push) re-pulls every readout
        persist_read_data(LOCATION_STORE_PERSIST_KEY, &s_state, sizeof(s_state));
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
