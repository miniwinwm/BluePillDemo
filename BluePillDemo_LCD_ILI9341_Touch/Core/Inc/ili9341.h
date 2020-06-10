#ifndef INC_ILI9341_H_
#define INC_ILI9341_H_

#include <stdint.h>
#include <stdbool.h>

#define ILI9341_RST_PORT           	GPIOA
#define ILI9341_RST_PIN             GPIO_PIN_9
#define ILI9341_DC_PORT             GPIOA
#define ILI9341_DC_PIN              GPIO_PIN_8
#define ILI9341_CS_PORT             GPIOA
#define ILI9341_CS_PIN              GPIO_PIN_10
#define ILI9341_LCD_WIDTH           240U
#define ILI9341_LCD_HEIGHT          320U

typedef uint16_t colour_t;

// colour bit layout rrrr rggg gggb bbbb
#define BLACK 						((colour_t)0x0000)
#define YELLOW 						((colour_t)0xffe0)
#define RED 						((colour_t)0xf800)
#define GREEN 						((colour_t)0x07e0)
#define BLUE 						((colour_t)0x001f)
#define WHITE 						((colour_t)0xffff)
#define PINK 						((colour_t)0xf80e)
#define PURPLE 						((colour_t)0xf83f)
#define	GREY15						((colour_t)0x1082)
#define	GREY14						((colour_t)0x2104)
#define	GREY13						((colour_t)0x3186)
#define	GREY12						((colour_t)0x4208)
#define	GREY11						((colour_t)0x528a)
#define	GREY10						((colour_t)0x630c)
#define	GREY9						((colour_t)0x738e)
#define	GREY8						((colour_t)0x8410)
#define	GREY7						((colour_t)0x9492)
#define	GREY6						((colour_t)0xa514)
#define	GREY5						((colour_t)0xb596)
#define	GREY4						((colour_t)0xc618)
#define	GREY3						((colour_t)0xd69a)
#define	GREY2						((colour_t)0xe71c)
#define	GREY1						((colour_t)0xf79e)
#define ORANGE 						((colour_t)0xfb80)
#define CYAN						((colour_t)0x07ff)
#define DARK_CYAN					((colour_t)0x0492)
#define LIGHT_ORANGE				((colour_t)0xfe20)
#define BRICK_RED					((colour_t)0xb104)

void ILI9341Reset(void);
void ILI9341Init(void);
void ILI9341Pixel(uint16_t x, uint16_t y, colour_t colour);
void ILI9341FilledRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, colour_t colour);
void ILI9341DrawMonoBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData, colour_t fgColour, colour_t bgColour);
void ILI9341DrawColourBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData);

#endif
