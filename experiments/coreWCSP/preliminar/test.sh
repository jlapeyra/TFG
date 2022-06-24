timeout=3600
#timeout=1
solver=../../../wcsp/main

cpu_time_csv=cpu_time.csv
real_time_csv=real_time.csv
cost_csv=cost.csv


headers="name, sat-lb,sat-lub-greedy,sat-lub-model,sat-lub-callback, fc-lb,fc-lub-greedy,fc-lub-model,fc-lub-callback"
echo $headers > $cpu_time_csv
echo $headers > $real_time_csv
echo $headers > $cost_csv

mkdir -p info
rm -f info/*

for file in ../../../instances_preproc/CP13*.wcsp
do
    problem=`basename $file .wcsp`
    echo $problem
    echo -n $problem >> $cpu_time_csv
    echo -n $problem >> $real_time_csv
    echo -n $problem >> $cost_csv
    
    for csp in "sat" "fc"
    do
        for alg in "lb" "lub -v greedy" "lub -v model" "lub -v callback"
        do
            echo $csp $alg
            echo -n , >> $cpu_time_csv
            echo -n , >> $real_time_csv
            echo -n , >> $cost_csv
            alg_o=`echo $alg | cut -d" " -f1`
            hv_o=`echo $alg | cut -d" " -f3`
            out=info/$problem-$csp-$alg_o-$hv_o.out
            err=info/$problem-$csp-$alg_o-$hv_o.err
            
            timeout $timeout $solver $file -c $csp -a $alg > $out 2> $err

            if [ $? -eq 124 ]
            then
                echo -n TO >> $cpu_time_csv
                echo -n TO >> $real_time_csv
                echo -n NA >> $cost_csv
            else
                cost=`tail -n 3 $out | head -n 1 | cut -d" " -f3`
                r_time=`tail -n 2 $out | head -n 1 | cut -d" " -f3`
                p_time=`tail -n 1 $out | head -n 1 | cut -d" " -f3`

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

 
#IFS=$Field_Separator
