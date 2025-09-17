
OBJS := start.o lib.o

all: lib.o start.o 

lib.o: lib.S
	$(CC) $(AFLAGS) -c lib.S -DTEXT_BASE=$(LD_ADDR) -I $(INCLUDE)

start.o: start.S
	$(CC) $(AFLAGS) -c start.S -DTEXT_BASE=$(LD_ADDR) -I $(INCLUDE)

clean:
	@rm -f *.o *.a
