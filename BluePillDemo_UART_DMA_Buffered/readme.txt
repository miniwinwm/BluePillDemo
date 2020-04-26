This example sets up USART1 as asynchronous serial at 115200 baud with DMA enabled. It
also provides an API to use the UART in a buffered manner. Data can be written to send
out of the serial port while the former data is still being sent. It is queued up (if
there is space in the buffer) until it can be sent. Reading is done by DMA and is also
queued in a buffer and can be read at any time later.
 
In the main loop every second the receive buffer is read and any received data is echoed
by sending it back using the transmit API.

Before using this API the init function must be called. The driver is separated out into
a header and a source file. These are hard coded to use UART1 but can be easily changed.
The size of the buffers can be set in the header file.

This driver needs the function HAL_UART_IRQHandler_2 in stm32f1xx_it.c adding to the 
generated code.

To test this the easiest way is to obtain a FTDI TTL to RS232 USB cable (3.3 Volt version) and 
connect the cable's Tx, Rx and Ground lines to the Blue Pill. This is shown on the schematic 
provided. An image file shows the cable colours of the FTDI cable. Open a serial terminal
on the host at 115200 baud.