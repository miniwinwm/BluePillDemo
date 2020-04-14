#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

#include "stm32f1xx_hal.h"

typedef struct
{
	float pressure;
	float temperature;
} pressure_sensor_data_t;

void pressure_sensor_init(I2C_HandleTypeDef *hi2c);
void pressure_sensor_start_measurement(void);
void pressure_sensor_get_measurement(float *pressure_mb, float *temp_c);

#endif
