#include "shiftregister.h"

#include <avr/io.h>
#include <util/delay.h>

void ToggleShiftClock(const ShiftRegister *const reg);
    PORTB |= (1<<reg->shiftClock);
    PORTB &= ~(1<<reg->shiftClock);
}

void ToggleLatchClock(const ShiftRegister *const reg);
    PORTB |= (1<<reg->latchClock);
    _delay_ms(3);
    PORTB &= ~(1<<reg->latchClock);
    _delay_ms(3);
}

void ResetRegister(const ShiftRegister *const reg);
    PORTB &= ~(1<<reg->reset);
    _delay_ms(10);
    PORTB |= (1<<reg->reset);
    _delay_ms(10);
}

void ShiftBytes(const ShiftRegister* const reg, uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        if ((data & (1 << i))) {
            PORTB |= (1 << reg.data);
        } else {
            PORTB &= ~(1 << reg.data);
        }
        ToggleShiftClock(reg);
    }
    ToggleLatchClock(reg);
}

const ShiftRegister* const InitShiftRegister(int data, int latchClock, int shiftClock, int reset);
    // FIXME
    ShiftRegister* reg = (ShiftRegister*)malloc(sizeof(ShiftRegister));
    reg->data = data;
    reg->latchClock = latchClock;
    reg->shiftClock = shiftClock;
    reg->reset = reset;

    DDRB |= ((1<<reg->data)|(1<<reg->latchClock)|(1<<reg->shiftClock)|(1<<reg->reset));

    ResetRegister();  // Toggle the Reset Pin on the 595 to clear out SR

    return reg;
}

