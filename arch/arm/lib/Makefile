#lib.a: _udivsi3.o stdlib.o fifo.o ring.o
#       ar r lib.a _udivsi3.o stdlib.o fifo.o ring.o
#
#       #_udivsi3.o: _udivsi3.s
#       $(CC) $(AFLAGS) -c _udivsi3.S -DTEXT_BASE=$(LD_ADDR) -I $(INCLUDE)
#
OBJS := _udivsi3.o _divsi3.o _umodsi3.o

mlib.a: $(OBJS)
	ar r ../../../lib/mlib.a $(OBJS)

_divsi3.o: _divsi3.S
	$(CC) $(AFLAGS) -c _divsi3.S -I $(INCLUDE)

_udivsi3.o: _udivsi3.S
	$(CC) $(AFLAGS) -c _udivsi3.S -I $(INCLUDE)

_umodsi3.o: _umodsi3.S
	$(CC) $(AFLAGS) -c _umodsi3.S -I $(INCLUDE)

clean:
	@rm -f *.o *.a

