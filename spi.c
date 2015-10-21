#include "spi.h"

#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
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
        for (int bit = 7; bit >= 0; --bit) {
            if ((data[byte] & (1 << bit))) {
                PORTB |= (1 << dev->serialDataInput);
            } else {
                PORTB &= ~(1 << dev->serialDataInput);
            }
            ToggleSerialClock(dev);
        }
    }
    SetChipSelectHigh(dev);
}

SpiDevice *const InitSoftwareSpi(int chipSelect, int serialClock, int serialDataInput) {
    const int sdsize = sizeof(SpiDevice);

    SpiDevice *const dev = (SpiDevice*)malloc(sdsize);
    dev->chipSelect = chipSelect;
    dev->serialClock = serialClock;
    dev->serialDataInput = serialDataInput;

    DDRB |= (1<<dev->chipSelect)|(1<<dev->serialClock)|(1<<dev->serialDataInput);

    SetChipSelectHigh(dev);

    return dev;
}

// While this is pretty great, the attiny85's registers are 8-bit so 
// I can't use it to set the MCP41010's 16-bit registers. Still need to do bit-banging.
void InitUsi(int datamode) {
    USICR &= ~((1<<USISIE)|(1<<USIOIE)|(1<<USIWM1));
    USICR |= (1<<USIWM0)|(1<<USICS1)|(1<<USICLK);

    USICR |= (1<<USICS0);

    // Set SCK and MOSI as output
    // DDRB |= (1<<PB2)|(1<<PB0)|(1<<chipSelect);
    DDRB |= (1<<USI_SCK_PIN)|(1<<USI_DO_PIN);
    DDRB &= ~(1<<USI_DI_PIN);
}

uint8_t UsiSend(uint8_t data) {
    USIDR = data;
    USISR = (1<<USIOIF);

    // asm volatile("nop");
    while (!(USISR & (1<<USIOIF))) USICR |= (1<<USITC);

    return USIDR;
}

SpiDevice *const InitSpiMaster(int chipSelect) {
    const int sdsize = sizeof(SpiDevice);
    SpiDevice *const dev = (SpiDevice*)malloc(sdsize);
    dev->chipSelect = chipSelect;

    USICR &= ~((1<<USISIE)|(1<<USIOIE)|(1<<USIWM1));
    USICR |= (1<<USIWM0)|(1<<USICS1)|(1<<USICLK);

    // Set SCK and MOSI as output
    // DDRB |= (1<<PB2)|(1<<PB0)|(1<<chipSelect);
    DDRB |= (1<<USI_SCK_PIN)|(1<<USI_DO_PIN)|(1<<dev->chipSelect);
    DDRB &= ~(1<<USI_DI_PIN);

    SetChipSelectHigh(dev);

    return dev;
}

uint8_t SpiSend(SpiDevice *const dev, uint8_t data) {
    SetChipSelectLow(dev);

    uint8_t ret = UsiSend(data);

    SetChipSelectHigh(dev);

    return ret;
}

uint8_t SpiSend16(SpiDevice *const dev, uint8_t byte1, uint8_t byte2) {
    SetChipSelectLow(dev);
    _delay_ms(2000); // I want to see this happening

    uint8_t ret = UsiSend(byte1);
    ret = UsiSend(byte2);

    SetChipSelectHigh(dev);
    _delay_ms(500);

    return ret;
}

/*
uint16_t SpiSend16(SpiDevice *const dev, uint16_t bytes) {
    // TODO: see how the Arduino SPI.h library does it
    return 0xFFFF;
}
*/
