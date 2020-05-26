#include <stddef.h>
#include "st7789.h"
#include "stm32f1xx_hal.h"

#define DMA_BUFFER_SIZE 			64U
#define ST7789CMD_NOP               0x00U
#define ST7789CMD_SWRESET           0x01U
#define ST7789CMD_RDDID             0x04U
#define ST7789CMD_RDDST             0x09U
#define ST7789CMD_RDDPM             0x0aU
#define ST7789CMD_RDDMADCTL         0x0bU
#define ST7789CMD_RDDCOLMOD         0x0cU
#define ST7789CMD_RDDIM             0x0dU
#define ST7789CMD_RDDSM             0x0eU
#define ST7789CMD_RDDSDR            0x0fU
#define ST7789CMD_SLPIN             0x10U
#define ST7789CMD_SLPOUT            0x11U
#define ST7789CMD_PTLON             0x12U
#define ST7789CMD_NORON             0x13U
#define ST7789CMD_INVOFF            0x20U
#define ST7789CMD_INVON             0x21U
#define ST7789CMD_GAMSET            0x26U
#define ST7789CMD_DISPOFF           0x28U
#define ST7789CMD_DISPON            0x29U
#define ST7789CMD_CASET             0x2aU
#define ST7789CMD_RASET             0x2bU
#define ST7789CMD_RAMWR             0x2cU
#define ST7789CMD_RAMRD             0x2eU
#define ST7789CMD_PTLAR             0x30U
#define ST7789CMD_VSCRDEF           0x33U
#define ST7789CMD_TEOFF             0x34U
#define ST7789CMD_TEON              0x35U
#define ST7789CMD_MADCTL            0x36U
#define ST7789CMD_VSCRSADD          0x37U
#define ST7789CMD_IDMOFF            0x38U
#define ST7789CMD_IDMON             0x39U
#define ST7789CMD_COLMOD            0x3aU
#define ST7789CMD_RAMWRC            0x3cU
#define ST7789CMD_RAMRDC            0x3eU
#define ST7789CMD_TESCAN            0x44U
#define ST7789CMD_RDTESCAN          0x45U
#define ST7789CMD_WRDISBV           0x51U
#define ST7789CMD_RDDISBV           0x52U
#define ST7789CMD_WRCTRLD           0x53U
#define ST7789CMD_RDCTRLD           0x54U
#define ST7789CMD_WRCACE            0x55U
#define ST7789CMD_RDCABC            0x56U
#define ST7789CMD_WRCABCMB          0x5eU
#define ST7789CMD_RDCABCMB          0x5fU
#define ST7789CMD_RDABCSDR          0x68U
#define ST7789CMD_RDID1             0xdaU
#define ST7789CMD_RDID2             0xdbU
#define ST7789CMD_RDID3             0xdcU
#define ST7789CMD_RAMCTRL           0xb0U
#define ST7789CMD_RGBCTRL           0xb1U
#define ST7789CMD_PORCTRL           0xb2U
#define ST7789CMD_FRCTRL1           0xb3U
#define ST7789CMD_GCTRL             0xb7U
#define ST7789CMD_DGMEN             0xbaU
#define ST7789CMD_VCOMS             0xbbU
#define ST7789CMD_LCMCTRL           0xc0U
#define ST7789CMD_IDSET             0xc1U
#define ST7789CMD_VDVVRHEN          0xc2U
#define ST7789CMD_VRHS              0xc3U
#define ST7789CMD_VDVSET            0xc4U
#define ST7789CMD_VCMOFSET          0xc5U
#define ST7789CMD_FRCTR2            0xc6U
#define ST7789CMD_CABCCTRL          0xc7U
#define ST7789CMD_REGSEL1           0xc8U
#define ST7789CMD_REGSEL2           0xcaU
#define ST7789CMD_PWMFRSEL          0xccU
#define ST7789CMD_PWCTRL1           0xd0U
#define ST7789CMD_VAPVANEN          0xd2U
#define ST7789CMD_CMD2EN            0xdfU
#define ST7789CMD_PVGAMCTRL         0xe0U
#define ST7789CMD_NVGAMCTRL         0xe1U
#define ST7789CMD_DGMLUTR           0xe2U
#define ST7789CMD_DGMLUTB           0xe3U
#define ST7789CMD_GATECTRL          0xe4U
#define ST7789CMD_PWCTRL2           0xe8U
#define ST7789CMD_EQCTRL            0xe9U
#define ST7789CMD_PROMCTRL          0xecU
#define ST7789CMD_PROMEN            0xfaU
#define ST7789CMD_NVMSET            0xfcU
#define ST7789CMD_PROMACT           0xfeU

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

void ST7789Reset(void)
{
	HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(10UL);
	HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(120UL);
}

void ST7789Init(void)
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

void ST7789Pixel(uint16_t x, uint16_t y, colour_t colour)
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
