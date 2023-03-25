
all: \
	undefined_opcodes_mads.xex \
	test_65c02_ca65.raw \
	lowhigh_mads.xex

undefined_opcodes_mads.xex: undefined_opcodes_mads.s
	mads -o:$@ $<

test_65c02_ca65.raw: test_65c02_ca65.s
	cl65 -t none -o $@ $<

lowhigh_mads.xex: lowhigh_mads.s
	mads -o:$@ $<

clean:
	rm -f *~ *.o
	
cleaner: clean
	rm -f *.xex *.raw *.prg
