
all: lxdiag

mconf:  zconf.tab.c lxdiag
	gcc mconf.c zconf.tab.c -o mconf

lxdiag: mconf
	make -C lxdialog

clean:
	make -C lxdialog clean
	rm -f mconf
