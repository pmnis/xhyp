#!/bin/ksh


for dir in serial block network console
do
	[[ -d ${dir} ]] || continue
	print "	- ${dir}"
	[[ -x ${dir}/Build.sh ]] && (cd ${dir} ; ./Build.sh $*)
	[[ $? != 0 ]] && exit 1
done
exit 0
