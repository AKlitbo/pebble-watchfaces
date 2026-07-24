# Changelog - LCARS Stardate

All notable changes to the LCARS Stardate watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.5.0] - 2026-07-23

> [!IMPORTANT]
> This release corrects the watchface's message keys. As a result, some settings may reset to their defaults after updating and need to be set again.

### Fixed

- Fixed settings changes occasionally not reaching the watch.
- Fixed large settings changes occasionally failing to save.
- Fixed weather sometimes remaining blank after a failed update.
- Fixed weather sometimes remaining blank when using Fallback to Manual Location.

### Changed

- Reworked communication between the phone and watch to improve the reliability of settings, weather, and other updates.

### Notes

- This release changes how weather settings are stored. If weather appears stuck after updating, switch to a different weather provider, save, then switch back to your preferred provider and save again. This refreshes the stored weather configuration.

## [1.4.0] - 2026-07-03

### Added

- Weather now keeps its last reading when a refresh fails or the watchface reloads.
- Weather conditions now use night-specific icons after dark.

### Changed

- Updated watchface backgrounds and settings layout to improve readability.

## [1.3.0] - 2026-06-25

### Added

- Added three fully grayscale Mono theme variations: Classic Mono, Lower Decks Mono, and Lower Decks PADD Mono.

## [1.2.2] - 2026-06-24

### Fixed

- Fixed weather provider errors and rate limits leaving stale weather.
- Fixed location search occasionally saving the wrong city.
- Fixed non-weather settings changes unnecessarily refreshing the weather.

## [1.2.1] - 2026-06-24

### Fixed

- Fixed unrecognised weather conditions showing no icon.
- Fixed switching to or from Swatch Internet Time (.beats) interrupting weather updates.

## [1.2.0] - 2026-06-20

### Added

- Added a Bluetooth connection icon with an optional visibility toggle.
- Added optional Bluetooth connection alerts with configurable vibration patterns.

### Fixed

- Fixed long date formats being clipped in the date banner.

### Fixed

- The longest date formats now shrink to fit within the date banner instead of being clipped. Wide layouts such as the text month and day-of-year formats previously overflowed the banner, and the smaller fallback sizes now sit centered within it.

## [1.1.0] - 2026-06-18

### Added

- Added additional date format options, including day-first, month-first, year-first, text month, and day-of-year formats.

## [1.0.3] - 2026-06-17

### Fixed

- Fixed Swatch Internet Time (.beats) updating only once per minute.

### Changed

- Reduced unnecessary date redraws.

## [1.0.2] - 2026-06-17

### Fixed

- Fixed the Lower Decks PADD battery gauge being difficult to read.

### Changed

- Unified the battery gauge style across all themes.

## [1.0.1] - 2026-06-16

> [!IMPORTANT]
> Due to a build mix-up, the 1.0.0 app store release was not built from the initial commit on main. This changelog tracks changes against the main branch history, so some entries below may not match the exact contents of the published 1.0.0 binary.

### Fixed

- Fixed coordinate readouts occasionally displaying the wrong final digit.
- Fixed saved settings not surviving app updates.
- Fixed the settings screen not reflecting the current watch configuration.
- Fixed unnecessary weather icon reloads.
- Fixed the weather condition text clipping at night.

### Changed

- Rounded traversal distance to the nearest tenth.
- Reduced health stat updates to improve battery life.

## [1.0.0] - 2026-06-14

### Added

- First release of the LCARS Stardate watchface.
