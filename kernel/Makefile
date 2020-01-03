
all: xhyp.a

clean:
	@rm -f *.o *.a

-include ../.config

OBJS := main.o
OBJS +=	domain.o panic.o irq.o scheduler.o timer.o

ifeq ("$(CONFIG_SCHED_POSIX)", "y")
OBJS += sched_posix.o
endif

ifeq ("$(CONFIG_SCHED_ARINC)", "y")
OBJS += sched_arinc.o
endif

ifeq ("$(CONFIG_SCHED_EDF)", "y")
OBJS += sched_edf.o
endif

OBJS += hypercalls.o mm.o tag.o 
OBJS +=	copypage.o io.o event.o

xhyp.a: $(OBJS)
	ar r ../lib/xhyp.a $(OBJS)

