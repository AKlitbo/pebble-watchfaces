/**
 * @file phone_gate.h
 * @brief Starts the phone store live only while a panel shows the phone's battery.
 *
 * The phone sends every change in its battery, so a live store with nothing showing it would wake
 * the watch for each one. A store started not live tells the phone so, and the phone stops sending.
 * This starts it once at launch, and again when a save adds the first Phone Battery or Battery
 * Battery panel to any layout or takes the last one off.
 *
 * Which panels are placed comes from the face's mosaic_places_module, in its settings_schema.h.
 *
 * @ingroup mosaic_engine
 */
#pragma once

/**
 * @brief Starts the phone store, live when a layout places a panel showing the phone's battery.
 *
 * Call with the face's other live store inits, after the system store, since the phone store hears
 * the phone come and go through it. A dev build that seeded the stores skips the call, and then a
 * save never restarts the store either, so the seeded reading stays on screen.
 */
void phone_gate_start(void);

/**
 * @brief Starts the phone store again when a save changed whether any layout shows the phone's battery.
 *
 * Call from the face's settings handler. It does nothing before phone_gate_start, and nothing when
 * the answer is the same as before.
 */
void phone_gate_settings_changed(void);
