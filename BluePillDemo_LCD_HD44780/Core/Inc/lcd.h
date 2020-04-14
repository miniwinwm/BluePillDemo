#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_WIDTH_CHARS			16U
#define CG_CHARACTER_SIZE_BYTES	8U

// call once at startup
void lcd_init(void);

// row and column are zero based, length of string + column must be < LCD_WIDTH_CHARS
void lcd_puts(uint8_t row, uint8_t column, const char *s);

// clears whole screen
void lcd_clear(void);

// set up a user defined character, bytes must point to 8 bytes
void lcd_set_cg_character(uint8_t position, const uint8_t *bytes);

#endif
