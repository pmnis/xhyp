
all: xhyp.a

clean:
	@rm -f *.o *.a

-include ../.config

#OBJS := main.o
OBJS =	domain.o panic.o irq.o scheduler.o timer.o

ifeq ("$(CONFIG_SCHED_POSIX)", "y")
OBJS += sched_posix.o
endif

ifeq ("$(CONFIG_SCHED_ARINC)", "y")
OBJS += sched_arinc.o
endif

OBJS += hypercalls.o mm.o tag.o 
OBJS +=	copypage.o event.o

ifeq ("$(CONFIG_QPORT_CONSOLE)", "y")
OBJS += io.o
endif


xhyp.a: main.o $(OBJS)
	ar r ../lib/xhyp.a $(OBJS)
	cp main.o ../objs/

