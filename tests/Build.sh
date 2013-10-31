#!/bin/ksh


for dir in *
do
	[[ -d ${dir} ]] || continue
	print "	- ${dir}"
	[[ -r ${dir}/Build.sh ]] && (cd ${dir} ; ./Build.sh $*)
done
