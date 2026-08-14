# Changelog - LCARS Stardate

All notable changes to the LCARS Stardate watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.9.0] - 2026-08-12

### Added

- Added drag-and-drop panel configuration, with a preview of the watchface and available readouts.
- Added a Default button that restores the arrangement the watchface ships with, and a Clear button that empties all four panels.
- Added LCARS-styled settings matching the watchface.
- Added support for splitting the left column into two panels.
- Added moon readouts for illumination, phase, and days until the next full or new moon.
- Added sun readouts for sunrise, sunset, daylight duration, and countdowns to the next sunrise or sunset.
- Added weather readouts for humidity, wind, UV index, and daily high and low.
- Added readouts for battery level, calories, sleep, and active minutes.
- Added calendar readouts for Julian date, day of year, and week number.
- Added an Epoch Clock readout showing the raw Unix time.
- Added a Swatch Beats readout, so you can see internet time without setting the main clock to it.
- Added an Alternate Time Zone readout. Search for a city under Location Settings and the panel shows that city's time under its name.

### Changed

- Panel bars are now drawn by the watchface and take their colour from the theme, reducing the artwork by about a third.
- Redrew the battery and active minutes icons. The battery bolt was a doubled glyph that turned to mush at panel size, and active minutes now has a runner rather than a plain clock.
- Sleep, active minutes, and calories are read from the watch again now that they can be displayed.
- Upgrading preserves the existing layout, with panels starting on weather, heart rate, and steps.

### Fixed

- Fixed the partly cloudy icon sitting against the panel bar above it.

## [1.8.0] - 2026-08-03

### Changed

- The watchface no longer tracks or stores health history that it never displays, eliminating unnecessary storage writes.
- The watchface no longer reads sleep, active minutes, or calories from the watch. It has never displayed them, and reading them required an unnecessary storage access once per minute.

### Fixed

- Fixed the watchface making the watch feel slow and unresponsive. It was building and storing a full history of heart rate and step activity that it never displays, and rebuilding it every minute delayed the watch's background work.
- Fixed the heart rate, step, and distance readings going stale while the watch was sitting still. They now refresh once per minute whether or not you have been moving.

## [1.7.0] - 2026-07-29

### Added

- Added a 12-hour time format without a leading zero, so 8:30 rather than 08:30.

## [1.6.1] - 2026-07-28

### Fixed

- Fixed the saved weather reading being lost whenever the watchface reloaded, leaving the watchface blank until the next weather update.
- Fixed failed weather updates not being retried until the weather reading changed.
- Weather now refreshes when the phone reconnects if the previous reading has gone stale.

## [1.6.0] - 2026-07-27

### Added

- Added a Quiet Time indicator beside the Bluetooth icon. A new Show Quiet Time Icon setting under Appearance lets you turn it on or off.
- Added an Hourly Vibration option that vibrates at the top of every hour and stays silent during Quiet Time.
- Added two new frame themes, Voyager and Voyager Mono.

### Changed

- Reorganized settings into Clock, Health, and Weather sections.
- Replaced the Fahrenheit toggle with a Temperature Unit dropdown.

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
