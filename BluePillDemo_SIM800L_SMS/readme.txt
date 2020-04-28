This example uses the Blue Pill board to drive a SIM800L GSM modem on a development board
using its AT command interface. It is intended to be the simplest possible example of a 
working example as all it does is provide an API to send and receive text messages. 

GSM modems and their interface are not simple things and as such this example is not
simple either. The reason that interfacing to a modem is not simple is...

1) The modem's AT interface is huge covering many hundreds of pages of documentation. 
This example uses very little of the available AT commands.

2) Communication with modems is asynchronous. Sending your data to the modem via an AT
command is fine, but you don't know what the modem is going to reply with and when. It 
usually sends its reply in response to your starting a command sequence with an AT command,
but not always. Sometimes the modem sends a response without an AT command. These are
called Unsolicited Response Codes (URC). This example switches off URC's and uses a simple
polling method.

A more complex schematic is provided for this example than other examples. This is because
the modem has a particular power requirement. It needs a 4.2V supply which can provide up
to 2A momentarily. A power supply schematic has been provided using a LM217. All parts
are available for about £5.

The modem is connected to the Blue Pill board's STM32F103C8 via USART1 running at 115200
baud. In addition USART2 is connected to a FTDI 3.3V TTL to USB serial cable to send
output to a computer, also 115200 baud.

The USART driver in the HAL library is not particularly useful for this example. The
responses coming back from the modem are of unknown length and arrive at unknown time.
The blocking USART reads in the HAL library are no good as you don't know when the
data will arrive. The interrupt and DMA USART reads are also no good as you only get
a notification when a specified number of bytes have arrived. To overcome this a proper
buffered USART driver that uses DMA has been used, taken from the 
BluePillDemo_USART_DMA_Buffered project. The buffer size is adjustable in the header file.

There is a simple API to use the modem in file modem.h for sending and receiving SMS
messages. The receive reads all the waiting messages at once and provides them to the 
caller by a callback which are then written out on the second serial port. If too many
messages are retrieved at once the USART driver buffer will overflow and you'll get
corruption - it's only a simple example to build on! To do it properly it would be
necessary to handle the new SMS message URC but that's not done here.

The code is quick and dirty example code. It uses unsafe C library string handling 
functions, is inefficient and doesn't have much error checking, but it works and is a
start.

The example code in main sends a text message. You need to adjust the phone number.
After a few other AT commands to set things up and send a message the code goes into
a loop periodically scanning for and reading newly arrived SMS messages which are then
deleted from the modem's memory.

Schematic provided. Check the voltage output from the regulator before connecting
it to the modem!

