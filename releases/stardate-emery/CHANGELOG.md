# Changelog - Stardate Emery

All notable changes to the Stardate Emery watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.2] - 2026-06-17

### Fixed

- The battery gauge is now legible on the Lower Decks PADD theme. Its dark-blue fill was indistinguishable from the empty segments, so the charge level could not be read.

### Changed

- The battery gauge now uses a single consistent style across every theme. Charged cells are filled in the theme's accent color, and empty cells are hollow and outlined in the same color.

## [1.0.1] - 2026-06-16

> [!IMPORTANT]
> Due to a build mix-up, the 1.0.0 app store release was not built from the initial commit on main. This changelog tracks changes against the main branch history, so some entries below may not match the exact contents of the published 1.0.0 binary.

### Changed

- TRAVERSAL distance is now rounded to the nearest tenth instead of truncated, so an actual distance of 4.96 km now shows as 5.0 KM rather than the old, truncated 4.9 KM.

### Fixed

- Coordinate readout no longer shows an incorrect final digit. The fractional portion is now rounded instead of truncated.
- Saved settings now survive app updates. Stored settings are versioned and migrated across versions, and anything unreadable falls back to safe defaults instead of blanking the date line or flipping the clock into the wrong time format.
- The settings screen now opens with the current saved values instead of defaults. The watch sends its active configuration to the phone when the app launches, so saving no longer overwrites existing choices.
- Weather icons now reload only when the condition actually changes, avoiding a redundant bitmap reload on every refresh.
- The weather condition readout no longer clips a clear night to "CLEA...". It now shows "CLEAR" while still using the night (moon) icon.
- Battery life is improved by refreshing health stats (heart rate, steps, distance) only when the watch reports new readings, rather than re-reading them every minute and on every battery change.

## [1.0.0] - 2026-06-14
