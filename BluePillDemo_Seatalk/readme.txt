This example provides a driver to interface the Blue Pill board with a Seatalk bus. 
Seatalk is a legacy serial protocol for marine electronics from Raymarine. It is a 
multi-drop single wire bus and runs at a slow speed of 4800 baud. It uses an 
unusual 9 bit data size which does not fit easily with the standard UARTS found on 
microcontrollers. It also needs collision avoidance as all nodes share the 
same wire. No node is a master but each transmitter reads the bus as it transmits
and if a discrepancy is found it backs off for a random delay and tries again.

As the hardware is not suitable for UARTS and the data rate is slow the most 
reliable method of implementing it is to bit bang it. That is what this driver
does. It sets up a timer with a high priority interrupt which fires at a rate of
38.4 kHz. This makes 8 ticks per Seatalk bit. In the interrupt handler it waits 
until the 4th tick and then samples the Seatalk bus. It does this for all 9 bits.

A collection of Seatalk 9 bit bytes is called a sentence. These are a variable
length but always have a length field. A start byte is signalled by the extra bit
to the byte which is why each byte has 9 bits - 8 data and 1 signal. As bytes 
arrive in they are buffered in an array of sentences by the timer interrupt
handler. It is up to the main thread to call seatalk_parse_next_message() 
frequently (every 10ms) which reads the next sentence, parses it and informs the
main thread that a message has arrived via a callback.

Transmitting works similarly. Writing a sentence buffers it. When the function
seatalk_parse_next_message() is called the transmit worker function should also
be called - seatalk_send_next_message().

This driver does not parse or allow to be sent all types of Seatalk sentences, 
only those implemented in seatalk.c. If other sentences are required they can 
follow the same pattern.

In the parser of seatalk.c extra derived data are calculated. For example apparent
wind speed and boat speed are Seatalk sentences but true wind speed is not. It
can be calculated from those two and this is done in seatalk_parse_next_message()
and a callback made.

When a callback has been made indicating that a sentence has arrived the main 
task can call the getters in seatalk.c to get the data.

Seatalk is a trademark of Raymarine. Use of this code is at your own risk of 
being sued by Raymarine. Seatalk is also a closed standard. However, it is not
encrypted or obfuscated in any way. In the European Union reverse engineering 
protocols for interoperability purposes is allowed and has been tested in court. 
In other parts of the world it may not be legal.

The reverse engineering of Seatalk was done by Thomas Knauf. He has a website 
describing the sentences. If you need to implement other sentences, send or 
receive, that are not implemented here consult his website for details. 

The example code in main.c simply sends periodically a speed over ground message
and also receives the same message type. It therefore receives the message it
has just sent.

A schematic is provided. The Seatalk hardware interface is also taken from 
Thomas Knauf's website, which is here...

http://www.thomasknauf.de/seatalk.htm