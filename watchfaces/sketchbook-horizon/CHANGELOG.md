# Changelog - Sketchbook Horizon

All notable changes to the Sketchbook Horizon watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Changed the weather API Key to show only for OpenWeatherMap and WeatherAPI.com, the two sources that need one.
- Changed the weather API Key to keep a key for each source. Switching between OpenWeatherMap and WeatherAPI.com shows that source's own key, and switching back no longer needs the key pasted again.

### Fixed

- Fixed the weather fetching twice each time the watchface starts.
- Fixed the settings page opening on a numbered Date Format while the watch wrote the date in words, until you saved one. Saving without touching it switched the date to numbers.
- Fixed the settings page showing extra space under and beside its controls in the Pebble app on Android while plugins are on.

### Notes

- This release needs watch firmware 4.44.3 or later. On older firmware the watch lists Sketchbook Horizon as incompatible until it updates.

## [1.0.0] - 2026-10-16

### Added

- First release of the Sketchbook Horizon watchface.

### Notes

Sketchbook Horizon brings the four separate Ridgeline, Shoreline, Skyline, and Treeline faces together into one. For the history of those faces, see their changelogs below.

- [Ridgeline](.retired/sketchbook/ridgeline/CHANGELOG.md)
- [Shoreline](.retired/sketchbook/shoreline/CHANGELOG.md)
- [Skyline](.retired/sketchbook/skyline/CHANGELOG.md)
- [Treeline](.retired/sketchbook/treeline/CHANGELOG.md)
