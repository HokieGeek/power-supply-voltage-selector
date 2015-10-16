#ifndef __SHIFTREGISTER_H__
#define __SHIFTREGISTER_H__

#include <stdint.h>

typedef struct {
    int data;
    int latchClock;
    int shiftClock;
    int reset;
} ShiftRegister;

// #define 

void ToggleShiftClock(ShiftRegister *const reg);
void ToggleLatchClock(ShiftRegister *const reg);
void ResetShiftRegister(ShiftRegister *const reg);
void ShiftBytes(ShiftRegister *const reg, uint8_t data);
ShiftRegister *const InitShiftRegister(int data, int latchClock, int shiftClock, int reset);

#endif //__SHIFTREGISTER_H__
