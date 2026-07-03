/**
 * @file appmessage.c
 * @brief AppMessage transport: weather requests out, settings + readings in. Decodes
 * the wire format and hands results to the app through handlers, so it never
 * reaches into the ui shell
 */
#include "io/appmessage/appmessage.h"

#include "system/settings/settings.h"
#include <limits.h>

// registered channel handlers. each stays NULL until a consumer opts in
static struct
{
    WeatherHandler         on_weather;
    CoordsHandler          on_coords;
    SettingsChangedHandler on_settings_changed;
    WeatherExtraHandler    on_weather_extra;
    WeatherForecastHandler on_weather_forecast;
    LocationNameHandler    on_location_name;
} s_handlers;

void appmessage_on_weather(WeatherHandler cb)                  { s_handlers.on_weather = cb; }
void appmessage_on_coords(CoordsHandler cb)                    { s_handlers.on_coords = cb; }
void appmessage_on_settings_changed(SettingsChangedHandler cb) { s_handlers.on_settings_changed = cb; }
void appmessage_on_weather_extra(WeatherExtraHandler cb)       { s_handlers.on_weather_extra = cb; }
void appmessage_on_weather_forecast(WeatherForecastHandler cb) { s_handlers.on_weather_forecast = cb; }
void appmessage_on_location_name(LocationNameHandler cb)       { s_handlers.on_location_name = cb; }

#define WEATHER_RETRY_MAX 3          // phone-side pkjs suspend retry count
#define WEATHER_RETRY_DELAY_MS 5000  // delay to ride out transient drop

static AppTimer *s_weather_retry_timer; // timer for weather retry
static int s_weather_retries_left;      // number of retries left

// the outbox carries one message at a time so the most recent send's kind tells
// its delivery/failure callback whether it owns the weather retry
typedef enum
{
    OUTBOX_NONE,
    OUTBOX_WEATHER,
    OUTBOX_SETTINGS
} OutboxKind;
static OutboxKind s_outbox_kind;

/**
 * @brief Resend after a delay, and re-arm again if the outbox is still busy.
 *
 * @param data Timer context (unused).
 */
static void weather_retry(void *data);

/**
 * @brief Write and send REQUEST_WEATHER.
 *
 * @return false if the outbox refused it (a previous send still in flight, or no buffer)
 */
static bool send_weather_request(void)
{
    DictionaryIterator *iter;
    if (app_message_outbox_begin(&iter) != APP_MSG_OK)
    {
        return false;
    }

    dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
    if (app_message_outbox_send() != APP_MSG_OK)
    {
        return false;
    }

    s_outbox_kind = OUTBOX_WEATHER;
    return true;
}

/**
 * @brief Arm a retry if the budget allows, replacing any retry already pending.
 */
static void schedule_weather_retry(void)
{
    if (s_weather_retries_left <= 0)
    {
        return;
    }

    s_weather_retries_left--;

    if (s_weather_retry_timer)
    {
        app_timer_reschedule(s_weather_retry_timer, WEATHER_RETRY_DELAY_MS);
    }
    else
    {
        s_weather_retry_timer = app_timer_register(WEATHER_RETRY_DELAY_MS, weather_retry, NULL);
    }
}

static void weather_retry(void *data)
{
    s_weather_retry_timer = NULL;

    if (!send_weather_request())
    {
        schedule_weather_retry();
    }
}

void appmessage_request_weather(void)
{
    s_weather_retries_left = WEATHER_RETRY_MAX;

    if (!send_weather_request())
    {
        schedule_weather_retry();
    }
}

/**
 * @brief Send the current settings to the phone.
 *
 * Allows Clay to seed its store from the watch (the watch persist is the
 * source of truth, the phone's clay-settings can go stale).
 */
static void send_settings(void)
{
    DictionaryIterator *iter;
    if (app_message_outbox_begin(&iter) != APP_MSG_OK)
    {
        return;
    }

    settings_serialize(iter);

    // tag the send as settings so its delivery callback won't cancel a weather
    // retry waiting on the timer (only a weather delivery owns that)
    s_outbox_kind = OUTBOX_SETTINGS;
    app_message_outbox_send();
}

/**
 * @brief Apply an inbox message: weather, coords, and any changed settings.
 *
 * @param iterator The dictionary iterator containing the message.
 * @param context Application context (unused).
 */
static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
    // the phone asks for the watch's settings on launch to seed Clay so reply and stop
    if (dict_find(iterator, MESSAGE_KEY_REQUEST_SETTINGS))
    {
        send_settings();
        return;
    }

    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
    Tuple *wx_ok_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_OK);

    // the payload is phone-controlled so the wire types are checked before reading
    // temperature rides an int
    // conditions a cstring and the ok flag an int
    bool temp_is_int = temp_tuple && (temp_tuple->type == TUPLE_INT || temp_tuple->type == TUPLE_UINT);
    bool conditions_is_str = conditions_tuple && conditions_tuple->type == TUPLE_CSTRING;

    if (temp_is_int && conditions_is_str)
    {
        // missing ok flag (older JS) is treated as a real reading
        bool wx_ok = (!wx_ok_tuple) ||
            ((wx_ok_tuple->type == TUPLE_INT || wx_ok_tuple->type == TUPLE_UINT) && wx_ok_tuple->value->int32 == 1);

        static char temperature_buffer[12];  // sign + up to 3 digits + degree + unit (multibyte) + NUL
        static char conditions_buffer[32];

        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

        if (wx_ok)
        {
            int temp_value = (int)temp_tuple->value->int32;
            // clamp to a sane range so an extreme/corrupt value can't truncate mid-multibyte
            if (temp_value < -99)
            {
                temp_value = -99;
            }

            if (temp_value > 199)
            {
                temp_value = 199;
            }
            
            snprintf(temperature_buffer, sizeof(temperature_buffer),
                settings_u8(SETTING_TEMPERATURE_UNIT) ? "%d°F" : "%d°C", temp_value);

            if (s_handlers.on_weather)
            {
                s_handlers.on_weather(temperature_buffer, conditions_buffer);
            }
        }
        else
        {
            // no live reading (e.g. no location set or network error) so clear the placeholder
            APP_LOG(APP_LOG_LEVEL_INFO, "Weather Unavailable: %s", conditions_buffer);
            if (s_handlers.on_weather)
            {
                s_handlers.on_weather("--", NULL);
            }
        }
    }

    // extra weather readings only for faces that declare the keys (e.g. Modular).
    // the keys are absent from other faces' message_keys so guard the whole block
    // on the generated #defines to keep the shared transport compiling everywhere
#if defined(HAS_MESSAGE_KEY_HUMIDITY) && defined(HAS_MESSAGE_KEY_WIND_SPEED) && \
    defined(HAS_MESSAGE_KEY_WIND_DIR) && defined(HAS_MESSAGE_KEY_SUNRISE) && defined(HAS_MESSAGE_KEY_SUNSET)
    if (s_handlers.on_weather_extra)
    {
        Tuple *humidity_t = dict_find(iterator, MESSAGE_KEY_HUMIDITY);
        Tuple *wind_spd_t = dict_find(iterator, MESSAGE_KEY_WIND_SPEED);
        Tuple *wind_dir_t = dict_find(iterator, MESSAGE_KEY_WIND_DIR);
        Tuple *sunrise_t = dict_find(iterator, MESSAGE_KEY_SUNRISE);
        Tuple *sunset_t = dict_find(iterator, MESSAGE_KEY_SUNSET);

        // these keys only ride a weather message so any one present marks it as one
        if (humidity_t || wind_spd_t || wind_dir_t || sunrise_t || sunset_t)
        {
            int humidity = (humidity_t && (humidity_t->type == TUPLE_INT || humidity_t->type == TUPLE_UINT))
                ? (int)humidity_t->value->int32 : -1;
            int wind_kmh = (wind_spd_t && (wind_spd_t->type == TUPLE_INT || wind_spd_t->type == TUPLE_UINT))
                ? (int)wind_spd_t->value->int32 : -1;
            const char *wind_dir = (wind_dir_t && wind_dir_t->type == TUPLE_CSTRING) ? wind_dir_t->value->cstring : "";
            const char *sunrise = (sunrise_t && sunrise_t->type == TUPLE_CSTRING) ? sunrise_t->value->cstring : "";
            const char *sunset = (sunset_t && sunset_t->type == TUPLE_CSTRING) ? sunset_t->value->cstring : "";

            s_handlers.on_weather_extra(humidity, wind_kmh, wind_dir, sunrise, sunset);
        }
    }
#endif

    // daily forecast bits (uv now plus today's high/low plus precip chance) same opt-in guard.
    // absent fields ride out as INT_MIN so a real negative temperature is never mistaken
    // for no-data (unlike the -1 the extra readings use)
#if defined(HAS_MESSAGE_KEY_UV_INDEX) && defined(HAS_MESSAGE_KEY_TEMP_MAX) && \
    defined(HAS_MESSAGE_KEY_TEMP_MIN) && defined(HAS_MESSAGE_KEY_PRECIP_CHANCE)
    if (s_handlers.on_weather_forecast)
    {
        Tuple *uv_t = dict_find(iterator, MESSAGE_KEY_UV_INDEX);
        Tuple *tmax_t = dict_find(iterator, MESSAGE_KEY_TEMP_MAX);
        Tuple *tmin_t = dict_find(iterator, MESSAGE_KEY_TEMP_MIN);
        Tuple *pchance_t = dict_find(iterator, MESSAGE_KEY_PRECIP_CHANCE);

        if (uv_t || tmax_t || tmin_t || pchance_t)
        {
            int uv = (uv_t && (uv_t->type == TUPLE_INT || uv_t->type == TUPLE_UINT))
                ? (int)uv_t->value->int32 : INT_MIN;
            int temp_max = (tmax_t && (tmax_t->type == TUPLE_INT || tmax_t->type == TUPLE_UINT))
                ? (int)tmax_t->value->int32 : INT_MIN;
            int temp_min = (tmin_t && (tmin_t->type == TUPLE_INT || tmin_t->type == TUPLE_UINT))
                ? (int)tmin_t->value->int32 : INT_MIN;
            int precip_chance = (pchance_t && (pchance_t->type == TUPLE_INT || pchance_t->type == TUPLE_UINT))
                ? (int)pchance_t->value->int32 : INT_MIN;

            s_handlers.on_weather_forecast(uv, temp_max, temp_min, precip_chance);
        }
    }
#endif

    // coordinates arrive pre-formatted as LCARS dash strings like "33-44" and "-112-07"
    // each only read when present and actually a cstring
    Tuple *lat_t = dict_find(iterator, MESSAGE_KEY_LATITUDE);
    Tuple *lon_t = dict_find(iterator, MESSAGE_KEY_LONGITUDE);
    if ((lat_t || lon_t) && s_handlers.on_coords)
    {
        const char *lat = (lat_t && lat_t->type == TUPLE_CSTRING) ? lat_t->value->cstring : "";
        const char *lon = (lon_t && lon_t->type == TUPLE_CSTRING) ? lon_t->value->cstring : "";
        s_handlers.on_coords(lat, lon);
    }

#if defined(HAS_MESSAGE_KEY_LOCATION_NAME)
    Tuple *loc_name_t = dict_find(iterator, MESSAGE_KEY_LOCATION_NAME);
    if (loc_name_t && loc_name_t->type == TUPLE_CSTRING && s_handlers.on_location_name)
    {
        // read through a char pointer so gcc does not flag the zero-length cstring[]
        // flexible array member subscript
        const char *loc_name = loc_name_t->value->cstring;
        if (loc_name[0] != '{')
        {
            s_handlers.on_location_name(loc_name);
        }
    }
#endif

    // decode every settings field the message carries. the table owns which keys
    // exist and how each encodes so this transport never names a field
    SettingsInbound settings = settings_apply_inbox(iterator);
    if (settings.changed)
    {
        settings_save();

        if (s_handlers.on_settings_changed)
        {
            s_handlers.on_settings_changed(settings.layout_changed);
        }

        if (settings.weather_changed)
        {
            appmessage_request_weather();
        }
    }
}

/**
 * @brief Log a dropped inbox message.
 *
 * @param reason The reason the message was dropped.
 * @param context Application context (unused).
 */
static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message Dropped!");
}

/**
 * @brief Outbox send was nacked (usually pkjs asleep).
 *
 * Retry the weather request so it lands once the OS wakes the phone
 * (no-op when no request is in its retry budget).
 *
 * @param iter The dictionary iterator.
 * @param reason The reason the message failed.
 * @param context Application context (unused).
 */
static void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_WARNING, "Outbox failed: %d", (int)reason);

    // a failed settings reply must not arm a weather retry. only retry weather sends
    if (s_outbox_kind == OUTBOX_WEATHER)
    {
        schedule_weather_retry();
    }
}

/**
 * @brief Outbox delivered.
 *
 * Only a delivered weather request proves the weather channel is up, so a
 * settings reply sharing the outbox must not cancel a pending retry.
 *
 * @param iter The dictionary iterator.
 * @param context Application context (unused).
 */
static void outbox_sent_callback(DictionaryIterator *iter, void *context)
{
    // a settings reply sharing the outbox proves nothing about the weather channel
    if (s_outbox_kind != OUTBOX_WEATHER)
    {
        return;
    }

    s_outbox_kind = OUTBOX_NONE;
    s_weather_retries_left = 0;

    if (s_weather_retry_timer)
    {
        app_timer_cancel(s_weather_retry_timer);
        s_weather_retry_timer = NULL;
    }
}

void appmessage_open(void)
{
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    // Clay sends every setting in one message on save. a face with many fields
    // (e.g. Modular's layout string + goals) can exceed a small inbox and drop
    // the whole message. size generously. the outbox carries the settings seed
    // back which for a fully-themed face is the layout string + all goals +
    // CUSTOM_COLORS (~240B) and would overflow a 512B outbox and silently drop
    // the last-serialized field (CUSTOM_COLORS) so the config opens blank
    app_message_open(1024, 1024);
}
