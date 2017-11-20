#!/bin/ksh

PATH=$PATH:/root/armbin
BASE=$(pwd)
export INCLUDE=${BASE}/include

export BOARD=${BOARD:-versatile}

export DEBFLGS="-g"

# export tool chain you find in toolchain.inc
source ../../arch/${ARCH}/${BOARD}/toolchain.inc


#export CROSS_COMPILE=${CROSS_COMPILE:-arm-none-linux-gnueabi-}
export CC=${CROSS_COMPILE}gcc
export LD=${CROSS_COMPILE}ld
export AS=${CROSS_COMPILE}as
export OBJCOPY=${CROSS_COMPILE}objcopy
export STRIP=${CROSS_COMPILE}strip

export LD_ADDR=0x00200000


case $1 in
	"clean")
		(cd lib ; make clean || exit 1)
		make clean || exit 1
		rm -f irq_test domain.bin main.striped
		exit 0;
		;;
	*)
		(cd lib ; make || exit 1)
		make || exit 1
		;;
esac

cp irq_test ../../os/irq_test.dom

