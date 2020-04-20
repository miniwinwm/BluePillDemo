#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "lcd.h"

#define E_PORT					GPIOB
#define E_PIN					GPIO_PIN_0
#define	RS_PORT					GPIOA
#define RS_PIN					GPIO_PIN_8
#define D7_PORT					GPIOC
#define D7_PIN					GPIO_PIN_13
#define D6_PORT					GPIOA
#define D6_PIN					GPIO_PIN_0
#define D5_PORT					GPIOA
#define D5_PIN					GPIO_PIN_1
#define D4_PORT					GPIOA
#define D4_PIN					GPIO_PIN_2

extern osMessageQueueId_t lcdQueueHandle;

static void lcd_wait_until_not_busy();
static void lcd_wait_until_not_busy();
static void lcd_strobe(void);
static void lcd_write_nibble(uint8_t c);

static void lcd_strobe(void)
{
	// E = ENABLED;
	HAL_GPIO_WritePin(E_PORT, E_PIN, GPIO_PIN_SET);

	// E = DISABLED;
	HAL_GPIO_WritePin(E_PORT, E_PIN, GPIO_PIN_RESET);
}

static void lcd_wait_until_not_busy()
{
	volatile uint32_t i;

	for (i = 0UL; i < 5000UL; i++)
	{
		__asm__("NOP");
	}
}
void lcd_clear(void)
{
  lcd_packet_t lcd_packet;

  lcd_packet.command = CLEAR_SCREEN;
  osMessageQueuePut(lcdQueueHandle, &lcd_packet, 0U, 0U);
}

void lcd_puts(uint8_t row, uint8_t column, const char *s)
{
	lcd_packet_t lcd_packet;
	size_t length = strlen(s);

	lcd_packet.command = WRITE_TEXT;
	lcd_packet.length = (uint8_t)length;
	lcd_packet.row = row;
	lcd_packet.column = column;
	(void)memcpy(lcd_packet.text, s, length);
	osMessageQueuePut(lcdQueueHandle, &lcd_packet, 0U, 0U);
}

static void lcd_write_nibble(uint8_t c)
{
	HAL_GPIO_WritePin(D4_PORT, D4_PIN, (GPIO_PinState)(c & 0x01U));
	HAL_GPIO_WritePin(D5_PORT, D5_PIN, (GPIO_PinState)((c >> 1) & 0x01U));
	HAL_GPIO_WritePin(D6_PORT, D6_PIN, (GPIO_PinState)((c >> 2) & 0x01U));
	HAL_GPIO_WritePin(D7_PORT, D7_PIN, (GPIO_PinState)((c >> 3) & 0x01U));
}

void lcd_write_command(uint8_t c)
{
	lcd_wait_until_not_busy();

	// RS = COMMAND
	HAL_GPIO_WritePin(RS_PORT, RS_PIN, GPIO_PIN_RESET);

	lcd_write_nibble(c >> 4);
    lcd_strobe();
	lcd_write_nibble(c);
    lcd_strobe();
}

void lcd_write_data(uint8_t c)
{
	// wait until previous operation is finished
	lcd_wait_until_not_busy();

	// RS = DATA
	HAL_GPIO_WritePin(RS_PORT, RS_PIN, GPIO_PIN_SET);

	lcd_write_nibble(c >> 4);
    lcd_strobe();
	lcd_write_nibble(c);
    lcd_strobe();
}

void lcd_set_cg_character(uint8_t position, const uint8_t *bytes)
{
	uint8_t row;
	uint8_t cg_address;

	cg_address = position * 8U + 0x40;
	lcd_write_command(cg_address);

	for (row = 0U; row < 8; row++)
	{
		lcd_write_data(bytes[row]);
	}
}

void lcd_init(void)
{
	// pins E_PORT:E_PIN, RS_PORT:RS_PIN and RW_PORT:RW_PIN all default to zero on reset so don't need setting here

    HAL_Delay(15U);
    lcd_write_nibble(0x03U);
    lcd_strobe();
    HAL_Delay(5U);
    lcd_strobe();
    HAL_Delay(200U);
    lcd_strobe();
    HAL_Delay(1U);
	lcd_write_nibble(0x02U);
    lcd_strobe();
	lcd_write_command(0x28U);
	HAL_Delay(40U);
    lcd_write_command(0x06U);
    lcd_write_command(0x0cU);
}
