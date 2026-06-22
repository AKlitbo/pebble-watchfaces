/**
 * @file shell.c
 * @brief ui shell implementation
 */
#include "shell.h"

#include "fonts.h"
#include "zone.h"
#include "connection/connection.h"
#include "settings/settings.h"
#include "units/units.h"
#include "vibe/vibe.h"
#include "text_case.h"

static const WatchfaceDescriptor *s_face; // Active face
static const Zone *s_zones;               // Face's zone table

static Window *s_main_window;           // Main shell window
static TextLayer *s_layers[ZONE_COUNT]; // Text layers for each zone

static char s_hr_buffer[12];    // Buffer for heart rate
static char s_steps_buffer[12]; // Buffer for steps

static int s_last_steps;        // Cached steps reading (for steps-mode changes)
static int s_last_distance_m;   // Cached distance reading
static char s_weather_temp[12]; // Cached weather temp string
static char s_weather_cond[12]; // Cached weather condition string
static char s_lat_buffer[18];   // Cached latitude string ("34.05N 118.24W")
static char s_lon_buffer[18];   // Cached longitude string

static bool s_bt_connected = true; // Assume connected until the first peek/event

/**
 * @brief Re-render the steps slot from cached readings.
 */
static void render_steps_readout(void);

/**
 * @brief Fire the chosen vibe pattern (0=None, 1=Short, 2=Long, 3=Double).
 *
 * Stays silent during quiet time, so the link dropping at night won't buzz.
 *
 * @param pattern The vibe pattern to fire.
 */
static void play_vibe(uint8_t pattern)
{
    switch (pattern)
    {
        case 1: vibe_pulse(VibePulseShort); break;
        case 2: vibe_pulse(VibePulseLong); break;
        case 3: vibe_pulse(VibePulseDouble); break;
        default: break;  // None
    }
}

/**
 * @brief Push the current bluetooth state to the face.
 *
 * Honours the show/hide setting.
 */
static void apply_bluetooth(void)
{
    if (!s_face->set_bluetooth)
    {
        return;
    }

    BluetoothStatus status = !settings_u8(SETTING_BLUETOOTH_ICON)
        ? BT_HIDDEN
        : (s_bt_connected ? BT_CONNECTED : BT_DISCONNECTED);

    s_face->set_bluetooth(status);
}

/**
 * @brief Connection event handler.
 *
 * Buzzes the configured pattern on a real transition, then repaints.
 *
 * @param connected True if connected, false otherwise.
 */
static void connection_handler(bool connected)
{
    if (connected != s_bt_connected)
    {
        play_vibe(connected ? settings_u8(SETTING_BLUETOOTH_VIBE_CONNECT)
                            : settings_u8(SETTING_BLUETOOTH_VIBE_DISCONNECT));
    }

    s_bt_connected = connected;
    apply_bluetooth();
}

/**
 * @brief Set a zone's text.
 *
 * Tolerates a slot the active face chose to omit (NULL layer).
 * A face omits a slot by leaving its zone zeroed in the table.
 *
 * @param id The zone id.
 * @param text The text to set.
 */
static void set_zone_text(ZoneId id, const char *text)
{
    if (s_layers[id])
    {
        text_layer_set_text(s_layers[id], text);
    }
}

void shell_update_display(void)
{
    if (!s_main_window)
    {
        return;
    }

    s_face->update_theme(settings_u8(SETTING_THEME));

    // a theme change may have recoloured the zone table; re-apply to the live layers
    for (int i = 0; i < ZONE_COUNT; i++)
    {
        if (s_layers[i])
        {
            text_layer_set_text_color(s_layers[i], s_zones[i].color);
        }
    }

    render_steps_readout();  // reflect a changed steps mode immediately
    apply_bluetooth();   // reflect a toggled bluetooth indicator immediately
    s_face->refresh_overlays();
}

void shell_update_time(struct tm *tick_time)
{
    static char s_time_buffer[8];

    uint8_t time_format = settings_u8(SETTING_TIME_FORMAT);

    // Swatch .beats - replaces the clock
    if (time_format == 3)
    {
        snprintf(s_time_buffer, sizeof(s_time_buffer), "@%03d", units_swatch_beats());
    }
    else
    {
        // 24 -> 12 -> System
        const char *fmt = (time_format == 2) ? "%H:%M"
            : (time_format == 1)
                ? "%I:%M"
                : (clock_is_24h_style() ? "%H:%M" : "%I:%M");

        strftime(s_time_buffer, sizeof(s_time_buffer), fmt, tick_time);
    }

    set_zone_text(ZONE_TIME, s_time_buffer);

    // AM/PM only makes sense on a 12-hour clock (explicit 12-hour, or System when
    // the watch isn't in 24h mode). Cleared otherwise so it never shows on 24h/.beats
    bool h12 = (time_format == 1) || (time_format == 0 && !clock_is_24h_style());
    set_zone_text(ZONE_MERIDIEM, h12 ? (tick_time->tm_hour < 12 ? "AM" : "PM") : "");

    static char s_date_buffer[24];
    char date_now[24];
    strftime(date_now, sizeof(date_now), settings_str(SETTING_DATE_FORMAT), tick_time);
    text_to_upper(date_now);

    // the date only changes at midnight (or when the format changes), but this runs
    // every tick/beat - skip the pixel-width fit + relayout unless the text differs
    if (s_layers[ZONE_DATE] && strcmp(date_now, s_date_buffer) != 0)
    {
        strncpy(s_date_buffer, date_now, sizeof(s_date_buffer) - 1);
        s_date_buffer[sizeof(s_date_buffer) - 1] = '\0';
        zone_set_text_fit(s_layers[ZONE_DATE], &s_zones[ZONE_DATE], s_date_buffer);
    }
}

static void render_steps_readout(void)
{
    if (!s_layers[ZONE_STEPS])
    {
        return;
    }

    uint8_t mode = settings_u8(SETTING_STEPS_MODE);
    if (mode == 1 || mode == 2)
    {
        units_format_distance(s_steps_buffer, sizeof(s_steps_buffer), s_last_distance_m,
                              mode == 1);
    }
    else
    {
        snprintf(s_steps_buffer, sizeof(s_steps_buffer), "%d", s_last_steps);
    }

    text_layer_set_text(s_layers[ZONE_STEPS], s_steps_buffer);
}

void shell_update_info(int hr, int steps, int distance_m)
{
    if (hr > 0)
    {
        snprintf(s_hr_buffer, sizeof(s_hr_buffer), "%d", hr);
    }
    else
    {
        snprintf(s_hr_buffer, sizeof(s_hr_buffer), "--");
    }

    set_zone_text(ZONE_HR, s_hr_buffer);

    s_last_steps = steps;
    s_last_distance_m = distance_m;
    render_steps_readout();
}

void shell_set_battery(int level)
{
    s_face->set_battery(level);
}

void shell_set_weather(const char *temp, const char *condition)
{
    strncpy(s_weather_temp, temp ? temp : "--", sizeof(s_weather_temp) - 1);
    s_weather_temp[sizeof(s_weather_temp) - 1] = '\0';
    set_zone_text(ZONE_WEATHER, s_weather_temp);

    strncpy(s_weather_cond, condition ? condition : "--", sizeof(s_weather_cond) - 1);
    s_weather_cond[sizeof(s_weather_cond) - 1] = '\0';

    // drop the night marker so "CLEAR_NIGHT" fits as "CLEAR" (the icon below uses the full string)
    char *night = strchr(s_weather_cond, '_');
    if (night)
    {
        *night = '\0';
    }

    set_zone_text(ZONE_COND, s_weather_cond);

    s_face->set_weather_icon(condition);  // full string (incl. _NIGHT) drives the glyph
}

void shell_set_coords(const char *lat, const char *lon)
{
    // JS pre-formats these in LCARS dash style ("33-44", "-112-07") - empty when
    // GPS is off or unavailable, in which case we show a placeholder
    strncpy(s_lat_buffer, (lat && lat[0]) ? lat : "--", sizeof(s_lat_buffer) - 1);
    s_lat_buffer[sizeof(s_lat_buffer) - 1] = '\0';
    set_zone_text(ZONE_LAT, s_lat_buffer);

    strncpy(s_lon_buffer, (lon && lon[0]) ? lon : "--", sizeof(s_lon_buffer) - 1);
    s_lon_buffer[sizeof(s_lon_buffer) - 1] = '\0';
    set_zone_text(ZONE_LON, s_lon_buffer);
}

/**
 * @brief Window load handler: creates the scene and zones.
 *
 * @param window The window being loaded.
 */
static void window_load(Window *window)
{
    Layer *root = window_get_root_layer(window);

    // fixed field colour; readout colours come from each zone
    window_set_background_color(window, GColorBlack);

    s_face->load_fonts();

    // frame bitmap + painted overlays sit at the bottom; zone text layers go on top
    s_face->create_scene(root, settings_u8(SETTING_THEME));

    s_zones = s_face->zones();
    for (int i = 0; i < ZONE_COUNT; i++)
    {
        // a face omits a slot by leaving its zone zeroed (zero-size rect)
        if (s_zones[i].rect.size.w == 0 || s_zones[i].rect.size.h == 0)
        {
            s_layers[i] = NULL;
            continue;
        }

        s_layers[i] = zone_make_layer(root, &s_zones[i]);
    }

    // placeholders for the slots that wait on phone/health data
    set_zone_text(ZONE_WEATHER, "--");
    set_zone_text(ZONE_COND, "--");
    set_zone_text(ZONE_LAT, "--");
    set_zone_text(ZONE_LON, "--");

    // seed the live state first so connection_init's initial callback matches it
    // and doesn't read the seed as a transition (no startup vibe). real changes
    // from here buzz per the configured pattern
    s_bt_connected = connection_is_connected();
    connection_init(connection_handler);

    shell_update_display();
}

/**
 * @brief Destroy the zone layers and let the face tear down its frame/overlays/fonts.
 *
 * @param window The window being unloaded.
 */
static void window_unload(Window *window)
{
    connection_deinit();

    for (int i = 0; i < ZONE_COUNT; i++)
    {
        if (s_layers[i])
        {
            text_layer_destroy(s_layers[i]);
            s_layers[i] = NULL;
        }
    }

    s_face->destroy_scene();
}

void shell_init(const WatchfaceDescriptor *face)
{
    s_face = face;
    s_main_window = window_create();
    window_set_window_handlers(s_main_window,
        (WindowHandlers){.load = window_load, .unload = window_unload});
    window_stack_push(s_main_window, true);
}

void shell_deinit(void)
{
    window_destroy(s_main_window);
}
