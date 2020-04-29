#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "eeprom.h"

extern SPI_HandleTypeDef hspi2;

static void chip_select(void);
static void chip_deselect(void);

void eeprom_init(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
}

static void chip_select(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
}

static void chip_deselect(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
}

uint16_t eeprom_read_string(uint16_t address, char *buffer, uint16_t max_length)
{
	uint16_t bytes_read = 0U;

	while (true)
	{
		*(buffer + bytes_read) = eeprom_read_byte(address + bytes_read);
		bytes_read++;

		if (*(buffer + bytes_read - 1U) == '\0')
		{
			break;
		}

		if (bytes_read == max_length)
		{
			*(buffer + bytes_read - 1U) = '\0';
			break;
		}
	}

	return bytes_read;
}

void eeprom_write_buf(uint16_t address, uint8_t *buffer, uint16_t length)
{
	uint16_t i;

	for (i = 0U; i < length; i++)
	{
		eeprom_write_byte(address + i, buffer[i]);
	}
}

void eeprom_read_buf(uint16_t address, uint8_t *buffer, uint16_t length)
{
	uint16_t i;

	for (i = 0U; i < length; i++)
	{
		*(buffer + i) = eeprom_read_byte(address + i);
	}
}

void eeprom_write_byte(uint16_t address, uint8_t data)
{
	uint8_t buf[4];

	chip_select();
	buf[0] = 0x06U;
	HAL_SPI_Transmit(&hspi2, buf, 1U, 100U);
	chip_deselect();

	chip_select();
	buf[0] = 0x02U;
	buf[1] = address >> 8;
	buf[2] = address & 0xffU;
	buf[3] = data;
	HAL_SPI_Transmit(&hspi2, buf, 4U, 100U);
	chip_deselect();

	HAL_Delay(3U);
}

uint8_t eeprom_read_byte(uint16_t address)
{
	uint8_t buf_send[4];
	uint8_t buf_receive[4];

	chip_select();
	buf_send[0] = 0x03U;
	buf_send[1] = address >> 8;
	buf_send[2] = address & 0xffU;
	HAL_SPI_TransmitReceive(&hspi2, buf_send, buf_receive, 4U, 100U);
	chip_deselect();

	return buf_receive[3];
}
