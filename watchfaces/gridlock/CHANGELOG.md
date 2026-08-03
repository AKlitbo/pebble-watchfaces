# Changelog - Gridlock

All notable changes to the Gridlock watchface are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

> [!IMPORTANT]
> This release changes how heart rate history is stored. The saved graph is cleared once when you update, then begins filling again immediately. It will be back to a full hour within an hour.

### Changed

- Reduced what the watchface writes to storage for the heart rate graph. Everything else it was saving is already available from the watch's own health data at startup, so keeping a second copy served no purpose.

### Fixed

- Fixed the watchface making the watch feel slow and unresponsive. The health panels were rebuilding the entire day's step history from storage every minute, delaying the watch's background work.
- Fixed activity readings being re-read every few seconds while walking instead of once per minute. Each read was also written back to storage, creating unnecessary work while the watch was in active use.
- Fixed the heart rate graph being written to storage on every sensor reading. It is now written once per minute, matching how often the graph gains a new point.
- Fixed the watchface taking a long time to appear, both on first load and when returning from an app or the menu. It was rebuilding the entire day's step history before drawing anything, leaving the previous screen frozen until it finished.
- Fixed the Steps Graph re-reading the current hour from storage every minute. It now works the current hour out from the daily step total, so storage is only read when each hour ends.

## [1.0.0] - 2026-07-31

### Added

- Added a library of four layouts, all editable from the same grid. Changes are kept automatically as you switch between them.
- Added Day and Night layout assignments, allowing any two layouts to be shown automatically.
- Added automatic Day/Night layout switching using either sunrise and sunset from your weather provider or manually configured times.

### Changed

- Replaced the experimental Saved Layout slots with the layout library.

### Fixed

- Fixed clearing a layout and saving leaving the previous one on the watch.

## [0.14.1] - 2026-07-28

### Fixed

- Fixed the layout builder on the settings page not matching the appearance editor, leaving part of the page in a lighter style than the rest.
- Fixed the Saved Layouts and Import / Export Layout popups on the settings page appearing as unstyled white boxes.
- Fixed the location search suggestion list on the settings page appearing as an unstyled white list.

## [0.14.0] - 2026-07-17

### Added

- Added two personal layout slots for saving and restoring layouts.
- Added a Goal Met Vibe for steps, calories, distance, and active minutes, with Short, Long, Double, five celebration rhythms, and a Custom pattern you can type in yourself.

### Changed

- Calendar feeds are now parsed with ical.js for broader iCalendar (.ics) compatibility, including recurring events, time zones, and exceptions for cancelled or rescheduled events.

### Fixed

- Fixed settings changes sometimes not reaching the watch when the configuration became too large.
- Fixed Alternate Time Zone resetting for cities with accented names.
- Fixed the Week Number panel calculating the wrong number of weeks in some years.
- Fixed the Watchlist occasionally going blank after a quote request timed out.
- Fixed newly added Stock and Watchlist panels sometimes remaining empty until the next scheduled refresh.
- Fixed stock providers occasionally exceeding their daily request limits.
- Fixed weather readings and the Show Connection Icon setting occasionally being interpreted incorrectly.
- Fixed weather sometimes remaining blank after the watchface started, including when using Fallback to Manual Location.
- Fixed icons occasionally disappearing after the watchface had been running for an extended period.
- Fixed a crash when saving settings with very little free memory.
- Fixed the 2x2 Steps panel showing the wrong caption when displaying distance.
- Fixed the UV panel clipping its risk label.
- Fixed layouts being offset when they contained an invalid panel size.
- Fixed the longitude occasionally being cleared during location updates.
- Fixed calendar events with distant end dates being handled incorrectly.
- Fixed cached weather, stock, health, and location data being reused after cache format changes.
- Fixed the 2x4 Forecast and Hourly Forecast panels misaligning their columns when fewer readings were available.
- Fixed panel headers permanently losing their patterned fill when memory was low.

### Notes

- This release changes how weather settings are stored. If weather appears stuck after updating, switch to a different weather provider, save, then switch back to your preferred provider and save again. This refreshes the stored weather configuration.
- Saved Layouts are an experimental feature. They should survive future updates, but if they prove unreliable, they may be removed in a later release.

## [0.13.0] - 2026-07-14

### Added

- Added a 2x4 Month Grid panel showing the current month with today highlighted.
- Added 1x2 Week Number and Weeks Left panels.
- Added 1x2 Day of Year and Days Left panels.
- Added a 1x2 Epoch panel showing the current Unix timestamp.
- Added a 1x2 Weekday Dots panel showing the current day of the week.
- Added a 1x2 Next Moon panel counting down to the next full or new moon.
- Added a 1x2 Julian Date panel.
- Added a 1x2 Status panel combining battery, Bluetooth, and Quiet Time.
- Added a Week Starts On setting with Sunday and Monday options for calendar panels.
- Added 2x2 Big Hour and Big Minutes panels with oversized time readouts for improved readability.
- Added a 2x4 Big Time panel showing the current time as a large HH:MM display.

### Changed

- Moved the Analog Clock Style setting to the Clock section.

### Fixed

- Fixed the Alternate Time Zone setting so choosing a city now applies its correct offset. Previously the Alternate Time module ignored the chosen city and fell back to UTC.

## [0.12.1] - 2026-07-11

### Fixed

- Fixed weather sometimes remaining blank when using Fallback to Manual Location.
- Improved weather location handling by reusing recent GPS fixes and retrying failed location requests automatically.

## [0.12.0] - 2026-07-11

### Added

- Added Roman and Grid Analog Face styles, bringing the Analog Clock to nine dial styles.

### Changed

- Redrew every Analog Face style with cleaner, more even markers and tidier corners, and the dials now fill more of the panel.

### Removed

- The Segmented Bezel Analog Face no longer lights the current hour's cell.

### Fixed

- Fixed the 2x2 Timeline drawing its event time smaller than the matching Next Event panel, so the two now use the same size.

## [0.11.0] - 2026-07-10

### Added

- Added an Hourly Vibration option with None, Short, Long, and Double patterns. It remains silent during Quiet Time.
- Added Calendar support using iCal (.ics) feeds from Google, Apple iCloud, Outlook, and other calendar providers, with configurable refresh intervals.
- Added Next Event panels in 2x2 and 1x4 sizes, showing your next event's time, title, and location.
- Added an Agenda panel in 2x4, listing your upcoming events with two-letter day labels.
- Added Timeline panels in 1x4, 2x4, and 2x2 sizes, showing your schedule for the next few hours.
- Added Calendar event reminders with 15-minute, 5-minute, and on-start alerts. Reminders respect Quiet Time and are enabled when a Calendar panel is present.

### Fixed

- Fixed weather, stock, and calendar data sometimes remaining stale after reconnecting to the phone.
- Fixed hourly and daily forecast panels sometimes loading blank when using OpenWeatherMap or WeatherAPI.
- Fixed weather and stock data sometimes remaining stale until a Bluetooth reconnect.
- Fixed the 2x2 Moon panel clipping the end of its phase name.
- Fixed the 2x2 UV Index gauge not being centred.

## [0.10.0] - 2026-07-09

### Added

- Added a Header Font option with eight font choices.
- - Fonts: Share Tech Mono, LECO, Press Start 2P, Pixelify Sans, Aldrich, Kode Mono, Electrolize, or Quantico
- Added a 2x2 Steps Graph panel showing hourly step counts, the running daily total, and your step goal.
- Added a 2x2 UV Index panel showing the current UV Index on a low to high gauge.
- Added seven Analog Face styles for all Analog Clock panels.
- Added Yahoo as a stock data source with real-time quotes and broad market coverage. It is an unofficial feed that can change or be taken offline without notice.
- Added Twelve Data as a stock data source for global markets.

### Changed

- The 2x2 Heart Rate Graph now uses a fixed 40 to 120 bpm scale that expands automatically when needed.

### Removed

- Removed the panel tap menu from the layout builder. Panels are now added, moved, swapped, and removed entirely by dragging.

### Fixed

- Weather updates now write to storage once per refresh instead of multiple times.
- Fixed stock panels occasionally freezing after a failed quote update.
- Fixed stock panels not always refreshing immediately after changing tickers or data sources.
- Fixed imported layouts sometimes placing full-width panels outside the grid.

## [0.9.2] - 2026-07-08

### Fixed

- Fixed the Moon panel's Vibrant colour not matching in the appearance editor, which still previewed the old blue instead of the purple.

## [0.9.1] - 2026-07-08

### Fixed

- Fixed the Moon panel using a blue accent colour in the Vibrant theme instead of the intended purple.
- Fixed the 2x2 Heart Rate Graph staying empty or showing only a few stray dots. It now reads the latest heart rate once a minute and builds a continuous graph over the last hour instead of relying on the watch's occasional background readings. This only reads values the watch has already measured, so it does not turn the sensor on more often or use extra battery.
- Fixed the 2x2 Digital Clock showing a placeholder icon instead of its panel preview in the layout builder.

## [0.9.0] - 2026-07-08

### Added

- Added Feels Like, Pressure, and Dew Point weather panels in 1x2.
- Added a Moon panel showing the current moon phase in 1x2 and 2x2 sizes.
- Added a 2x2 Heartrate Graph panel charting recent heart rate readings.
- Added a Bluetooth panel showing the Bluetooth connection status.
- Added a Quiet Time panel showing when Quiet Time status.
- Added a 2x2 digital clock panel variant.

### Changed

- Reduced how much work the watchface does on each update. Only the panels affected by a change are redrawn now instead of the whole screen every time, which is a little easier on the battery.

### Fixed

- Fixed stock quotes being fetched twice when the watchface starts, which could reach the Alpha Vantage daily request limit sooner than expected.
- Fixed the stock panels staying on an error for up to an hour after a failed Alpha Vantage update, rather than retrying on the next update.
- Fixed a Finnhub error sometimes showing as a network error instead of the correct Invalid Key or Rate Limit status.
- Fixed a Finnhub stock quote occasionally showing the next day's date after the US market close.
- Fixed the day of year date format option previewing the wrong separator in the settings.

## [0.8.0] - 2026-07-07

### Added

- Added Stock (2x2) and Watchlist (2x4) panels for tracking one or more stock tickers.
- Added stock data support via Finnhub (live) and Alpha Vantage (end of day).

### Changed

- Renamed several modules for clarity, including Sun to Day/Night Tracker, UV to UV Index, Hi/Low to Temperature Hi/Lo, and Time Zone 1 to Alternate Time.
- Refreshed the settings UI with the Gridlock visual style, including the layout builder and Edit Module Appearance.
- Edit Module Appearance now keeps its legend and bulk actions visible while scrolling.
- Refreshed the Weather, Sleep, and Creator layout presets.

### Fixed

- Fixed the missing Time Zone icon in the layout builder.

## [0.7.1] - 2026-07-07

### Fixed

- Fixed the 2x2 Day/Night Tracker panel so it correctly shows the moon overnight and displays the sunrise and sunset times in the correct order.
- Fixed the missing Time Zone 1 icon in the Clay layout builder.

## [0.7.0] - 2026-07-06

### Added

- Added a Day/Night Tracker panel with countdown and sun progress views.
- Added day and night weather icons to the Hourly Forecast panel.
- Added a 2x4 Hourly Forecast panel.
- Added a 2x4 Daily Forecast panel.

### Fixed

- Fixed the MI, KM, °C, and °F labels to use the smaller label font for a more consistent appearance.

## [0.6.0] - 2026-07-05

### Added

- Added a 1x4 Hourly Forecast panel.
- Added a 1x4 Daily Forecast panel.
- Added bulk Border and Header toggles to Edit Module Appearance.

## [0.5.2] - 2026-07-04

### Changed

- Moved the per-module Vibrant option into the colour picker, allowing the header, value, icon, and caption colours to be applied individually.

### Fixed

- Fixed an issue where some layouts could lose a panel when applied. Existing saved layouts are reset once with this update.

## [0.5.1] - 2026-07-04

### Fixed

- Fixed an issue where Custom theme colours and borders could be lost after customising many modules. Existing saved Custom theme colours are reset once with this update.

## [0.5.0] - 2026-07-03

### Added

- Added standalone Distance panels in 1x2 and 2x2 sizes.
- Added a per-module Vibrant option in Edit Module Appearance.
- Added a per-module Border toggle in Edit Module Appearance.
- Added a live preview to the colour picker in Edit Module Appearance.

### Changed

- Weather now retains the last successful reading until fresh data is available.
- Edit Module Appearance now previews each module instead of using generic icons.
- Steps and Distance panels now have independent distance unit settings.

## [0.4.0] - 2026-07-01

### Added

- Added UV Index, Temperature Hi/Lo, and Precipitation weather panels.
- Added 12, 3, 6, and 9 markers to the Analog Clock.

### Fixed

- Fixed panel icon alignment for a more consistent appearance.

## [0.3.0] - 2026-06-30

### Added

- Added a Mono (Black on White) theme.
- Added a fully customisable Custom theme.
- Added live module previews to the layout builder.
- Added an Analog Clock panel.
- Added a per-module Header toggle in Edit Module Appearance.

### Changed

- Renamed the Mono theme to Mono (White on Black).
- Refreshed the layout builder layout and controls.
- Time format options now display example times.

### Fixed

- Fixed panel header colours to remain readable across all themes.
- Fixed the Steps Readout setting when displaying distance.
- Fixed the default time and date settings on a fresh install.
- Fixed importing layouts containing a full-width panel.
- Fixed weather icons remaining visible after a failed update.
- Fixed unnecessary weather icon reloads after transient loading failures.

### Known Issues

- Some weather condition icons remain slightly misaligned and will be refined in a future release.

## [0.2.1] - 2026-06-29

### Fixed

- Disabled development mode, which was accidentally enabled in the previous release.
- Fixed manual weather locations not loading correctly.

## [0.2.0] - 2026-06-29

### Added

- Added an Alternate Time panel.
- Added layout import and export to the layout builder.
- Added a Vibrant colour theme.
- Added Sleep, Creator, and Default layout presets.
- Added a 12-hour time format option without a leading zero.

### Changed

- Changed the temperature unit setting to a Celsius/Fahrenheit dropdown.
- Reorganised the settings page.
- Reworked the layout builder with grouped size tabs and a more compact layout.
- Renamed the Fit layout preset to Activity.

### Removed

- Removed the previous accent colour themes.
- Removed the Show Connection Icon setting. The Bluetooth icon is now always shown.

### Fixed

- Fixed Bluetooth connection and disconnection vibration alerts.
- Fixed Heart Rate showing 0 before a reading is available.
- Fixed sunrise, sunset, and daylight times to respect the selected time format.
- Fixed sunrise and sunset times being truncated.
- Fixed corrupted weather details.
- Fixed weather remaining visible after a failed update.
- Fixed icons disappearing after many different icons had been displayed.

### Known Issues

- Some icons may appear slightly misaligned due to the grid engine refactor.

## [0.1.2] - 2026-06-28

### Fixed

- Fixed the 1x2 Weather panel displaying the Temperature panel instead.

## [0.1.1] - 2026-06-27

### Fixed

- Fixed extended weather readings not being compiled correctly when using OpenWeatherMap.
