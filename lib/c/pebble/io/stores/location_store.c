/**
 * @file location_store.c
 * @brief The active location store: holds the phone's coordinates and owns the appmessage
 * coords channel.
 */
#include "io/stores/location_store.h"
#include "io/stores/store_persist.h"

#include <stdio.h>
#include <string.h>

#include "io/appmessage/appmessage.h"
#include "wire/coords.h"

// persist slot for the last good fix so a relaunch (e.g. after the timeline) shows the coords
// straight away instead of blanking to "--". own key clear of the weather store (255) the
// stock store (254) and the settings keys which sit in a low band (1 to 8)
#define LOCATION_STORE_PERSIST_KEY 253

static struct
{
    uint8_t tag; // STORE_TAG_LOCATION, so a restore can tell this blob from another shape
    char lat[20];
    char lon[20];
} s_state;
_Static_assert(sizeof(s_state) <= PERSIST_DATA_MAX_LENGTH, "location state must fit one persist key");

static void (*s_cb)(void);
static bool s_live;  // true = a live face, so the cache is worth reading and writing

// stash the coords so a relaunch can restore them. only a live face writes, and only a real fix,
// so a dropped one can't stomp the last good location
static void persist_save(void)
{
    if (s_live && coords_look_real(s_state.lat, s_state.lon))
    {
        store_save(LOCATION_STORE_PERSIST_KEY, &s_state, sizeof(s_state), STORE_TAG_LOCATION);
    }
}

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
    else if (cfg.live)
    {
        // restore the last good fix so a relaunch shows it right away. s_cb is still NULL so no
        // redraw fires here, but the first paint (window push) re-pulls every readout
        store_restore(LOCATION_STORE_PERSIST_KEY, &s_state, sizeof(s_state), STORE_TAG_LOCATION);
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
