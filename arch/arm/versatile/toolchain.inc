
PATH=$PATH:/root/armbin
export CROSS_COMPILE=${CROSS_COMPILE:-arm-none-linux-gnueabi-}

export CC=${CROSS_COMPILE}gcc
export LD=${CROSS_COMPILE}ld
export AS=${CROSS_COMPILE}as

export OBJCOPY=${CROSS_COMPILE}objcopy

export CFLAGS="${DEBFLGS}  -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -Wall -Wstrict-prototypes -fno-stack-protector -I ${INCLUDE} -Wall -Werror"
export AFLAGS="-D__ASSEMBLY__ ${DEBFLGS}  -Os   -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -I ${INCLUDE}"

export DEBFLGS=""
export DEBFLGS="-g"
