#!/bin/bash

timeout=3600
#timeout=1
solver=../../../maxSAT/MaxSAT.py

cpu_time_csv=cpu_time.csv
real_time_csv=real_time.csv
cost_csv=cost.csv


headers=",lb-hitman,lb-cplex,lb-static,lb-dynamic,lb-dynamic+, lub-hitman,lb-cplex,lub-static,lub-dynamic,lub-dynamic+,"
echo $headers > $cpu_time_csv
echo $headers > $real_time_csv
echo $headers > $cost_csv

mkdir -p info
rm -f info/*

for file in ../../../instances_wcnf/CP13*.wcnf
do
    problem=`basename $file .wcnf`
    echo -n $problem >> $cpu_time_csv
    echo -n $problem >> $real_time_csv
    echo -n $problem >> $cost_csv
    echo $problem

    for alg in "lb" "lub"
    do
        for hit in "hitman" "cplex" "static" "dynamic"
        do
            echo $alg $hit
            echo -n , >> $cpu_time_csv
            echo -n , >> $real_time_csv
            echo -n , >> $cost_csv
            out=info/$problem-$alg-$hit.out
            timeout $timeout python3 $solver $file -a $alg -s $hit > $out
            if [ $? -eq 124 ]
            then
                echo -n TO >> $cpu_time_csv
                echo -n TO >> $real_time_csv
                echo -n NA >> $cost_csv
            else
                cost=`tail -n 3 $out | head -n 1 | cut -d" " -f2`
                r_time=`tail -n 2 $out | head -n 1 | cut -d" " -f2`
                p_time=`tail -n 1 $out | head -n 1 | cut -d" " -f2`

                echo -n $cost >> $cost_csv
                echo -n $r_time >> $real_time_csv
                echo -n $p_time >> $cpu_time_csv
            fi
        done
    done
    echo >> $cpu_time_csv
    echo >> $real_time_csv
    echo >> $cost_csv
done

echo timeout=$timeout
echo timeout=$timeout >> $cpu_time_csv
echo timeout=$timeout >> $real_time_csv
