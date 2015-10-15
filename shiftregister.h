#include <stdint.h>
#include <avr/io.h>

// TODO: struct that defines the shift register pins and gets passed around to these methods

typedef struct {
    int data;
    int latchClock;
    int shiftClock;
    int reset;
} ShiftRegister;


void ToggleShiftClock(int pin);
void ToggleLatchClock(int pin);
void ResetRegister(int pin);
void ShiftBytes(const ShiftRegister* const reg, uint8_t data) {
const ShiftRegister* const InitShiftRegister(int data, int latchClock, int shiftClock, int reset);
