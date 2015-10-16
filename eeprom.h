#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdint.h>

void EEPROM_write(uint8_t address, uint8_t data);
uint8_t EEPROM_read(uint8_t address);

#endif //__EEPROM_H__
