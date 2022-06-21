#!/bin/bash
# $1 : wcsp_file
# $2 : treedecomp_file

tmp1=.tree-sh.tmp

toulbar2 $1 -B=1 -O=-3 -z=2 -v=1 > $tmp1
rm -rf problem.wcsp*
rm -rf `basename $1`.info

line=`grep -n "Number of" $tmp1 | awk -F'[^0-9]+' '{print $1;}'`
tail -n +${line} $tmp1 > $2
rm -rf $tmp1

