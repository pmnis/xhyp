#!/bin/ksh


for dir in serial block network console
do
	[[ -d ${dir} ]] || continue
	print "	- ${dir}"
	[[ -r ${dir}/Build.sh ]] && (cd ${dir} ; ./Build.sh $*)
done
