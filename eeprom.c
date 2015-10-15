#include "eeprom.h"

#include <avr/io.h>

/* ================ EEPROM ================ */
void EEPROM_write(uint8_t address, uint8_t data) {
    while (EECR & (1<<EEPE)) {} // Wait for completion of previous write

    EECR = (0<<EEPM1) | (0<<EEPM0); // Set Programming mode

    // Set up address and data registers
    EEAR = address;
    EEDR = data;

    EECR |= (1<<EEMPE); // Write logical one to EEMPE
    EECR |= (1<<EEPE); // Start eeprom write by setting EEPE
}

uint8_t EEPROM_read(uint8_t address) {
    while (EECR & (1<<EEPE)) {} // Wait for completion of previous write

    EEAR = address; // Set up address register
    EECR |= (1<<EERE); // Start eeprom read by writing EERE

    return EEDR; // Return data from data register
}

