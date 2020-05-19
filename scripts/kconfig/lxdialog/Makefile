

all: lxdialog

lxdialog: checklist.c menubox.c textbox.c yesno.c inputbox.c util.c lxdialog.c msgbox.c
	gcc -D_GNU_SOURCE -DCURSES_LOC="<ncurses.h>" -o lxdialog -I /usr/include checklist.c menubox.c textbox.c yesno.c inputbox.c util.c lxdialog.c msgbox.c -lncurses -ltinfo

clean:
	rm -f lxdialog *.o
