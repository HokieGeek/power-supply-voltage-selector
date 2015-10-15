#!/bin/bash

here=$(cd $(dirname $0); pwd)
src=${1}
bin=`echo ${src} | cut -d'.' -f1`".hex"

${here}/compile.sh ${src} && sudo ${here}/upload.sh ${bin}
