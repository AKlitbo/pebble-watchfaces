/**
 * @file vibe.h
 * @brief Haptic feedback wrapper
 *
 * @ingroup lib_system
 */
#pragma once
#include <pebble.h>

/**
 * @addtogroup lib_system
 * @{
 */

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
 * @brief Buzz a custom on and off rhythm unless in quiet time. The durations are milliseconds
 * that alternate buzz then pause then buzz and so on, starting on a buzz. Lets a caller play a
 * little tune that the three canned pulses cannot.
 *
 * @param durations The on and off times in milliseconds. The first entry is a buzz.
 * @param num_segments How many entries the durations array has.
 */
void vibe_custom(const uint32_t *durations, uint32_t num_segments);

/**
 * @brief Fire the pattern named by a VibeChoice (VIBE_NONE stays silent). The caller resolves
 * the choice from wherever it likes, so the lib never has to know which setting it came from.
 *
 * @param choice A VibeChoice value (VIBE_NONE / VIBE_SHORT / VIBE_LONG / VIBE_DOUBLE).
 */
void vibe_choice(uint8_t choice);

/**
 * @brief The default bluetooth-transition buzz: fire the pattern the user picked in settings
 * for a connect or disconnect. A face installs this as system_store's vibe policy so the
 * store itself never has to know about settings or buzzing.
 *
 * @param connected True when the link just came up, false when it just dropped.
 */
void vibe_bt_transition(bool connected);

/** @} */
