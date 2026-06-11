// appmessage transport: decodes the inbox and notifies the app via handlers
#pragma once
#include <pebble.h>

// callbacks the app supplies so the transport stays decoupled from the ui shell
typedef struct
{
    void (*on_weather)(const char *temp, const char *condition);
    void (*on_coords)(const char *lat, const char *lon);
    void (*on_settings_changed)(bool time_or_date_changed);
} AppMessageHandlers;

void appmessage_init(AppMessageHandlers handlers);
void appmessage_request_weather(void);
