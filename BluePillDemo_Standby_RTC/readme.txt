This example puts the STM32F103C8 processor into the deepest sleep mode - stand by. In this
mode all memory contents are lost. The processor is set to wake up on a RTC alarm.

The example code shows the Blue Pill board's LED for 5 seconds then goes into standby mode 
for a further 5 seconds before the RTC alarm goes off and the processor is woken and the
cycle repeats.

If debugging when the processor goes into standy mode debugging fails. When the processor
is in standby mode it cannot be reflashed. To reflash once this code is on wake up the 
processor with the reset button and start flashing or debugging during the 5 seconds the
processor is awake.

The VBatt pin is connected to 3.3V to keep the RTC alive during standby.

Schematic provided.