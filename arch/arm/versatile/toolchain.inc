
export CFLAGS="${DEBFLGS}  -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -Wall -Wstrict-prototypes -fno-stack-protector -I ${INCLUDE} -Wall -Werror"
export AFLAGS="-D__ASSEMBLY__ ${DEBFLGS}  -Os   -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -I ${INCLUDE}"

export DEBFLGS=""
export DEBFLGS="-g"

export CFLAGS="-msoft-float -ffreestanding -nostdinc -marm -mabi=aapcs-linux -march=armv5te -fno-stack-protector -Wstrict-prototypes -Wall -Werror"
export AFLAGS="-Os -msoft-float -ffreestanding -nostdinc -marm -mabi=aapcs-linux -march=armv5te"

export CFLAGS="${DEBFLGS} -msoft-float -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -Wall -Wstrict-prototypes -fno-stack-protector -I ${INCLUDE} -Wall -Werror"
export AFLAGS="-D__ASSEMBLY__ ${DEBFLGS} -Os -msoft-float -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -I ${INCLUDE}"
