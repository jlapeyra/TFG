#!/bin/bash
# USAGE: ./wcsp2sat.sh in.wcsp out.wcnf

./wcsp2sat.x $1 > $1.tmp
tail -1 $1.tmp > $2
head -n -1 $1.tmp >> $2
rm $1.tmp $1.err.tmp
