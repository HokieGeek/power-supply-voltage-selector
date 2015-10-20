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
void SpiWriteBytes(SpiDevice *const dev, int numBytes, uint8_t data[]);
SpiDevice *const InitSoftwareSpi(int chipSelect, int serialClock, int serialDataInput);

SpiDevice *const InitSpiMaster(int chipSelect);
uint8_t SpiSend(SpiDevice *const dev, uint8_t data);
uint8_t SpiSend16(SpiDevice *const dev, uint8_t byte1, uint8_t byte2);
// uint16_t SpiSend16(SpiDevice *const dev, uint16_t bytes);

#endif //__SPI_H__
