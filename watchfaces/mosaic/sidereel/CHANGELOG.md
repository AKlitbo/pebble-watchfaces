# Changelog - Sidereel

All notable changes to the Sidereel watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.3] - 2026-09-27

### Fixed

- Fixed the settings page stopping after the Layout description, leaving out the layout editor and every setting below it.

## [1.2.2] - 2026-09-27

### Fixed

- Fixed the temperature showing the old number with the new unit, such as 23F for 23°C, after switching units while the phone was offline, when a refresh already under way finished just after, on the first save after installing, or after the Pebble app's data was cleared.
- Fixed your settings sometimes not coming back after an update, leaving the defaults until the Pebble app restarted. Going back to an older version could leave the defaults for good.
- Fixed Goal Met Vibration still buzzing after it was set back to None.
- Fixed the heart rate graph drawn just after installing being shifted back in time.
- Fixed rain chance and UV showing 0 where the weather service had no reading for them. They show a dash instead.
- Fixed the Pressure panel reading far too low with Open-Meteo, the default weather provider, the higher you live. It now shows sea level pressure like the other providers.
- Fixed the sun panels when sunset falls after midnight, as in a northern summer. Daylight read as night, and the countdown to sunrise showed hours that were already daylight.
- Fixed the Next Moon panel jumping to 29 days partway through the night of a new or full moon.
- Fixed the Alternate Time Zone list on the settings page opening again after you picked a zone, where a stray tap could change your pick.
- Fixed the UTC offset shown for a zone on the settings page sometimes reading a minute short. Typing an offset such as UTC+5 also showed a name that read as UTC-5.
- Fixed a second finger on the screen moving or dropping the panel you were dragging in the layout editor.
- Fixed a city typed on the settings page and saved without tapping one of the suggestions being lost. The settings page now asks you to pick a place from the list.
- Fixed the Steps Graph bars shifting by an hour on the days the clocks change, with the current hour's bar near empty or doubled. An hour the clocks went back through could also read empty after the face reopened.
- Fixed clearing the alternate time zone on the settings page leaving the old city on the watch. The panel now shows dashes whenever no zone is picked.
- Fixed the wind reading 0 km/h rather than a dash when OpenWeatherMap had no wind reading.
- Fixed the heart rate graph freezing for up to an hour after the watch's clock was set back. Coming back from a watchapp also left a gap for the time you were away, or a blank graph after an hour.
- Fixed the weather showing clear skies when Open-Meteo, the default weather provider, had no reading for the current conditions.
- Fixed the steps showing 0 with Health turned off. They show dashes until there is a reading.
- Fixed sunrise and sunset, and what follows them, showing hours off when the weather location is in another time zone. Today's high, low, and rain chance could also come from the day before or after.
- Fixed yesterday's high, low, UV, and rain chance staying on the watch as today's when part of the weather refresh kept failing.
- Fixed the weather retrying when the key was wrong or out of calls, or no location was set, which used up more of the key's calls.
- Fixed the weather being fetched twice on every refresh, which used up a weather key's calls twice as fast.
- Fixed the weather readouts sometimes staying on dashes for up to half an hour after the watchface started.

### Notes

- This release clears the saved weather once. The weather readouts show dashes until the first refresh after the update, and nothing needs doing to bring them back.

## [1.2.1] - 2026-09-22

### Changed

- Changed the wording in the Clay layout builder.
- A new install now starts with no Alternate Time Zone rather than London, reading UTC until you pick a city.
- The prompt to pick a city again now stands out as a boxed note rather than a loose line of text.

### Fixed

- Fixed the Day, Night and Quiet Time pickers in the Clay layout builder being squeezed thin. Each caption now sits above its picker.
- Fixed inconsistent wording and spacing through the Clay settings page, including notes that named panels Sidereel does not have.
- Fixed the Alternate Time Zone opening empty on the settings page while the watch still showed the old zone. The place name now comes back from the watch.
- Fixed a settings page with no city chosen dropping the Alternate Time Zone to UTC under a bare TZ heading.
- Fixed a city picked for the Alternate Time Zone saving as UTC when the page was saved straight away or with no connection.
- Fixed your settings being replaced by the defaults after installing a new version.

## [1.2.0] - 2026-09-20

### Added

- Added a Quiet Time layout that takes over whenever the watch is on Quiet Time, and wins over the night layout. Leave it on None and nothing changes.
- Added a fifth slot to the layout library, so there is one more grid to keep a design in.
- Added time zones to the Alternate Time Zone picker, so UTC, a zone name such as Europe/London, or an offset like UTC+05:30 can be picked as well as a city.

### Fixed

- Fixed the night layout never taking effect. The settings page offered a Night picker and a schedule, but the watchface had nowhere to keep either.
- Fixed the layouts you build being forgotten. Only the one on screen was kept, so the rest were gone the next time you opened the settings.

## [1.1.2] - 2026-09-18

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