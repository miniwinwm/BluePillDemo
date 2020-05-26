#include <string.h>
#include "stm32f1xx_hal.h"
#include "ssd1306.h"
#include <math.h>
#include <stdlib.h>

#define SSD1306_I2C_ADDRESS        0x3CU

extern I2C_HandleTypeDef hi2c1;

static uint8_t SSD1306_Buffer[SSD1306_OLED_WIDTH * SSD1306_OLED_HEIGHT / 8U];

static void writeCommand(uint8_t byte);
static void writeData(uint8_t* buffer, uint16_t size);

static void writeCommand(uint8_t byte)
{
	HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDRESS << 1, 0x00U, 1U, &byte, 1U, 100U);
}

static void writeData(uint8_t* buffer, uint16_t size)
{
	HAL_I2C_Mem_Write(&hi2c1, SSD1306_I2C_ADDRESS << 1, 0x40U, 1U, buffer, size, 100U);
}

void SSD1306Init(void)
{
	uint8_t i;
	const uint8_t initCommands[] = "\xae\x20\x00\xb0"
#ifdef SSD1306_MIRROR_VERTICAL
			"\xc0"
#else
			"\xc8"
#endif
			"\x00\x10\x40\x81\xff"
#ifdef SSD1306_MIRROR_HORIZONTAL
			"\xa0"
#else
			"\xa1"
#endif
#ifdef SSD1306_INVERT_COLOUR
			"\xa7"
#else
			"\xa6"
#endif
#if (SSD1306_OLED_HEIGHT == 128U)
			"\xff"
#else
			"\xa8"
#endif
#if (SSD1306_OLED_HEIGHT == 32U)
			"\x1f"
#else
			"\x3f"
#endif
			"\xa4\xd3\x00\xd5\xf0\xd9\x22\xda"
#if (SSD1306_OLED_HEIGHT == 32U)
			"\x02"
#else
			"\x12"
#endif
			"\xdb\x20\x8d\x14\xaf";

	for (i = 0U; i < sizeof(initCommands); i++)
	{
		writeCommand(initCommands[i]);
	}

    memset(SSD1306_Buffer, 0x00, sizeof(SSD1306_Buffer));
    SSD1306UpdateDisplay();
}

void SSD1306UpdateDisplay(void)
{
	uint8_t row;

    for (row = 0U; row < SSD1306_OLED_HEIGHT / 8U; row++)
    {
        writeCommand(0xB0U + row);
        writeCommand(0x00U);
        writeCommand(0x10U);
        writeData(&SSD1306_Buffer[SSD1306_OLED_WIDTH * row], SSD1306_OLED_WIDTH);
    }
}

void SSD1306Pixel(uint16_t x, uint16_t y, colour_t colour)
{
    if (x >= SSD1306_OLED_WIDTH || y >= SSD1306_OLED_HEIGHT)
    {
        return;
    }

    if (colour)
    {
        SSD1306_Buffer[x + (y / 8U) * SSD1306_OLED_WIDTH] |= 1U << (y % 8U);
    }
    else
    {
        SSD1306_Buffer[x + (y / 8U) * SSD1306_OLED_WIDTH] &= ~(1U << (y % 8U));
    }
}

void SSD1306FilledRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, colour_t colour)
{
	uint16_t xc;
	uint16_t yc;

	for (xc = x; xc < x + width; xc++)
	{
		for (yc = y; yc < y + height; yc++)
		{
			SSD1306Pixel(xc, yc, colour);
		}
	}
}

void SSD1306DrawMonoBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData)
{
	uint16_t xc;
	uint16_t yc;
	uint16_t a;
	uint8_t imageByte;
	uint8_t mask;
	uint16_t arrayWidthBytes;

	arrayWidthBytes = width / 8U;
	if (width % 8U > 0U)
	{
		arrayWidthBytes++;
	}

	for (yc = 0U; yc < height; yc++)
	{
		for (a = 0U; a < arrayWidthBytes; a++)
		{
			imageByte = imageData[yc * arrayWidthBytes + a];
			mask = 0x80U;
			for (xc = 0U; xc < 8U; xc++)
			{
				if ((a * 8U) + xc == width)
				{
					break;
				}

				if ((imageByte & mask) == 0U)
				{
					SSD1306Pixel((a * 8U) + xc + x, yc + y, true);
				}
				else
				{
					SSD1306Pixel((a * 8U) + xc + x, yc + y, false);
				}
				mask >>= 1;
			}
		}
	}
}
