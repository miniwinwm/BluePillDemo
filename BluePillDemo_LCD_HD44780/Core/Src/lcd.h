#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_WIDTH_CHARS			16U
#define CG_CHARACTER_SIZE_BYTES	8U

void lcd_init(void);
void lcd_puts(uint8_t row, uint8_t column, const char *s);
void lcd_clear(void);
void lcd_set_cg_character(uint8_t position, const uint8_t *bytes);

#endif
