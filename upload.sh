#!/bin/sh

bin=${1}
chip=`echo ${bin} | cut -d'/' -f1 | tr '[:upper:]' '[:lower:]'`

echo "Chip: ${chip}"
echo "Binary: ${bin}"

avrdude -p ${chip} -P usb -c usbtiny -U flash:w:${bin}
