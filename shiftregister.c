#include "shiftregister.h"

#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

void ToggleShiftClock(ShiftRegister *const reg) {
    PORTB |= (1<<reg->shiftClock);
    PORTB &= ~(1<<reg->shiftClock);
}

void ToggleLatchClock(ShiftRegister *const reg) {
    PORTB |= (1<<reg->latchClock);
    _delay_ms(3);
    PORTB &= ~(1<<reg->latchClock);
    _delay_ms(3);
}

void ResetShiftRegister(ShiftRegister *const reg) {
    PORTB &= ~(1<<reg->reset);
    _delay_ms(10);
    PORTB |= (1<<reg->reset);
    _delay_ms(10);
}

void ShiftBytes(ShiftRegister *const reg, uint8_t data) {
    for (uint8_t bit = 7; bit >= 0; --bit) {
        if ((data & (1 << bit))) {
            PORTB |= (1 << reg->data);
        } else {
            PORTB &= ~(1 << reg->data);
        }
        ToggleShiftClock(reg);
    }
    ToggleLatchClock(reg);
}

ShiftRegister* const InitShiftRegister(int data, int latchClock, int shiftClock, int reset) {
    const int srsize = sizeof(ShiftRegister);
    ShiftRegister *const reg = (ShiftRegister*)malloc(srsize);
    reg->data = data;
    reg->latchClock = latchClock;
    reg->shiftClock = shiftClock;

    DDRB |= ((1<<reg->data)|(1<<reg->latchClock)|(1<<reg->shiftClock));

    if (reset >= 0) {
        reg->reset = reset;
        DDRB |= (1<<reg->reset);
    }

    return reg;
}
