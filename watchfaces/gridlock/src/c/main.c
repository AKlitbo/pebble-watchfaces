/**
 * @file main.c
 * @brief Gridlock start point. Wires the watch services (through the shared stores) to
 * the grid engine directly.
 * @ingroup gridlock_engine
 */
#include <pebble.h>

#include "io/appmessage/appmessage.h"
#include "io/stores/time_store.h"
#include "io/stores/weather_store.h"
#include "io/stores/stock_store.h"
#include "io/stores/calendar_store.h"
#include "io/stores/health_store.h"
#include "io/stores/system_store.h"
#include "ui/fonts.h"
#include "draw/fonts.h"
#include "ui/engine/engine.h"
#include "engine/grid_engine.h"
#include "engine/catalog.h"
#include "draw/header_fonts.h"
#include "system/settings/settings.h"
#include "system/settings/setting_values.h"
#include "settings_schema.h"
#include "goal_vibe.h"
#include "night.h"
#include "persist_keys.h"
#include "theme/theme.h"
#include "system/vibe/vibe.h"

#include "dev/dev.h"

static Window *s_window;

// the license Required Notice, baked into the binary. kept reachable in init() so --gc-sections
// does not drop it
static const char GRIDLOCK_NOTICE[] =
    "Required Notice: Copyright (c) 2026 Andrew Klitbo. "
    "PolyForm Noncommercial 1.0.0.";

// weather polls the phone this often. the reconnect catch-up reuses it as the staleness gate so
// the two never drift
#define WEATHER_POLL_MIN 30

// the header font is the one font that changes at runtime (the user's Header Font pick). we own
// its handle so we can unload the previous one on a swap, since fonts_register does not free it.
// 0xFF forces the first apply to load rather than short-circuit on an unchanged choice
static GFont   s_header_font;
static uint8_t s_header_choice = 0xFF;

/**
 * @brief Loads the user's chosen header font under FONT_HEADER, swapping out the previous one.
 *
 * Cheap to call on any settings push: it only reloads when the Header Font choice actually
 * changed. fonts_unload_all in deinit frees the final handle, so there is no separate teardown.
 */
static void apply_header_font(void)
{
    uint8_t choice = settings_u8(SETTING_HEADER_FONT);
    if (choice == s_header_choice)
    {
        return;
    }

    if (s_header_font)
    {
        fonts_unload_custom_font(s_header_font);
    }
    s_header_font = fonts_load_custom_font(resource_get_handle(header_font_spec(choice)->resource));
    fonts_register(FONT_HEADER, s_header_font);
    s_header_choice = choice;
}

/**
 * @brief Signs this face's fonts up under their font slots.
 */
static void load_fonts(void)
{
    fonts_register(FONT_TEKO_96, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_96)));
    fonts_register(FONT_TEKO_88, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_88)));
    fonts_register(FONT_TEKO_72, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_72)));
    fonts_register(FONT_TEKO_54, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_54)));
    fonts_register(FONT_TEKO_46, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_46)));
    fonts_register(FONT_TEKO_34, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_34)));
    fonts_register(FONT_TEKO_26, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_26)));
    fonts_register(FONT_TEKO_22, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_22)));
    fonts_register(FONT_TEKO_24, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TEKO_24)));
    fonts_register(FONT_STM_14, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_14)));
    fonts_register(FONT_STM_12, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_STM_12)));
    apply_header_font();
}

/**
 * @brief Redraws after the settings change.
 *
 * @param time_or_date_changed True if a time or date format changed (unused: a rebuild
 * re-pulls every readout anyway).
 */
static void on_settings_changed(bool time_or_date_changed)
{
    // the night schedule may have changed, so settle the flag before the rebuild below reads it
    night_layout_settings_changed();

    // the theme may have changed so re-apply the window background, and the header font may have
    // changed so swap it (a no-op when unchanged) before rebuilding the cells
    window_set_background_color(s_window, theme_background(settings_u8(SETTING_THEME)));
    apply_header_font();
    engine_rebuild();
    engine_mark_dirty();

    // the stock poll interval may have changed so re-arm the timer to the new value. dev/seed
    // paths never reach here since they get no phone settings pushes
    stock_store_reconfigure((StockConfig){.enabled = true, .live = true, .poll_min = gridlock_stock_poll_min()});
    // the calendar poll interval may have changed too, so re-arm it the same way
    calendar_store_reconfigure((CalendarConfig){.enabled = true, .live = true, .poll_min = gridlock_calendar_poll_min()});
}

// buzz once at the top of the hour with the pattern the user picked. the minute tick drives this
// so it lands exactly when tm_min hits 0, and vibe_pulse stays silent during quiet time
static void hourly_vibe(void)
{
    if (time_store_tm()->tm_min == 0)
    {
        vibe_choice(gridlock_hourly_vibe());
    }
}

// buzz for an upcoming calendar event. only runs when a Calendar panel is on the layout so a face
// without one stays quiet. each reminder fires as its lead time crosses. the 60s window means the
// once-a-minute tick lands in it exactly once so a reminder never buzzes twice and a fresh launch
// never buzzes for an event already past the mark. vibe_pulse stays silent during quiet time
static void calendar_vibe(void)
{
    if (!gridlock_has_module_either(MOD_CALENDAR_COUNTDOWN) &&
        !gridlock_has_module_either(MOD_CALENDAR_AGENDA) &&
        !gridlock_has_module_either(MOD_CALENDAR_FREEBUSY))
    {
        return;
    }

    uint8_t mode = gridlock_calendar_vibe_mode();
    if (mode == CAL_VIBE_MODE_NONE)
    {
        return;
    }

    // the mode says which reminders fire. the 15 min one only runs on the full schedule and the
    // 5 min one drops out when it is start-only. on start fires in every non-none mode. each active
    // reminder pairs with its own vibe pick
    const int lead_s[] = {15 * 60, 5 * 60, 0};
    const bool active[] = {mode == CAL_VIBE_MODE_15_5_START,
                           mode == CAL_VIBE_MODE_15_5_START || mode == CAL_VIBE_MODE_5_START,
                           true};
    const uint8_t vibe[] = {gridlock_calendar_vibe_15min(),
                            gridlock_calendar_vibe_5min(),
                            gridlock_calendar_vibe_start()};

    time_t now = time(NULL);
    const CalendarStrip *strip = calendar_store_strip();

    for (int t = 0; t < 3; t++)
    {
        if (!active[t] || vibe[t] == VIBE_NONE)
        {
            continue;
        }
        for (uint8_t i = 0; i < strip->count; i++)
        {
            const CalendarEvent *event = &strip->event[i];
            if (event->all_day)
            {
                continue; // an all-day event has no real start time to count down to
            }
            int remaining = (int)(event->start - now);
            // the minute the event crosses this lead time. only one tick lands in the 60s window
            if (remaining <= lead_s[t] && remaining > lead_s[t] - 60)
            {
                vibe_choice(vibe[t]);
                break; // one buzz per reminder per tick
            }
        }
    }
}

// each store notifies through its own hub tag, so the engine only repaints the cells that read
// from it instead of the whole grid
// the minute tick lives on the time store, so ride it to sample the heart rate once a minute.
// that keeps the graph a continuous line instead of the stray dots the sparse hr events leave
static void on_time_changed(void)    { health_store_poll_hr(); hourly_vibe(); calendar_vibe(); night_layout_tick(); engine_mark_dirty_tags(FEATURE_TIME); }
static void on_weather_changed(void) { night_layout_tick(); engine_mark_dirty_tags(FEATURE_WEATHER); }
static void on_stock_changed(void)   { engine_mark_dirty_tags(FEATURE_STOCK); }
static void on_calendar_changed(void) { engine_mark_dirty_tags(FEATURE_CALENDAR); }
static void on_health_changed(void)  { goal_vibe_update(); engine_mark_dirty_tags(FEATURE_HEALTH); }
static void on_system_changed(void)  { engine_mark_dirty_tags(FEATURE_SYSTEM); }

// a readout is worth re-requesting when it never synced, or is at least its own poll interval
// old. a feature with polling off (poll_min 0) is left alone, same as its store
static bool reconnect_should_refresh(int age_s, int poll_min)
{
    if (poll_min <= 0)
    {
        return false;
    }
    return age_s < 0 || age_s >= poll_min * 60;
}

// the phone app just reconnected. its pkjs may have been closed while away, so catch up any
// readout that went stale during the gap rather than waiting out the poll (weather is 30 min).
// gating on age means bluetooth flapping cannot re-fetch faster than the normal cadence, so a
// metered weather/stock api is never overshot. keys this face lacks are compile-time no-ops and
// the outbox nack/retry wakes an asleep pkjs
static void on_phone_reconnected(void)
{
    if (reconnect_should_refresh(weather_store_age_s(), WEATHER_POLL_MIN))
    {
        appmessage_request_weather();
    }
    if (reconnect_should_refresh(stock_store_age_s(), gridlock_stock_poll_min()))
    {
        appmessage_request_stock();
    }
    if (reconnect_should_refresh(calendar_store_age_s(), gridlock_calendar_poll_min()))
    {
        appmessage_request_calendar();
    }
}

static void init(void)
{
    settings_init(gridlock_settings_schema());

    // start with a clean goal-vibe slate before the first health reading can land
    goal_vibe_init();

    // keep the notice reachable so --gc-sections does not drop it. the empty asm template is a
    // genuine use with no data cost beyond the string itself
    __asm__ volatile("" :: "r"(GRIDLOCK_NOTICE));

    // settings overrides only. must run before the stores and engine come up
    dev_apply_overrides();

    // each store owns its own source. init starts it (live) or seeds it. dev mode seeds every
    // store from a fixed fixture (live=false) for deterministic screenshots
    if (!dev_seed_stores())
    {
        system_store_init((SystemConfig){.enabled = true, .live = true, .vibe = vibe_bt_transition}, NULL);
        health_store_init((HealthConfig){.enabled = true, .live = true, .persist_key = HEALTH_STORE_KEY}, NULL);
        // run the minute tick only. the .beats readout rides that same tick
        time_store_init((TimeConfig){.enabled = true, .live = true, .minute_tick = true, .beats = false}, NULL);
        weather_store_init((WeatherConfig){.enabled = true, .live = true, .poll_min = WEATHER_POLL_MIN, .persist_key = WEATHER_STORE_KEY}, NULL);
        // the poll interval comes from the config (Finnhub only, see the Clay note)
        stock_store_init((StockConfig){.enabled = true, .live = true, .poll_min = gridlock_stock_poll_min(), .persist_key = STOCK_STORE_KEY}, NULL);
        // the calendar polls the phone for a fresh agenda on its own interval, same as stocks
        calendar_store_init((CalendarConfig){.enabled = true, .live = true, .poll_min = gridlock_calendar_poll_min(), .persist_key = CALENDAR_STORE_KEY}, NULL);
        // no location_store: Gridlock shows no coordinates
    }

    s_window = window_create();
    window_set_background_color(s_window, theme_background(settings_u8(SETTING_THEME)));
    window_stack_push(s_window, true);

    load_fonts();

    // settle day or night before the first build, so a watchface launched after dark comes up on
    // the night layout instead of flashing the day one
    night_layout_init();
    engine_init(s_window, gridlock_build);

    // a store change repaints only the cells that read from that hub (see the wrappers above)
    time_store_subscribe(on_time_changed);
    weather_store_subscribe(on_weather_changed);
    stock_store_subscribe(on_stock_changed);
    calendar_store_subscribe(on_calendar_changed);
    health_store_subscribe(on_health_changed);
    system_store_subscribe(on_system_changed);
    system_store_on_reconnect(on_phone_reconnected);

    // the stores own their channels + poll. main only wires settings
    appmessage_on_settings_changed(on_settings_changed);
    // custom colours are stored split across two keys, so they bypass the settings table: split on
    // the way in, rebuild on the way out
    appmessage_on_custom_colors(gridlock_set_custom_colors);
    appmessage_set_custom_colors_provider(gridlock_get_custom_colors);
    appmessage_open();

#ifdef BUILD_WATCHAPP
    // the app opens cold each launch, so ask for weather now instead of waiting for the 30-min poll
    appmessage_request_weather();
#endif

    dev_taps_init(s_window);
}

static void deinit(void)
{
    dev_taps_deinit();
    system_store_deinit();
    engine_deinit();
    gridlock_engine_cleanup();
    modules_cleanup();
    fonts_unload_all();
    window_destroy(s_window);
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
