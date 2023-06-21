!addr zp1 = $22
!addr zp2 = $50

!addr random_byte = $d41b

red = $1c
green = $1e

* = $0801 ; 10 SYS 2064 ($0810)
!byte $0c, $08, $0a, $00, $9e, $20, $32, $30, $36, $34, $00, $00, $00

* = $0810
jmp setup

key_none !byte %00000000, %11111111
key_one  !byte %01111111, %00000001
key_two  !byte %01111111, %00001000
key_stop !byte %01111111, %10000000

!src "universal.asm"
!src "util.asm"

setup !zone setup {
    sei

    ; setup nmi
    lda #<nmi
    sta $0318
    lda #>nmi
    sta $0319

    ; setup irq
    lda #<irq
    sta $0314
    lda #>irq
    sta $0315

    ; init keyboard scanning
    lda #$ff
    sta $dc02
	lda #$00
    sta $dc03

    ; setup SID as simple random source
    lda #$ff  ; maximum frequency value
    sta $d40e ; voice 3 frequency low byte
    sta $d40f ; voice 3 frequency high byte
    lda #$80  ; noise waveform, gate bit off
    sta $d412 ; voice 3 control register

    cli
    jmp menu
}

nmi !zone nmi {
    +scan key_stop
    beq .back_to_menu

    +jump_via_rti quit

.back_to_menu
   +jump_via_rti menu
}

irq !zone irq {
    ; disable system irq to avoid kernal keyboard scans
    lda $dc0d ; still need to ack interrupts, though
    jmp $ea81 ; pull regs and rti
}

quit !zone quit {
    rts
}

menu !zone menu {
    ; black background and border
    lda #$00
    sta $d020
    sta $d021

    ; green text
    lda #green
    jsr chrout

    ; clear screen and home cursor
    jsr clrhome

    +print .menu

.scan:
    +scan key_none
    beq .scan

    +scan key_one
    beq +

    jsr test_echo
    jmp menu

+   +scan key_two
    beq +

    jsr test_resistance
    jmp menu

+   jmp .scan

.menu
!text "WIC64 TESTSUITE", $0d, $0d
!text "(1) DATA TRANSFER", $0d
!text "(2) NOISE RESISTANCE", $0d
!byte $0d
!text "(RESTORE) ABORT TEST AND RETURN TO MENU"
!byte $00
}

!src "tests/echo.asm"
!src "tests/noise.asm"

request
request_api  !text "W"
request_size !byte $00, $00
request_id   !byte $ff
request_data ; Up to 16kb of random payload data

* = * + $4000

response ; at least 16kb free for response data