This example is the same as project BluePillDemo_Seatalk_MQTT_Website but has
NMEA0183 as its source of data that it publishes.

Only NMEA0183 messages DPT and RMC are parsed so far. This means that the following
data values are sent to the MQTT broker:

	Depth
	SOG
	COG
	Latitude
	Longitude
	
Other message types can be added by extending the code already present. There are
'todo' comments in the code where this is required.

See the readme.txt file of project BluePillDemo_Seatalk_MQTT_Website for more 
information.

Schematic available.
