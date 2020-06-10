This example builds on the driver for the ILI9341 LCD display by adding a touch screen
driver. The touch screens are measured by a chip on the display with a very simple SPI
interface. There are only 2 SPI commands that need to be used - read the X touch point
and read the Y touch point. Whether the screen is being touched is reported by a 
dedicated pin from the touch screen driver chip.

Although the touch screen SPI interface is simple getting stable touch point readings
in screen coordinates is less simple for these reasons:

1) The touch reading is noisy. If you hold a stylus still on the screen and repeatedly
read the touched points you will get values that vary significantly with the 
occasional wild point nowhere near the touched point.

2) The raw data from the touch driver chip are not in screen coordinates. The range of
values will depend on the number of bits in the ADC in the chip and the range of values
will not be from 0 to the maximum according to the ADC but in a range that starts above
zero and ends less than the ADC's theoritical maximum value, i.e. the ADC might have a
theortical range of 0 to 4095 but the raw values you will see could be in the range 137
to 3811 (for example).

3) The co-ordinate system origin may not be in the top left corner and may not increase
to the right and down.

The solution to problem 1 is to make multiple readings and process them. One method is to
take a simple average, but this can be badly influenced by a raw reading that is way off
which periodically happens. A better algorithm is to take multiple readings, sort them, 
then take the average of the middle two readings. This method avoids the influence of 
outliers.

The solution to the 2nd and 3rd problems is to calibrate the screen by asking the user to 
touch 3 known points. This can then be used to convert raw readings from the chip into
calibrated screen coordinate readings. The calibration procedure returns a calibration
matrix. This can be stored in processor flash or external EEPROM so that it only has to
be performed once at initial power up. In this simple example it is not stored in 
flash so is performed at every power up.

There is a driver layer in the code in touch.c/h with a very simple API which is explained
in the Doxygen comments in touch.h.

The main program starts by requesting a calibration and then displays the touched state
and touch point as text and drawing the touched point.

Schematic provided.
