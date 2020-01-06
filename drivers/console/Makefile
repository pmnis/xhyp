
LDFLAGS:= -Bstatic -T domain.ld

all: main

ifeq ("$(CONFIG_SCHED_POSIX)", "y")
main: main.S domain.o serial.o lib/lib.a domain.ld
	$(CC) $(AFLAGS) -c main.S
	$(LD) $(LDFLAGS) main.o domain.o serial.o lib/lib.a -o main
endif

ifeq ("$(CONFIG_SCHED_ARINC)", "y")
main: main.S console_arinc.o serial.o lib/lib.a domain.ld
	$(CC) $(AFLAGS) -c main.S
	$(LD) $(LDFLAGS) main.o console_arinc.o serial.o lib/lib.a -o main
endif

ifeq ("$(CONFIG_SCHED_EDF)", "y")
main: main.S console_edf.o serial.o lib/lib.a domain.ld
	$(CC) $(AFLAGS) -c main.S
	$(LD) $(LDFLAGS) main.o console_edf.o serial.o lib/lib.a -o main
endif

console_edf.o: console_edf.c

console_arinc.o: console_arinc.c

domain.o: domain.c

clean:
	@rm -f *.o
	@rm -f main *.bin

