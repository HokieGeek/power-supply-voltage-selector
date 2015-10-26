# power-supply-voltage-selector
Trying to learn more electronics by building a circuit that lets me have different voltage outputs for use with other prototypes and mostly breadboards

## Prototype
![Current progress](http://i.imgur.com/n8TwQP8.jpg)

# Schematics
## Primary
![Logic schematics](https://raw.githubusercontent.com/HokieGeek/power-supply-voltage-selector/master/schematics/voltage-selector-and-output.png)
## LEDs, buttons, and outputs
![LEDs and Buttons](https://raw.githubusercontent.com/HokieGeek/power-supply-voltage-selector/master/schematics/leds-and-buttons.png)

# Boards
## Primary
![Main board](https://raw.githubusercontent.com/HokieGeek/power-supply-voltage-selector/master/schematics/voltage-selector-and-output.brd.png)
## LEDs, buttons, and outputs
![Board of LEDs and Buttons](https://raw.githubusercontent.com/HokieGeek/power-supply-voltage-selector/master/schematics/leds-and-buttons.brd.png)

# Parts list
## ICs
* [ATTINY85](http://www.atmel.com/images/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf) - Drives the digi pot and shift register
* [LM7805CT](http://www.mouser.com/ds/2/149/LM7805-189995.pdf) - 5v voltage regulator for Vic
* [MIC2940](http://www.micrel.com/_PDF/mic2940.pdf) - Adjustable voltage regulator as target output regulator
* [MCP41010](http://ww1.microchip.com/downloads/en/DeviceDoc/11195c.pdf) - 10 kΩ digital potentiometer
* [SN74LS595N](http://www.ti.com/lit/ds/symlink/sn74ls596.pdf) - 8-bit shift registers with latches

## Discrete components
* 12V Transformer
* 4x 1N4007 Rectifiers
* 2x 47uF Electrolytic capacitors
* 3x 0.1uF Ceramic capacitors
* 1x 330 Ohm Resistor network (SIL-6)
* 1x 10 kOhm 5% Resistors
* 2x 1 kOhm 5% Resistors
* 4x 3mm LEDs (green)
* 1x 3mm LED (red)
* 1x USB-A Female
* LED Volt meter thingy
* Toggle switch
* Momentatry switch
* Jumpers and headers
