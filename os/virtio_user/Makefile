
LDFLAGS:= -Bstatic -T domain.ld


all: main.S virtio_user.o lib/lib.a domain.ld
	$(CC) $(AFLAGS) -c main.S
	$(LD) $(LDFLAGS) main.o virtio_user.o lib/lib.a -o main
	cp main ../../os/virtio.dom

virtio_user.o: virtio_user.c

clean:
	@rm -f *.o
	@rm -f main *.bin
	@rm -f ../../os/virtio.dom
