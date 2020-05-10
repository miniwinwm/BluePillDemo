This example uses the Blue Pill board to drive a SIM800L GSM modem on a development board
using its AT command interface. It includes the embedded, MQTT and webpage code from project 
BlueBillDemo_SIM800L_MQTT_FreeRTOS_Website and the Seatalk interface code from project
BlueBillDemo_Seatalk. When running the main loop reads boat data from the Seatalk network
and publishes them periodically to the MQTT broker. Also included is a html webpage that
connects to the same broker, subscribes to the messages and displays them graphically in
a webpage, including a map to show the location of the latitude and longitude received.

All the SIM800L modem and MQTT code is the same as in the 
BlueBillDemo_SIM800L_MQTT_FreeRTOS_Website project and the Seatalk code is the same as in 
the BlueBillDemo_Seatalk project so will not be further described here.

The schematic provided is also a combination of the two projects mentioned above with a
connection and power supply for the modem and the seatalk interface. The serial connection
for the debug messages is optional and is marked as sunch on the schematic.

The main loop is simple. It goes round reading the data and publishing them. So far there
is no check for unchanged data (which does not need to be republished so frequently) and 
no attempt to reconnect to the broker if the connection fails. This will be added soon.

If you are going to use this for your own data you need to change the publish topic
root part in main.c in the embedded code from BluePillDemo to something of your choosing. 
This is a #define at the top of main.c in the embedded code:

	#define			MQTT_PUBLISH_TOPIC_ROOT		"BluePillDemo"

If you want to create your own webpage to view your data you need an account on a 
webserver that allows you to upload your own html files. https://www.000webhost.com
provide this service. You need to create a free account and upload these files from this
project:

	Mapiator.js
	segment-display.js
	index.html
	
You also need to create a folder called images in the folder that contains the above files 
and upload these files:

	images/zoomIn_blur.png
	images/zoomin_focus.png
	images/zoomOut_blur.png
	images/zoomOut_focus.png

Your folder sytucture if on 000webhost.com will look like this:

	public_html/Mapiator.js
	public_html/segment-display.js
	public_html/index.html
	public_html/images/zoomIn_blur.png
	public_html/images/zoomin_focus.png
	public_html/images/zoomOut_blur.png
	public_html/images/zoomOut_focus.png
	
Before uploading index.html you need to edit it and change the MQTT topic root part to a
value that matches what you set in your embedded code, otherwise you will be seeing my data!

On 000webhost.com (and probably other webservers too) the page is served at the 
http://xxxx.000webhostapp.com URL and the https://xxxx.000webhostapp.com URL (where xxxx
is your account name). Only the http page works if you are using the Hive public MQTT broker
as this project does. The https version will not get the data from the broker. See 
appendix for an explanation.
		
You need to set your access point name, user name (if any) and password (if any) for 
your SIM at the top of main.c in the embedded code. These are #define's:

	#define ACCESS_POINT_NAME			"everywhere"
	#define USER_NAME					"eesecure"
	#define PASSWORD					"secure"

Schematic provided. Check the voltage output from the regulator before connecting
it to the modem!

------------------------------------------------------------------------------------------

Why http and not https?

The Hive public MQTT test broker has a websockets port so that webpages can access the broker 
on port 8000. Being a free service it provides only a non-secure websocket access and not a 
secure websocket access (you have to pay to get this). The webpage index.html, if accessed by
https (i.e. with security) cannot then acces a non-secure websocket, i.e. once secure then
everything must be secure.

The alternative mosquitto public MQTT broker does provide a secure websocket interface but this
broker is used to test the broker and is frquently unavailable.
