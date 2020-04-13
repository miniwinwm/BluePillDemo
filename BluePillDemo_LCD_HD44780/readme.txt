This example provides a driver for a simple 16x2 text LCD using the HD44780 interface in 4 bit mode.
The hardware is driven using simple GPIO outputs. The display's busy line is not polled but instead
a fixed delay is used after every write. This is because not all cheap clone LCD displays seem to
implement the busy signal.

The driver is separated out into files lcd.h/c with a simple API...

// call once at startup
void lcd_init(void);	

// row and column are zero based, length of string + column must be < LCD_WIDTH_CHARS
void lcd_puts(uint8_t row, uint8_t column, const char *s);

// clears whole screen
void lcd_clear(void);

// set up a user defined character, bytes must point to 8 bytes
void lcd_set_cg_character(uint8_t position, const uint8_t *bytes);

The API allows user defined characters to be stored in the LCD's memory. These are lost
at power off.

The main function shows how to use the API including setting up and displaying user defined
characters.

Schematic provided.