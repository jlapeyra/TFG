for dir in instances_preproc instances_wcnf
do
    cd $dir
    for i in *
    do
        mv $i CP13-${i#*-}
    done
    cd ..
done