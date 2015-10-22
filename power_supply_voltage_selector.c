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

#define HC595_PIN_DATA PB1
#define HC595_PIN_LATCHCLOCK PB3
#define HC595_PIN_SHIFTCLOCK PB2

#define MCP41010_PIN_DATA PB1
#define MCP41010_PIN_SERIALCLOCK PB2
#define MCP41010_PIN_CHIPSELECT PB4

#define MCP4XXXX_POT_0 0
#define MCP4XXXX_POT_1 1
#define MCP4XXXX_COMMAND_P0_WRITE 0b00010001
#define MCP4XXXX_COMMAND_P0_SHUTDOWN 0b00100001

#define NUM_VOLTAGE_SELECTIONS 5
#define SELECTED_VOLTAGE_EEPROM_ADDRESS 0b01000000

typedef struct {
    uint8_t led;
    uint8_t potData;
} VoltageSelection;

uint8_t currentVoltageSelection = 0;
VoltageSelection* voltageSelections;

ShiftRegister *shiftReg;
SpiDevice *spi;

typedef struct {
    int pinId;
    long lastReleaseTime;
    long lastDebounceTime;
    int state;
    int clickCount;
    void (*handler)(int, int, int);
} BUTTON_ITEM;

BUTTON_ITEM* handledButtons;

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
    // FIXME: how on earth can I tell which button was pressed?
    BUTTON_ITEM* btn = debouncedButton(BUTTON_PIN);
    btn->handler(btn->pinId, btn->state, btn->clickCount);
}

void voltageSelectionsInit(int numVoltages) {
    int size = sizeof(VoltageSelection)*numVoltages;
    voltageSelections = (VoltageSelection*)malloc(size);
    memset(voltageSelections, 0, size);

    // Initialize the selections
    //// 3.3v
    voltageSelections[0].led = 0b01000000;
    // voltageSelections[0].led = 0b00000010;
    voltageSelections[0].potData = 43;
    //// 5v
    voltageSelections[1].led = 0b00100000;
    // voltageSelections[1].led = 0b00000100;
    voltageSelections[1].potData = 76;
    //// 9v
    voltageSelections[2].led = 0b00010000;
    // voltageSelections[2].led = 0b00001000;
    voltageSelections[2].potData = 161;
    //// 12v
    voltageSelections[3].led = 0b00001000;
    // voltageSelections[3].led = 0b00010000;
    voltageSelections[3].potData = 224;
    //// Adj
    voltageSelections[4].led = 0b00000100;
    // voltageSelections[4].led = 0b00100000;
    voltageSelections[4].potData = 0;
}

void setVoltageSelection(int selection) {
    // Adjust the pot
    uint8_t potDataByte = voltageSelections[selection].potData;
    if (potDataByte == 0) {
        MCP41XXX_shutdown(spi);
    } else {
        MCP41XXX_write(spi, potDataByte);
    }

    // Set the LEDs
    ShiftBytes(shiftReg, voltageSelections[selection].led);

    // Store in memory
    currentVoltageSelection = selection;
    EEPROM_write(SELECTED_VOLTAGE_EEPROM_ADDRESS, selection);
}

int nextVoltage() {
    int next = currentVoltageSelection + 1;
    if (next == NUM_VOLTAGE_SELECTIONS) {
        next = 0;
    }
    return next;
}

void buttonHandler(int btnId, int state, int clickCount) {
    switch (btnId) {
    case BUTTON_PIN:
        if (state && clickCount == 2) {
            setVoltageSelection(nextVoltage());
        }
       break;
    }
}

void MCP4XXXX_send(SpiDevice *const dev, uint8_t command, uint8_t data) {
    uint8_t input[2];
    input[0] = command;
    input[1] = data;

    SpiWriteBytes(dev, 2, input);
}

void MCP41XXX_shutdown(SpiDevice *const dev) {
    MCP4XXXX_send(dev, MCP4XXXX_COMMAND_P0_SHUTDOWN, 0b00000000);
}

void MCP41010_shutdown(dev) {
    MCP41XXX_shutdown(dev);
}

void MCP41XXX_write(SpiDevice *const dev, uint8_t value) {
    MCP4XXXX_send(dev, MCP4XXXX_COMMAND_P0_WRITE, value);
}

void MCP41010_write(dev, value) {
    MCP41XXX_write(dev, value);
}

void init_pins() {
    shiftReg = InitShiftRegister(HC595_PIN_DATA, HC595_PIN_LATCHCLOCK, HC595_PIN_SHIFTCLOCK, -1);

    spi = Init3WireSpiDevice(MCP41010_PIN_CHIPSELECT,
                             MCP41010_PIN_SERIALCLOCK,
                             MCP41010_PIN_DATA);

}

void init_interrupts() {
    // Do the interrupts
    GIMSK |= (1<<PCIE); // Pin Change Interrupt Enable
    PCMSK |= (1<<PCINT0); // Turn on interrupt for Pin0

    sei(); // Enable global interrupts 
}

void init() {
    init_pins();

    buttonsInit(1);
    addButton(BUTTON_PIN, &buttonHandler);

    voltageSelectionsInit(NUM_VOLTAGE_SELECTIONS);

    init_interrupts();

    // Set all initial values
    uint8_t currentVoltageSelection = EEPROM_read(SELECTED_VOLTAGE_EEPROM_ADDRESS);
    setVoltageSelection(currentVoltageSelection);
}

int main(void) {
    // 1. Read EEPROM for previous setting, set respective pins to high
    // 2. Go to sleep and wait for a button to be pushed
    // 3. Wake up when a button is pushed
    // 4. Determine which button (decode)
    // 5. If valid, write it to EEPROM
    // 6. Set the pot value
    // 7. Set the LEDs
    // 8. Go back to bed

    init();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Use the Power Down sleep mode

    for (;;) {
        sleep_mode();   // go to sleep and wait for interrupt...
    }
}
