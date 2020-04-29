#ifndef INC_BUFFERED_SERIAL_H_
#define INC_BUFFERED_SERIAL_H_

#include "stm32f1xx_hal.h"

#define RECEIVE_BUFFER_SIZE     512U
#define RECEIVE_FIFO_SIZE		16U
#define TRANSMIT_BUFFER_SIZE 	512U
#define TRANSMIT_FIFO_SIZE 		16U

/**
 * Call this once before using the rest of the API
 */
void serial_init(void);

/**
 * Write data to UART 0
 *
 * @param length Number of bytes pointed to by data
 * @param data The bytes to send
 * @return How many bytes were transferred into the send buffer which may be less than length if the buffer is full
 */
uint16_t serial_write_data(uint16_t length, uint8_t *data);

/**
 * Read data from UART 0
 *
 * @param length Length of buffer pointed to by data
 * @param data Buffer to contain the read data
 * @return How many bytes were transferred into the buffer which may be less than length if there were not enough data available
 */
uint16_t serial_read_data(uint16_t buffer_length, uint8_t *data);

/**
 * Returns the number of bytes in the receive buffer waiting to be read
 *
 * @return Number of bytes waiting
 */
uint16_t serial_received_bytes_waiting(void);

/**
 * Returns the number of bytes space in the send buffer
 *
 * @return Number of bytes space
 */
uint16_t serial_send_bytes_space(void);

/**
 * Called by the driver not the user
 */
void HAL_UART_IRQHandler_2(UART_HandleTypeDef *huart);

#endif
