#!/bin/bash

timeout=3600
#timeout=1
solver=MaxHS/bin/maxhs

mkdir -p info
rm -f info/*

echo name,cost,time
for file in ../../instances_wcnf/*.wcnf
do
    problem=`basename $file .wcnf`
    out=info/$problem.out
    err=info/$problem.err
    echo -n $problem,
    timeout $timeout $solver $file > $out 2> $err
    if [ $? -eq 124 ]
    then
        echo NA,TO
    else
        cost=`grep -w o $out | cut -d" " -f2`
        time=`grep -w "c CPU" $out | cut -d" " -f3`
        echo $cost,$time
    fi
done
echo timeout=$timeout


