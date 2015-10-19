# power-supply-voltage-selector
Trying to learn more electronics by building a circuit that lets me have different voltage outputs for use with other prototypes and mostly breadboards

# Schematics
## Selection logic
![Logic schematics](https://raw.githubusercontent.com/HokieGeek/power-supply-voltage-selector/master/schematics/voltage-selector-and-output.png)
## LEDs, buttons, and outputs
![LEDs and Buttons](https://raw.githubusercontent.com/HokieGeek/power-supply-voltage-selector/master/schematics/leds-and-buttons.png)

## Boards
## Selection Logic
![Main board](https://raw.githubusercontent.com/HokieGeek/power-supply-voltage-selector/master/schematics/voltage-selector-and-output.brd.png)
## LEDs, buttons, and outputs
![Board of LEDs and Buttons](https://raw.githubusercontent.com/HokieGeek/power-supply-voltage-selector/master/schematics/leds-and-buttons.brd.png)

# Datasheets
* [LM7805CT](http://www.mouser.com/ds/2/149/LM7805-189995.pdf) - 5v voltage regulator for Vic
* [LM7812C](https://www.fairchildsemi.com/datasheets/LM/LM7812.pdf) - 12v voltage regulator for Vcc from wall
* [ATTINY85](http://www.atmel.com/images/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf) - Drives the digi pot and shift register
* [MIC2940](http://www.micrel.com/_PDF/mic2940.pdf) - Adjustable voltage regulator as target output regulator
* [MCP41010](http://ww1.microchip.com/downloads/en/DeviceDoc/11195c.pdf) - 10 kΩ digital potentiometer
* [SN74LS595N](http://www.ti.com/lit/ds/symlink/sn74ls596.pdf) - 8-bit shift registers with latches
