
#lib.a: _udivsi3.o stdlib.o fifo.o ring.o
#	ar r lib.a _udivsi3.o stdlib.o fifo.o ring.o

#_udivsi3.o: _udivsi3.s
#	$(CC) $(AFLAGS) -c _udivsi3.S -DTEXT_BASE=$(LD_ADDR) -I $(INCLUDE)

lib.a: stdlib.o fifo.o ring.o
	ar r lib.a stdlib.o fifo.o ring.o

stdlib.o: stdlib.c $(INCLUDE)/xhyp/stdlib.h

fifo.o: fifo.c $(INCLUDE)/sys/fifo.h

ring.o: ring.c $(INCLUDE)/xhyp/ring.h

clean:
	@rm -f *.o *.a
