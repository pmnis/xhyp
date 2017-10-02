#!/bin/ksh

export ABI="aapcs-linux"
export ABI="iwmmxt"
export ABI="atpcs"
export ABI="apcs-gnu"
export CFLAGS="${DEBFLGS}  -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -mabi=${ABI} -mno-thumb-interwork -mcpu=cortex-a8 -march=armv7-a -Wall -Wstrict-prototypes -fno-stack-protector -I ${INCLUDE} -Wall -Werror -marm"
export AFLAGS="-D__ASSEMBLY__ ${DEBFLGS}  -Os   -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__  -mabi=${ABI} -mno-thumb-interwork -mcpu=cortex-a8 -march=armv7-a -I ${INCLUDE} -marm"

export DEBFLGS="-g"
export DEBFLGS=""

