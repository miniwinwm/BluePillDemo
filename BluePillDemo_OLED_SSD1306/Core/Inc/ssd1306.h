#ifndef INC_SSD1306_H_
#define INC_SSD1306_H_

#include <stdint.h>
#include <stdbool.h>

typedef bool colour_t;

// define these for your display
#define SSD1306_OLED_HEIGHT          64U
#define SSD1306_OLED_WIDTH           128U
//#define SSD1306_MIRROR_VERTICAL
//#define SSD1306_MIRROR_HORIZONTAL
//#define SSD1306_INVERT_COLOUR

#if (SSD1306_OLED_HEIGHT != 32)
#if (SSD1306_OLED_HEIGHT != 64)
#if (SSD1306_OLED_HEIGHT != 128)
#error "Display height must be 32, 64 or 128"
#endif
#endif
#endif

void SSD1306Init(void);
void SSD1306UpdateDisplay(void);
void SSD1306Pixel(uint16_t x, uint16_t y, colour_t colour);
void SSD1306FilledRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, colour_t colour);
void SSD1306DrawMonoBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData);

#endif
