
LDFLAGS:= -Bstatic -T domain.ld


all: main

main: main.S virtio_serial.o lib/lib.a domain.ld
	$(CC) $(AFLAGS) -c main.S
	$(LD) $(LDFLAGS) main.o virtio_serial.o lib/mlib.a lib/lib.a -o main
	cp main ../../os/virtio-serial.dom

virtio_user.o: virtio_user.c

clean:
	@rm -f *.o
	@rm -f main *.bin
	@rm -f ../../os/virtio-serial.dom
