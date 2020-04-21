This example sets up SPI1 and integrates it with the FatFS middleware and FreeRTOS to provide 
access to a FAT32 file system on a SD card within a multi-threaded RTOS. SPI is the best method 
of accessing an external file system on the Blue Pill's STM32F103C8 processor as it does not 
support SDIO or USB host. All SD cards support SPI as a means of access. It is however slower 
than other methods. The SPI is set to run at approximately 10 Mbps which is sufficient for 
accessing small files slowly.

When accessing the FatFS API from multiple threads re-entrancy protection has to be enabled
to prevent file corruption. The STM32CubeIDE will do this for you if FreeRTOS is chosen in
combination with FatFS, but not very well! If CMSIS_V1 RTOS API is chosen, it's fine. But if
CMSIS_V2 API is chosen the generated code is incorrect. It builds until you try to call a 
function in the FatFS API and then it doesn't link. The reason is that some of the generated
code is still using CMSIS_V1 API which has been deprecated. The generated code in file
Middlewares/Third_Party/FatFS/src/option/syscalls.c has been hand edited in this example to 
fix this. If you regenerate the code these changes will be over-written and you will need to 
replace this file's contents from this repo. Details of this bug are reported here...

https://community.st.com/s/question/0D53W0000055JO9SAM/bug-stm32cubeide-code-generation-with-fatfsfreertos-is-using-a-deprecated-api

STM32CubeIDE puts the call to initialize FatFS (MX_FATFS_Init) at the top of the first
thread defined in the configurator. It's not a useful thing to do but it can't be changed in 
the generator. To ensure other threads don't start and begin using the FatFS API before this
init function is called some thread synchronisation is required.

The main function creates 2 threads which both access the file system in their thread functions.

FatFS needs a lot of stack space for each thread. The STM32F103C8 doesn't really have enough
RAM to do multi-thread file access as although it works it doesn't leave much memory for
the rest of your application but this example can be ported to a processor with more memory.

This example does not set the real time clock and hemce uses no time for file creation time
and date. It is possible also to enable the RTC module and have proper file creation timestamps
once the RTC has been initialized.

To connect to the SD card a cheap SD card slot with SPI connections is used.

Schematic provided.