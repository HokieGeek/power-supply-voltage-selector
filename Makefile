#!/bin/bash --- BUILD ---

here=$(cd $(dirname $0); pwd)
src=${1}
bin=`echo ${src} | cut -d'.' -f1`".hex"

${here}/compile.sh ${src} && sudo ${here}/upload.sh ${bin}
#!/bin/sh

set -x 
[ $# -gt 0 ] && cd $(cd `dirname $0`; pwd)/$1

rm -rf *.{eep,elf,hex,lss,lst,map,o,sym}

#!/bin/sh --- COMPILE ---

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

#!/bin/bash --- FUSES ---

chip=${1}
device="usbtiny"
port="usb"
fmt="b"

function toHex() {
    bin=`echo $1 | cut -d'b' -f2`
    s="/tmp/toHex.$$.bc"
    {
        echo "obase=16"
        echo "ibase=2"
        echo "${bin}"
        echo "quit"
    } > $s
    hex=`bc -q $s`
    rm -rf $s

    echo -n "$hex"
}

function displayFuse() {
    lbl=${1}
    file=${2}
    bin=`cat ${file}`
    echo -n "${lbl}: ${bin} (0x"
    toHex ${bin}
    echo -n ") "
}

function displayFuses() {
    fH="/tmp/avrfuses.$$.high.bin"
    fL="/tmp/avrfuses.$$.low.bin"
    fE="/tmp/avrfuses.$$.extended.bin"

    avrdude -p ${chip} -P ${port} -c ${device} -U hfuse:r:${fH}:${fmt} -U lfuse:r:${fL}:${fmt} -U efuse:r:${fE}:${fmt}

    echo "================================ FUSES ================================"
    displayFuse "LOW" ${fL}
    displayFuse "HIGH" ${fH}
    [ -f ${fE} ] && displayFuse "EXT" ${fE}
    echo ""
    echo "======================================================================="

    rm -rf ${fileH} ${fileL} ${fileE}
}

displayFuses

#!/bin/sh --- UPLOAD ---

bin=${1}
chip=`echo ${bin} | cut -d'/' -f1 | tr '[:upper:]' '[:lower:]'`

echo "Chip: ${chip}"
echo "Binary: ${bin}"

avrdude -p ${chip} -P usb -c usbtiny -U flash:w:${bin}
