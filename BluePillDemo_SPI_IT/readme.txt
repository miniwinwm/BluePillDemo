This example configures both SPI modules on the STM32F103C8, one as master and the other as slave,
and then they are connected to each other. The SPI modules are configured to use interrupts with the 
master module having a lower priority than the slave. A non-blocking receive call is then initiated 
on the slave and a write on the master. Using the debugger, after a short delay, the data written
to the master is shown as received into the slave's buffer.

Schematic provided.
