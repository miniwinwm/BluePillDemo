This project uses the Blue Pill board to connect to a MAX30102 pulse oximeter sensor
mounted on a break out board by I2C. This sensor can detect harbeats and blood oxygen
saturation. It outputs the results as a SpO2 percentage (peripheral blood oxidation
level) and hear rate in beats per minute to a serial terminal at 115200 baud. Also
connected is a LED which flashes with the detected heartbeat.

These sensors are quite picky about your finger being placed correctly and without
movement on the sensor. You might need to shift it around a bit before you get a 
reading. Once you have found a position where you are getting a strong clear pulse
showing on the LED then you will normally get a reading. Readings are harder to get 
from your finger tips if you are bodily cold as the body shuts down blood flow to
the extremities when you are cold.

This code is a port of Arduino code for the same sensor chip found here...

https://github.com/aromring/MAX30102_by_RF

which itself is based on the reference code from the sensor manufacturer Maxim
Integrated which is found here...

https://www.maximintegrated.com/en/design/reference-design-center/system-board/6300.html/tb_tab0

Important note. The thieving Chinese have created a fake cloned copy of the MAX30102 
sensor chip which are sold on the cheap MAX30102 breakout boards found on ebay. They 
either haven't got it quite right or have modified it slightly. The difference between
the genuine and the fake sensor chips is that the fakes have the 2 LEDs (red and infra-
red) the opposite way round. There is a #define in the code in main.c called this...

FAKE_MAX30102_CLONE

If the code doesn't work try defining this to see if it helps.

If you buy your MAX30102 sensor from a reputable vendor it will be genuine. If you get
a cheap one from ebay it will almost certainly be fake. It's unknown what else the 
intellectual property thieves have changed or got wrong so don't rely in any way on
the results from this example. Don't anyway - even if you have a genuine sensor. 

Schematic provided.