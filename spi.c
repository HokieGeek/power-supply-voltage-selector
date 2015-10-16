#include "spi.h"

#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>

void SetChipSelectHigh(SpiDevice *const dev) {
    PORTB |= (1<<dev->chipSelect);
}

void SetChipSelectLow(SpiDevice *const dev) {
    PORTB &= ~(1<<dev->chipSelect);
}

void ToggleSerialClock(SpiDevice *const dev) {
    PORTB |= (1<<dev->serialClock);
    PORTB &= ~(1<<dev->serialClock);
}

void SpiWriteBytes(SpiDevice *const dev, int numBytes, uint8_t data[]) {
    SetChipSelectLow(dev);
    for (int byte = 0; byte < numBytes; byte++) {
        for (uint8_t i = 0; i < 8; i++) {
            if ((data[byte] & (1 << i))) {
                PORTB |= (1 << dev->serialDataInput);
            } else {
                PORTB &= ~(1 << dev->serialDataInput);
            }
            ToggleSerialClock(dev);
        }
    }
    SetChipSelectHigh(dev);
}

SpiDevice *const Init3WireSpiDevice(int chipSelect, int serialClock, int serialDataInput) {
    const int sdsize = sizeof(SpiDevice);
    SpiDevice *const dev = (SpiDevice*)malloc(sdsize);
    dev->chipSelect = chipSelect;
    dev->serialClock = serialClock;
    dev->serialDataInput = serialDataInput;

    DDRB |= ((1<<dev->chipSelect)|(1<<dev->serialClock)|(1<<dev->serialDataInput));

    SetChipSelectHigh(dev);

    return dev;
}

/*
// While this is pretty great, the attiny85's registers are 8-bit so 
// I can't use it to set the MCP41010's 16-bit registers. Still need to do bit-banging.
void InitSpiMaster(void) {
    // Set SCK and MOSI as output
    DDRB |= (1<<PB2)|(1<<PB0)
    
    // Enable SPI as master, but what is SPR0?
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void SpiSend(uint8_t data) {
    SPDR = data;
    
    while(!(SPSR & (1<<SPIF)));
}
*/
