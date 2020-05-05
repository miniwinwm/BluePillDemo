This example uses the Blue Pill board to drive a SIM800L GSM modem on a development board
using its AT command interface. The example provides an interface to allow a MQTT client
to connect with a MQTT broker, publish messages, subscribe to topics and receive
incoming published messages from the broker. This example builds on project 
BluePillDemo_SIM800L_TCP_FreeRTOS which implements a TCP connection and uses the same
modem driver. A MQTT layer is added which uses the modem's TCP API.

Only a subset of MQTT is implemented in this example. Only QoS 0 messages and their
responses are handled. This still allows 2 way message transfer but no guarantee of
delivery is available (fire and forget only). Ping is implemented to allow long-lived 
connections to remain alive.

The following is taken from the BluePillDemo_SIM800L_TCP_FreeRTOS project readme.txt.
Further information specific to this project is provided later...


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

---- MQTT PART ----

The MQTT protocol is simple but it adds another layer of asynchronous sequencing on
top of the AT commands. MQTT takes place over a TCP connection in variable length 
packets. The packets that are handled in this example are these:

- Connect (outgoing)
- Connect acknowledge (incoming)
- Ping (outgoing)
- Ping response (incoming)
- Publish (incoming and outgoing)
- Subscribe (outgoing)
- Subscribe acknowledge (incoming)
- Unsubscribe (outgoing)
- Unsubscribe acknowledge (incoming)
- Disconnect (outgoing)

Apart from connect acknowledge, which always is the first incoming response after a 
connect packet is sent, the other incoming packets arrive asynchronously and possibly
out of order. For example, the user may have sent a ping to the broker but before it 
arrived the broker sent a publish. This would cause this order of packets:

	Client send ping
	Client receives incoming publish
	Client receives ping acknowledge 

To handle this the client provides the MQTT driver with callback functions for the 
incoming packets which are called by the MQTT driver when an incoming packet is 
received. Callbacks are made for these incoming packets:

- Ping response
- Incoming publish
- Subscribe acknowledge
- Unsubscribe acknowledge

As the client and the MQTT driver run in the same thread it is necessary for the
client code to check periodically whether an incoming message has arrived. If it
has then this will cause the respective callback to be called in the same thread.
The function to check for incoming packets is this...

MqttStatus_t MqttHandleResponse(uint32_t timeoutMs);

The example code in main.c establishes a GPRS connection and opens a TCP socket to 
a public MQTT broker (two brokers have been tested, test.mosquitto.org and 
broker.mqttdashboard.com, both are provided in the code, one commented out). if
this is successfult the following sequence takes place: 

- A MQTT connect is then issed. This is a blocking call up to the timeout value and
the connect acknowledge is performed inside this call - there is no callback for
connect acknowledge.

- A ping is sent to the broker and a loop waits for the ping response by calling
MqttHandleResponse() to check for any response. This continues until the response
is received.

- A subscribe is made to a topic and then a loop waits for the subscribe response by 
calling MqttHandleResponse() to check for any response. This continues until the 
response is received. If there is a retained messages for this topic this will 
also arrive and the incoming publish callback will be called and the data written to 
the serial port.

- A loop of 20 seconds is started in which checks are made for incoming packets.
Half way through this loop a publish is made to the previously subscribed topic.
This should arrive back within the remainder of the loop and the incoming data
written to the serial port.

- An unsubscribe to the previous subscribed topic is made and the response waited
for as before.

- A second 10 second loop follows checking for incoming responses. Half way through
this another publish is made to the same topic as before but because the topic
has now been unsubscribed from there should be no incoming publish.

- The connection to the MQTT broker is closed, the TCP connection is closed, the 
modem is powered down.

The buffers for the serial port (and thence the TCP connection) are only small as 
the BluePill's processor is only small. Therefore this code is only suitable for
sending MQTT messages up to a few 10's of bytes in size. The GPRS connection is
also slow and messages need to be sent slowly within the capabilities of the GSM
connection. If you need to send kilobyes of data at megabits of speed get a 4G
modem and a Raspberry Pi!

This is example code and may not be entirely robust. Little account of overflow
handling is provided and this code may fall over terminally if too many messages
are sent. Aspects of MQTT available are limited - no QoS1 or 2, no retained message
capability etc. There is no security over the TCP connection. If it is used in
any commercial or real life project then it is at your own risk. It is suitable for 
demonstration, learning, non-critical maker projects or just to fill your time
until COVID-19 lockdown is over (as it has mine).

---- MQTT PART END ----

You need to set your access point name, user name (if any) and password (if any) for 
your SIM at the top of main.c.

Schematic provided. Check the voltage output from the regulator before connecting
it to the modem!
