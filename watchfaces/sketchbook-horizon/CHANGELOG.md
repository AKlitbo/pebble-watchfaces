# Changelog - Sketchbook Horizon

All notable changes to the Sketchbook Horizon watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Added a Pebble App: Preferred Source choice to the weather Data Source, which shows the weather for the first location in the Pebble app's weather settings. While it is picked, the units and location come from the plugin, and the watchface's own unit and location settings are hidden.
- Added a list on the settings page of the Pebble app plugins this watchface works with, each with the version that has been tested.

### Changed

- Changed a weather source you have never picked to start on Pebble App: Preferred Source when a plugin on your phone serves it. A source you already picked stays as it is.
- Changed the weather API Key to show only for OpenWeatherMap and WeatherAPI.com, the two sources that need one.
- Changed the weather API Key to keep a key for each source. Switching between OpenWeatherMap and WeatherAPI.com shows that source's own key, and switching back no longer needs the key pasted again.
- Changed the settings page to carry its own hand lettering, so it draws in Patrick Hand without a connection and needs no font site allowed in the Pebble app.

### Fixed

- Fixed the weather fetching twice each time the watchface starts.
- Fixed the settings page opening on a numbered Date Format while the watch wrote the date in words, until you saved one. Saving without touching it switched the date to numbers.
- Fixed the settings page showing extra space under and beside its controls in the Pebble app on Android while plugins are on.
- Fixed the Save button on the settings page turning bright orange while you hold it.

### Notes

- This release needs watch firmware 4.44.3 or later. On older firmware the watch lists Sketchbook Horizon as incompatible until it updates.
- The Pebble App: Preferred Source choice needs plugins turned on in the Pebble app under Settings, Plugins.
- Pebble app plugins, including my Weather Enhanced, work the same way as the weather sources on this settings page. If the Pebble app makes plugins the default, these sources may be phased out in a later release.
- The first time the watchface starts after this update, your phone sends your settings to the watch once more. Nothing needs doing.

## [1.0.0] - 2026-10-16

### Added

- First release of the Sketchbook Horizon watchface.

### Notes

Sketchbook Horizon brings the four separate Ridgeline, Shoreline, Skyline, and Treeline faces together into one. For the history of those faces, see their changelogs below.

- [Ridgeline](.retired/sketchbook/ridgeline/CHANGELOG.md)
- [Shoreline](.retired/sketchbook/shoreline/CHANGELOG.md)
- [Skyline](.retired/sketchbook/skyline/CHANGELOG.md)
- [Treeline](.retired/sketchbook/treeline/CHANGELOG.md)
