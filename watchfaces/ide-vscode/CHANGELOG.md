# Changelog - IDE VSCode

All notable changes to the IDE VSCode watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.3.1] - 2026-07-28

### Fixed

- Fixed the saved weather reading being lost whenever the watchface reloaded, leaving the watchface blank until the next weather update.
- Fixed failed weather updates not being retried until the weather reading changed.
- Weather now refreshes when the phone reconnects if the previous reading has gone stale.

## [1.3.0] - 2026-07-27

### Added

- Added an Hourly Vibration option that vibrates at the top of every hour and stays silent during Quiet Time.

### Changed

- Reorganized settings into Clock, Health, and Weather sections.
- Replaced the Fahrenheit toggle with a Temperature Unit dropdown.

## [1.2.0] - 2026-07-23

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

## [1.1.0] - 2026-07-03

### Added

- Weather now keeps its last reading when a refresh fails or the watchface reloads.
- Weather conditions now use night-specific icons after dark.

### Changed

- Updated the watchface backgrounds and settings layout to improve readability.

## [1.0.0] - 2026-06-25

### Added

- First release of the IDE VSCode watchface.