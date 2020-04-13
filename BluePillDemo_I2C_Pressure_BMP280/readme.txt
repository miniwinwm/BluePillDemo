This example uses I2C to communicate with a BMP280 pressure/temperature sensor. This sensor
is cheaply available on breakout boards with the only connections being power, ground, SCL and SDA.
Usually these boards require 2-6 Volts which can be taken from the Blue Pill board's 3.3V line.

The first step to using this sensor is to read 12 constants programmed into the sensor at the
factory when it is calibrated (these will be different for each product). They only need to
be read once.

Then a reading start command can be sent. A delay is needed before reading the results.

After reading the raw results there is a some mathematical processing that needs 
to be performed on the raw readings using the 12 constants read earlier. The resulting pressure
is given in millibars. Temperature not calculated yet.

The code to operate this device is given in a separate files pressure_sensor.h/c.

Schematic provided.