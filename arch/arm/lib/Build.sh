#!/bin/ksh

make $* || exit 2

echo "==========================="

case $1 in
	"clean")
		exit 0
		;;
esac

echo "==========================="
exit 0
