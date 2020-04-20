This example builds on project BluePillDemo_LCD_HD44780. It uses the same code to
drive the same LCD panel but this time it wraps the driving of the display in a
FreeRTOS task. A second task communicates via a queue and sends LCD display commands to
clear the screen or display text or user defined characters in LCD control packets.

The reason for separating the LCD driving code into a separate task is because it
is a low priority job in a real time system. If the LCD display is not updated
for a few tens of milliseconds that will not be noticeable. However, it may be a
requirement that there are higher priority tasks that must not be delayed or 
interrupted. Separating low priority tasks out like this is a standard design
pattern for which a RTOS like FreeRTOS helps to do.

The driving of the LCD display hardware and the API are the same as project 
BluePillDemo_LCD_HD44780. See that project for details.

The main task periodically updates the display, clearing it and sending text and characters.
LCD control packets are added on to the LCD queue. The LCD task removes these packets and
enacts them.

Different parts of the code in lcd.c can run in multiple threads - the calling task and 
the lcd task. No static data must be shared between the parts of the code in different 
threads.

The layout of the code is not optimal - it would be better if the lcd task function was
in lcd.c rather than main.c. However, that is not how STM32CubeIDE generates the code
so the arrangement of the lcd task in main.c has been left as generated as this is 
only example code.

Schematic provided.