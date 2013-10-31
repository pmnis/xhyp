
LDFLAGS:= -Bstatic

all: init

init: init.S
	$(CC) $(AFLAGS) -c init.S
	$(LD) $(LDFLAGS) init.o -o init

clean:
	@rm -f *.o
	@rm -f init *.bin
