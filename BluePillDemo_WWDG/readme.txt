This example shows the Window Watchdog. This watchdog must be refreshed not too early and not too
late but within a specified period - the window.

The way this code runs is the Blue Pill board's LED and an external button are used. The LED is 
on when in the refresh window and off when not. Pressing a button refreshes the watchdog and the
led is turned off. If the button is not pressed in time or is pressed too early the processor 
resets. A test at startup detects a watchdog reset and flashes the LED quickly.

Normally the window watchdog's refresh period is pretty quick, a few microseconds to milliseconds.
This would be too quick for button pressing so to get the window refresh period expanded the
processor clock and peripheral clock that drives watchdog timer are slowed right down. Do not
use the clock settings in the project as an example for a normal project.

This code needs to read the counter value of the window watchdog to control the LED. This is not 
normal code - a normal application using the window watchdog is unlikely to need to do this and 
therefore there is no API in HAL to do this. Instead a direct register read is used.

The watchdog timer is a bit unintuitive. This is a timeline as it counts down...

Reload value     Window value         Reset value    
0x7f ----------> 0x5f ==============> 0x40
^                            |
|________refresh_____________v

The window when the watchdog can be reset is shown by the =========> arrow. The counter never
goes below 0x40.

Schematic provided.