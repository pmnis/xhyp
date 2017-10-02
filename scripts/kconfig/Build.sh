#!/usr/bin/ksh

function build_it
{
	gcc mconf.c zconf.tab.c -o mconf
}

case $1 in
	"clean")
		rm -f *.o mconf 
		;;
	*)
		build_it
		;;
esac

cd lxdialog
./Build.sh $*

exit 0
