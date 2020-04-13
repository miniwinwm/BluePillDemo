This example sets up USART1 as asynchronous serial at 115200 baud. Then using polling the code
reads a byte at a time if available from the USART and echoes it back out.

To test this the easiest way is to obtain a FTDI TTL to RS232 USB cable (3.3 Volt version) and 
connect the cable's Tx, Rx and Ground lines to the Blue Pill. This is shown on the schematic 
provided. An image file shows the cable colours of the FTDI cable. Open a serial terminal
on the host at 115200 baud.