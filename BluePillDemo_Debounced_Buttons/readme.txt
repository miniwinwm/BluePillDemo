This example uses the EXTI interrupt on change and a general timer to provide 2 debounced button inputs.
The button inputs can distinguish between a short and long press on each button. Although only 2 buttons
are monitored in this code it is easy to expand it to more. However, this code only recognises a single button
push at a time. It won't detect multiple button simultaneous pushes.

The code is all run in interrupt handlers. The main function is empty in this example, but by using
debugging and breaking after pushing a button the value in variable buttons_value shows which button
was pressed and if it was short or long press...

Top button short	1
Bottom button short	2
Top button long		3
Bottom button long	4

Schematic provided. 