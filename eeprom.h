#ifndef _EEPROM_H_
#define _EEPROM_H_

#include <stdint.h>

void EEPROM_write(uint8_t address, uint8_t data);
uint8_t EEPROM_read(uint8_t address);

#endif // _EEPROM_H_
