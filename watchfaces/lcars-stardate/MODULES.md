# LCARS Stardate Readouts

Every readout the four ops slots can show, at each size it supports.

The face has four pickable slots, two per column, under a fixed clock and stardate banner. A slot
carries no fixed reading. What it shows comes from the catalogue below, and its bar word and glyph
follow the pick, so changing a slot needs no new artwork for any theme.

For the themes themselves, see the previews in the [README](../../README.md).

## Arrangements

Six ways the same four slots read. Each is a picked set rather than a mode, so any readout can go
in any slot and you can mix them however you like.

<img src="../../.github/images/lcars-stardate/ops_body.png" width="105" title="Body"> <img src="../../.github/images/lcars-stardate/ops_weather.png" width="105" title="Weather"> <img src="../../.github/images/lcars-stardate/ops_sun.png" width="105" title="Sun"> <img src="../../.github/images/lcars-stardate/ops_moon.png" width="105" title="Moon"> <img src="../../.github/images/lcars-stardate/ops_calendar.png" width="105" title="Calendar"> <img src="../../.github/images/lcars-stardate/ops_alt-time.png" width="105" title="Alternate Time">

## Slot

The ordinary size, and what all four slots take.

| | | | | | | |
|:--:|:--:|:--:|:--:|:--:|:--:|:--:|
| **Heart Rate**<br>![](resources/thumbnails/heart-slot.png) | **Steps / Distance**<br>![](resources/thumbnails/steps-slot.png) | **Battery**<br>![](resources/thumbnails/battery-slot.png) | **Calories**<br>![](resources/thumbnails/calories-slot.png) | **Sleep**<br>![](resources/thumbnails/sleep-slot.png) | **Active Minutes**<br>![](resources/thumbnails/active-slot.png) | **Moon Phase %**<br>![](resources/thumbnails/moon-pct-slot.png) |
| **Moon Phase Name**<br>![](resources/thumbnails/moon-phase-slot.png) | **Next Full / New Moon**<br>![](resources/thumbnails/moon-next-slot.png) | **Sunrise**<br>![](resources/thumbnails/sunrise-slot.png) | **Sunset**<br>![](resources/thumbnails/sunset-slot.png) | **Length of Day**<br>![](resources/thumbnails/daylight-slot.png) | **Next Sun Event**<br>![](resources/thumbnails/sun-next-slot.png) | **Humidity**<br>![](resources/thumbnails/humidity-slot.png) |
| **Wind**<br>![](resources/thumbnails/wind-slot.png) | **UV Index**<br>![](resources/thumbnails/uv-slot.png) | **High / Low**<br>![](resources/thumbnails/hilo-slot.png) | **Julian Date**<br>![](resources/thumbnails/julian-slot.png) | **Day of Year**<br>![](resources/thumbnails/day-of-year-slot.png) | **Week Number**<br>![](resources/thumbnails/week-slot.png) | **Temperature**<br>![](resources/thumbnails/temp-slot.png) |
| **Conditions**<br>![](resources/thumbnails/conditions-slot.png) | **Epoch Clock**<br>![](resources/thumbnails/epoch-slot.png) | **Swatch Beats**<br>![](resources/thumbnails/beats-slot.png) | **Alternate Time Zone**<br>![](resources/thumbnails/zone1-slot.png) | | | |

Epoch takes no glyph on purpose. Ten digits only fit once the row hands its icon space back to the
value, which any readout with no glyph gets. The alternate zone names its own bar from the city you
search for, so a slot set to London reads LONDON.

## Tall

One readout fills a whole column instead of a slot: the condition glyph over a large temperature,
at a size the ordinary rows cannot give it.

| |
|:--:|
| **Sensors Block**<br>![](resources/thumbnails/sensors-tall.png) |

It only goes in the upper left, which is the one column the face draws it in, and it takes the lower
left slot with it. The builder will not let you drop it anywhere else, and the firmware makes the
same correction, so a hand-edited setting cannot smuggle one into the right column.
