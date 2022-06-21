#!/bin/bash

input=$1
output=$2

tmp1=.preproc.tmp

toulbar2 -z=2 -A $input > $tmp1
mv problem.wcsp $output
rm -rf problem.wcsp.degree
rm -rf problem.wcsp.dot
rm -rf `basename $input`.info

rm $tmp1
