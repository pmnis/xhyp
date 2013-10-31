#!/bin/ksh

DIRS="common lib"

for dir in ${DIRS}
do
	(
	cd ${dir}
	./Build.sh $*
	) || exit 2
done
exit 0
