#!/bin/sh

src=$1
file=`basename ${src} | cut -d'.' -f1`
chip=`echo ${src} | cut -d'/' -f1 | tr '[:upper:]' '[:lower:]'`

cpu_freq=""
#cpu_freq="-DF_CPU=128000UL"
#cpu_freq="-DF_CPU=8000000UL"
#cpu_freq="-DF_CPU=12000000UL"
#cpu_freq="-DF_CPU=16000000UL"

echo "src: ${src}"
echo "file: ${file}"
echo "Chip: ${chip}"

#exit 42

cd $chip

echo "Compiling: ${file}.c"
avr-gcc -c -mdeb -mmcu=${chip} -I. -gdwarf-2 ${cpu_freq} -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=./${file}.lst -std=gnu99 ${file}.c -o ${file}.o

echo "Linking: ${file}.elf"
avr-gcc -mmcu=${chip} -I. -mdeb -gdwarf-2 ${cpu_freq} -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Wall -Wstrict-prototypes -Wa,-adhlns=${file}.o  -std=gnu99 -MMD -MP -MF .dep/${file}.elf.d ${file}.o --output ${file}.elf -Wl,-Map=${file}.map,--cref     -lm

echo "Creating load file for Flash: ${file}.hex"
avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature ${file}.elf ${file}.hex

echo "Creating load file for EEPROM: ${file}.eep"
avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" \
        --change-section-lma .eeprom=0 --no-change-warnings -O ihex ${file}.elf ${file}.eep || exit 0

echo "Creating Extended Listing: ${file}.lss"
avr-objdump -h -S -z ${file}.elf > ${file}.lss

echo "Creating Symbol Table: ${file}.sym"
avr-nm -n ${file}.elf > ${file}.sym
