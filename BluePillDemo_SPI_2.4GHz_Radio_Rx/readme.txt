This project is part of a pair of projects, the other being BluePillDemo_SPI_2.4GHz_Radio_Tx.
These projects provide and test a driver for wireless 2.4 GHz modules containing the 
NRF24L01 radio chip. This project incorporates code from Brennen Ball who wrote the driver
for this radio chip.

To run these projects you need 2 Blue Pill boards and 2 RF modules. The schematic is the same
for both projects.

The BluePillDemo_SPI_2.4GHz_Radio_Tx code periodically sends a single byte of data via the 
radio chip. The BluePillDemo_SPI_2.4GHz_Radio_Rx project is set up to receive and when 
new data are available asserts an interrupt line which is connected to an EXTI interrupt
pin on the processor. The interrupt handler reads the data and toggles the Blue Pill
board's LED.

Running these projects in debug mode does not always work. This is because the NRF24L01
chip on the RF module does not like being initialized more than once. If the code is 
already flashed on the processor when you power it up it will run initializing the radio
chip. When you subsequently start debugging the code is run again, the radio chip 
reinitialized again, and then fails to work.

In the file nrf24l01 there is a function that asserts and deasserts the CE line 
(nrf24l01_transmit). This assertion/deassertion needs a 10us delay. In the code here this
is done with a busy wait loop around a NOP instruction. This works at the processor clock
speed chosen (72 MHz) but may need adjusting if a different clock speed is used. The 
proper way would be to use a hardware timer to provide this 10uS delay.

Schematic provided which is common to the BluePillDemo_SPI_2.4GHz_Radio_Rx project.

