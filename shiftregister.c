#include "shiftregister.h"

// #define SREG_PIN_DATA PB1
// #define SREG_PIN_LATCHCLOCK PB2
// #define SREG_PIN_SHIFTCLOCK PB3
// #define SREG_PIN_RESET PB4

/* ================ SHIFT REG ================ */
void ToggleShiftClock(int pin) {
    // PORTB |= (1<<SREG_PIN_SHIFTCLOCK);
    // PORTB &= ~(1<<SREG_PIN_SHIFTCLOCK);
    PORTB |= (1<<pin);
    PORTB &= ~(1<<pin);
}

void ToggleLatchClock(int pin) {
    // PORTB |= (1<<SREG_PIN_LATCHCLOCK);
    PORTB |= (1<<pin);
    _delay_ms(3);
    // PORTB &= ~(1<<SREG_PIN_LATCHCLOCK);
    PORTB &= ~(1<<pin);
    _delay_ms(3);
}

void ResetRegister(int pin) {
    // PORTB &= ~(1<<SREG_PIN_RESET);
    PORTB &= ~(1<<pin);
    _delay_ms(10);
    // PORTB |= (1<<SREG_PIN_RESET);
    PORTB |= (1<<pin);
    _delay_ms(10);
}

void ShiftBytes(const ShiftRegister* const reg, uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        if ((data & (1 << i))) {
            // PORTB |= (1 << SREG_PIN_DATA);
            PORTB |= (1 << reg.data);
        } else {
            // PORTB &= ~(1 << SREG_PIN_DATA);
            PORTB &= ~(1 << reg.data);
        }
        // toggleShiftClock();
        toggleShiftClock(reg.shiftClock);
    }
    // toggleLatchClock();
    toggleLatchClock(reg.latchClock);
}

// void initShiftRegister() {
const ShiftRegister* const InitShiftRegister(int data, int latchClock, int shiftClock, int reset);
    DDRB |= ((1<<SREG_PIN_DATA)|(1<<SREG_PIN_LATCHCLOCK)|(1<<SREG_PIN_SHIFTCLOCK)|(1<<SREG_PIN_RESET));

    ResetRegister();  // Toggle the Reset Pin on the 595 to clear out SR

    // shiftBytes(currentVoltage);
}

