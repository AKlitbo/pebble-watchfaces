/**
 * @file pebble.h
 * @brief The sliver of the SDK the host specs need, for the pure sources that include it.
 *
 * A few files under lib/c/pebble are plain logic that happen to include <pebble.h> for a constant
 * or the log macro. This stands in for that much and nothing else: no persist, no timers, no
 * dictionaries, no AppMessage. Nothing here mirrors SDK behaviour, so nothing here can drift out
 * of step with it and quietly turn a passing test into a wrong watch.
 *
 * That is the whole line. A source that wants a real SDK call tested is a source this cannot help,
 * and it stays on the device. See the README for why.
 */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

/** The SDK's own value, which the poll maths multiplies minutes by. */
#define SECONDS_PER_MINUTE (60)

#define APP_LOG_LEVEL_ERROR 1
#define APP_LOG_LEVEL_WARNING 50
#define APP_LOG_LEVEL_INFO 100
#define APP_LOG_LEVEL_DEBUG 200

/** Swallows a log line. Takes the arguments so an unused variable still counts as used. */
static inline void host_app_log(int level, const char *fmt, ...)
{
    (void)level;
    (void)fmt;
}

#define APP_LOG(level, ...) host_app_log(level, __VA_ARGS__)
