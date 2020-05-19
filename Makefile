
export BASEDIR = $(PWD)
export INCLUDE = $(BASEDIR)/include

export CFLAGS =  -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -Wall -Wstrict-prototypes -fno-stack-protector -I $(INCLUDE) -Wall -Werror

export AFLAGS = -D__ASSEMBLY__  -Os   -fno-common -ffixed-r8 -msoft-float  -D__KERNEL__ -fno-builtin -ffreestanding -nostdinc -pipe  -DCONFIG_ARM -D__ARM__ -marm  -mabi=aapcs-linux -mno-thumb-interwork -march=armv5te -I $(INCLUDE)

export CROSS_COMPILE = arm-linux-

export CC = $(CROSS_COMPILE)gcc
export AS = $(CROSS_COMPILE)gcc
export LD = $(CROSS_COMPILE)ld


SUBDIRS = lib kernel arch/arm os drivers domains

LIBS = arch/arm/arch.a lib/xhyp.a lib/mlib.a lib/lib.a generated/generated.a

all: subdirs generated
	$(LD)  objs/start.o objs/main.o $(LIBS) domains/domain01/domain.ho domains/domain02/domain.ho  domains/domain03/domain.ho  domains/domain04/domain.ho  -Bstatic -T xhyp.ld -o xhyp

subdirs:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir; done

#arinc:
#	./scripts/Build_arinc # > generated/arinc_table.c

clean:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
	@rm -rf generated
	@rm -f xhyp xhyp.ld

cscope:
	find . -name "*.[hScs]" > cscope.files
