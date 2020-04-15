This example puts the STM32F103C8 processor into the deepest sleep mode - stand by. In this
mode all memory contents are lost. The processor is set to wake up on a rising edge on the
WKUP pin (A0). 

The example code shows the Blue Pill board's LED for 5 seconds then goes into standby mode.
Pressing the button wakes up the processor and the cycle repeats.

If debugging when the orcoessor goes into standy mode debugging fails. When the processor
is in standby mode it cannot be reflashed. To reflash once this code is on wake up the 
processor with the push button and start flashing or debugging during the 5 seconds the
processor is awake.

Schematic provided.