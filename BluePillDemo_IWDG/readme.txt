This example sets up the indpendent watchdog with a maximum refresh period of about 4
seconds. A push button input is also enabled. After boot the Blue Pill board's LED is
set off. On pushing the push button the LED is set on. Keep on pressing the push
button at least every 4 seconds otherwise the watchdog kicks in, the processor is 
reset, and the LED is set off.

Schematic available.