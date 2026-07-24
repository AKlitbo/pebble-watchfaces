/**
 * @file appmessage.c
 * @brief AppMessage transport: weather requests out, settings + readings in. Decodes
 * the wire format and hands results to the app through handlers, so it never
 * reaches into the UI itself.
 */
#include "io/appmessage/appmessage.h"

#include "io/tuple_read.h"
#include "system/settings/settings.h"
#include <limits.h>

// registered channel handlers. each stays NULL until a consumer opts in
static struct
{
    WeatherHandler              on_weather;
    CoordsHandler               on_coords;
    SettingsChangedHandler      on_settings_changed;
    WeatherExtraHandler         on_weather_extra;
    WeatherForecastHandler      on_weather_forecast;
    WeatherAirHandler           on_weather_air;
    WeatherForecastStripHandler on_forecast_hourly;
    WeatherForecastStripHandler on_forecast_daily;
    LocationNameHandler         on_location_name;
    StockStripHandler           on_stock_strip;
    CalendarStripHandler        on_calendar_strip;
    CustomColorsHandler         on_custom_colors;
    CustomColorsProvider        custom_colors_provider;
    InboxCompleteHandler        on_inbox_complete;
} s_handlers;

void appmessage_on_weather(WeatherHandler cb)                  { s_handlers.on_weather = cb; }
void appmessage_on_coords(CoordsHandler cb)                    { s_handlers.on_coords = cb; }
void appmessage_on_settings_changed(SettingsChangedHandler cb) { s_handlers.on_settings_changed = cb; }
void appmessage_on_weather_extra(WeatherExtraHandler cb)       { s_handlers.on_weather_extra = cb; }
void appmessage_on_weather_forecast(WeatherForecastHandler cb) { s_handlers.on_weather_forecast = cb; }
void appmessage_on_weather_air(WeatherAirHandler cb)          { s_handlers.on_weather_air = cb; }
void appmessage_on_weather_forecast_hourly(WeatherForecastStripHandler cb) { s_handlers.on_forecast_hourly = cb; }
void appmessage_on_weather_forecast_daily(WeatherForecastStripHandler cb)  { s_handlers.on_forecast_daily = cb; }
void appmessage_on_location_name(LocationNameHandler cb)       { s_handlers.on_location_name = cb; }
void appmessage_on_stock_strip(StockStripHandler cb)          { s_handlers.on_stock_strip = cb; }
void appmessage_on_calendar_strip(CalendarStripHandler cb)   { s_handlers.on_calendar_strip = cb; }
void appmessage_on_custom_colors(CustomColorsHandler cb)      { s_handlers.on_custom_colors = cb; }
void appmessage_set_custom_colors_provider(CustomColorsProvider cb) { s_handlers.custom_colors_provider = cb; }
void appmessage_on_inbox_complete(InboxCompleteHandler cb)    { s_handlers.on_inbox_complete = cb; }

// only one AppMessage send can be in flight at a time, so every outbound message (the weather,
// stock and calendar requests plus the settings reply) goes through one queue: send the head, wait
// for its sent/failed callback, then send the next. the queue drains a pass at a time. a request
// the phone nacks (its pkjs was asleep, and that first send is usually what wakes it) is held in a
// failed set and retried on a later pass, so a whole poll is not lost to one asleep-phone nack and
// left stale until the next interval
#define REQUEST_RETRY_MAX 3
#define REQUEST_RETRY_DELAY_MS 5000
#define OUTBOX_QUEUE_MAX 6

typedef enum
{
    OUTBOX_NONE,
    OUTBOX_WEATHER,
    OUTBOX_SETTINGS,
    OUTBOX_STOCK,
    OUTBOX_CALENDAR
} OutboxKind;

typedef struct
{
    OutboxKind kind;
    int        retries_left; ///< Retry passes left after a failed send (0 for the settings reply)
} OutboxJob;

static OutboxJob s_queue[OUTBOX_QUEUE_MAX];  // this pass's work queue
static int       s_queue_len;
static OutboxJob s_failed[OUTBOX_QUEUE_MAX]; // requests that nacked this pass, held for the next
static int       s_failed_len;
static bool      s_sending;                  // a send is out, waiting on its sent/failed callback
static OutboxJob s_inflight;                 // the job in flight, valid while s_sending
static AppTimer *s_retry_timer;              // delay before the next retry pass

static void pump(void);
static void write_settings(DictionaryIterator *iter);

// true for the phone-data requests. the settings reply is not one, so it never retries
static bool is_request_kind(OutboxKind kind)
{
    return kind == OUTBOX_WEATHER || kind == OUTBOX_STOCK || kind == OUTBOX_CALENDAR;
}

// build the message for a kind and hand it to the outbox. returns false if the outbox refused it
static bool send_job(OutboxKind kind)
{
    DictionaryIterator *iter;
    if (app_message_outbox_begin(&iter) != APP_MSG_OK)
    {
        return false;
    }

    switch (kind)
    {
        case OUTBOX_WEATHER:
            dict_write_uint8(iter, MESSAGE_KEY_WEATHER_REQUEST, 1);
            break;
#if defined(HAS_MESSAGE_KEY_STOCK_REQUEST)
        case OUTBOX_STOCK:
            dict_write_uint8(iter, MESSAGE_KEY_STOCK_REQUEST, 1);
            break;
#endif
#if defined(HAS_MESSAGE_KEY_CALENDAR_REQUEST)
        case OUTBOX_CALENDAR:
            dict_write_uint8(iter, MESSAGE_KEY_CALENDAR_REQUEST, 1);
            break;
#endif
        case OUTBOX_SETTINGS:
            write_settings(iter);
            break;
        default:
            return false; // a face that doesn't declare the key never asks for that kind
    }

    return app_message_outbox_send() == APP_MSG_OK;
}

// true when a kind already sits in flight, in the work queue, or in the failed set
static bool kind_pending(OutboxKind kind)
{
    if (s_sending && s_inflight.kind == kind)
    {
        return true;
    }
    for (int i = 0; i < s_queue_len; i++)
    {
        if (s_queue[i].kind == kind)
        {
            return true;
        }
    }
    for (int i = 0; i < s_failed_len; i++)
    {
        if (s_failed[i].kind == kind)
        {
            return true;
        }
    }
    return false;
}

// add a job to the work queue unless its kind is already pending, then pump
static void enqueue(OutboxKind kind, int retries)
{
    if (kind_pending(kind))
    {
        return; // a poll that repeats just keeps the one already waiting
    }
    if (s_queue_len < OUTBOX_QUEUE_MAX)
    {
        s_queue[s_queue_len].kind = kind;
        s_queue[s_queue_len].retries_left = retries;
        s_queue_len++;
    }
    pump();
}

// drop the front job off the work queue
static void queue_pop_front(void)
{
    for (int i = 1; i < s_queue_len; i++)
    {
        s_queue[i - 1] = s_queue[i];
    }
    s_queue_len--;
}

// hold a nacked request for the next pass, if it still has retries and is a real request
static void hold_failed(OutboxJob job)
{
    if (!is_request_kind(job.kind) || job.retries_left <= 0)
    {
        return;
    }
    if (s_failed_len < OUTBOX_QUEUE_MAX)
    {
        s_failed[s_failed_len].kind = job.kind;
        s_failed[s_failed_len].retries_left = job.retries_left - 1;
        s_failed_len++;
    }
}

static void retry_pass(void *data)
{
    s_retry_timer = NULL;

    // move the whole failed set back into the work queue for another pass
    while (s_failed_len > 0 && s_queue_len < OUTBOX_QUEUE_MAX)
    {
        s_queue[s_queue_len++] = s_failed[0];
        for (int i = 1; i < s_failed_len; i++)
        {
            s_failed[i - 1] = s_failed[i];
        }
        s_failed_len--;
    }

    pump();
}

// send the head job when the outbox is free, once the phone is there
static void pump(void)
{
    if (s_sending)
    {
        return;
    }

    // nothing reaches a phone that isn't there. dump everything and let a store's next poll ask
    // again once it reconnects
    if (!connection_service_peek_pebble_app_connection())
    {
        s_queue_len = 0;
        s_failed_len = 0;
        if (s_retry_timer)
        {
            app_timer_cancel(s_retry_timer);
            s_retry_timer = NULL;
        }
        return;
    }

    // this pass is done: if any request failed, arm the next pass and stop
    if (s_queue_len == 0)
    {
        if (s_failed_len > 0 && !s_retry_timer)
        {
            s_retry_timer = app_timer_register(REQUEST_RETRY_DELAY_MS, retry_pass, NULL);
        }
        return;
    }

    if (send_job(s_queue[0].kind))
    {
        s_inflight = s_queue[0];
        queue_pop_front();
        s_sending = true; // wait for the sent/failed callback before the next send
    }
    else
    {
        // the outbox refused it even though the phone is there (rare). hold it for a retry pass and
        // move on, so a stuck head can't wedge the queue
        hold_failed(s_queue[0]);
        queue_pop_front();
        pump();
    }
}

void appmessage_request_weather(void)
{
    enqueue(OUTBOX_WEATHER, REQUEST_RETRY_MAX);
}

void appmessage_request_stock(void)
{
#if defined(HAS_MESSAGE_KEY_STOCK_REQUEST)
    enqueue(OUTBOX_STOCK, REQUEST_RETRY_MAX);
#endif
}

void appmessage_request_calendar(void)
{
#if defined(HAS_MESSAGE_KEY_CALENDAR_REQUEST)
    enqueue(OUTBOX_CALENDAR, REQUEST_RETRY_MAX);
#endif
}

/**
 * @brief Write the settings reply payload into the outbox iterator.
 *
 * The watch persist is the source of truth so Clay can seed its store from it. Carries the current
 * settings, a fresh flag, and the custom colours.
 *
 * @param iter The outbox iterator to write into.
 */
static void write_settings(DictionaryIterator *iter)
{
    settings_serialize(iter);

    // tell the phone whether we booted with no saved settings (wiped by an install/update), so it
    // can push its own config back instead of letting these defaults seed over it
#if defined(HAS_MESSAGE_KEY_SETTINGS_FRESH)
    dict_write_uint8(iter, MESSAGE_KEY_SETTINGS_FRESH, settings_was_fresh() ? 1 : 0);
#endif

    // custom colours ride their own key (split across two persist blobs on the watch), so the
    // settings table does not carry them: the provider rebuilds the combined string into a stack
    // buffer that lives only for this send since dict_write_cstring copies it into the outbox
#if defined(HAS_MESSAGE_KEY_APPEARANCE_CUSTOM_COLORS)
    if (s_handlers.custom_colors_provider)
    {
        char combined[APPMESSAGE_CUSTOM_COLORS_MAX];
        s_handlers.custom_colors_provider(combined, sizeof(combined));
        dict_write_cstring(iter, MESSAGE_KEY_APPEARANCE_CUSTOM_COLORS, combined);
    }
#endif
}

/**
 * @brief Queue the settings reply so it serialises with the data requests through the one outbox.
 */
static void send_settings(void)
{
    enqueue(OUTBOX_SETTINGS, 0);
}

// the forecast/stock/calendar strips all ride as packed byte arrays through a same-signature
// handler, so one dispatcher covers them: guard on the handler, find the tuple, type-check, pass
// the raw bytes on. the handler comes in as an argument (not a table) so it stays out of .data
static void dispatch_bytes(DictionaryIterator *iter, uint32_t key,
                           void (*cb)(const uint8_t *data, uint16_t len))
{
    if (!cb)
    {
        return;
    }
    Tuple *tuple = dict_find(iter, key);
    if (tuple && tuple->type == TUPLE_BYTE_ARRAY)
    {
        cb(tuple->value->data, tuple->length);
    }
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
    if (dict_find(iterator, MESSAGE_KEY_SETTINGS_REQUEST))
    {
        send_settings();
        return;
    }

    Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMPERATURE);
    Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_CONDITIONS);
    Tuple *wx_ok_tuple = dict_find(iterator, MESSAGE_KEY_WEATHER_OK);

    // the payload is phone-controlled so everything here reads through the tuple helpers above.
    // they check the width as well as the type, which the type on its own cannot do: the SDK
    // sizes an int by its length and packs the tuples back to back, so reading int32 off a
    // narrower one picks up the next tuple's key
    const char *conditions = tuple_str_or(conditions_tuple, NULL);
    bool temp_is_int = temp_tuple && (temp_tuple->type == TUPLE_INT || temp_tuple->type == TUPLE_UINT);

    if (temp_is_int && conditions)
    {
        // missing ok flag (older JS) is treated as a real reading. one that is there but does not
        // read as an int is not: a fallback of 0 keeps a malformed flag on the unavailable side
        bool wx_ok = !wx_ok_tuple || tuple_int_or(wx_ok_tuple, 0) == 1;

        static char conditions_buffer[32];

        snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions);

        if (wx_ok)
        {
            int temp_value = tuple_int_or(temp_tuple, 0);
            // clamp to a sane range so a corrupt reading can't display absurdly or overflow the
            // store's int16 field. the face turns the number into "23°C"/"23°F" when it draws
            if (temp_value < -99)
            {
                temp_value = -99;
            }
            if (temp_value > 199)
            {
                temp_value = 199;
            }

            if (s_handlers.on_weather)
            {
                s_handlers.on_weather(temp_value, conditions_buffer);
            }
        }
        else
        {
            // no live reading (e.g. no location set or network error). the NULL cond tells the
            // store to keep its last good reading rather than blank out (the temp is ignored)
            APP_LOG(APP_LOG_LEVEL_INFO, "Weather Unavailable: %s", conditions_buffer);
            if (s_handlers.on_weather)
            {
                s_handlers.on_weather(0, NULL);
            }
        }
    }

    // extra weather readings only for faces that declare the keys
    // the keys are absent from other faces' message_keys so guard the whole block
    // on the generated #defines to keep the shared transport compiling everywhere
#if defined(HAS_MESSAGE_KEY_WEATHER_HUMIDITY) && defined(HAS_MESSAGE_KEY_WEATHER_WIND_SPEED) && \
    defined(HAS_MESSAGE_KEY_WEATHER_WIND_DIR) && defined(HAS_MESSAGE_KEY_WEATHER_SUNRISE) && defined(HAS_MESSAGE_KEY_WEATHER_SUNSET)
    if (s_handlers.on_weather_extra)
    {
        Tuple *humidity_t = dict_find(iterator, MESSAGE_KEY_WEATHER_HUMIDITY);
        Tuple *wind_spd_t = dict_find(iterator, MESSAGE_KEY_WEATHER_WIND_SPEED);
        Tuple *wind_dir_t = dict_find(iterator, MESSAGE_KEY_WEATHER_WIND_DIR);
        Tuple *sunrise_t = dict_find(iterator, MESSAGE_KEY_WEATHER_SUNRISE);
        Tuple *sunset_t = dict_find(iterator, MESSAGE_KEY_WEATHER_SUNSET);

        // these keys only ride a weather message so any one present marks it as one
        if (humidity_t || wind_spd_t || wind_dir_t || sunrise_t || sunset_t)
        {
            int humidity = tuple_int_or(humidity_t, -1);
            int wind_kmh = tuple_int_or(wind_spd_t, -1);
            const char *wind_dir = tuple_str_or(wind_dir_t, "");
            const char *sunrise = tuple_str_or(sunrise_t, "");
            const char *sunset = tuple_str_or(sunset_t, "");

            s_handlers.on_weather_extra(humidity, wind_kmh, wind_dir, sunrise, sunset);
        }
    }
#endif

    // daily forecast bits (uv now plus today's high/low plus precip chance) same opt-in guard.
    // absent fields ride out as INT_MIN so a real negative temperature is never mistaken
    // for no-data (unlike the -1 the extra readings use)
#if defined(HAS_MESSAGE_KEY_WEATHER_UV_INDEX) && defined(HAS_MESSAGE_KEY_WEATHER_TEMP_MAX) && \
    defined(HAS_MESSAGE_KEY_WEATHER_TEMP_MIN) && defined(HAS_MESSAGE_KEY_WEATHER_PRECIP_CHANCE)
    if (s_handlers.on_weather_forecast)
    {
        Tuple *uv_t = dict_find(iterator, MESSAGE_KEY_WEATHER_UV_INDEX);
        Tuple *tmax_t = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMP_MAX);
        Tuple *tmin_t = dict_find(iterator, MESSAGE_KEY_WEATHER_TEMP_MIN);
        Tuple *pchance_t = dict_find(iterator, MESSAGE_KEY_WEATHER_PRECIP_CHANCE);

        if (uv_t || tmax_t || tmin_t || pchance_t)
        {
            int uv = tuple_int_or(uv_t, INT_MIN);
            int temp_max = tuple_int_or(tmax_t, INT_MIN);
            int temp_min = tuple_int_or(tmin_t, INT_MIN);
            int precip_chance = tuple_int_or(pchance_t, INT_MIN);

            s_handlers.on_weather_forecast(uv, temp_max, temp_min, precip_chance);
        }
    }
#endif

    // air readings (feels-like plus surface pressure plus dew point). same opt-in guard,
    // absent fields ride out as INT_MIN so a real negative temperature is never no-data
#if defined(HAS_MESSAGE_KEY_WEATHER_FEELS_LIKE) && defined(HAS_MESSAGE_KEY_WEATHER_PRESSURE) && \
    defined(HAS_MESSAGE_KEY_WEATHER_DEW_POINT)
    if (s_handlers.on_weather_air)
    {
        Tuple *feels_t = dict_find(iterator, MESSAGE_KEY_WEATHER_FEELS_LIKE);
        Tuple *pressure_t = dict_find(iterator, MESSAGE_KEY_WEATHER_PRESSURE);
        Tuple *dew_t = dict_find(iterator, MESSAGE_KEY_WEATHER_DEW_POINT);

        if (feels_t || pressure_t || dew_t)
        {
            int feels_like = tuple_int_or(feels_t, INT_MIN);
            int pressure = tuple_int_or(pressure_t, INT_MIN);
            int dew_point = tuple_int_or(dew_t, INT_MIN);

            s_handlers.on_weather_air(feels_like, pressure, dew_point);
        }
    }
#endif

    // the forecast, stock, and calendar strips ride as packed byte arrays. each is opt-in via its
    // own key, and the store owns the wire format, so we just hand the raw bytes on
#if defined(HAS_MESSAGE_KEY_WEATHER_FORECAST_HOURLY)
    dispatch_bytes(iterator, MESSAGE_KEY_WEATHER_FORECAST_HOURLY, s_handlers.on_forecast_hourly);
#endif
#if defined(HAS_MESSAGE_KEY_WEATHER_FORECAST_DAILY)
    dispatch_bytes(iterator, MESSAGE_KEY_WEATHER_FORECAST_DAILY, s_handlers.on_forecast_daily);
#endif
#if defined(HAS_MESSAGE_KEY_STOCK_STRIP)
    dispatch_bytes(iterator, MESSAGE_KEY_STOCK_STRIP, s_handlers.on_stock_strip);
#endif
#if defined(HAS_MESSAGE_KEY_CALENDAR_STRIP)
    dispatch_bytes(iterator, MESSAGE_KEY_CALENDAR_STRIP, s_handlers.on_calendar_strip);
#endif

    // coordinates arrive pre-formatted as dash strings like "33-44" and "-112-07". a fix is the
    // pair, so both keys have to be there. one on its own would hand the store an empty string
    // for the other half and blank a good coordinate
    Tuple *lat_t = dict_find(iterator, MESSAGE_KEY_LOCATION_LATITUDE);
    Tuple *lon_t = dict_find(iterator, MESSAGE_KEY_LOCATION_LONGITUDE);
    if (lat_t && lon_t && s_handlers.on_coords)
    {
        const char *lat = tuple_str_or(lat_t, "");
        const char *lon = tuple_str_or(lon_t, "");
        s_handlers.on_coords(lat, lon);
    }

#if defined(HAS_MESSAGE_KEY_LOCATION_NAME)
    Tuple *loc_name_t = dict_find(iterator, MESSAGE_KEY_LOCATION_NAME);
    if (s_handlers.on_location_name)
    {
        const char *loc_name = tuple_str_or(loc_name_t, "");
        if (loc_name[0] != '\0' && loc_name[0] != '{')
        {
            s_handlers.on_location_name(loc_name);
        }
    }
#endif

    // custom colours ride their own key, not the settings table: the combined "~3 colours | flags"
    // string can exceed a persist key, so read it straight off the tuple and let the face split +
    // store it into the two blobs
    bool custom_changed = false;
#if defined(HAS_MESSAGE_KEY_APPEARANCE_CUSTOM_COLORS)
    Tuple *custom_colors_t = dict_find(iterator, MESSAGE_KEY_APPEARANCE_CUSTOM_COLORS);
    if (custom_colors_t && s_handlers.on_custom_colors)
    {
        // this string gets split straight into the two persist blobs, so an unterminated one
        // would carry whatever followed it in the inbox all the way into flash
        const char *custom = tuple_str_or(custom_colors_t, NULL);
        if (custom)
        {
            s_handlers.on_custom_colors(custom);
            custom_changed = true;
        }
    }
#endif

    // decode every settings field the message carries. the table owns which keys
    // exist and how each encodes so this transport never names a field
    SettingsInbound settings = settings_apply_inbox(iterator);
    if (settings.changed || custom_changed)
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

    // the whole message is dispatched: let a store that took several channels commit them
    // once (the weather store coalesces its per-channel writes into a single persist here)
    if (s_handlers.on_inbox_complete)
    {
        s_handlers.on_inbox_complete();
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
 * @brief Outbox send was nacked (usually the phone's pkjs is asleep).
 *
 * Frees the outbox and holds a real request for a later retry pass, then pumps the rest of the
 * queue. The settings reply is not held.
 *
 * @param iter The dictionary iterator.
 * @param reason The reason the message failed.
 * @param context Application context (unused).
 */
static void outbox_failed_callback(DictionaryIterator *iter, AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_WARNING, "Outbox failed: %d", (int)reason);

    // the in-flight send is done (nacked). hold a real request for a later retry pass, then carry
    // on with the queue. the send often wakes an asleep pkjs so the next pass tends to land
    s_sending = false;
    hold_failed(s_inflight);
    pump();
}

/**
 * @brief Outbox delivered. Frees the outbox and sends the next queued job.
 *
 * @param iter The dictionary iterator.
 * @param context Application context (unused).
 */
static void outbox_sent_callback(DictionaryIterator *iter, void *context)
{
    // the in-flight send landed, so free the outbox and send the next queued job
    s_sending = false;
    pump();
}

void appmessage_open(void)
{
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    // Clay sends every setting in one message on save, and that dict is large and variable: the
    // calendar URL, custom colours, saved-layout slots, api keys, and vibe patterns can add up
    // past 2KB. a message bigger than the inbox is dropped whole, so open both buffers at the
    // platform maximum rather than guess a fixed size (the outbox carries the settings seed back)
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}
