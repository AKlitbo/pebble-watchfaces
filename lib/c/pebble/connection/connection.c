/**
 * @file connection.c
 * @brief pebble connection service: subscribes to bluetooth connection changes
 */
#include "connection.h"

static ConnectionChangedHandler s_handler;

/**
 * @brief Forward a connection change to the face's handler.
 *
 * @param connected True if connected, false otherwise.
 */
static void connection_handler(bool connected)
{
    if (s_handler)
    {
        s_handler(connected);
    }
}

void connection_init(ConnectionChangedHandler handler)
{
    s_handler = handler;

    connection_service_subscribe((ConnectionHandlers){
        .pebble_app_connection_handler = connection_handler,
    });

    if (s_handler)
    {
        s_handler(connection_is_connected());
    }
}

void connection_deinit(void)
{
    connection_service_unsubscribe();
    s_handler = NULL;
}

bool connection_is_connected(void)
{
    return connection_service_peek_pebble_app_connection();
}
