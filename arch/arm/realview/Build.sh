#!/bin/ksh

#(cd lib ; make $*) || exit 2

make $* || exit 2

case $1 in
	"clean")
		exit 0
		;;
esac

mv *.o ../../../objs
exit 0
