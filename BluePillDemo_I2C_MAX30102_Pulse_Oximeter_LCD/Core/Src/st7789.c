#include <stddef.h>
#include "st7789.h"
#include "stm32f1xx_hal.h"

#define DMA_BUFFER_SIZE 64U

typedef struct
{
	uint8_t command;
	uint8_t delayMs;
	uint8_t dataSize;
	const uint8_t *data;
} ST7789Command_t;

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;

static volatile bool txComplete;
static uint8_t caset[4] = {0};
static uint8_t raset[4] = {0};

static void SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
static void WriteDataDMA(const void *data, uint16_t length);
static void WaitForDMAWriteComplete(void);

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	txComplete = true;
}

void ILI9341Reset(void)
{
	HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(10UL);
	HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(120UL);
}

void ILI9341Init(void)
{
	uint8_t i;

	caset[3] = (uint8_t)(ST7789_LCD_WIDTH - 1U);
	raset[3] = (uint8_t)(ST7789_LCD_HEIGHT - 1U);

	const ST7789Command_t initSequence[] =
	{
		{ ST7789CMD_SLPIN, 10U, 0U, NULL },
		{ ST7789CMD_SWRESET, 200U, 0U, NULL },
		{ ST7789CMD_SLPOUT, 120U, 0U, NULL },
		{ ST7789CMD_MADCTL, 0U, 1U, (const uint8_t *)"\x00" },
		{ ST7789CMD_COLMOD, 0U, 1U, (const uint8_t *)"\x55" },
		{ ST7789CMD_INVON, 0U, 0U, NULL },
		{ ST7789CMD_CASET, 0U, sizeof(caset), (const uint8_t *)&caset },
		{ ST7789CMD_RASET, 0U, sizeof(raset), (const uint8_t *)&raset },
		{ ST7789CMD_PORCTRL, 0U, 5U, (const uint8_t *)"\x0c\x0c\x00\x33\x33" },
		{ ST7789CMD_GCTRL, 0U, 1U, (const uint8_t *)"\x35" },
		{ ST7789CMD_VCOMS, 0U, 1U, (const uint8_t *)"\x1f" },
		{ ST7789CMD_LCMCTRL, 0U, 1U, (const uint8_t *)"\x2c" },
		{ ST7789CMD_VDVVRHEN, 0U, 2U, (const uint8_t *)"\x01\xc3" },
		{ ST7789CMD_VDVSET, 0U, 1U, (const uint8_t *)"\x20" },
		{ ST7789CMD_FRCTR2, 0U, 1U, (const uint8_t *)"\x0f" },
		{ ST7789CMD_PWCTRL1, 0U, 2U, (const uint8_t *)"\xa4\xa1" },
		{ ST7789CMD_DISPON, 100U, 0U, NULL },
		{ ST7789CMD_SLPOUT, 100U, 0U, NULL },
		{ ST7789CMD_TEON, 0U, 0U, NULL }
	};

	for (i = 0U; i < sizeof(initSequence) / sizeof(ST7789Command_t); i++)
	{
		// set command mode
		HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);

		HAL_SPI_Transmit(&hspi1, (uint8_t *)&initSequence[i].command, 1U, 100UL);
		if (initSequence[i].dataSize > 0U)
		{
			// set data mode
			HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

			// send data
			HAL_SPI_Transmit(&hspi1, (uint8_t *)initSequence[i].data, initSequence[i].dataSize, 100UL);
		}

		if (initSequence[i].delayMs > 0U)
		{
			HAL_Delay((uint32_t)initSequence[i].delayMs);
		}
	}
}

static void SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
	uint8_t command;

	caset[1] = (uint8_t)xStart;
	caset[3] = (uint8_t)xEnd;
	raset[1] = (uint8_t)yStart;
	raset[3] = (uint8_t)yEnd;

	// set command mode
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);

	// transmit command
	command = ST7789CMD_CASET;
	HAL_SPI_Transmit(&hspi1, &command, 1U, 100UL);

	// set data mode
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

	// transmit data
	HAL_SPI_Transmit(&hspi1, caset, sizeof(caset), 100UL);

	// set command mode
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);

	// transmit command
	command = ST7789CMD_RASET;
	HAL_SPI_Transmit(&hspi1, &command, 1U, 100UL);

	// set data mode
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);

	// transmit data
	HAL_SPI_Transmit(&hspi1, raset, sizeof(raset), 100UL);

	// set command mode
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET);

	// transmit command
	command = ST7789CMD_RAMWR;
	HAL_SPI_Transmit(&hspi1, &command, 1U, 100UL);

	// set data mode
	HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET);
}

void ILI9341Pixel(uint16_t x, uint16_t y, colour_t colour)
{
	colour_t beColour = __builtin_bswap16(colour);

	if (x >= ST7789_LCD_WIDTH || y >= ST7789_LCD_HEIGHT)
	{
		return;
	}

	SetWindow(x, y, x, y);
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&beColour, 2U, 100UL);
}

void ST7789DrawColourBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData)
{
	uint16_t bytestToWrite;

	SetWindow(x, y, x + width - 1U, y + height - 1U);
	bytestToWrite = width * height * 2U;

	WriteDataDMA(imageData, bytestToWrite);
	WaitForDMAWriteComplete();
}

void ST7789DrawMonoBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData, colour_t fgColour, colour_t bgColour)
{
	colour_t beFgColour = __builtin_bswap16(fgColour);
	colour_t beBgColour = __builtin_bswap16(bgColour);
	colour_t dmaBuffer[DMA_BUFFER_SIZE];
	uint32_t totalBytesToWrite;
	uint32_t bytesToWriteThisTime;
	uint8_t mask = 0x80U;
	uint16_t pixelsWritten = 0U;
	uint8_t i;

	SetWindow(x, y, x + width - 1U, y + height - 1U);
	totalBytesToWrite = (uint32_t)width * (uint32_t)height * (uint32_t)sizeof(colour_t);
	bytesToWriteThisTime = DMA_BUFFER_SIZE * (uint32_t)sizeof(colour_t);

	while (totalBytesToWrite > 0UL)
	{
		if (totalBytesToWrite < bytesToWriteThisTime)
		{
			bytesToWriteThisTime = totalBytesToWrite;
		}
		totalBytesToWrite -= bytesToWriteThisTime;

		for (i = 0U; i < bytesToWriteThisTime / 2UL; i++)
		{
			if ((mask & *imageData) == 0U)
			{
				dmaBuffer[i] = beFgColour;
			}
			else
			{
				dmaBuffer[i] = beBgColour;
			}
			pixelsWritten++;
			mask >>= 1;
			if (mask == 0U)
			{
				mask = 0x80U;
				imageData++;
			}

			if (pixelsWritten % width == 0U && mask != 0x80U)
			{
				mask = 0x80U;
				imageData++;
			}
		}

		WriteDataDMA(&dmaBuffer, bytesToWriteThisTime);
		WaitForDMAWriteComplete();
	}
}

void ST7789FilledRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, colour_t colour)
{
	colour_t dmaBuffer[DMA_BUFFER_SIZE];
	uint8_t i;
	uint32_t totalBytesToWrite;
	uint32_t bytesToWriteThisTime;

	for (i = 0U; i < DMA_BUFFER_SIZE; i++)
	{
		dmaBuffer[i] = __builtin_bswap16(colour);
	}

	SetWindow(x, y, x + width - 1U, y + height - 1U);
	totalBytesToWrite = (uint32_t)width * (uint32_t)height * (uint32_t)sizeof(colour_t);
	bytesToWriteThisTime = DMA_BUFFER_SIZE * (uint16_t)sizeof(colour_t);

	while (totalBytesToWrite > 0UL)
	{
		if (totalBytesToWrite < bytesToWriteThisTime)
		{
			bytesToWriteThisTime = totalBytesToWrite;
		}
		totalBytesToWrite -= bytesToWriteThisTime;

		WriteDataDMA(&dmaBuffer, bytesToWriteThisTime);
		WaitForDMAWriteComplete();
	}
}

static void WriteDataDMA(const void *data, uint16_t length)
{
	txComplete = false;
	HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)data, length);
}

static void WaitForDMAWriteComplete(void)
{
	while (txComplete == false)
	{
	}
}
