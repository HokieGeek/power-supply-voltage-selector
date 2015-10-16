#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>

typedef struct {
    int chipSelect;
    int serialClock;
    int serialDataInput;
} SpiDevice;

void SetChipSelectHigh(SpiDevice *const dev);
void SetChipSelectLow(SpiDevice *const dev);
void ToggleSerialClock(SpiDevice *const dev);
void WriteBytes(SpiDevice *const dev, int numBytes, uint8_t data[]);
SpiDevice *const Init3WireSpiDevice(int chipSelect, int serialClock, int serialDataInput);

#endif //__SPI_H__
