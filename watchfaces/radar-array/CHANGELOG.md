# Changelog - Radar Array

All notable changes to the Radar Array watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.3.0] - 2026-07-23

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

## [1.2.0] - 2026-07-03

### Added

- Weather now keeps its last reading when a refresh fails or the watchface reloads.
- GPS coordinates are now retained across watchface reloads.

### Changed

- Updated watchface backgrounds and settings layout to improve readability.

## [1.1.0] - 2026-06-25

### Added

- Added a fully grayscale Mono theme.

## [1.0.2] - 2026-06-24

### Fixed

- Fixed weather provider errors and rate limits leaving stale weather.
- Fixed location search occasionally saving the wrong city.
- Fixed non-weather settings changes unnecessarily refreshing the weather.

## [1.0.1] - 2026-06-24

### Fixed

- Fixed unrecognised weather conditions showing no icon.
- Fixed switching to or from Swatch Internet Time (.beats) interrupting weather updates.

## [1.0.0] - 2026-06-22

### Added

- First release of the Radar Array watchface.