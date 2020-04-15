This example sets up the STM32F103C8's on-chip real time clock to use the Blue Pill's on-board 32.768 kHz
oscillator. The time is set, there is a 5 second delay, and the time is read. This can be verified in the 
debugger.

Hardware note: if the Blue Pill board is plugged into a breadboard the extra capacitance on the lines PC14
and PC15 which are connected to the RTC external 32.768 oscillator will mess things up and the RTC may
not run at the correct speed. To solve this do one of the following:

1) Remove the Blue Pill board from the breadboard or remove the header pins on PC13 and PC14
2) Change the clock souce in the Clock Configuration diagram from LSE to LSI.

No schematic file for this example. 