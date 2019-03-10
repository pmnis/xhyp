#!/bin/ksh

[[ $CONFIG_VIRTIO_CONSOLE ]] || exit 0

BASE=$(pwd)
export INCLUDE=${BASE}/include
export CC=${CROSS_COMPILE}gcc
export LD=${CROSS_COMPILE}ld
export AS=${CROSS_COMPILE}as
export OBJCOPY=${CROSS_COMPILE}objcopy
export STRIP=${CROSS_COMPILE}strip

export LD_ADDR=0x00200000

export CFLAGS="-g  -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -DTEXT_BASE=${LD_ADDR} -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -Wall -Wstrict-prototypes -fno-stack-protector -I ${INCLUDE} -Wall -Werror"

export AFLAGS="-D__ASSEMBLY__ -g  -Os   -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -DTEXT_BASE=${LD_ADDR} -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -I ${INCLUDE}"


export LDFLAGS="-Bstatic -T xhyp.ld"

case $1 in
	"clean")
		(cd lib ; make clean || exit 1)
		make clean || exit 1
		rm -f main domain.bin main.striped
		exit 0;
		;;
	*)
		(cd lib ; make || exit 1)
		make || exit 1
		;;
esac

cp main ../../os/virtio_console.dom || exit 1

exit 0
