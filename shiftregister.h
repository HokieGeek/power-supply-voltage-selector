#ifndef _SHIFTREGISTER_H_
#define _SHIFTREGISTER_H_

#include <stdint.h>

typedef struct {
    int data;
    int latchClock;
    int shiftClock;
    int reset;
} ShiftRegister;

// #define 

void ToggleShiftClock(const ShiftRegister *const reg);
void ToggleLatchClock(const ShiftRegister *const reg);
void ResetRegister(const ShiftRegister *const reg);
void ShiftBytes(const ShiftRegister *const reg, uint8_t data);
const ShiftRegister *const InitShiftRegister(int data, int latchClock, int shiftClock, int reset);

#endif // _SHIFTREGISTER_H_
