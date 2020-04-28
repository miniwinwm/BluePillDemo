#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "buffered_serial.h"

static volatile uint16_t bytes_free_in_transmit_buffer = TRANSMIT_BUFFER_SIZE;
static volatile uint8_t transmit_fifo[TRANSMIT_FIFO_SIZE];
static volatile uint8_t transmit_buffer[TRANSMIT_BUFFER_SIZE];
static volatile uint16_t transmit_buffer_next_read_position;
static volatile uint16_t transmit_buffer_next_write_position;
static volatile uint8_t receive_fifo[RECEIVE_FIFO_SIZE];
static volatile uint8_t receive_buffer[RECEIVE_BUFFER_SIZE];
static volatile uint16_t receive_next_write_position;
static volatile uint16_t receive_next_read_position;
static volatile uint16_t bytes_available_in_receive_buffer;
static volatile uint8_t bytes_aleady_copied_out_of_receive_fifo;

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;

uint16_t serial_received_bytes_waiting(void)
{
	return bytes_available_in_receive_buffer;
}

uint16_t serial_send_bytes_space(void)
{
	return bytes_free_in_transmit_buffer;
}

void serial_init(void)
{
	// enable receive idle interrupt
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

	// kick off the dma receive
	HAL_UART_Receive_DMA(&huart1, (uint8_t *)receive_fifo, RECEIVE_FIFO_SIZE);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uint16_t byte_to_transfer_to_transmit_fifo;
	uint8_t i;

	__HAL_DMA_DISABLE(&hdma_usart1_tx);

	byte_to_transfer_to_transmit_fifo = TRANSMIT_BUFFER_SIZE - bytes_free_in_transmit_buffer;
	if (byte_to_transfer_to_transmit_fifo > TRANSMIT_FIFO_SIZE)
	{
		byte_to_transfer_to_transmit_fifo = TRANSMIT_FIFO_SIZE;
	}
	bytes_free_in_transmit_buffer += byte_to_transfer_to_transmit_fifo;

	if (byte_to_transfer_to_transmit_fifo > 0U)
	{
		for (i = 0U; i < byte_to_transfer_to_transmit_fifo; i++)
		{
			transmit_fifo[i] = transmit_buffer[transmit_buffer_next_read_position];
			transmit_buffer_next_read_position++;
			if (transmit_buffer_next_read_position == TRANSMIT_BUFFER_SIZE)
			{
				transmit_buffer_next_read_position = 0U;
			}
		}

		HAL_UART_Transmit_DMA(&huart1, (uint8_t *)transmit_fifo, byte_to_transfer_to_transmit_fifo);
	}
}

uint16_t serial_write_data(uint16_t length, uint8_t *data)
{
	uint16_t i;
	uint16_t bytes_to_transfer_to_transmit_buffer;
	uint16_t byte_to_transfer_to_transmit_fifo;

	__HAL_DMA_DISABLE_IT(&hdma_usart1_tx, DMA_IT_TC);

	if (length > bytes_free_in_transmit_buffer)
	{
		bytes_to_transfer_to_transmit_buffer = bytes_free_in_transmit_buffer;
	}
	else
	{
	    bytes_to_transfer_to_transmit_buffer = length;
	}
	bytes_free_in_transmit_buffer -= bytes_to_transfer_to_transmit_buffer;

	__HAL_DMA_ENABLE_IT(&hdma_usart1_tx, DMA_IT_TC);

	for (i = 0U; i < bytes_to_transfer_to_transmit_buffer; i++)
	{
		transmit_buffer[transmit_buffer_next_write_position] = data[i];
		transmit_buffer_next_write_position++;
		if (transmit_buffer_next_write_position == TRANSMIT_BUFFER_SIZE)
		{
			transmit_buffer_next_write_position = 0U;
		}
	}

	if ((hdma_usart1_tx.Instance->CCR & DMA_CCR_EN) == 0UL && bytes_to_transfer_to_transmit_buffer > 0U)
	{
		byte_to_transfer_to_transmit_fifo = TRANSMIT_BUFFER_SIZE - bytes_free_in_transmit_buffer;
		if (byte_to_transfer_to_transmit_fifo > TRANSMIT_FIFO_SIZE)
		{
			byte_to_transfer_to_transmit_fifo = TRANSMIT_FIFO_SIZE;
		}
		bytes_free_in_transmit_buffer += byte_to_transfer_to_transmit_fifo;

		for (i = 0U; i < byte_to_transfer_to_transmit_fifo; i++)
		{
			transmit_fifo[i] = transmit_buffer[transmit_buffer_next_read_position];
			transmit_buffer_next_read_position++;
			if (transmit_buffer_next_read_position == TRANSMIT_BUFFER_SIZE)
			{
				transmit_buffer_next_read_position = 0U;
			}
		}

		HAL_UART_Transmit_DMA(&huart1, (uint8_t *)transmit_fifo, byte_to_transfer_to_transmit_fifo);

	}

	return bytes_to_transfer_to_transmit_buffer;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t i;

	for (i = bytes_aleady_copied_out_of_receive_fifo; i < RECEIVE_FIFO_SIZE; i++)
	{
		receive_buffer[receive_next_write_position] = receive_fifo[i];
		receive_next_write_position++;
		bytes_available_in_receive_buffer++;
		if (receive_next_write_position == RECEIVE_BUFFER_SIZE)
		{
			receive_next_write_position = 0U;
		}
	}

	bytes_aleady_copied_out_of_receive_fifo = 0U;
}

uint16_t serial_read_data(uint16_t buffer_length, uint8_t *data)
{
	uint16_t bytes_to_read_from_receive_buffer;
	uint16_t i;

	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_TC);

	if (bytes_available_in_receive_buffer < buffer_length)
	{
		bytes_to_read_from_receive_buffer = bytes_available_in_receive_buffer;
	}
	else
	{
		bytes_to_read_from_receive_buffer = buffer_length;
	}

	bytes_available_in_receive_buffer -= bytes_to_read_from_receive_buffer;

	__HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_TC);

	for (i = 0U; i < bytes_to_read_from_receive_buffer; i++)
	{
		data[i] = receive_buffer[receive_next_read_position];
		receive_next_read_position++;
		if (receive_next_read_position == RECEIVE_BUFFER_SIZE)
		{
			receive_next_read_position = 0U;
		}
	}

	return bytes_to_read_from_receive_buffer;
}

void HAL_UART_IRQHandler_2(UART_HandleTypeDef *huart)
{
	uint8_t i;
	uint8_t bytes_available_in_receive_fifo;

	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE) == true)
	{
    	bytes_available_in_receive_fifo = RECEIVE_FIFO_SIZE -
    										hdma_usart1_rx.Instance->CNDTR -
											bytes_aleady_copied_out_of_receive_fifo;
    	bytes_available_in_receive_buffer += bytes_available_in_receive_fifo;

    	for (i = 0U; i < bytes_available_in_receive_fifo; i++)
    	{
    		receive_buffer[receive_next_write_position] = receive_fifo[i + bytes_aleady_copied_out_of_receive_fifo];
    		receive_next_write_position++;
    		if (receive_next_write_position == RECEIVE_BUFFER_SIZE)
    		{
    			receive_next_write_position = 0U;
    		}
    	}

    	bytes_aleady_copied_out_of_receive_fifo += bytes_available_in_receive_fifo;

    	// clear the IDLE flag in SR register according to section 27.6.1 of ST document RM0008
    	(void)huart->Instance->SR;
    	(void)huart->Instance->DR;
	}
}
