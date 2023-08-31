ASM = acme
ASMFLAGS = -f cbm -l main.sym -v3 --color -Wno-label-indent
SOURCES = *.asm

.PHONY: all clean

all: update.prg

update.prg: clean $(SOURCES)
	$(ASM) $(ASMFLAGS) -o update.prg  update.asm

clean:
	rm -f *.{prg,sym}
