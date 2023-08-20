ASM = acme
ASMFLAGS = -f cbm -l main.sym -v3 --color -Wno-label-indent
SOURCES = *.asm tests/*.asm generated-version.asm

.PHONY: all clean generated-version.asm

all: main.prg

generated-version.asm:
	./make-version.sh

main.prg: clean $(SOURCES)
	$(ASM) $(ASMFLAGS) -o main.prg  main.asm

small.prg: clean $(SOURCES)
	$(ASM) $(ASMFLAGS) -Dwic64_optimize_for_size=1 -o small.prg main.asm

clean:
	rm -f *.{prg,sym,bin}
	rm -f generated-version.asm
