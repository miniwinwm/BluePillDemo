The STM32F103C8 has 16 bytes in flash that are extra to the main flash area. These are called 
the option bytes. They are mainly for controlling aspects of the watchdog and read and write
protection of the main flash. In addition however there are 2 bytes that are unused for these
purposes and can be used by the programmer as non-volatile storage for any reason. This may
be useful if the user only needs minimal storage and does not want to use a whole 1 kbyte of
normal flash to do this.

Although this can be done - it's not straightforward. Firstly the 2 user bytes cannot be
erased on their own and being flash must be erased if any bits need setting from 0 to 1. Only
the whole 16 byte area of the option bytes can be erased in one go. As you probably don't 
want to change any of the other data in there you need to read and cache the existsing data,
change the user data and write the whole lot back. The API is a bit strange as well in HAL,
only 1 user data option byte can be written at a time so to write both it's a bit of a job...

Read existing option bytes and cache in memory (this doesn't read the 2 user bytes)
Unlock 2 flash areas
Set the first user data byte in cached memory
Do a combined erase/write back to flash
Set the second user data byte in cached memory
Do a write back to flash without erase
Lock 2 flash areas.

Reading is simpler, you just use the HAL API to read one of the 2 user data bytes at a time.

Here's a further problem - the HAL options byte code is broken. It is fixed in this project. 
The file that needs copying from this project is... 

Drivers/STM32F1xxHAL_Driver/Src/stm32f1xx_hal_flash_ex.c

If you don't do this then you will corrupt the option bytes in flash and have to use the 
ST_LINK utility software to set it back to factory condition.

Note: Debugging and stopping part way through this code or powering off in a similar way is
likely to corrupt your option bytes. You will need to reset them to factor condition using
ST-LINK utility if you do this.

No schematic for this project.
