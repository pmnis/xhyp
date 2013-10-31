
OBJS := start.o

start.o: start.S
	$(CC) $(AFLAGS) -c start.S -DTEXT_BASE=$(LD_ADDR) -I $(INCLUDE)

clean:
	@rm -f *.o *.a
