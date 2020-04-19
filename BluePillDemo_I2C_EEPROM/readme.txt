This example uses I2C to communicate with a I2C serial EEPROM chip, in this case the 128 kBit 
24LC128 from Microchip. The I2C is used in polling mode.

The code to access the chip is eparated out into eeprom.h/c. In the main function this is
tested by writing a small fixed string to the EEPROM and reading it back which can be
checked in the debugger.

Schematic provided.