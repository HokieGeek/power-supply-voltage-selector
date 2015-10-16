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
    _delay_ms(3);
    PORTB &= ~(1<<dev->serialClock);
    _delay_ms(3);
}

void WriteBytes(SpiDevice *const dev, int numBytes, uint8_t data[]) {
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
