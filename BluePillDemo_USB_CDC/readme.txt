This example demonstrates using the Blue Pill board as a USB CDC device. When connected to a host
a virtual serial port appears in the host. The code here does 2 things:

1) Periodically sends out a fixed text message
2) Echos any data received from the host back to the host.

To run this example connect a USB lead from the Blue Pill to a host. Open a serial terminal on 
the host using the cirtual serial port that has appeared. The baud rate does not need to be set.
Start typing in the serial terminal on the host to see the characters echoed.

No schematic file for this example.