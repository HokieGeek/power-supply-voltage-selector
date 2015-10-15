#!/bin/sh

set -x 
[ $# -gt 0 ] && cd $(cd `dirname $0`; pwd)/$1

rm -rf *.{eep,elf,hex,lss,lst,map,o,sym}
