

DIRS = common lib versatile

all: arch.a

subdirs:
	for dir in $(DIRS); do \
		$(MAKE) -C $$dir; \
	done


arch.a: subdirs
	ar cr arch.a common/*.o lib/*.o
	cp versatile/start.o ../../objs/
