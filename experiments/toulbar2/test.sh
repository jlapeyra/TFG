#!/bin/bash

timeout=3600
#timeout=5

mkdir -p info
rm -f info/*

echo name,cost,time
for file in ../../instances_preproc/*.wcsp
do
    problem=`basename $file .wcsp`
    out=info/$problem.out
    err=info/$problem.err
    echo -n $problem,
    timeout $timeout toulbar2 -timer=$timeout $file > $out 2> $err
    result=`grep Optimum $out`
    if [ $? -eq 1 ]
    then
        echo NA,TO
    else
        cost=`echo $result | cut -d" " -f2`
        time=`echo $result | cut -d" " -f15`
        echo $cost,$time
    fi
done
echo timeout=$timeout


