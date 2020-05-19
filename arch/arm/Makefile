

SUBDIRS = common lib versatile

all: arch.a

subdirs:
	for dir in $(SUBDIRS); do $(MAKE) -C $$dir ; done

arch.a: subdirs
	ar cr arch.a common/*.o lib/*.o
	mkdir -p ../../objs/
	cp versatile/start.o ../../objs/

clean:
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir clean; done
	@rm -f ../../objs/start.o
	@rm -f arch.a
