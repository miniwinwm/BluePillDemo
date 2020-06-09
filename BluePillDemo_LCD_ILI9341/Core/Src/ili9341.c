#include <stddef.h>
#include "stm32f1xx_hal.h"
#include "ili9341.h"

#define DMA_BUFFER_SIZE 			64U

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_tx;

static volatile bool txComplete;

static void SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
static void WriteDataDMA(const void *data, uint16_t length);
static void WaitForDMAWriteComplete(void);
static void WriteCommand(uint8_t command);
static void WriteData(uint8_t data);

static void WriteCommand(uint8_t command)
{
	HAL_GPIO_WritePin(ILI9341_DC_PORT, ILI9341_DC_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &command, 1U, 100U);
}

static void WriteData(uint8_t data)
{
	HAL_GPIO_WritePin(ILI9341_DC_PORT, ILI9341_DC_PIN, GPIO_PIN_SET);
	HAL_SPI_Transmit(&hspi1, &data, 1U, 100U);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	txComplete = true;
}

void ILI9341Reset(void)
{
	HAL_GPIO_WritePin(ILI9341_RST_PORT, ILI9341_RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(200UL);
	HAL_GPIO_WritePin(ILI9341_RST_PORT, ILI9341_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(200UL);
}

void ILI9341Init(void)
{
	HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);

	WriteCommand(0x01U);
	HAL_Delay(1000UL);
	WriteCommand(0xCBU);
	WriteData(0x39U);
	WriteData(0x2CU);
	WriteData(0x00U);
	WriteData(0x34U);
	WriteData(0x02U);
	WriteCommand(0xCFU);
	WriteData(0x00U);
	WriteData(0xC1U);
	WriteData(0x30U);
	WriteCommand(0xE8U);
	WriteData(0x85U);
	WriteData(0x00U);
	WriteData(0x78U);
	WriteCommand(0xEAU);
	WriteData(0x00U);
	WriteData(0x00U);
	WriteCommand(0xEDU);
	WriteData(0x64U);
	WriteData(0x03U);
	WriteData(0x12U);
	WriteData(0x81U);
	WriteCommand(0xF7U);
	WriteData(0x20U);
	WriteCommand(0xC0U);
	WriteData(0x23U);
	WriteCommand(0xC1U);
	WriteData(0x10U);
	WriteCommand(0xC5U);
	WriteData(0x3EU);
	WriteData(0x28U);
	WriteCommand(0xC7U);
	WriteData(0x86U);
	WriteCommand(0x36U);
	WriteData(0x48U);
	WriteCommand(0x3AU);
	WriteData(0x55U);
	WriteCommand(0xB1U);
	WriteData(0x00U);
	WriteData(0x18U);
	WriteCommand(0xB6U);
	WriteData(0x08U);
	WriteData(0x82U);
	WriteData(0x27U);
	WriteCommand(0xF2U);
	WriteData(0x00U);
	WriteCommand(0x26U);
	WriteData(0x01U);
	WriteCommand(0xE0U);
	WriteData(0x0FU);
	WriteData(0x31U);
	WriteData(0x2BU);
	WriteData(0x0CU);
	WriteData(0x0EU);
	WriteData(0x08U);
	WriteData(0x4EU);
	WriteData(0xF1U);
	WriteData(0x37U);
	WriteData(0x07U);
	WriteData(0x10U);
	WriteData(0x03U);
	WriteData(0x0EU);
	WriteData(0x09U);
	WriteData(0x00U);
	WriteCommand(0xE1U);
	WriteData(0x00U);
	WriteData(0x0EU);
	WriteData(0x14U);
	WriteData(0x03U);
	WriteData(0x11U);
	WriteData(0x07U);
	WriteData(0x31U);
	WriteData(0xC1U);
	WriteData(0x48U);
	WriteData(0x08U);
	WriteData(0x0FU);
	WriteData(0x0CU);
	WriteData(0x31U);
	WriteData(0x36U);
	WriteData(0x0FU);
	WriteCommand(0x11U);
	HAL_Delay(120UL);
	WriteCommand(0x29U);
	WriteCommand(0x36U);
	WriteData(0x48U);
}

static void SetWindow(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
	WriteCommand(0x2AU);
	WriteData(xStart >> 8);
	WriteData(xStart);
	WriteData(xEnd >> 8);
	WriteData(xEnd);

	WriteCommand(0x2BU);
	WriteData(yStart >> 8);
	WriteData(yStart);
	WriteData(yEnd >> 8);
	WriteData(yEnd);

	WriteCommand(0x2CU);

	HAL_GPIO_WritePin(ILI9341_DC_PORT, ILI9341_DC_PIN, GPIO_PIN_SET);
}

void ILI9341Pixel(uint16_t x, uint16_t y, colour_t colour)
{
	colour_t beColour = __builtin_bswap16(colour);

	if (x >= ILI9341_LCD_WIDTH || y >= ILI9341_LCD_HEIGHT)
	{
		return;
	}

	SetWindow(x, y, x, y);

	HAL_SPI_Transmit(&hspi1, (uint8_t *)&beColour, 2U, 100UL);
}

void ILI9341DrawColourBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData)
{
	uint16_t bytestToWrite;

	SetWindow(x, y, x + width - 1U, y + height - 1U);
	bytestToWrite = width * height * 2U;

	WriteDataDMA(imageData, bytestToWrite);
	WaitForDMAWriteComplete();
}

void ILI9341DrawMonoBitmap(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *imageData, colour_t fgColour, colour_t bgColour)
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

void ILI9341FilledRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, colour_t colour)
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
