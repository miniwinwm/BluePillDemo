This example sets up SPI1 and integrates it with the FatFS middleware to provide access to a FAT32
file system on a SD card. This is the best method of accessing an external file system on the Blue
Pill's STM32F103C8 processor as it does not support SDIO or USB host. All SD cards support SPI as
a means of access. It is however slower than other methods. The SPI is set to run at approximately
10 Mbps which is sufficient for accessing small files slowly.

The main function creates a file, writes some data to it, reads it back and closes the file. Check
the values using the debugger.

This example does not set the real time clock and hemce uses no time for file creation time
and date. It is possible also to enable the RTC module and have proper file creation timestamps
once the RTC has been initialized.

To connect to the SD card a cheap SD card slot with SPI connections is used.

Schematic provided.