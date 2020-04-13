This example uses an external push button to create an interrupt-on-change (EXTI) when the 
button is pressed. The interrupt handler toggles the Blue Pill's on-board LED. There is no
debouncing in this code or hardware so the LED toggling may be inconsistent.

Schematic provided.