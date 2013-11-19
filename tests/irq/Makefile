
LDFLAGS:= -Bstatic -T domain.ld

all: irq_test

irq_test: main.o domain.o lib/lib.a domain.ld
	$(LD) $(LDFLAGS) main.o domain.o lib/lib.a -o irq_test

domain.o: domain.c
	$(CC) $(CFLAGS) -c domain.c

main.o: main.S
	$(CC) $(AFLAGS) -c main.S

clean:
	@rm -f *.o
	@rm -f main *.bin
