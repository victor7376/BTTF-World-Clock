# BTTF-World-Clock
Delorean clock - Back to the future style clock - with 3 Timezones

This is a combination of the Marquee Scroller code & the Delorean Back to the Future style clock.

Red clock top gives one timezone (clock) with a date of your choice

Green clock middle gives your the actual timezone that you're from

Orange clock gives the last timezone (clock) with date of your choice

Thingiverse:
BTTF Style clock - makes 3 clocks work
https://www.thingiverse.com/thing:2980120

Main code from:
Marquee Scroller - makes the WiFi with correct time
https://www.thingiverse.com/thing:2867294

Timezone Library: (makes the two clocks - Red & Orange work)
https://github.com/JChristensen/Timezone

Everything works from a 5V 2A / 3A power supply hooked up to an ESP32-DevKitC V4
using 2 WAGO clips:

Amazon (I do not get anything from this link, its mostly info for you):
ESP32 - set of three
https://www.amazon.co.uk/gp/product/B08DXSMZSB/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1

WAGO clips
https://www.amazon.co.uk/gp/product/B07Q4WZ1J9/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1


==================================

Features:

* Middle row YOUR timezone
* Top row (RED) Australia Timezone (Can be changed by looking onn the Timezone Library)
* Bottom row (Orange) Los Angeles Timezone (Can be changes in the settings like Red Zone)
* Shows AM & PM For all timezones
* Turn on and off at certain times (Power saver) displays turns off at 22:30 to 09:30 can be changed (line 328)

If you have any issues with the displays, it can be useful to remove the resisters on the back of the displays.
Just Google 'Sevensegment display remove capacitors'.