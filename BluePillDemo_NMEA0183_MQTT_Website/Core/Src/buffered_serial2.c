#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "buffered_serial.h"

static volatile uint8_t receive_buffer[RECEIVE_BUFFER_SIZE];
static uint16_t receive_next_read_position;

extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;

void serial2_init(void)
{
	// kick off the dma receive
	HAL_UART_Receive_DMA(&huart3, (uint8_t *)receive_buffer, RECEIVE_BUFFER_SIZE);
}

uint16_t serial2_received_bytes_waiting(void)
{
	uint16_t receive_next_write_position = RECEIVE_BUFFER_SIZE - (uint16_t)hdma_usart3_rx.Instance->CNDTR;

	if (receive_next_write_position >= receive_next_read_position)
	{
		return receive_next_write_position - receive_next_read_position;
	}
	else
	{
		return RECEIVE_BUFFER_SIZE - (receive_next_read_position - receive_next_write_position);
	}
}

uint16_t serial2_read_data(uint16_t buffer_length, uint8_t *data)
{
	uint16_t i;
	uint16_t receive_next_write_position = RECEIVE_BUFFER_SIZE - (uint16_t)hdma_usart3_rx.Instance->CNDTR;

	for (i = 0U; i < buffer_length; i++)
	{
		if (receive_next_read_position != receive_next_write_position)
		{
			data[i] = receive_buffer[receive_next_read_position];
			receive_next_read_position++;
			if (receive_next_read_position == RECEIVE_BUFFER_SIZE)
			{
				receive_next_read_position = 0U;
			}
		}
		else
		{
			break;
		}
	}

	return i;
}
