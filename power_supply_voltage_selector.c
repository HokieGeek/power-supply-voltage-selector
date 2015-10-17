#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "eeprom.h"
#include "shiftregister.h"
#include "spi.h"

//#define TIMER_PRESCALE ~(1<<WDP0)&~(1<<WDP1)&~(1<<WDP2)&~(1<<WDP3) // 16ms
#define TIMER_PRESCALE_MILLIS 16
//#define TIMER_PRESCALE (1<<WDP0) // 32ms
//#define TIMER_PRESCALE_MILLIS 32
//#define TIMER_PRESCALE (1<<WDP1) // 64ms
//#define TIMER_PRESCALE_MILLIS 64
//#define TIMER_PRESCALE (1<<WDP1) | (1<<WDP0) // 1/8s
//#define TIMER_PRESCALE_MILLIS 125
//#define TIMER_PRESCALE (1<<WDP2) // 1/4s
//#define TIMER_PRESCALE_MILLIS 250
//#define TIMER_PRESCALE (1<<WDP2) | (1<<WDP0) // 1/2s
//#define TIMER_PRESCALE_MILLIS 500
#define TIMER_TIMEOUT 2000 // 2s

#define BTN_DEBOUNCE_DELAY_MILLIS 25
#define BTN_DOUBLE_CLICK_DELAY_MILLIS 200
#define BUTTON_PIN PB0

#define SREG_PIN_DATA PB1
#define SREG_PIN_LATCHCLOCK PB2
#define SREG_PIN_SHIFTCLOCK PB3
#define SREG_PIN_RESET PB4

#define DIGIPOT_PIN_DATA PB1
#define DIGIPOT_PIN_SERIALCLOCK PB2
#define DIGIPOT_PIN_CHIPSELECT PB3
#define MCP41010_COMMAND_BYTE 0b00010001

#define OPTIONS_RANGE_START 0b01000000
#define OPTIONS_RANGE_END 0b00000010

#define SELECTED_VOLTAGE_ADDRESS 0b01000000

typedef struct {
    int pinId;
    long lastReleaseTime;
    long lastDebounceTime;
    int state;
    int clickCount;
    void (*handler)(int, int, int);
} BUTTON_ITEM;

BUTTON_ITEM* handledButtons;
ShiftRegister *shiftReg;
SpiDevice *spi;

uint8_t currentVoltage = OPTIONS_RANGE_START;

// ================ TIME ================ //
long elapsedTime = 0;
uint8_t timerEnabled = 0;

long clock_diff(long old_clock, long new_clock) {
  if (new_clock >= old_clock)
    return new_clock - old_clock;
  else
    return new_clock + (65535 - old_clock);
}

// ================ BUTTONS ================ //
BUTTON_ITEM* debouncedButton(int pin) {
    BUTTON_ITEM* btn = &handledButtons[pin];

    if (timerEnabled == 0) {
        startTimer();
        btn->lastDebounceTime = 0;
        btn->lastReleaseTime = 0;
    }

    int currentState = (PINB & (1 << btn->pinId));

    cli();
    long time = elapsedTime;
    sei();

    if (currentState != btn->state) {
        btn->lastDebounceTime = time;
    } 

    if (!currentState) {
    // if (bit_is_clear(PINB, btn->pinId)) {
        btn->lastReleaseTime = time;
    }

    // if ((btn->lastDebounceTime > 0 && time > btn->lastDebounceTime) && ((time - btn->lastDebounceTime) >= BTN_DEBOUNCE_DELAY_MILLIS)) {
        btn->state = currentState;
        if ((btn->lastReleaseTime > 0 && time > btn->lastReleaseTime) && ((time - btn->lastReleaseTime) <= BTN_DOUBLE_CLICK_DELAY_MILLIS)) {
        // if ((time - btn->lastReleaseTime) <= BTN_DOUBLE_CLICK_DELAY_MILLIS) {
        // if (clock_diff(btn->lastReleaseTime, time) <= BTN_DOUBLE_CLICK_DELAY_MILLIS) {
            stopTimer();
            btn->clickCount = 2;
        } else {
            btn->clickCount = 1;
        }
    // }

    return btn;
}

void addButton(int pin, void (*handler)(int, int, int)) {
    // FIXME: need to check for range
    handledButtons[pin].pinId = pin;
    handledButtons[pin].lastDebounceTime = 0;
    handledButtons[pin].lastReleaseTime = 0;
    handledButtons[pin].state = 0;
    handledButtons[pin].handler = handler;
    handledButtons[pin].clickCount = 1;
}

void buttonsInit(int numPins) {
    int size = sizeof(BUTTON_ITEM)*numPins;
    handledButtons = (BUTTON_ITEM*)malloc(size);
    memset(handledButtons, 0, size);
}

void startTimer() {
    // Start the WDT
    if (timerEnabled == 0) {
        WDTCR |= (1<<WDIE); // Enable watchdog timer interrupts
        timerEnabled = 1;
    }
}

void stopTimer() {
    // Stop the WDT
    WDTCR &= ~(1<<WDIE); // Disable watchdog timer interrupts
    elapsedTime = 0L;
    timerEnabled = 0;
}

ISR(WDT_vect) {
    elapsedTime += TIMER_PRESCALE_MILLIS;
    if (elapsedTime >= TIMER_TIMEOUT)
        stopTimer();
}

ISR (PCINT0_vect) {
    // PCINT****0**** denotes that PB0 was pressed
    BUTTON_ITEM* btn = debouncedButton(BUTTON_PIN);
    btn->handler(btn->pinId, btn->state, btn->clickCount);
}

void nextVoltage() {
    currentVoltage = currentVoltage >> 1;
    if (currentVoltage & OPTIONS_RANGE_END)
        currentVoltage = OPTIONS_RANGE_START;
    ShiftBytes(shiftReg, currentVoltage);
}

void buttonHandler(int btnId, int state, int clickCount) {
    switch (btnId) {
    case BUTTON_PIN:
        if (state && clickCount == 2) {
            nextVoltage();
            // TODO: Adjust Pot
            EEPROM_write(SELECTED_VOLTAGE_ADDRESS, currentVoltage);
        }
       break;
    }
}

void MCP41010_write(SpiDevice *const dev, uint8_t value) {

    uint8_t input[2];
    input[0] = MCP41010_COMMAND_BYTE;
    input[1] = value;

    SpiWriteBytes(dev, 2, input);
}

void init_pins() {
    shiftReg = InitShiftRegister(SREG_PIN_DATA, SREG_PIN_LATCHCLOCK,
                                 SREG_PIN_SHIFTCLOCK, SREG_PIN_RESET);

    spi = Init3WireSpiDevice(DIGIPOT_PIN_CHIPSELECT,
                             DIGIPOT_PIN_SERIALCLOCK,
                             DIGIPOT_PIN_DATA);

    // DDRB |= (1 << BUTTON_PIN); // pull-up resistor

    buttonsInit(1);
    addButton(BUTTON_PIN, &buttonHandler);
}

void init_interrupts() {
    PCMSK |= (1<<PCINT0); // Enable external interrupts PCINT0
    MCUCR  = (1<<ISC00);
    GIMSK |= (1<<PCIE); // Pin Change Interrupt Enable

    // WDTCR |= TIMER_PRESCALE;
    // WDTCR |= (1<<WDIE); // Enable watchdog timer interrupts

    sei(); // Enable global interrupts 
}

/*
*/
int main(void) {
    // 1. Read EEPROM for previous setting, set respective pins to high
    // 2. Go to sleep and wait for a button to be pushed
    // 3. Wake up when a button is pushed
    // 4. Determine which button (decode)
    // 5. If valid, write it to EEPROM
    // 6. Set respective pins
    // 7. Go back to bed

    uint8_t currVal = EEPROM_read(SELECTED_VOLTAGE_ADDRESS);
    // if (currVal & 0b11111111)
        // currVal = OPTIONS_RANGE_START;
    // TODO: some checks
    currentVoltage = currVal;

    init_pins();
    init_interrupts();

    ShiftBytes(shiftReg, currentVoltage);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Use the Power Down sleep mode

    for (;;) {
        sleep_mode();   // go to sleep and wait for interrupt...
    }
}

/*
int main(void) {
    init_pins();

    while(1) {
        for (int level = 0; level < 255; level++) {
            MCP41010_write(spi, level);
            _delay_ms(10);
        }
        _delay_ms(1500);

        for (int level = 255; level > 0; level--) {
            MCP41010_write(spi, level);
            _delay_ms(10);
        }
    }
}
*/
