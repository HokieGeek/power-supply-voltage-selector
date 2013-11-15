#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
//#include <util/delay.h>

#define LED_PIN PB4

#define BUTTON_PIN PB0
#define VOLTAGE_OPTIONS 5

ISR (PCINT0_vect) {
    if (PINB & (1 << BUTTON_PIN)) {
        PORTB ^= (1 << LED_PIN);
    }
}

void init_pins() {
    DDRB = 1 << DDB4; // LED PIN as output
}

void init_interrupts() {
    PCMSK |= (1 << PCINT0); // Enable external interrupts PCINT0
    MCUCR  = (1 << ISC00); // (1 << ISC01);
    GIMSK |= (1 << PCIE); // Pin Change Interrupt Enable
 
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

    // How do I read from EEPROM?

    init_pins();
    init_interrupts();

    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // Use the Power Down sleep mode

    for (;;) {
        sleep_mode();   // go to sleep and wait for interrupt...
    }

    /*
    DDRB |= (1 << LED_PIN);

    while(1) {
        if ((PINB & (1 << LED_PIN))) {
            PORTB &= ~(1 << LED_PIN);
        } else {
            PORTB |= (1 << LED_PIN);
        }
        _delay_ms(800);
    }
    */
}
