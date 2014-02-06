

all: clean
	./Build.sh

menuconfig:
	./Build.sh menuconfig

help:
	./Build.sh help

cscope:
	./Build.sh cscope

clean:
	./Build.sh clean

mrproper:
	./Build.sh mrproper

