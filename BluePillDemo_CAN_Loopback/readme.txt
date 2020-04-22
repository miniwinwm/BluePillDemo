This example demonstrates the use of the CAN bus peripheral on the STM32F103C8 processor.
The example uses loopback mode where the transmitted packets are received by the same
CAN bus module so no external transceiver is required.

The code sets up the CAN peripheral to work at 500 kbaud and transmits a packet  every 
200 ms with a single byte data value which toggles between 0 and 1 each message. This 
packet is read and a handler called which uses the data value to switch the Blue Pill's 
on-board LED on or off.

To get this example to work with other devices on a CAN bus the mode of the CAN 
peripheral needs changing from Loopback to Normal and an external CAN transceiver needs
adding.

No schematic for this example. 