This example uses some of the STM32F103C8's on-chip flash to save user data. This processor
has 64 kBytes of flash space. The page size (the smallest size that can be erased) is 1 kByte.
This example erases the top 1 kByte of flash space and writes some data to it then reads back
the data to check that it has been written correctly.

The linker script file (STM32F103C8TX_FLASH.ld) has been modified to reduce the flash space
available to the linker from 64 kBytes to 63 kBytes to prevent it over-writing the user
flash space...

  FLASH	(rx)	: ORIGIN = 0x8000000,	LENGTH = 63K
  
No schematic file for this example. 