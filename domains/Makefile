
all: link_script arinc domains $(BASEDIR)/generated/generated.a

$(BASEDIR)/generated/generated.a: $(BASEDIR)/generated/domain_table.o $(BASEDIR)/generated/arinc_table.o
	@ar r $(BASEDIR)/generated/generated.a \
		$(BASEDIR)/generated/domain_table.o \
		$(BASEDIR)/generated/arinc_table.o

link_script:
	@./Build_ld.sh

arinc:
	@./Build_arinc.sh

domains:
	@./Build.sh

clean:
	@rm -rf domain??
