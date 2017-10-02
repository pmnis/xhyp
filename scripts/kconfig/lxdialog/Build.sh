#!/bin/sh

build_it()
{
gcc -D_GNU_SOURCE -DCURSES_LOC="<ncurses.h>" -o lxdialog -I /usr/include checklist.c menubox.c textbox.c yesno.c inputbox.c util.c lxdialog.c msgbox.c -lncurses -ltinfo
}

case $1 in
	"clean")
		rm -f *.o lxdialog
		;;
	*)
		build_it
		;;
esac
