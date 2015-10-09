#define F_CPU 8000000UL

#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

//#define TIMER_PRESCALE ~(1<<WDP0)&~(1<<WDP1)&~(1<<WDP2)&~(1<<WDP3) // 16ms
//#define TIMER_PRESCALE_MILLIS 16
#define TIMER_PRESCALE (1<<WDP0) // 32ms
#define TIMER_PRESCALE_MILLIS 32
//#define TIMER_PRESCALE (1<<WDP1) // 64ms
//#define TIMER_PRESCALE_MILLIS 64
//#define TIMER_PRESCALE (1<<WDP1) | (1<<WDP0) // 1/8s
//#define TIMER_PRESCALE_MILLIS 125
//#define TIMER_PRESCALE (1<<WDP2) // 1/4s
//#define TIMER_PRESCALE_MILLIS 250
//#define TIMER_PRESCALE (1<<WDP2) | (1<<WDP0) // 1/2s
//#define TIMER_PRESCALE_MILLIS 500

#define BTN_DEBOUNCE_DELAY_MILLIS 40
#define BTN_DOUBLE_CLICK_DELAY_MILLIS 150
#define BUTTON_PIN PB0

#define SREG_PIN_DATA PB1
#define SREG_PIN_LATCHCLOCK PB2
#define SREG_PIN_SHIFTCLOCK PB3
#define SREG_PIN_RESET PB4

#define OPTIONS_RANGE_START 0b01000000
#define OPTIONS_RANGE_END 0b00000010

#define SELECTED_VOLTAGE_ADDRESS 0b01000000

typedef enum { SINGLE, DOUBLE } BUTTON_CLICK_TYPE;

typedef struct {
    int pinId;
    long lastReleaseTime;
    long lastDebounceTime;
    int state;
    BUTTON_CLICK_TYPE clickType;
    void (*handler)(int, int, BUTTON_CLICK_TYPE);
} BUTTON_ITEM;

BUTTON_ITEM* handledButtons;

uint8_t currentVoltage = OPTIONS_RANGE_START;

/* ================ TIME ================ */
long elapsedTime = 0;
long millis(void);
// void buttonHandler(int,int,BUTTON_CLICK_TYPE);

long millis() {
    return elapsedTime;
}

/* ================ EEPROM ================ */
void EEPROM_write(uint8_t address, uint8_t data) {
    while (EECR & (1<<EEPE)) {} // Wait for completion of previous write

    EECR = (0<<EEPM1) | (0<<EEPM0); // Set Programming mode

    // Set up address and data registers
    EEAR = address;
    EEDR = data;

    EECR |= (1<<EEMPE); // Write logical one to EEMPE
    EECR |= (1<<EEPE); // Start eeprom write by setting EEPE
}

uint8_t EEPROM_read(uint8_t address) {
    while (EECR & (1<<EEPE)) {} // Wait for completion of previous write

    EEAR = address; // Set up address register
    EECR |= (1<<EERE); // Start eeprom read by writing EERE

    return EEDR; // Return data from data register
}

/* ================ SHIFT REG ================ */
void toggleShiftClock() {
    PORTB |= (1<<SREG_PIN_SHIFTCLOCK);
    PORTB &= ~(1<<SREG_PIN_SHIFTCLOCK);
}
 	
void toggleLatchClock() {
    PORTB |= (1<<SREG_PIN_LATCHCLOCK);
    _delay_ms(3);
    PORTB &= ~(1<<SREG_PIN_LATCHCLOCK);
    _delay_ms(3);
}

void resetRegister() {
    PORTB &= ~(1<<SREG_PIN_RESET);
    _delay_ms(10);
    PORTB |= (1<<SREG_PIN_RESET);
    _delay_ms(10);
}

void shiftBytes(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        if ((data & (1 << i))) {
            PORTB |= (1 << SREG_PIN_DATA);
        } else {
            PORTB &= ~(1 << SREG_PIN_DATA);
        }
	    toggleShiftClock();
    }
	toggleLatchClock();
}

void initShiftRegister() {
    DDRB |= ((1<<SREG_PIN_DATA)|(1<<SREG_PIN_LATCHCLOCK)|(1<<SREG_PIN_SHIFTCLOCK)|(1<<SREG_PIN_RESET));

    resetRegister();  // Toggle the Reset Pin on the 595 to clear out SR

    shiftBytes(currentVoltage);
}

/* ================ BUTTONS ================ */
/*
void startTimer() {
    WDTCR |= (1<<WDIE); // Enable watchdog timer interrupts
    timerEnabled = 1;
}

void stopTimer() {
    WDTCR &= ~(1<<WDIE); // Disable watchdog timer interrupts
    timerEnabled = 0;
}
*/

BUTTON_ITEM* debouncedButton(int pin) {
    // if (timerEnabled == 0)
        // startTimer();

    BUTTON_ITEM* btn = &handledButtons[pin];

    int currentState = (PINB & (1 << btn->pinId));

    if (currentState != btn->state) {
        btn->lastDebounceTime = millis();
    } 

    if (!currentState) {
        btn->lastReleaseTime = millis();
    }

    // if ((millis() - btn->lastDebounceTime) > BTN_DEBOUNCE_DELAY_MILLIS) {
        btn->state = currentState;
        if ((millis() - btn->lastReleaseTime) <= BTN_DOUBLE_CLICK_DELAY_MILLIS) {
            btn->clickType = DOUBLE;
            // stopTimer();
        } else {
            btn->clickType = SINGLE;
        }
    // }

    return btn;
}

void addButton(int pin, void (*handler)(int, int, BUTTON_CLICK_TYPE)) {
    // FIXME: need to check for range
    handledButtons[pin].pinId = pin;
    handledButtons[pin].lastDebounceTime = 0;
    handledButtons[pin].state = 0;
    handledButtons[pin].handler = handler;
    handledButtons[pin].clickType = SINGLE;
}

void buttonsInit(int numPins) {
    int size = sizeof(BUTTON_ITEM)*numPins;
    handledButtons = (BUTTON_ITEM*)malloc(size);
    memset(handledButtons, 0, size);
}

ISR(WDT_vect) {
    // PORTB ^= (1 << LED_PIN);
    elapsedTime += TIMER_PRESCALE_MILLIS;
    // if (elapsedTime >= TIMER_TIMEOUT)
        // stopTimer();
}

ISR (PCINT0_vect) {
    // TODO: need to figure out which button was pressed
    //       PCINT****0**** denotes that PB0 was pressed
    BUTTON_ITEM* btn = debouncedButton(BUTTON_PIN);
    btn->handler(btn->pinId, btn->state, btn->clickType);
}

//----------------------------

void nextVoltage() {
    currentVoltage = currentVoltage >> 1;
    if (currentVoltage & OPTIONS_RANGE_END)
        currentVoltage = OPTIONS_RANGE_START;
    shiftBytes(currentVoltage);
}

void buttonHandler(int btnId, int state, BUTTON_CLICK_TYPE clickCount) {
    switch (btnId) {
    case BUTTON_PIN:
        if (state && clickCount == SINGLE) {
            // Start the WDT
            // WDTCR |= (1<<WDIE); // Enable watchdog timer interrupts
        } else if (state && clickCount == DOUBLE) {
            nextVoltage();
            EEPROM_write(SELECTED_VOLTAGE_ADDRESS, currentVoltage);
            // Stop the WDT
            // WDTCR &= ~(1<<WDIE); // Enable watchdog timer interrupts
            // elapsedTime = 0L;
        }
       break; 
    }
}

void init_pins() {
    initShiftRegister();

    // DDRB &= (~(1 << BUTTON_PIN)); //0x00;
    buttonsInit(1);
    addButton(BUTTON_PIN, &buttonHandler);
}

void init_interrupts() {
    MCUCR  = (1<<ISC00);
    PCMSK |= (1<<PCINT0); // Enable external interrupts PCINT0
    GIMSK |= (1<<PCIE); // Pin Change Interrupt Enable TODO: Pg.54 says this might be redundant
 
    WDTCR |= TIMER_PRESCALE;
    WDTCR |= (1<<WDIE); // Enable watchdog timer interrupts

    // startTimer(); // TODO: remove

    sei(); // Enable global interrupts 
}

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

    // TODO: Take a look at PRR register to shutdown timer1/0, USI and ADC
    PRR |= (1<<PRTIM1) | (1<<PRTIM0) | (1<<PRUSI) | (1<<PRADC);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Use the Power Down sleep mode

    for (;;) {
        sleep_mode();   // go to sleep and wait for interrupt...
    }
}
