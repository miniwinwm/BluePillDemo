This example uses the Blue Pill board to drive a SIM800L GSM modem on a development board
using its AT command interface. It is intended to be the next simplest example after
the SMS project. This example esatblishes a GPRS data connection and using the modem's
HTTP AT commands sends an HTTP GET to a test server and reads the response which is then
sent out on a serial port.

GSM modems and their interface are not simple things and as such this example is not
simple either. The reason that interfacing to a modem is not simple is...

1) The modem's AT interface is huge covering many hundreds of pages of documentation. 
This example uses very little of the available AT commands.

2) Communication with modems is asynchronous. Sending your data to the modem via an AT
command is fine, but you don't know what the modem is going to reply with and when. It 
usually sends its reply in response to your starting a command sequence with an AT command,
but not always. Sometimes the modem sends a response without an AT command. These are
called Unsolicited Response Codes (URC). This example switches off some URC's and uses a 
simple polling method. Not all URC's are switched off (for example an incoming call)
and are not handled. If this happens this code will get lost and won't work any more.

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

There is a simple API to use the modem in file modem.h for configuring and starting
the modem's HTTP service, setting a URL, initiating a GET, POST or HEAD command and
then downloading the response.

The code is quick and dirty example code. It uses unsafe C library string handling 
functions, is inefficient and doesn't have much error checking, but it works and is a
start.

The example code in main establishes a GPRS connection and initialses the HTTP service
on the modem. It then sends a HTTP GET command and checks the response code of any response.
It also reads the length of the response and uses that length to download the response 
from the modem into a dynamically allocated buffer. The contents of the response are
sent out of USART2 and then the HTTP connection is closed. You need to set you 
Access point Name, user name (if any) and password (if any) for your SIM at the top of
main.c.

Schematic provided. Check the voltage output from the regulator before connecting
it to the modem!

