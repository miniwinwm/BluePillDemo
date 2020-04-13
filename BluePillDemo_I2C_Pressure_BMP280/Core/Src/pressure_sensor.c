#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f1xx_hal.h"

#define I2C_PRESSURE_SENSOR_ADDRESS			0x76U

static uint16_t dig_T1;
static int16_t dig_T2;
static int16_t dig_T3;
static uint16_t dig_P1;
static int16_t dig_P2;
static int16_t dig_P3;
static int16_t dig_P4;
static int16_t dig_P5;
static int16_t dig_P6;
static int16_t dig_P7;
static int16_t dig_P8;
static int16_t dig_P9;
static int32_t t_fine;
static I2C_HandleTypeDef *press_sensor_hi2c;

static int32_t bmp280_compensate_T_int32(int32_t adc_T);
static uint32_t bmp280_compensate_P_int64(int32_t adc_P);

static int32_t bmp280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1;
	int64_t var2;
	int64_t T;

	var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = ((int64_t)(t_fine * 5L + 128L)) >> 8;

	return T;
}

static uint32_t bmp280_compensate_P_int64(int32_t adc_P)
{
	int64_t varl;
	int64_t var2;
	int64_t p;

	varl = ((int64_t)t_fine) - 128000LL;
	var2 = varl * varl * (int64_t)dig_P6;
	var2 = var2 + ((varl * (int64_t)dig_P5) << 17);
	var2 = var2 + (((int64_t)dig_P4) << 35);
	varl = ((varl * varl * (int64_t)dig_P3) >> 8) + ((varl * (int64_t)dig_P2) << 12);
	varl = (((((int64_t)1LL) << 47) + varl)) * ((int64_t)dig_P1) >> 33;
	if (varl == 0LL)
	{
		return 0UL; // avoid exception caused by division by zero
	}

	p = 1048576LL - (int64_t)adc_P;
	p = (((p << 31) - var2) * 3125LL) / varl;
	varl = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)dig_P8) * p) >> 19;
	p = ((p + varl + var2) >> 8) + (((int64_t)dig_P7) << 4);

	return (uint32_t)p;
}

void pressure_sensor_init(I2C_HandleTypeDef *hi2c)
{
  press_sensor_hi2c = hi2c;

  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x88U, 1U, (uint8_t *)&dig_T1, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x8aU, 1U, (uint8_t *)&dig_T2, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x8cU, 1U, (uint8_t *)&dig_T3, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x8eU, 1U, (uint8_t *)&dig_P1, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x90U, 1U, (uint8_t *)&dig_P2, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x92U, 1U, (uint8_t *)&dig_P3, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x94U, 1U, (uint8_t *)&dig_P4, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x96U, 1U, (uint8_t *)&dig_P5, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x98U, 1U, (uint8_t *)&dig_P6, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x9aU, 1U, (uint8_t *)&dig_P7, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x9cU, 1U, (uint8_t *)&dig_P8, 2U, 100U);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0x9eU, 1U, (uint8_t *)&dig_P9, 2U, 100U);
}

void pressure_sensor_start_measurement_mb(void)
{
  uint8_t data = 0x25U;
  HAL_I2C_Mem_Write(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0xf4U, 1U, &data, 1U, 100U);
}

float pressure_sensor_get_measurement_mb(void)
{
  int32_t adc_T;
  int32_t adc_P;
  uint8_t msb;
  uint8_t lsb;
  uint8_t xlsb;
  uint32_t P;

  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0xfau, 1, &msb, 1U, 1000);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0xfbu, 1, &lsb, 1U, 1000);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0xfcu, 1, &xlsb, 1U, 1000);

  adc_T = (uint32_t)(xlsb >> 4);
  adc_T |= ((uint32_t)lsb) << 4;
  adc_T |= ((uint32_t)msb) << 12;

  (void)bmp280_compensate_T_int32((int32_t)adc_T);

  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0xf7u, 1, &msb, 1U, 1000);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0xf8u, 1, &lsb, 1U, 1000);
  HAL_I2C_Mem_Read(press_sensor_hi2c, I2C_PRESSURE_SENSOR_ADDRESS << 1, 0xf9u, 1, &xlsb, 1U, 1000);

  adc_P = (uint32_t)(xlsb >> 4);
  adc_P |= ((uint32_t)lsb) << 4;
  adc_P |= ((uint32_t)msb) << 12;

  P = bmp280_compensate_P_int64((int32_t)adc_P);
  return (float)P / 25600.0f;
}
