This example uses the Blue Pill board to drive a SIM800L GSM modem on a development board
using its AT command interface. The example provides an interface to allow a TCP client
socket to be opened and used to communicate with a TCP server.

GSM modems and their interface are not simple things and as such this example is not
simple either. The reason that interfacing to a modem is not simple is...

1) The modem's AT interface is huge covering many hundreds of pages of documentation. 
This example uses very little of the available AT commands.

2) Communication with modems is asynchronous. Sending your data to the modem via an AT
command is fine, but you don't know what the modem is going to reply with and when. It 
usually sends its reply in response to your starting a command sequence with an AT command,
but not always. Sometimes the modem sends a response without an AT command. These are
called Unsolicited Response Codes (URC) and need special care handling. 

A more complex schematic is provided for this example than other examples. This is because
the modem has a particular power requirement. It needs a 4.2V supply which can provide up
to 2A momentarily. A power supply schematic has been provided using a LM217. All parts
are available for about £5. Note that the schematic for this example is different from 
those of the non-FreeRTOS SIM800L examples. This is because the modem's RESET line is 
used at start up to get the modem into a known state.

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

The modem driving code in this example goes beyond the simple polling method used in 
the non-FreeRTOS SIM800L example projects. In this example a modem driver has 
been created. The modem driver runs in a FreeRTOS task as a server and waits on a 
command queue for incoming requests from client tasks. The server performs the AT command 
sequence and then sends the response back to the client on a response queue. The client
will be waiting on the response queue for the response, or a timeout, as all service
requests are given a timeout value. This is because modem responses are sometimes 
slow or fail. A response code is returned via the client API to the caller.

The modem server has a framework to handle incoming URC's. When a URC is received the
server parses it and stores the data from the URC in the server. There is a client
API to read these values. No callback mechanism is provided to signal the client that
a URC has arrived; it is up to the client to poll for a URC arrival using the provided
API. Only a very small subset of all possible URC's are handled. Unhandled ones are 
ignored. It is easy to extend the server and client API to add extra URC handling.
For example, the incoming call and SMS URC's are not handled but can be added.

Use of the modem client API is thread safe. Multiple clients can call the modem client
API. However, only one AT command sequence can be in progress at any one time. 
Therefore a mutex is used internally. If a task calls the modem client API and it is
busy servicing another request the 2nd call will block until it can be processed.

The modem driver code is implemented in file modem.c. For each AT command handled by
the modem there are 2 functions in file modem.c - the client one that runs in the caller's
thread and the server one that runs in the modem task thread. The client functions all
begin with Modem and server functions all begin with Server. 

The example code in main establishes a GPRS connection and opens a TCP socket to a 
webserver.It then sends a HTTP GET command and waits for the response. It reads the 
length of the response and uses that length to download the response from the modem 
into a dynamically allocated buffer. The contents of the response are sent out of 
USART2 and then the HTTP connection is closed. 

You need to set your access point name, user name (if any) and password (if any) for 
your SIM at the top of main.c.

Schematic provided. Check the voltage output from the regulator before connecting
it to the modem!
