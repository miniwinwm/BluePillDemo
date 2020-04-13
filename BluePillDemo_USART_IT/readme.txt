This example sets up USART1 as asynchronous serial at 115200 baud with interrupts enabled. 
The Rx complete and Tx complete interrupt handlers set flags when the interrupts are fired.
In the main loop a fixed text message is periodically sent stating nothing has been 
received when nothing has been received. If data have been received then this is echoed
instead.

To test this the easiest way is to obtain a FTDI TTL to RS232 USB cable (3.3 Volt version) and 
connect the cable's Tx, Rx and Ground lines to the Blue Pill. This is shown on the schematic 
provided. An image file shows the cable colours of the FTDI cable. Open a serial terminal
on the host at 115200 baud.