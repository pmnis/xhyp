
OBJS := abort.o bss.o exceptions.o mmu.o serial.o mmu_dbg.o tlb.o cache.o pl190.o sp804.o

xhyp.a: $(OBJS)
	ar r ../../../lib/xhyp.a $(OBJS)

bss.o: bss.S
	$(CC) $(AFLAGS) $(DEBFLGS) -c bss.S -DTEXT_BASE=$(LD_ADDR) -I $(INCLUDE)

clean:
	@rm -f *.o *.a
