ASM = acme
ASMFLAGS = -f cbm -l main.sym -v3 --color -Wno-label-indent
SOURCES = *.asm tests/*.asm generated-version.asm

.PHONY: all clean version.asm

all: main.prg

generated-version.asm:
	./make-version.sh

main.prg: $(SOURCES)
	$(ASM) $(ASMFLAGS) -o main.prg  main.asm

clean:
	rm -f *.{prg,sym}
	rm -f generated-version.asm
