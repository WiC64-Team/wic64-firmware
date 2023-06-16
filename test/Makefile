ASM = acme
ASMFLAGS = -f cbm -l main.sym -v3
SOURCES = main.asm util.asm universal.asm

all: main.prg

main.prg: $(SOURCES)
	$(ASM) $(ASMFLAGS) -o $@ main.asm

clean:
	rm -f main.{prg,sym}
