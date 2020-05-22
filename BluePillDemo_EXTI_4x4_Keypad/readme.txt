This example project drives a standard 16 button keypad with 4 rows and 4 
columns. The standard 4 x 4 16 key keypad has 8 connections arranged in 2 
groups of 4. These connections go to lines across the keypad under the 
switches. When a key is pressed 2 of the lines are connected. There are 16 
ways that 2 groups of 4 lines can be connected, hence the 8 lines to the 
keypad. The keypad contains no logic and requires no power supply - it is 
just 16 switches.

Driving these keypads therefore requires 8 digital inputs and outputs. 4 
need to be outputs and 4 inputs. In normal operation the 4 outputs are
all driven high and the 4 inputs are connected to EXTI inputs which causes
an interrupt to fire on a rising edge. The inputs are pulled low to stop
them floating when no key is pressed. 

In the interrupt handler the 4 outputs are set low then are driven high 
one at a time and the 4 inputs are checked in turn to see if any have a high
level from a pressed key connecting the line to the currently high output. 
When a key press is identified its value is calculated from the row * 4 +
column and a variable set.

Like all keys and buttons bounce can cause multiple keypresses to be reported
so there's a simple bit of time since last press checking and keypresses are
only reported after an interval of 250 ms.

The main loop looks for new key presses and sends out a message on a serial
port when one has been detected.

Schematic provided.