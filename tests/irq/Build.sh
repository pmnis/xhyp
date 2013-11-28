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

#export CFLAGS="-g  -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -DTEXT_BASE=${LD_ADDR} -fno-builtin -ffreestanding -nostdinc -isystem /root/ARM_SDK/arm-2008q3/bin/../lib/gcc/arm-none-linux-gnueabi/4.3.2/include -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -Wall -Wstrict-prototypes -fno-stack-protector -I ${INCLUDE} -Wall -Werror"

#export AFLAGS="-D__ASSEMBLY__ -g  -Os   -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -DTEXT_BASE=${LD_ADDR} -fno-builtin -ffreestanding -nostdinc -isystem /root/ARM_SDK/arm-2008q3/bin/../lib/gcc/arm-none-linux-gnueabi/4.3.2/include -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -I ${INCLUDE}"


#export LDFLAGS="-Bstatic -T xhyp.ld"

# -Ttext ${LD_ADDR}



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

