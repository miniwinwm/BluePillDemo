# BluePillDemo

These projects can be imported into STM32CubeIDE.
Each sub-folder has a readme.txt file describing the project and a schematic.pdf file showing the circuit used. Flashing and debugging needs a ST-LINK device connected to the Serial Wire 4 pin header at the end of the Blue Pill board.

A brief explanation of how to create a new project for the Blue Pill board in STM32CubeIDE using a ST-LINK device for flashing/debugging is shown below:

File|New|STM32 Project

In Target Selection type STM32F103C8 in Part Number Search

Select this processor when it is listed on the right, Next

Enter Project Name, select C++ if you need it, leave other selections default, Finish

In 'Project Name'.ioc editor Select Pinout & Configuration tab

In Pinout & Configuration choose Categories then expand System Core then select SYS

Under Debug select Serial Wire

In System Core select RCC

In both High Speed Clock (HSE) and Low Speed Clock (LSE) select Crystal/Ceramic Resonator

Select Clock Configuration Tab

In System Clock Mux select PLLCLK

In PLL Source Mux select HSE

In PLLMul select X9

In APB1 Prescalar select /2


For projects using USB, ADC or RTC further clock configuration is required. See example projects.



