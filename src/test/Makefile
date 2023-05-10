
all: \
	undefined_opcodes_mads.xex \
	test_65c02_ca65.raw \
	lowhigh_mads.xex \
	test_z80.com \


undefined_opcodes_mads.xex: undefined_opcodes_mads.s
	@echo
	mads -o:$@ $<
	@echo

test_65c02_ca65.raw: test_65c02_ca65.s
	@echo
	cl65 -t none -o $@ $<
	@echo

lowhigh_mads.xex: lowhigh_mads.s
	@echo
	mads -o:$@ $<
	@echo

test_z80.com: test_z80.asm
	@echo
	z80asm -v -o $@ $<
	@echo

clean:
	rm -f *~ *.o
	
cleaner: clean
	rm -f *.xex *.raw *.prg *.com
