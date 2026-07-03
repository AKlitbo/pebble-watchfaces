/**
 * @file vibe.h
 * @brief Haptic feedback wrapper
 *
 * @ingroup lib
 */
#pragma once
#include <pebble.h>

/** @addtogroup lib @{ */

/**
 * @brief Defines the available haptic pulse patterns.
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

/**
 * @brief The default bluetooth-transition buzz: fire the pattern the user picked in settings
 * for a connect or disconnect. A face installs this as system_store's vibe policy so the
 * store itself stays settings- and vibe-agnostic.
 *
 * @param connected True when the link just came up, false when it just dropped.
 */
void vibe_bt_transition(bool connected);

/** @} */
