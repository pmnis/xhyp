
LDFLAGS:= -Bstatic -T domain.ld

all: main
	cp main ../../os/console.dom

main: main.S domain.o serial.o lib/lib.a domain.ld
	$(CC) $(AFLAGS) -c main.S
	$(LD) $(LDFLAGS) main.o domain.o serial.o lib/lib.a -o main

domain.o: domain.c

clean:
	@rm -f *.o
	@rm -f main *.bin
	@rm -f ../../os/console.dom
