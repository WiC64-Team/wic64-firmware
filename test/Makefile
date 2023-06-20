ASM = acme
ASMFLAGS = -f cbm -l main.sym -v3
SOURCES = *.asm tests/*.asm

.PHONY: all clean

all: main.prg

main.prg: $(SOURCES)
	$(ASM) $(ASMFLAGS) -o main.prg  main.asm

clean:
	rm -f *.{prg,sym}
