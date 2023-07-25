ASM = acme
ASMFLAGS = -f cbm -l main.sym -v3 --color -Wno-label-indent
VERSION = $(shell git describe --tags --dirty)
SOURCES = *.asm tests/*.asm version.asm

.PHONY: all clean version.asm

all: main.prg

version.asm:
	./make-version.sh

main.prg: $(SOURCES)
	$(ASM) $(ASMFLAGS) -o main.prg  main.asm

clean:
	rm -f *.{prg,sym}
	rm -f version.asm
