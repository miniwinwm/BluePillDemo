#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

void eeprom_write_byte(uint16_t address, uint8_t byte);
uint8_t eeprom_read_byte(uint16_t address);
void eeprom_write_buf(uint16_t address, uint8_t *buffer, uint16_t length);
void eeprom_read_buf(uint16_t address, uint8_t *buffer, uint16_t length);
uint16_t eeprom_read_string(uint16_t address, char *buffer, uint16_t max_length);

#endif
