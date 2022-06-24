timeout=3600


echo name,cost,cpu_time,real_time
for file in ../../../instances_wcnf/*.wcnf
do
    p=`basename $file .wcnf`
    echo -n $p,
    timeout $timeout python3 rc2.py  $file
    if [ $? -eq 124 ]
    then
        echo NA,TO,TO
    fi
done

cat rc2.sh
