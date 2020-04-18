#ifndef NRF24L01_H_
#define NRF24L01_H_

#include <stddef.h>

#ifndef bool
#define bool unsigned char
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true !false
#endif

////////////////////////////////////////////////////////////////////////////////////
// SPI commands
//
// The following are defines for all of the commands and data masks on the SPI
//   interface.
////////////////////////////////////////////////////////////////////////////////////
//SPI command defines
#define nrf24l01_R_REGISTER		0x00
#define nrf24l01_W_REGISTER		0x20
#define nrf24l01_R_RX_PAYLOAD	0x61
#define nrf24l01_W_TX_PAYLOAD	0xA0
#define nrf24l01_FLUSH_TX		0xE1
#define nrf24l01_FLUSH_RX		0xE2
#define nrf24l01_REUSE_TX_PL	0xE3
#define nrf24l01_NOP			0xFF

//SPI command data mask defines
#define nrf24l01_R_REGISTER_DATA	0x1F
#define nrf24l01_W_REGISTER_DATA	0x1F

////////////////////////////////////////////////////////////////////////////////////
// Register definitions
//
// Below are the defines for each register's address in the 24L01.
////////////////////////////////////////////////////////////////////////////////////
#define nrf24l01_CONFIG			0x00
#define nrf24l01_EN_AA			0x01
#define nrf24l01_EN_RXADDR		0x02
#define nrf24l01_SETUP_AW		0x03
#define nrf24l01_SETUP_RETR		0x04
#define nrf24l01_RF_CH			0x05
#define nrf24l01_RF_SETUP		0x06
#define nrf24l01_STATUS			0x07
#define nrf24l01_OBSERVE_TX		0x08
#define nrf24l01_CD				0x09
#define nrf24l01_RX_ADDR_P0		0x0A
#define nrf24l01_RX_ADDR_P1		0x0B
#define nrf24l01_RX_ADDR_P2		0x0C
#define nrf24l01_RX_ADDR_P3		0x0D
#define nrf24l01_RX_ADDR_P4		0x0E
#define nrf24l01_RX_ADDR_P5		0x0F
#define nrf24l01_TX_ADDR		0x10
#define nrf24l01_RX_PW_P0		0x11
#define nrf24l01_RX_PW_P1		0x12
#define nrf24l01_RX_PW_P2		0x13
#define nrf24l01_RX_PW_P3		0x14
#define nrf24l01_RX_PW_P4		0x15
#define nrf24l01_RX_PW_P5		0x16
#define nrf24l01_FIFO_STATUS	0x17

////////////////////////////////////////////////////////////////////////////////////
// Default register values
//
// Below are the defines for each register's default value in the 24L01. Multi-byte
//   registers use notation B<X>, where "B" represents "byte" and <X> is the byte
//   number.
////////////////////////////////////////////////////////////////////////////////////
#define nrf24l01_CONFIG_DEFAULT_VAL			0x08
#define nrf24l01_EN_AA_DEFAULT_VAL			0x3F
#define nrf24l01_EN_RXADDR_DEFAULT_VAL		0x03
#define nrf24l01_SETUP_AW_DEFAULT_VAL		0x03
#define nrf24l01_SETUP_RETR_DEFAULT_VAL		0x03
#define nrf24l01_RF_CH_DEFAULT_VAL			0x02
#define nrf24l01_RF_SETUP_DEFAULT_VAL		0x0F
#define nrf24l01_STATUS_DEFAULT_VAL			0x0E
#define nrf24l01_OBSERVE_TX_DEFAULT_VAL		0x00
#define nrf24l01_CD_DEFAULT_VAL				0x00
#define nrf24l01_RX_ADDR_P0_B0_DEFAULT_VAL	0xE7
#define nrf24l01_RX_ADDR_P0_B1_DEFAULT_VAL	0xE7
#define nrf24l01_RX_ADDR_P0_B2_DEFAULT_VAL	0xE7
#define nrf24l01_RX_ADDR_P0_B3_DEFAULT_VAL	0xE7
#define nrf24l01_RX_ADDR_P0_B4_DEFAULT_VAL	0xE7
#define nrf24l01_RX_ADDR_P1_B0_DEFAULT_VAL	0xC2
#define nrf24l01_RX_ADDR_P1_B1_DEFAULT_VAL	0xC2
#define nrf24l01_RX_ADDR_P1_B2_DEFAULT_VAL	0xC2
#define nrf24l01_RX_ADDR_P1_B3_DEFAULT_VAL	0xC2
#define nrf24l01_RX_ADDR_P1_B4_DEFAULT_VAL	0xC2
#define nrf24l01_RX_ADDR_P2_DEFAULT_VAL		0xC3
#define nrf24l01_RX_ADDR_P3_DEFAULT_VAL		0xC4
#define nrf24l01_RX_ADDR_P4_DEFAULT_VAL		0xC5
#define nrf24l01_RX_ADDR_P5_DEFAULT_VAL		0xC6
#define nrf24l01_TX_ADDR_B0_DEFAULT_VAL		0xE7
#define nrf24l01_TX_ADDR_B1_DEFAULT_VAL		0xE7
#define nrf24l01_TX_ADDR_B2_DEFAULT_VAL		0xE7
#define nrf24l01_TX_ADDR_B3_DEFAULT_VAL		0xE7
#define nrf24l01_TX_ADDR_B4_DEFAULT_VAL		0xE7
#define nrf24l01_RX_PW_P0_DEFAULT_VAL		0x00
#define nrf24l01_RX_PW_P1_DEFAULT_VAL		0x00
#define nrf24l01_RX_PW_P2_DEFAULT_VAL		0x00
#define nrf24l01_RX_PW_P3_DEFAULT_VAL		0x00
#define nrf24l01_RX_PW_P4_DEFAULT_VAL		0x00
#define nrf24l01_RX_PW_P5_DEFAULT_VAL		0x00
#define nrf24l01_FIFO_STATUS_DEFAULT_VAL	0x11

////////////////////////////////////////////////////////////////////////////////////
// Register bitwise definitions
//
// Below are the defines for each register's bitwise fields in the 24L01.
////////////////////////////////////////////////////////////////////////////////////
//CONFIG register bitwise definitions
#define nrf24l01_CONFIG_RESERVED	0x80
#define	nrf24l01_CONFIG_MASK_RX_DR	0x40
#define	nrf24l01_CONFIG_MASK_TX_DS	0x20
#define	nrf24l01_CONFIG_MASK_MAX_RT	0x10
#define	nrf24l01_CONFIG_EN_CRC		0x08
#define	nrf24l01_CONFIG_CRCO		0x04
#define	nrf24l01_CONFIG_PWR_UP		0x02
#define	nrf24l01_CONFIG_PRIM_RX		0x01

//EN_AA register bitwise definitions
#define nrf24l01_EN_AA_RESERVED		0xC0
#define nrf24l01_EN_AA_ENAA_ALL		0x3F
#define nrf24l01_EN_AA_ENAA_P5		0x20
#define nrf24l01_EN_AA_ENAA_P4		0x10
#define nrf24l01_EN_AA_ENAA_P3		0x08
#define nrf24l01_EN_AA_ENAA_P2		0x04
#define nrf24l01_EN_AA_ENAA_P1		0x02
#define nrf24l01_EN_AA_ENAA_P0		0x01
#define nrf24l01_EN_AA_ENAA_NONE	0x00

//EN_RXADDR register bitwise definitions
#define nrf24l01_EN_RXADDR_RESERVED	0xC0
#define nrf24l01_EN_RXADDR_ERX_ALL	0x3F
#define nrf24l01_EN_RXADDR_ERX_P5	0x20
#define nrf24l01_EN_RXADDR_ERX_P4	0x10
#define nrf24l01_EN_RXADDR_ERX_P3	0x08
#define nrf24l01_EN_RXADDR_ERX_P2	0x04
#define nrf24l01_EN_RXADDR_ERX_P1	0x02
#define nrf24l01_EN_RXADDR_ERX_P0	0x01
#define nrf24l01_EN_RXADDR_ERX_NONE	0x00

//SETUP_AW register bitwise definitions
#define nrf24l01_SETUP_AW_RESERVED	0xFC
#define nrf24l01_SETUP_AW			0x03
#define nrf24l01_SETUP_AW_5BYTES	0x03
#define nrf24l01_SETUP_AW_4BYTES	0x02
#define nrf24l01_SETUP_AW_3BYTES	0x01
#define nrf24l01_SETUP_AW_ILLEGAL	0x00

//SETUP_RETR register bitwise definitions
#define nrf24l01_SETUP_RETR_ARD			0xF0
#define nrf24l01_SETUP_RETR_ARD_4000	0xF0
#define nrf24l01_SETUP_RETR_ARD_3750	0xE0
#define nrf24l01_SETUP_RETR_ARD_3500	0xD0
#define nrf24l01_SETUP_RETR_ARD_3250	0xC0
#define nrf24l01_SETUP_RETR_ARD_3000	0xB0
#define nrf24l01_SETUP_RETR_ARD_2750	0xA0
#define nrf24l01_SETUP_RETR_ARD_2500	0x90
#define nrf24l01_SETUP_RETR_ARD_2250	0x80
#define nrf24l01_SETUP_RETR_ARD_2000	0x70
#define nrf24l01_SETUP_RETR_ARD_1750	0x60
#define nrf24l01_SETUP_RETR_ARD_1500	0x50
#define nrf24l01_SETUP_RETR_ARD_1250	0x40
#define nrf24l01_SETUP_RETR_ARD_1000	0x30
#define nrf24l01_SETUP_RETR_ARD_750		0x20
#define nrf24l01_SETUP_RETR_ARD_500		0x10
#define nrf24l01_SETUP_RETR_ARD_250		0x00
#define nrf24l01_SETUP_RETR_ARC			0x0F
#define nrf24l01_SETUP_RETR_ARC_15		0x0F
#define nrf24l01_SETUP_RETR_ARC_14		0x0E
#define nrf24l01_SETUP_RETR_ARC_13		0x0D
#define nrf24l01_SETUP_RETR_ARC_12		0x0C
#define nrf24l01_SETUP_RETR_ARC_11		0x0B
#define nrf24l01_SETUP_RETR_ARC_10		0x0A
#define nrf24l01_SETUP_RETR_ARC_9		0x09
#define nrf24l01_SETUP_RETR_ARC_8		0x08
#define nrf24l01_SETUP_RETR_ARC_7		0x07
#define nrf24l01_SETUP_RETR_ARC_6		0x06
#define nrf24l01_SETUP_RETR_ARC_5		0x05
#define nrf24l01_SETUP_RETR_ARC_4		0x04
#define nrf24l01_SETUP_RETR_ARC_3		0x03
#define nrf24l01_SETUP_RETR_ARC_2		0x02
#define nrf24l01_SETUP_RETR_ARC_1		0x01
#define nrf24l01_SETUP_RETR_ARC_0		0x00

//RF_CH register bitwise definitions
#define nrf24l01_RF_CH_RESERVED	0x80

//RF_SETUP register bitwise definitions
#define nrf24l01_RF_SETUP_RESERVED	0xE0
#define nrf24l01_RF_SETUP_PLL_LOCK	0x10
#define nrf24l01_RF_SETUP_RF_DR		0x08
#define nrf24l01_RF_SETUP_RF_PWR	0x06
#define nrf24l01_RF_SETUP_RF_PWR_0	0x06
#define nrf24l01_RF_SETUP_RF_PWR_6 	0x04
#define nrf24l01_RF_SETUP_RF_PWR_12	0x02
#define nrf24l01_RF_SETUP_RF_PWR_18	0x00
#define nrf24l01_RF_SETUP_LNA_HCURR	0x01

//STATUS register bitwise definitions
#define nrf24l01_STATUS_RESERVED					0x80
#define nrf24l01_STATUS_RX_DR						0x40
#define nrf24l01_STATUS_TX_DS						0x20
#define nrf24l01_STATUS_MAX_RT						0x10
#define nrf24l01_STATUS_RX_P_NO						0x0E
#define nrf24l01_STATUS_RX_P_NO_RX_FIFO_NOT_EMPTY	0x0E
#define nrf24l01_STATUS_RX_P_NO_UNUSED				0x0C
#define nrf24l01_STATUS_RX_P_NO_5					0x0A
#define nrf24l01_STATUS_RX_P_NO_4					0x08
#define nrf24l01_STATUS_RX_P_NO_3					0x06
#define nrf24l01_STATUS_RX_P_NO_2					0x04
#define nrf24l01_STATUS_RX_P_NO_1					0x02
#define nrf24l01_STATUS_RX_P_NO_0					0x00
#define nrf24l01_STATUS_TX_FULL						0x01

//OBSERVE_TX register bitwise definitions
#define nrf24l01_OBSERVE_TX_PLOS_CNT	0xF0
#define nrf24l01_OBSERVE_TX_ARC_CNT		0x0F

//CD register bitwise definitions
#define nrf24l01_CD_RESERVED	0xFE
#define nrf24l01_CD_CD			0x01

//RX_PW_P0 register bitwise definitions
#define nrf24l01_RX_PW_P0_RESERVED	0xC0

//RX_PW_P0 register bitwise definitions
#define nrf24l01_RX_PW_P0_RESERVED	0xC0

//RX_PW_P1 register bitwise definitions
#define nrf24l01_RX_PW_P1_RESERVED	0xC0

//RX_PW_P2 register bitwise definitions
#define nrf24l01_RX_PW_P2_RESERVED	0xC0

//RX_PW_P3 register bitwise definitions
#define nrf24l01_RX_PW_P3_RESERVED	0xC0

//RX_PW_P4 register bitwise definitions
#define nrf24l01_RX_PW_P4_RESERVED	0xC0

//RX_PW_P5 register bitwise definitions
#define nrf24l01_RX_PW_P5_RESERVED	0xC0

//FIFO_STATUS register bitwise definitions
#define nrf24l01_FIFO_STATUS_RESERVED	0x8C
#define nrf24l01_FIFO_STATUS_TX_REUSE	0x40
#define nrf24l01_FIFO_STATUS_TX_FULL	0x20
#define nrf24l01_FIFO_STATUS_TX_EMPTY	0x10
#define nrf24l01_FIFO_STATUS_RX_FULL	0x02
#define nrf24l01_FIFO_STATUS_RX_EMPTY	0x01

////////////////////////////////////////////////////////////////////////////////////
// Function declarations
//
// Below are all function definitions contained in the library.  Please see
//   nrf24l01.c for comments regarding the usage of each function.
////////////////////////////////////////////////////////////////////////////////////
//initialization functions
void nrf24l01_initialize(unsigned char config,
						 unsigned char opt_rx_standby_mode,
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
						 unsigned char rx_pw_p5);
void nrf24l01_initialize_debug(bool rx, unsigned char p0_payload_width, bool enable_auto_ack);
void nrf24l01_initialize_debug_lite(bool rx, unsigned char p0_payload_width);

//power-up, power-down functions
void nrf24l01_power_up(bool rx_active_mode);
void nrf24l01_power_up_param(bool rx_active_mode, unsigned char config);
void nrf24l01_power_down(void);
void nrf24l01_power_down_param(unsigned char config);

//SPI commands defined by the spec
//for regnumber values, see section above titled "register definitions"
//all functions return the STATUS register
unsigned char nrf24l01_write_register(unsigned char regnumber, unsigned char * data, unsigned int len);
unsigned char nrf24l01_read_register(unsigned char regnumber, unsigned char * data, unsigned int len);
unsigned char nrf24l01_write_tx_payload(unsigned char * data, unsigned int len, bool transmit);
unsigned char nrf24l01_read_rx_payload(unsigned char * data, unsigned int len);
unsigned char nrf24l01_flush_tx(void);
unsigned char nrf24l01_flush_rx(void);
unsigned char nrf24l01_reuse_tx_pl(void);
unsigned char nrf24l01_nop(void);

//RX/TX setting functions
void nrf24l01_set_as_rx(bool rx_active_mode);
void nrf24l01_set_as_rx_param(bool rx_active_mode, unsigned char config);
void nrf24l01_rx_standby_to_active(void);
void nrf24l01_rx_active_to_standby(void);
void nrf24l01_set_as_tx(void);
void nrf24l01_set_as_tx_param(unsigned char config);

//register-oriented get/set functions for commonly-used registers during operation
unsigned char nrf24l01_get_config(void);
void nrf24l01_set_config(unsigned char config);
unsigned char nrf24l01_get_rf_ch(void);
void nrf24l01_set_rf_ch(unsigned char channel);
unsigned char nrf24l01_get_status(void);
unsigned char nrf24l01_get_observe_tx(void);
void nrf24l01_set_rx_addr(unsigned char * address, unsigned int len, unsigned char rxpipenum);
void nrf24l01_set_tx_addr(unsigned char * address, unsigned int len);
void nrf24l01_set_rx_pw(unsigned char payloadwidth, unsigned char rxpipenum);
unsigned char nrf24l01_get_rx_pw(unsigned char rxpipenum);
unsigned char nrf24l01_get_fifo_status(void);

//auto-ack and pipe-related functions
bool nrf24l01_aa_enabled(unsigned char rxpipenum);
void nrf24l01_aa_enable(unsigned char rxpipenum);
void nrf24l01_aa_disable(unsigned char rxpipenum);
bool nrf24l01_rx_pipe_enabled(unsigned char rxpipenum);
void nrf24l01_rx_pipe_enable(unsigned char rxpipenum);
void nrf24l01_rx_pipe_disable(unsigned char rxpipenum);
unsigned char nrf24l01_get_plos_cnt(void);
void nrf24l01_clear_plos_cnt(void);
void nrf24l01_clear_plos_cnt_param(unsigned char rf_ch);
unsigned char nrf24l01_get_arc_cnt(void);

//utility functions
bool nrf24l01_cd_active(void);
void nrf24l01_clear_flush(void);
unsigned char nrf24l01_get_rx_pipe(void);
unsigned char nrf24l01_get_rx_pipe_from_status(unsigned char status);
void nrf24l01_get_all_registers(unsigned char * data);

//interrupt check/clear functions
bool nrf24l01_irq_pin_active(void);
bool nrf24l01_irq_rx_dr_active(void);
bool nrf24l01_irq_tx_ds_active(void);
bool nrf24l01_irq_max_rt_active(void);
void nrf24l01_irq_clear_all(void);
void nrf24l01_irq_clear_rx_dr(void);
void nrf24l01_irq_clear_tx_ds(void);
void nrf24l01_irq_clear_max_rt(void);

//FIFO_STATUS check functions
bool nrf24l01_fifo_tx_reuse(void);
bool nrf24l01_fifo_tx_full(void);
bool nrf24l01_fifo_tx_empty(void);
bool nrf24l01_fifo_rx_full(void);
bool nrf24l01_fifo_rx_empty(void);

//IO interface-related functions
void nrf24l01_transmit(void);
void nrf24l01_clear_ce(void);
void nrf24l01_set_ce(void);
void nrf24l01_clear_csn(void);
void nrf24l01_set_csn(void);
bool nrf24l01_ce_pin_active(void);
bool nrf24l01_csn_pin_active(void);

//low-level functions for library use only
unsigned char nrf24l01_execute_command(unsigned char instruction, unsigned char * data, unsigned int len, bool copydata);
void nrf24l01_spi_send_read(unsigned char * data, unsigned int len, bool copydata);

#endif /*NRF24L01_H_*/
