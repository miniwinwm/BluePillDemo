This project builds on BluePillDemo_MQTT_FreeRTOS. It has all the same MQTT 
functionality but has different test code in the main task. All files
are the same apart from:

	main.c is different
	index.html is added
	green_led.gif is added
	grey_led.gif is added
	
This project demostrates how to make a webpage interact with a MQTT broker to
display incoming published message data from a broker and publish messages
back to the broker. The source and destination of the MQTT messages are the
embedded code running on the Blue Pill board which has a connection to the same
MQTT broker as the webpage.

There is some additional hardware in this example. A 10K potentiometer and a 
push button are connected to provide a source of data that the embedded code
sends to the broker. The Blue Pill board's LED is used to display incoming 
data.

A html webpage file is provided (index.html) along with the 2 .gif files it
references. This html file uses the PAHO library to implement MQTT requirements.
The PAHO library is javascript and is downloaded on demand when the webpage 
loads. There are 3 simple controls in the webpage - a textarea and an image
used to show incoming data published by the embedded code, and a check box to
allow the user to enter data that is sent to the embedded code.

There is a small amount of simple javascript in the html file that links the
html controls with the PAHO library. It's all pretty simple and there are lots
of examples on the internet that will explain it.

The embedded code sets up a connection to the broker then goes into a loop
reading the data from the potentiometer and the button and publishes them to
the broker on change. It also monitors for incoming published messages (the
checkbox state change on the webpage be the only one) and sets the board's
LED as required. If the potentiometer is turned to maximum and the push
button pressed simultaneously the loop exits. There's no error checking,
pings to prevent the broker timing out and disconnecting or attempts to
reconnect on disconnect in order to keep it simple. Messages are displayed
on UART2 at 115.2 kbaud the same as other SIM800L projects.

The html file can be run locally or on a webserver. It can even be run in
STM32CubeIDE by double clicking on it, or you can run it locally in an 
external web browser by dragging and dropping the file into one (works on
Chrome). It can also be uploaded to a free web server that allows plain
html uploads. 000webhost.com is one, and it has been uploaded there for you
to try here...

http://bluepilldemo.000webhostapp.com

It's also on the 000webhost server at https://bluepilldemo.000webhostapp.com
but this one doesn't work, only the http one works. This is because the 
public Hive MQTT broker used does not have a secure websockets interface on
its public broker, only a non-secure one, and a non-secure websocket cannot
be opened from a secure webpage. When you open the page make sure you type
in http:// as some browsers will default to https:// if you don't specify.

Note: the PAHO library uses websockets to connect to the broker. The port
of the broker's websocket interface is different from the TCP interface, so
the embedded code (which uses TCP) uses port 1883, and the html javascript
code which uses websockets uses 8000. It all arrives at the same data at the
same broker.

Schematic provided.
