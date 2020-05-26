This example provides sample code for a TFT LCD display with a ST7789 driver chip driving a 
1.3" 240 x 240 pixel display via a half-duplex SPI interface. These panels are readily 
available on ebay at low cost. The colour depth is 16 bit in 565 format.

The code is separated into 3 layers. The lowest layer is the ST7789 driver that uses the HAL
to access the SPI1 peripheral. This is in files st7789.h/c. Above that is a graphics 
library layer in files graphics.h/c. This provides a simple graphics library for plotting
lines, shapes, bitmaps, circles and 2 fonts vertically and horizontally. It isn't perfect, 
but is a start. At the top level is some example code in main.c that uses the graphics 
library.

In the driver DMA is used for large data transfers to the LCD. Although the DMA completion
flag is polled using DMA still speeds up the transfer compared to doing it in software.

The API is documented in doxygen comments in graphics.h. The graphics library is unbuffered.
Calls to graphics.c appear immediately on the display.

Image files can be converted to C source files online. A website that does this is given
below:

https://www.digole.com/tools/PicturetoC_Hex_converter.php

Schematic provided.
