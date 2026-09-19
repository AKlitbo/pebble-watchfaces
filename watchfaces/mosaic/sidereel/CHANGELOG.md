# Changelog - Sidereel

All notable changes to the Sidereel watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.2] - Unreleased

### Changed

- The Alternate Time Zone now follows daylight saving, so a city picked in winter keeps the right time through the summer. A city picked before this version has no saved time zone, so the settings page asks you to pick it again.
- Weather from OpenWeatherMap arrives sooner, since its extra readings are now fetched at the same time rather than after it. The extra readings still arrive when the OpenWeatherMap key is rejected.
- The download is smaller. It no longer carries the phone code for stocks and calendars, which Sidereel never shows.

### Fixed

- Fixed weather getting stuck until the Pebble app was restarted after an update failed to reach the watch.
- Fixed the watch starting a new weather fetch every time it asked again while one was already running.
- Fixed every settings save fetching fresh weather, even when nothing that affects the weather had changed.
- Fixed drizzle showing the N/A icon with OpenWeatherMap.
- Fixed a weather panel added while the watchface was running getting no data until it was restarted.

## [1.1.1] - 2026-09-07

### Fixed

- Fixed the bell on the Next Alarm panel turning grey when no alarm was scheduled. It now keeps the panel's colour in every theme, matching the time shown beside it.

## [1.1.0] - 2026-09-06

### Added

- Added a Next Alarm panel at both panel sizes. It shows when your next alarm goes off, and the larger size also counts down how long you have left. The panel reads the alarm straight off the watch, so nothing needs setting up.

## [1.0.1] - 2026-08-09

### Fixed

- Fixed preset buttons showing wrong labels and stale module data in the layout builder.

## [1.0.0] - 2026-08-09

### Added

- First release of the Sidereel watchface.