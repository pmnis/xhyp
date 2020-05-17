
#export INCLUDE = /home/pmorel/Tests/include
export INCLUDE = $(PWD)/include

export CFLAGS =  -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -Wall -Wstrict-prototypes -fno-stack-protector -I $(INCLUDE) -Wall -Werror

export AFLAGS = -D__ASSEMBLY__  -Os   -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -I $(INCLUDE)

export CROSS_COMPILE = arm-linux-

export CC = $(CROSS_COMPILE)gcc
export AS = $(CROSS_COMPILE)gcc
export LD = $(CROSS_COMPILE)ld


SUBDIRS = lib kernel arch/arm os drivers generated

LIBS = arch/arm/arch.a lib/xhyp.a lib/mlib.a lib/lib.a generated/generated.a

all: subdirs generate
	$(LD)  objs/start.o $(LIBS) domains/domain01/domain.ho domains/domain02/domain.ho  domains/domain03/domain.ho  domains/domain04/domain.ho  -Bstatic -T xhyp.ld -o xhyp

subdirs: generate
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir; done


generate: arinc domains

arinc:
	./scripts/Build_arinc > generated/arinc_table.c

domains:
	./scripts/Build_ld

clean:
	find . -name "*.o"  -exec rm {} \;
	find . -name "*.a" -exec rm {} \;
	rm -f xhyp
