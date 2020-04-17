This example uses a general purpose timer's capture capability to perform a frequency measurement
on a pin. The frequency comes from the processor's RTC clock which has it's tick signal output on
pin PC13 (the Blue Pill board's LED pin - so that will partially show). This tick signal is the 
RTC clock divided by 64 and using the HSE as the RTC clock source gives a signal on pin PC13 of
approximately 1 kHz.

The timer used (TIM2) has 4 channels for input and the first channel is used which is on pin PA0.
In the circuit diagram pins PC13 and PA0 are connected together.

TIM2's source clock is set up as PCLCK1 (36 MHz) divided by 4. When a rising edge is detected on 
PA0 the counter value of TIM2 is captures and from this and the previous rising edge's capture
value the frequency can be calculated.

Schematic provided.