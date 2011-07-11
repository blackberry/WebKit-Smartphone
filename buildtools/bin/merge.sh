#!/bin/sh

# Passing the following parameters to mergetool:
#  local base remote merge_result

alocal=$1
base=$2
remote=$3
result=$4

if [ -f $base ]
then
	p4merge.exe -dl "$base" "$alocal" "$remote" "$result" 
else
	p4merge.exe -dl "$result" "$alocal" "$remote" "$result" 
fi
