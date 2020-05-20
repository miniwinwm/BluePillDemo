/** \file max30102.cpp ******************************************************
*
* Project: MAXREFDES117#
* Filename: max30102.cpp
* Description: This module is an embedded controller driver for the MAX30102
*
* Revision History:
*\n 1-18-2016 Rev 01.00 GL Initial release.
*\n 12-22-2017 Rev 02.00 Significantlly modified by Robert Fraczkiewicz
*\n to use Wire library instead of MAXIM's SoftI2C
*
* --------------------------------------------------------------------
*
* This code follows the following naming conventions:
*
* char              ch_pmod_value
* char (array)      s_pmod_s_string[16]
* float             f_pmod_value
* int32_t           n_pmod_value
* int32_t (array)   an_pmod_value[16]
* int16_t           w_pmod_value
* int16_t (array)   aw_pmod_value[16]
* uint16_t          uw_pmod_value
* uint16_t (array)  auw_pmod_value[16]
* uint8_t           uch_pmod_value
* uint8_t (array)   auch_pmod_buffer[16]
* uint32_t          un_pmod_value
* int32_t *         pn_pmod_value
*
* ------------------------------------------------------------------------- */
/*******************************************************************************
* Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/
#include <stdbool.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "max30102.h"

extern I2C_HandleTypeDef hi2c1;

/**
* \brief        Write a value to a MAX30102 register
* \par          Details
*               This function writes a value to a MAX30102 register
*
* \param[in]    uch_addr    - register address
* \param[in]    uch_data    - register data
*/
void maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data)
{
	HAL_I2C_Mem_Write(&hi2c1, I2C_WRITE_ADDR, uch_addr, 1U, &uch_data, 1U, 100U);
}

/**
* \brief        Read a MAX30102 register
* \par          Details
*               This function reads a MAX30102 register
*
* \param[in]    uch_addr    - register address
* \param[out]   puch_data    - pointer that stores the register data
*/
void maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data)
{
	HAL_I2C_Mem_Read(&hi2c1, I2C_READ_ADDR, uch_addr, 1U, puch_data, 1U, 250U);
}

void maxim_max30102_init()
/**
* \brief        Initialize the MAX30102
* \par          Details
*               This function initializes the MAX30102
*
* \param        None
*/
{
	maxim_max30102_write_reg(REG_INTR_ENABLE_1, 0xc0U); 	// INTR setting
	maxim_max30102_write_reg(REG_INTR_ENABLE_2, 0x00U);
	maxim_max30102_write_reg(REG_FIFO_WR_PTR, 0x00U);  		//FIFO_WR_PTR[4:0]
	maxim_max30102_write_reg(REG_OVF_COUNTER, 0x00U);  		//OVF_COUNTER[4:0]
	maxim_max30102_write_reg(REG_FIFO_RD_PTR, 0x00U);  		//FIFO_RD_PTR[4:0]
	maxim_max30102_write_reg(REG_FIFO_CONFIG, 0x4fU);  		//sample avg = 4, fifo rollover=false, fifo almost full = 17
	maxim_max30102_write_reg(REG_MODE_CONFIG, 0x03U);   	//0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
	maxim_max30102_write_reg(REG_SPO2_CONFIG, 0x27U);  		// SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (411uS)
	maxim_max30102_write_reg(REG_LED1_PA, 0x24U);   		//Choose value for ~ 7mA for LED1
	maxim_max30102_write_reg(REG_LED2_PA, 0x24U);   		// Choose value for ~ 7mA for LED2
	maxim_max30102_write_reg(REG_PILOT_PA, 0x7fU);   		// Choose value for ~ 25mA for Pilot LED
}

/**
* \brief        Read a set of samples from the MAX30102 FIFO register
* \par          Details
*               This function reads a set of samples from the MAX30102 FIFO register
*
* \param[out]   *pun_red_led   - pointer that stores the red LED reading data
* \param[out]   *pun_ir_led    - pointer that stores the IR LED reading data
*/
void maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led)
{
  uint32_t un_temp;
  uint8_t uch_temp;
  uint8_t uch_i2c_data[6];

  maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_temp);
  maxim_max30102_read_reg(REG_INTR_STATUS_2, &uch_temp);

  HAL_I2C_Mem_Read(&hi2c1, I2C_READ_ADDR, REG_FIFO_DATA, 1U, uch_i2c_data, 6U, 250U);

  *pun_ir_led = 0UL;
  *pun_red_led = 0UL;

  un_temp = (uint32_t)uch_i2c_data[0];
  un_temp <<= 16;
  *pun_red_led += un_temp;
  un_temp = (uint32_t)uch_i2c_data[1];
  un_temp <<= 8;
  *pun_red_led += un_temp;
  un_temp = (uint32_t)uch_i2c_data[2];
  *pun_red_led += un_temp;
  un_temp = (uint32_t)uch_i2c_data[3];
  un_temp <<= 16;
  *pun_ir_led += un_temp;
  un_temp = (uint32_t)uch_i2c_data[4];
  un_temp <<= 8;
  *pun_ir_led += un_temp;
  un_temp = (uint32_t)uch_i2c_data[5];
  *pun_ir_led += un_temp;
  *pun_red_led &= 0x03FFFF;  	// Mask MSB [23:18]
  *pun_ir_led &= 0x03FFFF;  	// Mask MSB [23:18]

}

/**
* \brief        Reset the MAX30102
* \par          Details
*               This function resets the MAX30102
*
* \param        None
*/
void maxim_max30102_reset()
{
	maxim_max30102_write_reg(REG_MODE_CONFIG, 0x40U);
}

