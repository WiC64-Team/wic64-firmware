ASM = acme
ASMFLAGS = -f cbm -l main.sym -v3 --color -Wno-label-indent
SOURCES = *.asm tests/*.asm

.PHONY: all clean

all: main.prg

main.prg: clean $(SOURCES)
	$(ASM) $(ASMFLAGS) -o main.prg  main.asm

small.prg: clean $(SOURCES)
	$(ASM) $(ASMFLAGS) -Dwic64_optimize_for_size=1 -o small.prg main.asm

clean:
	rm -f *.{prg,sym,bin}
