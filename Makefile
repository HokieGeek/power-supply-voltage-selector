bin=power_supply_voltage_selector.hex
chip=attiny85
cpu_freq=
programmer=usbtiny
port=usb
CC=avr-gcc

DEPS = # eeprom.h # shiftregister.o # spi.o
OBJ = power_supply_voltage_selector.o # eeprom.o # shiftregister.o # spi.o

%.o: %.c
	@echo "== Compiling object file: $@" && \
	$(CC) -c -mdeb -mmcu=$(chip) -I. -gdwarf-2 $(cpu_freq) -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=./$@.lst -std=gnu99 $< -o $@ >/dev/null 2>&1 || true

$(bin): $(OBJ)
	@echo "== Linking" && \
	$(CC) -mmcu=$(chip) -I. -mdeb -gdwarf-2 $(cpu_freq) -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=$<  -std=gnu99 -MMD -MP -MF .dep/$<.elf.d $< --output $<.elf -Wl,-Map=$<.map,--cref -lm && \
 	avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $<.elf $@  && \
 	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ihex $<.elf $<.eep && \
	avr-objdump -h -S -z $<.elf > $<.lss && \
	avr-nm -n $<.elf > $<.sym

 	# @echo "=== Creating load file for Flash" && \
 	# @echo "=== Creating load file for EEPROM" && \
	# @echo "=== Creating Extended Listing" && \
	# @echo "=== Creating Symbol Table" && \

upload: $(bin)
	@echo "== Uploading to chip" && \
	sudo avrdude -p $(chip) -P $(port) -c $(programmer) -U flash:w:$(bin)

clean:
	rm -rf *.{eep,elf,hex,lss,lst,map,o,sym}

.PHONY: clean upload
