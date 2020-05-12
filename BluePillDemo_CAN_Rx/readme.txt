This example demonstrates the use of the CAN bus peripheral on the STM32F103C8 processor.
The example is part of a pair where one Blue Pill board acts as a transmitter and the
other as a receiver. You need 2 Boards, 2 CAN line drivers and 2 120R terminating
resistors.

The code sets up the CAN peripheral to work at 500 kbaud and transmits a packet  every 
100 ms with a single byte data value which toggles between 0 and 1 each message when the
transmitter boards button is pressed. The receiver reads this value and sets the on
board LED appropriately.

This project and the corresponding transmitter project use the HSI oscillator instead of
the external crystal. This is because the crystal has failed on one of my boards. The
speed is reduced slightly when using HSI. It should be able to set the oscillator source
back to the external crystal and everything work the same.

Schematic for this project includes the schematic for the corresponding transmitter 
project. 