This example provides sample code for a OLED display with a SSD1306 driver chip driving a 
0.9" 128 x 64 pixel display via a n I2C interface. These panels are readily available on 
ebay at low cost. The colour depth is 1 bit monochrome.

The code is separated into 3 layers. The lowest layer is the SSD1306 driver that uses the HAL
to access the I2C1 peripheral. This is in files ssd1306.h/c. Above that is a graphics 
library layer in files graphics.h/c. This provides a simple graphics library for plotting
lines, shapes, bitmaps, circles and 2 fonts vertically and horizontally. It isn't perfect, 
but is a start. At the top level is some example code in main.c that uses the graphics 
library.

The API is documented in doxygen comments in graphics.h. The graphics routines in this
example plot to a buffered version of the display, not the display directly. It is necessary
to send the buffer to the display to see the effevt. See the example code in main as an
example.

Image files can be converted to C source files online. A website that does this is given
below:

https://www.digole.com/tools/PicturetoC_Hex_converter.php

Schematic provided.
