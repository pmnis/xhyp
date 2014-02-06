
all: lib.a

lib.a: _udivsi3.o stdlib.o fifo.o
	ar r lib.a _udivsi3.o stdlib.o fifo.o

_udivsi3.o: _udivsi3.s
	$(CC) $(AFLAGS) -c _udivsi3.S -DTEXT_BASE=$(LD_ADDR) -I $(INCLUDE)

stdlib.o: stdlib.c $(INCLUDE)/xhyp/stdlib.h

fifo.o: fifo.c $(INCLUDE)/sys/fifo.h

clean:
	@rm -f *.o *.a
