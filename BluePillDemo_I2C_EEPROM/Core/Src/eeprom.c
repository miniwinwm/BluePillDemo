#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "eeprom.h"

extern I2C_HandleTypeDef hi2c1;

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
  HAL_I2C_Mem_Write(&hi2c1, 0x50 << 1, address, 2U, &data, 1U, 100U);

  HAL_Delay(5U);
}

uint8_t eeprom_read_byte(uint16_t address)
{
  uint8_t data;

  HAL_I2C_Mem_Read(&hi2c1, 0x50 << 1, address, 2U, &data, 1U, 100U);

  return data;
}
