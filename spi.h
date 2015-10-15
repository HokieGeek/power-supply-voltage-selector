#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>

typedef enum {
    3WIRE,
    5WIRE
} SpiDeviceTypeEnum;

typedef struct {
    int chipSelect;
    int serialDataInput;
    int serialClock;
} SpiDevice3Wire;

// #define 

void ToggleSerialClock(const SpiDevice *const dev);
// void ResetDevice(const SpiDevice *const dev);
void WriteData(const SpiDevice *const dev, uint8_t data);
const SpiDevice *const Init3WireSpiDevice(int chipSelect, int serialDataInput, int serialClock);
// const SpiDevice *const Init5WireSpiDevice(int chipSelect, int serialDataInput, int serialClock);

#endif // _SPI_H_
