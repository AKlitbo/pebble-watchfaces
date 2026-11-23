# Changelog - Sketchbook Horizon

All notable changes to the Sketchbook Horizon watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Added a Mount Fuji scene, with the mountain over a lake. On the Pebble Round 2 the temperature rides on a boat's sail.
- Added a Giza scene, with the three pyramids over the desert. On the Pebble Round 2 the temperature sits in a cartouche on a stone stele.

### Changed

- Changed the Forest palette's daytime outlines from dark green to black on the Ridgeline and Shoreline scenes. They now stand out against the pale sky, as they do on the other scenes.
- Changed the battery gauge, the Bluetooth and Quiet Time icons, and the date when it sits in the status bar to light grey under the Mono palette by day on the Ridgeline scene. They now match the other scenes.
- Changed the trail sign's post on the Ridgeline scene to the colour of the other marks on the ground on the Pebble Round 2.
- Changed the wake around the buoy on the Skyline scene to the colour of the ripples on the river on the Pebble Round 2.
- Changed the heart rate on the Pebble Time 2 to update at most every ten seconds during a workout, which saves battery. A new reading after a quiet spell still shows straight away.

### Fixed

- Fixed the stats row's icons and numbers coming apart when the heart rate came or went, and the temperature and steps staying in the middle of the row after the heart rate came back on the Ridgeline, Shoreline, Skyline, and Treeline scenes.
- Fixed the battery gauge following the Scene Palette rather than the palette on screen. When Shuffle, the Night Palette, or the Quiet Time Palette landed on Mono the gauge still turned red or amber, and a Mono Scene Palette kept it grey under a colour palette.

## [1.1.2] - 2026-11-20

### Changed

- Changed the list of supported plugins on the settings page to say "this face" rather than "this watchface".
- Changed the settings page to carry only the palette pictures for your own watch, which makes it over a third smaller.

### Notes

- This release lists Pebble Health among the watchface's permissions, with the reason "To show your Pebble Health readings". If the Pebble app is set to ask before older apps use permissions, it asks you once after the update to allow health.

## [1.1.1] - 2026-11-14

### Fixed

- Fixed installing or updating the watchface asking for every permission at once, which held the weather until you answered. Each permission now says what it is for, and is asked for when it is first needed.
- Fixed the settings page showing Location Settings above Weather. It now sits straight under Weather, so picking a weather plugin hides it right below.

## [1.1.0] - 2026-11-13

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

- [Ridgeline](../../.retired/sketchbook/ridgeline/CHANGELOG.md)
- [Shoreline](../../.retired/sketchbook/shoreline/CHANGELOG.md)
- [Skyline](../../.retired/sketchbook/skyline/CHANGELOG.md)
- [Treeline](../../.retired/sketchbook/treeline/CHANGELOG.md)
