/**
 * @file vibe.h
 * @brief haptic feedback wrapper
 */
#pragma once
#include <pebble.h>

/**
 * @brief Defines the available haptic pulse patterns.
 *
 * @ingroup lib
 */
typedef enum
{
    VibePulseShort,
    VibePulseLong,
    VibePulseDouble,
} VibePulse;

/**
 * @brief Fire a pulse unless in quiet time.
 *
 * @param pulse The pulse pattern to fire.
 */
void vibe_pulse(VibePulse pulse);
