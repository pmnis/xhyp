#!/bin/ksh

LDFLAGS=$(sh check-lxdialog.sh -ldflags gcc)
CCFLAGS=$(sh check-lxdialog.sh -ccflags gcc)


function build_it
{
gcc ${LDFLAGS} ${CCFLAGS} -o lxdialog \
	checklist.c menubox.c textbox.c yesno.c inputbox.c \
	util.c lxdialog.c msgbox.c 
}

case $1 in
	"clean")
		rm -f *.o lxdialog
		;;
	*)
		build_it
		;;
esac
exit 0
gcc ${LDFLAGS} ${CCFLAGS} -DCURSES_LOC="<ncurses.h>" -o lxdialog \
	checklist.c menubox.c textbox.c yesno.c inputbox.c \
	util.c lxdialog.c msgbox.c 
