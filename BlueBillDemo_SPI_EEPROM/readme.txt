This example uses SPI to communicate with a SPI serial EEPROM chip, in this case the 320 kBit 
25AA320A from Microchip. The SPI is used in polling mode.

The code to access the chip is eparated out into eeprom.h/c. In the main function this is
tested by writing a small fixed string to the EEPROM and reading it back which can be
checked in the debugger.

Schematic provided.