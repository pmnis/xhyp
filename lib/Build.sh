#!/bin/ksh



case $1 in
	"clean")
		make clean || exit 2
		exit 0
		;;
	*)
		make $* || exit 2
		mv *.o ../objs
		;;
esac
