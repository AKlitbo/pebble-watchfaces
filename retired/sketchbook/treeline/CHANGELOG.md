# Changelog - Treeline

All notable changes to the Treeline watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.4.2] - 2026-10-16

### Notes

- Treeline is no longer being updated. Sketchbook Horizon replaces it, bringing this scene and three others into one watchface you can switch between in settings. This release adds a note to the settings page with a link to it, and nothing else changes.

## [1.4.1] - 2026-10-15

### Changed

- On a round Pebble, each palette picture in the Scene Palette picker is shown as a round preview that matches the screen.

### Fixed

- Fixed the palette pictures in the Scene Palette picker looking rough in the settings page.

## [1.4.0] - 2026-10-14

### Added

- Added a picture of each palette, in its day and its night colours, to the Scene Palette picker, so you can see the firs and the sky before you choose.
- Added a Palette Mode in settings. Keep one palette fixed as before, Shuffle through them all on a schedule from every 15 minutes up to once a day, or pick Day / Night / Quiet to paint one palette by day and a different one after dark or while Quiet Time is on.

### Changed

- Gave the settings page a hand-drawn paper look, with a night version that follows your phone's light or dark mode.

### Fixed

- Fixed the Quiet Time speaker icon showing clipped, with its sound waves cut off. Its slot was a little too narrow for the full icon.

## [1.3.3] - 2026-10-12

### Fixed

- Fixed Hourly Vibration skipping some hours with the clock set to .beats in a time zone a half hour or 45 minutes off the hour, such as India or Newfoundland.
- Fixed Hourly Vibration buzzing twice in most hours with a date format ending in .beats.
- Fixed Hourly Vibration buzzing twice when you return to the watchface in the first minute of the hour.
- Fixed a reset or reinstalled watch staying on the default settings until you pressed Save. Its weather also stopped refreshing when due.
- Fixed the settings page describing Fallback to Manual Location wrongly. It only applies when Enable Phone GPS is on but cannot find you.

## [1.3.2] - 2026-09-27

### Fixed

- Fixed the temperature showing the old number with the new unit, such as 23F for 23°C, after switching units while the phone was offline, when a refresh already under way finished just after, on the first save after installing, or after the Pebble app's data was cleared.
- Fixed your settings sometimes not coming back after an update, leaving the defaults until the Pebble app restarted. Going back to an older version could leave the defaults for good.
- Fixed your settings being replaced by the defaults after installing a new version.
- Fixed the settings page opening on the default for a choice such as the temperature unit, rather than what the watch was set to.
- Fixed the sky showing night in the middle of the day when sunset falls after midnight, as in a northern summer.
- Fixed the sky switching between day and night at the wrong hour when the weather location is in another time zone.
- Fixed a city typed on the settings page and saved without tapping one of the suggestions being lost. The settings page now asks you to pick a place from the list.
- Fixed the Tracks Readout showing -1 just after the watchface opened, or 0 or 0.0 with Health turned off. It shows dashes until there is a reading.
- Fixed the weather retrying when the key was wrong or out of calls, or no location was set, which used up more of the key's calls.
- Fixed the weather being fetched twice on every refresh, which used up a weather key's calls twice as fast.
- Fixed the temperature sometimes staying on dashes for up to half an hour after the watchface started.

### Notes

- This release clears the saved weather once. The temperature shows dashes until the first refresh after the update, and nothing needs doing to bring it back.

## [1.3.1] - 2026-09-18

### Changed

- The download is smaller. It no longer carries the phone code for stocks and calendars, which Treeline never shows.

### Fixed

- Fixed weather getting stuck until the Pebble app was restarted after an update failed to reach the watch.
- Fixed the watch starting a new weather fetch every time it asked again while one was already running.
- Fixed every settings save fetching fresh weather, even when nothing that affects the weather had changed.

## [1.3.0] - 2026-08-26

### Added

- Added two date formats ending in a Swatch Beats reading, so the date line can show internet time while the clock stays on normal time. The date shrinks as needed to fit the longer format.

## [1.2.0] - 2026-08-03

### Changed

- The watchface no longer tracks or stores health history that it never displays, eliminating unnecessary storage writes.
- The watchface no longer reads sleep, active minutes, or calories from the watch. It has never displayed them, and reading them required an unnecessary storage access once per minute.

### Fixed

- Fixed the watchface making the watch feel slow and unresponsive. It was building and storing a full history of heart rate and step activity that it never displays, and rebuilding it every minute delayed the watch's background work.
- Fixed the heart rate, step, and distance readings going stale while the watch was sitting still. They now refresh once per minute whether or not you have been moving.

## [1.1.0] - 2026-08-01

### Added

- Added support for the Round 2 (Gabbro).

### Changed

- The Quiet Time mark now shows whether Quiet Time is on or off, rather than appearing only while Quiet Time is active.

### Notes

On the Round 2:

- Round 2 support has been developed and tested in the emulator. I don't currently own a Round 2, so there may be hardware-specific issues that weren't possible to verify.
- Balanced is the only layout. The Layout setting is not shown.
- The bottom readouts are omitted, and the Tracks Readout setting is hidden.
- The temperature is shown on the cabin sign instead of below the date.
- The cabin sign is hidden when no weather reading is available.

## [1.0.1] - 2026-07-31

### Fixed

- Fixed the shadowed part of the moon showing as a blue patch rather than the night sky behind it.

## [1.0.0] - 2026-07-30

### Added

- First release of the Treeline watchface.
