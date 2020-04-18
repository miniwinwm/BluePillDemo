/******************************************************************************
*
* File: nrf24l01.c
*
* Copyright S. Brennen Ball, 2006-2007
*
* The author provides no guarantees, warantees, or promises, implied or
*	otherwise.  By using this software you agree to indemnify the author
* 	of any damages incurred by using it.
*
*****************************************************************************/

#include "stm32f1xx_hal.h"
#include "nrf24l01.h"

extern SPI_HandleTypeDef hspi2;
static bool csn_active;
static bool ce_active;

//Arguments except opt_rx_standby_mode fill the actual register they are named
//  after. Registers that do not need to be initialized are not included here.
//The argument opt_rx_active_mode is only used if the user is initializing the
//  24L01 as a receiver.  If the argument is false, the receiver will remain in
//  standby mode and not monitor for packets.  If the argument is true, the CE
//  pin will be set and the 24L01 will monitor for packets.  In TX mode, the value
//  of this argument is insignificant.
//If the user wants to leave any 1-byte register in its default state, simply put
//  as that register's argument nrf24l01_<reg>_DEFAULT_VAL, where <reg> is the register
//  name.
//If the user wants to leave any of the 5-byte registers RX_ADDR_P0, RX_ADDR_P1, or
//  TX_ADDR in its default state, simply put NULL in the argument for that address value.
void nrf24l01_initialize(unsigned char config,
						 unsigned char opt_rx_active_mode,
						 unsigned char en_aa,
						 unsigned char en_rxaddr,
						 unsigned char setup_aw,
						 unsigned char setup_retr,
						 unsigned char rf_ch,
						 unsigned char rf_setup,
						 unsigned char * rx_addr_p0,
						 unsigned char * rx_addr_p1,
						 unsigned char rx_addr_p2,
						 unsigned char rx_addr_p3,
						 unsigned char rx_addr_p4,
						 unsigned char rx_addr_p5,
						 unsigned char * tx_addr,
						 unsigned char rx_pw_p0,
						 unsigned char rx_pw_p1,
						 unsigned char rx_pw_p2,
						 unsigned char rx_pw_p3,
						 unsigned char rx_pw_p4,
						 unsigned char rx_pw_p5)
{
	unsigned char data[5];

	data[0] = en_aa;
	nrf24l01_write_register(nrf24l01_EN_AA, data, 1);

	data[0] = en_rxaddr;
	nrf24l01_write_register(nrf24l01_EN_RXADDR, data, 1);

	data[0] = setup_aw;
	nrf24l01_write_register(nrf24l01_SETUP_AW, data, 1);

	data[0] = setup_retr;
	nrf24l01_write_register(nrf24l01_SETUP_RETR, data, 1);

	data[0] = rf_ch;
	nrf24l01_write_register(nrf24l01_RF_CH, data, 1);

	data[0] = rf_setup;
	nrf24l01_write_register(nrf24l01_RF_SETUP, data, 1);

	if(rx_addr_p0 != NULL)
		nrf24l01_set_rx_addr(rx_addr_p0, 5, 0);
	else
	{
		data[0] = nrf24l01_RX_ADDR_P0_B0_DEFAULT_VAL;
		data[1] = nrf24l01_RX_ADDR_P0_B1_DEFAULT_VAL;
		data[2] = nrf24l01_RX_ADDR_P0_B2_DEFAULT_VAL;
		data[3] = nrf24l01_RX_ADDR_P0_B3_DEFAULT_VAL;
		data[4] = nrf24l01_RX_ADDR_P0_B4_DEFAULT_VAL;

		nrf24l01_set_rx_addr(data, 5, 0);
	}

	if(rx_addr_p1 != NULL)
		nrf24l01_set_rx_addr(rx_addr_p1, 5, 1);
	else
	{
		data[0] = nrf24l01_RX_ADDR_P1_B0_DEFAULT_VAL;
		data[1] = nrf24l01_RX_ADDR_P1_B1_DEFAULT_VAL;
		data[2] = nrf24l01_RX_ADDR_P1_B2_DEFAULT_VAL;
		data[3] = nrf24l01_RX_ADDR_P1_B3_DEFAULT_VAL;
		data[4] = nrf24l01_RX_ADDR_P1_B4_DEFAULT_VAL;

		nrf24l01_set_rx_addr(data, 5, 1);
	}

	data[0] = rx_addr_p2;
	nrf24l01_set_rx_addr(data, 1, 2);

	data[0] = rx_addr_p3;
	nrf24l01_set_rx_addr(data, 1, 3);

	data[0] = rx_addr_p4;
	nrf24l01_set_rx_addr(data, 1, 4);

	data[0] = rx_addr_p5;
	nrf24l01_set_rx_addr(data, 1, 5);

	if(tx_addr != NULL)
		nrf24l01_set_tx_addr(tx_addr, 5);
	else
	{
		data[0] = nrf24l01_TX_ADDR_B0_DEFAULT_VAL;
		data[1] = nrf24l01_TX_ADDR_B1_DEFAULT_VAL;
		data[2] = nrf24l01_TX_ADDR_B2_DEFAULT_VAL;
		data[3] = nrf24l01_TX_ADDR_B3_DEFAULT_VAL;
		data[4] = nrf24l01_TX_ADDR_B4_DEFAULT_VAL;

		nrf24l01_set_tx_addr(data, 5);
	}

	data[0] = rx_pw_p0;
	nrf24l01_write_register(nrf24l01_RX_PW_P0, data, 1);

	data[0] = rx_pw_p1;
	nrf24l01_write_register(nrf24l01_RX_PW_P1, data, 1);

	data[0] = rx_pw_p2;
	nrf24l01_write_register(nrf24l01_RX_PW_P2, data, 1);

	data[0] = rx_pw_p3;
	nrf24l01_write_register(nrf24l01_RX_PW_P3, data, 1);

	data[0] = rx_pw_p4;
	nrf24l01_write_register(nrf24l01_RX_PW_P4, data, 1);

	data[0] = rx_pw_p5;
	nrf24l01_write_register(nrf24l01_RX_PW_P5, data, 1);

	if((config & nrf24l01_CONFIG_PWR_UP) != 0)
		nrf24l01_power_up_param(opt_rx_active_mode, config);
	else
		nrf24l01_power_down_param(config);
}

//powers up the 24L01 with all necessary delays
//this function takes the existing contents of the CONFIG register and sets the PWR_UP
//the argument rx_active_mode is only used if the user is setting up the
//  24L01 as a receiver.  If the argument is false, the receiver will remain in
//  standby mode and not monitor for packets.  If the argument is true, the CE
//  pin will be set and the 24L01 will monitor for packets.  In TX mode, the value
//  of this argument is insignificant.
//note: if the read value of the CONFIG register already has the PWR_UP bit set, this function
//  exits in order to not make an unecessary register write.
void nrf24l01_power_up(bool rx_active_mode)
{
	unsigned char config;

	nrf24l01_read_register(nrf24l01_CONFIG, &config, 1);

	if((config & nrf24l01_CONFIG_PWR_UP) != 0)
		return;

	config |= nrf24l01_CONFIG_PWR_UP;

	nrf24l01_write_register(nrf24l01_CONFIG, &config, 1);

	HAL_Delay(2U);

	if((config & nrf24l01_CONFIG_PRIM_RX) == 0)
		nrf24l01_clear_ce();
	else
	{
		if(rx_active_mode != false)
			nrf24l01_set_ce();
		else
			nrf24l01_clear_ce();
	}
}

//powers up the 24L01 with all necessary delays
//this function allows the user to set the contents of the CONFIG register, but the function
//  sets the PWR_UP bit in the CONFIG register, so the user does not need to.
//the argument rx_active_mode is only used if the user is setting up the
//  24L01 as a receiver.  If the argument is false, the receiver will remain in
//  standby mode and not monitor for packets.  If the argument is true, the CE
//  pin will be set and the 24L01 will monitor for packets.  In TX mode, the value
//  of this argument is insignificant.
void nrf24l01_power_up_param(bool rx_active_mode, unsigned char config)
{
	config |= nrf24l01_CONFIG_PWR_UP;

	nrf24l01_write_register(nrf24l01_CONFIG, &config, 1);

	HAL_Delay(2U);

	if((config & nrf24l01_CONFIG_PRIM_RX) == 0)
		nrf24l01_clear_ce();
	else
	{
		if(rx_active_mode != false)
			nrf24l01_set_ce();
		else
			nrf24l01_clear_ce();
	}
}

//powers down the 24L01
//this function takes the existing contents of the CONFIG register and simply
//  clears the PWR_UP bit in the CONFIG register.
//note: if the read value of the CONFIG register already has the PWR_UP bit cleared, this
//  function exits in order to not make an unecessary register write.
void nrf24l01_power_down()
{
	unsigned char config;

	nrf24l01_read_register(nrf24l01_CONFIG, &config, 1);

	if((config & nrf24l01_CONFIG_PWR_UP) == 0)
		return;

	config &= (~nrf24l01_CONFIG_PWR_UP);

	nrf24l01_write_register(nrf24l01_CONFIG, &config, 1);

	nrf24l01_clear_ce();
}

//powers down the 24L01
//this function allows the user to set the contents of the CONFIG register, but the function
//  clears the PWR_UP bit in the CONFIG register, so the user does not need to.
void nrf24l01_power_down_param(unsigned char config)
{
	config &= (~nrf24l01_CONFIG_PWR_UP);

	nrf24l01_write_register(nrf24l01_CONFIG, &config, 1);

	nrf24l01_clear_ce();
}

//executes the W_REGISTER SPI operation
//unsigned char regnumber indicates the register number assigned by the nrf24l01 specification.
//  For regnumber values, see section titled "register definitions" in nrf24l01.h.
//unsigned char * data should be of size 1 for all register writes except for RX_ADDR_P0, RX_ADDR_P1,
//	and TX_ADDR.  The size of data should be set according to the user-specified size of the address
//  length for the register the address is being sent to.
//unsigned int len is always the size of unsigned char * data.  For example, if data is declared as
//  data[6], len should equal 6.
//returns the value of the STATUS register
unsigned char nrf24l01_write_register(unsigned char regnumber, unsigned char * data, unsigned int len)
{
	return nrf24l01_execute_command(nrf24l01_W_REGISTER | (regnumber & nrf24l01_W_REGISTER_DATA), data, len, false);
}

//executes the R_REGISTER SPI operation
//unsigned char regnumber indicates the register number assigned by the nrf24l01 specification.
//  For regnumber values, see section titled "register definitions" in nrf24l01.h.
//unsigned char * data should be of size 1 for all register writes except for RX_ADDR_P0, RX_ADDR_P1,
//	and TX_ADDR.  The size of data should be set according to the user-specified size of the address
//  length for the register the address is being read from.
//unsigned int len is always the size of unsigned char * data.  For example, if data is declared as
//  data[6], len = 6.
//returns the value of the STATUS register
unsigned char nrf24l01_read_register(unsigned char regnumber, unsigned char * data, unsigned int len)
{
	return nrf24l01_execute_command(regnumber & nrf24l01_R_REGISTER_DATA, data, len, true);
}

//sets the TX address in the TX_ADDR register
//unsigned char * address is the actual address to be used.  It should be sized
//	according to the tx_addr length specified to the nrf24l01.
//unsigned int len is the length of the address.  Its value should be specified
//	according to the tx_addr length specified to the nrf24l01.
void nrf24l01_set_tx_addr(unsigned char * address, unsigned int len)
{
	nrf24l01_write_register(nrf24l01_TX_ADDR, address, len);
}

//sets the RX address in the RX_ADDR register that is offset by rxpipenum
//unsigned char * address is the actual address to be used.  It should be sized
//	according to the rx_addr length that is being filled.
//unsigned int len is the length of the address.  Its value should be specified
//	according to the rx_addr length specified to the nrf24l01.
//unsigned char rxpipenum is the pipe number (zero to five) whose address is being
//	specified.  If an invalid address (greater than five) is supplied, the function
//  does nothing.
void nrf24l01_set_rx_addr(unsigned char * address, unsigned int len, unsigned char rxpipenum)
{
	if(rxpipenum > 5)
		return;

	nrf24l01_write_register(nrf24l01_RX_ADDR_P0 + rxpipenum, address, len);
}

//clear all interrupts in the status register
void nrf24l01_irq_clear_all()
{
	unsigned char data = nrf24l01_STATUS_RX_DR | nrf24l01_STATUS_TX_DS | nrf24l01_STATUS_MAX_RT;

	nrf24l01_write_register(nrf24l01_STATUS, &data, 1);
}

//low-level spi send function for library use
//the user should not call this function directly, but rather use one of the 8 SPI data instructions
unsigned char nrf24l01_execute_command(unsigned char instruction, unsigned char * data, unsigned int len, bool copydata)
{
	unsigned char status;

	nrf24l01_clear_csn();

	status = instruction;
	nrf24l01_spi_send_read(&status, 1, true);
	nrf24l01_spi_send_read(data, len, copydata);

	nrf24l01_set_csn();

	return status;
}

//low-level spi send function for library use
//the user should not call this function directly, but rather use one of the 8 SPI data instructions
void nrf24l01_spi_send_read(unsigned char * data, unsigned int len, bool copydata)
{
	unsigned int count;
	unsigned char tempbyte;

	for(count = 0; count < len; count++)
	{
		if(copydata != false)
			HAL_SPI_TransmitReceive(&hspi2, &data[count], &data[count], 1U, 100U);
		else
		{
			uint8_t dummy;
			tempbyte = data[count];
			HAL_SPI_TransmitReceive(&hspi2, &tempbyte, &dummy, 1U, 100U);
		}
	}
}

//initializes the 24L01 to all default values except the PWR_UP and PRIM_RX bits
//this function also disables the auto-ack feature on the chip (EN_AA register is 0)
//bool rx is true if the device should be a receiver and false if it should be
//  a transmitter.
//unsigned char payload_width is the payload width for pipe 0.  All other pipes
//  are left in their default (disabled) state.
//bool enable_auto_ack controls the auto ack feature on pipe 0.  If true, auto-ack will
//  be enabled.  If false, auto-ack is disabled.
void nrf24l01_initialize_debug(bool rx, unsigned char p0_payload_width, bool enable_auto_ack)
{
	unsigned char config;
	unsigned char en_aa;

	config = nrf24l01_CONFIG_DEFAULT_VAL | nrf24l01_CONFIG_PWR_UP;

	if(enable_auto_ack != false)
		en_aa = nrf24l01_EN_AA_ENAA_P0;
	else
		en_aa = nrf24l01_EN_AA_ENAA_NONE;

	if(rx == true)
		config = config | nrf24l01_CONFIG_PRIM_RX;

	nrf24l01_initialize(config,
						true,
						en_aa,
						nrf24l01_EN_RXADDR_DEFAULT_VAL,
						nrf24l01_SETUP_AW_DEFAULT_VAL,
						nrf24l01_SETUP_RETR_DEFAULT_VAL,
						nrf24l01_RF_CH_DEFAULT_VAL,
						nrf24l01_RF_SETUP_DEFAULT_VAL,
						NULL,
						NULL,
						nrf24l01_RX_ADDR_P2_DEFAULT_VAL,
						nrf24l01_RX_ADDR_P3_DEFAULT_VAL,
						nrf24l01_RX_ADDR_P4_DEFAULT_VAL,
						nrf24l01_RX_ADDR_P5_DEFAULT_VAL,
						NULL,
						p0_payload_width,
						nrf24l01_RX_PW_P1_DEFAULT_VAL,
						nrf24l01_RX_PW_P2_DEFAULT_VAL,
						nrf24l01_RX_PW_P3_DEFAULT_VAL,
						nrf24l01_RX_PW_P4_DEFAULT_VAL,
						nrf24l01_RX_PW_P5_DEFAULT_VAL);
}

//initializes only the CONFIG register and pipe 0's payload width
//the primary purpose of this function is to allow users with microcontrollers with
//  extremely small program memories to still be able to init their 24L01.  This code
//  should have a smaller footprint than the above init functions.
//when using this method, the 24L01 MUST have its default configuration loaded
//  in all registers to work.  It is recommended that the device be reset or
//  have its power cycled immediately before this code is run.
//in normal circumstances, the user should use nrf24l01_initialize() rather than this
//  function, since this function does not set all of the register values.
void nrf24l01_initialize_debug_lite(bool rx, unsigned char p0_payload_width)
{
	unsigned char config;

	config = nrf24l01_CONFIG_DEFAULT_VAL;

	if(rx != false)
		config |= nrf24l01_CONFIG_PRIM_RX;

	nrf24l01_write_register(nrf24l01_RX_PW_P0, &p0_payload_width, 1);
	nrf24l01_power_up_param(true, config);
}

//sets up the 24L01 as a receiver with all necessary delays
//this function takes the existing contents of the CONFIG register and sets the PRIM_RX
//  bit in the CONFIG register.
//if the argument rx_active_mode is false, the receiver will remain in standby mode
//  and not monitor for packets.  If the argument is true, the CE pin will be set
//  and the 24L01 will monitor for packets.
//note: if the read value of the CONFIG register already has the PRIM_RX bit set, this function
//  exits in order to not make an unecessary register write.
void nrf24l01_set_as_rx(bool rx_active_mode)
{
	unsigned char config;

	nrf24l01_read_register(0, &config, 1);

	if((config & nrf24l01_CONFIG_PRIM_RX) != 0)
		return;

	config |= nrf24l01_CONFIG_PRIM_RX;

	nrf24l01_write_register(nrf24l01_CONFIG, &config, 1);

	if(rx_active_mode != false)
		nrf24l01_set_ce();
	else
		nrf24l01_clear_ce();
}

//sets up the 24L01 as a receiver with all necessary delays
//this function allows the user to set the contents of the CONFIG register, but the function
//  sets the PRIM_RX bit in the CONFIG register, so the user does not need to.
//if the argument rx_active_mode is false, the receiver will remain in standby mode
//  and not monitor for packets.  If the argument is true, the CE pin will be set
//  and the 24L01 will monitor for packets.
void nrf24l01_set_as_rx_param(bool rx_active_mode, unsigned char config)
{
	config |= nrf24l01_CONFIG_PRIM_RX;

	if((config & nrf24l01_CONFIG_PWR_UP) != 0)
		nrf24l01_power_up_param(rx_active_mode, config);
	else
		nrf24l01_power_down_param(config);
}

//takes a 24L01 that is already in RX standby mode and puts it in active RX mode
void nrf24l01_rx_standby_to_active()
{
	nrf24l01_set_ce();
}

//takes a 24L01 that is already in active RX mode and puts it in RX standy mode
void nrf24l01_rx_active_to_standby()
{
	nrf24l01_clear_ce();
}

//sets up the 24L01 as a transmitter
//this function takes the existing contents of the CONFIG register and simply
//  clears the PRIM_RX bit in the CONFIG register.
//note: if the read value of the CONFIG register already has the PRIM_RX bit cleared, this
//  function exits in order to not make an unecessary register write.
void nrf24l01_set_as_tx()
{
	unsigned char config;

	nrf24l01_read_register(nrf24l01_CONFIG, &config, 1);

	if((config & nrf24l01_CONFIG_PRIM_RX) == 0)
		return;

	config &= (~nrf24l01_CONFIG_PRIM_RX);

	nrf24l01_write_register(nrf24l01_CONFIG, &config, 1);

	nrf24l01_clear_ce();
}

//sets up the 24L01 as a transmitter
//this function allows the user to set the contents of the CONFIG register, but the function
//  clears the PRIM_RX bit in the CONFIG register, so the user does not need to.
void nrf24l01_set_as_tx_param(unsigned char config)
{
	config &= ~(nrf24l01_CONFIG_PRIM_RX);

	if((config & nrf24l01_CONFIG_PWR_UP) != 0)
		nrf24l01_power_up_param(false, config);
	else
		nrf24l01_power_down_param(config);
}

//executes the W_TX_PAYLOAD operation
//unsigned char * data is the actual payload to be sent to the nrf24l01.
//unsigned int len is the length of the payload being sent (this should be sized
//	according to the payload length specified by the receiving nrf24l01).
//if bool transmit is true, the nrf24l01 immediately transmits the data in the payload.
//	if false, the user must use the nrf24l01_transmit() function to send the payload.
//returns the value of the STATUS register
unsigned char nrf24l01_write_tx_payload(unsigned char * data, unsigned int len, bool transmit)
{
	unsigned char status;

	status = nrf24l01_execute_command(nrf24l01_W_TX_PAYLOAD, data, len, false);

	if(transmit == true)
		nrf24l01_transmit();

	return status;
}

//executes the FLUSH_TX SPI operation
//this funciton empties the contents of the TX FIFO
//returns the value of the STATUS register
unsigned char nrf24l01_flush_tx()
{
	return nrf24l01_execute_command(nrf24l01_FLUSH_TX, NULL, 0, true);
}

//executes the FLUSH_RX SPI operation
//this funciton empties the contents of the RX FIFO
//returns the value of the STATUS register
unsigned char nrf24l01_flush_rx()
{
	return nrf24l01_execute_command(nrf24l01_FLUSH_RX, NULL, 0, true);
}

//executes the REUSE_TX_PL SPI operation
//this funciton allows the user to constantly send a packet repeatedly when issued.
//returns the value of the STATUS register
unsigned char nrf24l01_reuse_tx_pl()
{
	return nrf24l01_execute_command(nrf24l01_REUSE_TX_PL, NULL, 0, true);
}

//executes the FLUSH_TX SPI operation
//this funciton does nothing
//returns the value of the STATUS register
unsigned char nrf24l01_nop()
{
	return nrf24l01_execute_command(nrf24l01_NOP, NULL, 0, true);
}

//sets the RX payload width on the pipe offset by rxpipenum
//unsigned char payloadwidth is the length of the payload for the pipe referenced in
//  rxpipenum.  It must be less than or equal to 32.  If an invalid payload width is
//  specified, the function does nothing.
//unsigned char rxpipenum is the pipe number (zero to five) whose address is being
//	specified.  If an invalid address (greater than five) is supplied, the function
//  does nothing.
void nrf24l01_set_rx_pw(unsigned char payloadwidth, unsigned char rxpipenum)
{
	if((rxpipenum > 5) || (payloadwidth > 32))
		return;

	nrf24l01_write_register(nrf24l01_RX_PW_P0 + rxpipenum, &payloadwidth, 1);
}

//gets the RX payload width on the pipe offset by rxpipenum
//unsigned char rxpipenum is the pipe number (zero to five) whose address is being
//	specified.  If an invalid address (greater than five) is supplied, the function
//  does nothing.
unsigned char nrf24l01_get_rx_pw(unsigned char rxpipenum)
{
	unsigned char data;

	if((rxpipenum > 5))
		return 0;

	nrf24l01_read_register(nrf24l01_RX_PW_P0 + rxpipenum, &data, 1);

	return data;
}

//returns the value of the CONFIG register
unsigned char nrf24l01_get_config()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_CONFIG, &data, 1);

	return data;
}

//sets the value of the CONFIG register
void nrf24l01_set_config(unsigned char config)
{
	nrf24l01_write_register(nrf24l01_CONFIG, &config, 1);
}

//returns the current RF channel in RF_CH register
unsigned char nrf24l01_get_rf_ch()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_RF_CH, &data, 1);

	return data;
}

//unsigned char channel is the channel to be changed to.
void nrf24l01_set_rf_ch(unsigned char channel)
{
	unsigned char data;

	data = channel & ~nrf24l01_RF_CH_RESERVED;

	nrf24l01_write_register(nrf24l01_RF_CH, &data, 1);
}

//returns the value of the OBSERVE_TX register
unsigned char nrf24l01_get_observe_tx()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_OBSERVE_TX, &data, 1);

	return data;
}

//returns the current PLOS_CNT value in OBSERVE_TX register
unsigned char nrf24l01_get_plos_cnt()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_OBSERVE_TX, &data, 1);

	return ((data & nrf24l01_OBSERVE_TX_PLOS_CNT) >> 4);
}

//clears the PLOS_CNT field of the OBSERVE_TX register
//this function makes a read of the current value of RF_CH and
//  simply writes it back to the register, clearing PLOS_CNT
void nrf24l01_clear_plos_cnt()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_RF_CH, &data, 1);
	nrf24l01_write_register(nrf24l01_RF_CH, &data, 1);
}

//clears the PLOS_CNT field of the OBSERVE_TX register
//this function allows the user to set the RF_CH register by using
//  the argument in the function during the PLOS_CNT clearing process
void nrf24l01_clear_plos_cnt_param(unsigned char rf_ch)
{
	nrf24l01_write_register(nrf24l01_RF_CH, &rf_ch, 1);
}

//returns the current ARC_CNT value in OBSERVE_TX register
unsigned char nrf24l01_get_arc_cnt()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_OBSERVE_TX, &data, 1);

	return (data & nrf24l01_OBSERVE_TX_ARC_CNT);
}

//returns true if auto-ack is enabled on the pipe that is offset by rxpipenum
//unsigned char rxpipenum is the pipe number (zero to five) whose address is being
//	specified.  If an invalid address (greater than five) is supplied, the function
//  returns false.
bool nrf24l01_aa_enabled(unsigned char rxpipenum)
{
	unsigned char data;

	if(rxpipenum > 5)
		return false;

	nrf24l01_read_register(nrf24l01_EN_AA, &data, 1);

	return (data & (0x01 << rxpipenum));
}

//enables auto-ack is enabled on the pipe that is offset by rxpipenum
//unsigned char rxpipenum is the pipe number (zero to five) whose address is being
//	does nothing.
void nrf24l01_aa_enable(unsigned char rxpipenum)
{
	unsigned char data;

	if(rxpipenum > 5)
		return;

	nrf24l01_read_register(nrf24l01_EN_AA, &data, 1);

	if((data & (0x01 << rxpipenum)) != 0)
		return;

	data |= 0x01 << rxpipenum;

	nrf24l01_write_register(nrf24l01_EN_AA, &data, 1);
}

//disables auto-ack is enabled on the pipe that is offset by rxpipenum
//unsigned char rxpipenum is the pipe number (zero to five) whose address is being
//	does nothing.
void nrf24l01_aa_disable(unsigned char rxpipenum)
{
	unsigned char data;

	if(rxpipenum > 5)
		return;

	nrf24l01_read_register(nrf24l01_EN_AA, &data, 1);

	if((data & (0x01 << rxpipenum)) == 0)
		return;

	data &= ~(0x01 << rxpipenum);

	nrf24l01_write_register(nrf24l01_EN_AA, &data, 1);
}

//returns true if the pipe is enabled that is offset by rxpipenum
//unsigned char rxpipenum is the pipe number (zero to five) whose address is being
//	specified.  If an invalid address (greater than five) is supplied, the function
//  returns false.
bool nrf24l01_rx_pipe_enabled(unsigned char rxpipenum)
{
	unsigned char data;

	if((rxpipenum > 5))
		return false;

	nrf24l01_read_register(nrf24l01_EN_RXADDR, &data, 1);

	return (data & (0x01 << rxpipenum));
}

//enables the pipe that is offset by rxpipenum
//unsigned char rxpipenum is the pipe number (zero to five) whose address is being
//	specified.  If an invalid address (greater than five) is supplied, the function
//  does nothing.
void nrf24l01_rx_pipe_enable(unsigned char rxpipenum)
{
	unsigned char data;

	if(rxpipenum > 5)
		return;

	nrf24l01_read_register(nrf24l01_EN_RXADDR, &data, 1);

	if((data & (0x01 << rxpipenum)) != 0)
		return;

	data |= 0x01 << rxpipenum;

	nrf24l01_write_register(nrf24l01_EN_RXADDR, &data, 1);
}

//disables the pipe that is offset by rxpipenum
//unsigned char rxpipenum is the pipe number (zero to five) whose address is being
//	specified.  If an invalid address (greater than five) is supplied, the function
//  does nothing.
void nrf24l01_rx_pipe_disable(unsigned char rxpipenum)
{
	unsigned char data;

	if(rxpipenum > 5)
		return;

	nrf24l01_read_register(nrf24l01_EN_RXADDR, &data, 1);

	if((data & (0x01 << rxpipenum)) == 0)
		return;

	data &= ~(0x01 << rxpipenum);

	nrf24l01_write_register(nrf24l01_EN_RXADDR, &data, 1);
}

//returns the status of the CD register (true if carrier detect [CD] is
//  active, false if not)
bool nrf24l01_cd_active()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_CD, &data, 1);

	return data;
}

//returns the value of the FIFO_STATUS register
unsigned char nrf24l01_get_fifo_status()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_FIFO_STATUS, &data, 1);

	return data;
}

//return the value of the status register
unsigned char nrf24l01_get_status()
{
	return nrf24l01_nop();
}

//returns true if TX_REUSE bit in FIFO_STATUS register is set, false otherwise
bool nrf24l01_fifo_tx_reuse()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_FIFO_STATUS, &data, 1);

	return (bool)(data & nrf24l01_FIFO_STATUS_TX_REUSE);
}

//returns true if TX_FULL bit in FIFO_STATUS register is set, false otherwise
bool nrf24l01_fifo_tx_full()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_FIFO_STATUS, &data, 1);

	return (bool)(data & nrf24l01_FIFO_STATUS_TX_FULL);
}

//returns true if TX_EMPTY bit in FIFO_STATUS register is set, false otherwise
bool nrf24l01_fifo_tx_empty()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_FIFO_STATUS, &data, 1);

	return (bool)(data & nrf24l01_FIFO_STATUS_TX_EMPTY);
}

//returns true if RX_FULL bit in FIFO_STATUS register is set, false otherwise
bool nrf24l01_fifo_rx_full()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_FIFO_STATUS, &data, 1);

	return (bool)(data & nrf24l01_FIFO_STATUS_RX_FULL);
}

//returns true if RX_EMPTYE bit in FIFO_STATUS register is set, false otherwise
bool nrf24l01_fifo_rx_empty()
{
	unsigned char data;

	nrf24l01_read_register(nrf24l01_FIFO_STATUS, &data, 1);

	return (bool)(data & nrf24l01_FIFO_STATUS_RX_EMPTY);
}

//returns true if IRQ pin is low, false otherwise
bool nrf24l01_irq_pin_active()
{
	bool irq = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8);
	return !irq;
}

//returns true if RX_DR interrupt is active, false otherwise
bool nrf24l01_irq_rx_dr_active()
{
	return (nrf24l01_get_status() & nrf24l01_STATUS_RX_DR);
}

//returns true if TX_DS interrupt is active, false otherwise
bool nrf24l01_irq_tx_ds_active()
{
	return (nrf24l01_get_status() & nrf24l01_STATUS_TX_DS);
}

//returns true if MAX_RT interrupt is active, false otherwise
bool nrf24l01_irq_max_rt_active()
{
	return (nrf24l01_get_status() & nrf24l01_STATUS_MAX_RT);
}

//clears only the RX_DR interrupt
void nrf24l01_irq_clear_rx_dr()
{
	unsigned char data = nrf24l01_STATUS_RX_DR;

	nrf24l01_write_register(nrf24l01_STATUS, &data, 1);
}

//clears only the TX_DS interrupt
void nrf24l01_irq_clear_tx_ds()
{
	unsigned char data = nrf24l01_STATUS_TX_DS;

	nrf24l01_write_register(nrf24l01_STATUS, &data, 1);
}

//clears only the MAX_RT interrupt
void nrf24l01_irq_clear_max_rt()
{
	unsigned char data = nrf24l01_STATUS_MAX_RT;

	nrf24l01_write_register(nrf24l01_STATUS, &data, 1);
}

//returns the current pipe in the 24L01's STATUS register
unsigned char nrf24l01_get_rx_pipe()
{
	return nrf24l01_get_rx_pipe_from_status(nrf24l01_get_status());
}

unsigned char nrf24l01_get_rx_pipe_from_status(unsigned char status)
{
	return ((status & 0xE) >> 1);
}

//flush both fifos and clear interrupts
void nrf24l01_clear_flush()
{
	nrf24l01_flush_rx();
	nrf24l01_flush_tx();
	nrf24l01_irq_clear_all();
}

//unsigned char * data must be at least 35 bytes long
void nrf24l01_get_all_registers(unsigned char * data)
{
	unsigned int outer;
	unsigned int inner;
	unsigned int dataloc = 0;
	unsigned char buffer[5];

	for(outer = 0; outer <= 0x17; outer++)
	{
		nrf24l01_read_register(outer, buffer, 5);

		for(inner = 0; inner < 5; inner++)
		{
			if(inner >= 1 && (outer != 0x0A && outer != 0x0B && outer != 0x10))
				break;

			data[dataloc] = buffer[inner];
			dataloc++;
		}
	}
}

//transmits the current tx payload
void nrf24l01_transmit()
{
	nrf24l01_set_ce();

	for (uint8_t i = 0; i<180;i++)
	{
		asm("nop");
	}

	nrf24l01_clear_ce();
}

//returns true if CE is high, false if not
bool nrf24l01_ce_pin_active()
{
	return ce_active;
}

//returns true if CSN is high, false if not
bool nrf24l01_csn_pin_active()
{
	return csn_active;
}

//executes the R_RX_PAYLOAD instruction
//unsigned char * data is the actual payload that has been received by the nrf24l01.
//	The user must size data according to the payload width specified to the nrf24l01.
//	This variable is filled by this function, so individual byte values need not be
//	initialized by the user.
//unsigned int len is the length of the payload being clocked out of the nrf24l01 (this
//	should be sized according to the payload length specified to the nrf24l01).
//returns the value of the STATUS register
unsigned char nrf24l01_read_rx_payload(unsigned char * data, unsigned int len)
{
	unsigned char status;

	nrf24l01_clear_ce();
	status = nrf24l01_execute_command(nrf24l01_R_RX_PAYLOAD, data, len, true);
	nrf24l01_set_ce();

	return status;
}

//clears the pin on the host microcontroller that is attached to the 24l01's CE pin
void nrf24l01_clear_ce()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
	ce_active = true;
}

//sets the pin on the host microcontroller that is attached to the 24l01's CE pin
void nrf24l01_set_ce()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
	ce_active = false;
}

//sets the pin on the host microcontroller that is attached to the 24l01's CSN pin
void nrf24l01_clear_csn()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
	csn_active = true;
}

//clears the pin on the host microcontroller that is attached to the 24l01's CSN pin
void nrf24l01_set_csn()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	csn_active = false;
}
