!addr zp1 = $22
!addr zp2 = $50

timeout = $03

!addr random_byte = $d41b

red = $1c
green = $1e
ron = $12
roff = $92

* = $0801 ; 10 SYS 2064 ($0810)
!byte $0c, $08, $0a, $00, $9e, $20, $32, $30, $36, $34, $00, $00, $00

* = $0810
jmp setup

key_none  !byte %00000000, %11111111
key_one   !byte %01111111, %00000001
key_two   !byte %01111111, %00001000
key_three !byte %11111101, %00000001
key_stop  !byte %01111111, %10000000

!src "version.asm"
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
    beq .menu

.quit
    +jmp_via_rti quit

.menu
   +jmp_via_rti menu
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

    ; print menu
    +print .menu_title
    +print version
    +paragraph
    +print .menu_text

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

+   +scan key_three
    beq +

    jsr test_get_ip
    jmp menu

+   jmp .scan

.menu_title
!text "WIC64 TEST FW ", $00

.menu_text
!text "", ron, "1", roff, " DATA TRANSFER", $0d
!text "", ron, "2", roff, " NOISE RESISTANCE", $0d
!text "", ron, "3", roff, " GET IP ADDRESS", $0d
!byte $00
}

!src "tests/echo.asm"
!src "tests/noise.asm"
!src "tests/get_ip.asm"

verify_error_text
!text red, "          => VERIFY ERROR <=", green, $0d, $0d, $00

timeout_error_text
!text red, "       => TRANSFER TIMED OUT <=", green, $0d, $0d, $00

restart_or_return_text
!text "  -- PRESS ANY KEY TO RESTART TEST --", $0d
!text $0d
restore_text
!text " -- PRESS RESTORE TO RETURN TO MENU --", $00

request
request_api  !text "W"
request_size !byte $00, $00
request_id   !byte $00
request_data ; Up to 16kb of random payload data

* = * + $4000

response ; at least 16kb free for response data