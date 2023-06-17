!addr random = $d41b
!addr chrout = $ffd2

!addr zp1 = $22
!addr zp2 = $50

* = $0801 ; 10 SYS 2064 ($0810)
!byte $0c, $08, $0a, $00, $9e, $20, $32, $30, $36, $34, $00, $00, $00

* = $0810
jmp main

!src "util.asm"

main !zone main {
    ; setup SID as simple random source
    lda #$ff  ; maximum frequency value
    sta $d40e ; voice 3 frequency low byte
    sta $d40f ; voice 3 frequency high byte
    lda #$80  ; noise waveform, gate bit off
    sta $d412 ; voice 3 control register

    ; black background and border
    lda #$00
    sta $d020
    sta $d021

    ; green text
    lda #$05
    sta $0286

    ; clear screen and home cursor
    jsr clrhome

loop
    jsr randomize
    jsr status
    jsr echo
    bcs .timeout

    jsr verify
    bcc loop

    +print .verify_error
    rts

.timeout
    +print .timeout_error
    rts

.verify_error !text "?VERIFY ERROR", $00
.timeout_error !text "?TRANSFER TIMEOUT", $00
}

randomize !zone randomize {
    ; calculate a random payload size up to 16kb
    lda #$04
    sta size
-   lda random
    beq -
    cmp #$41
    bcs -
+   sta size+1

    ; fill input buffer with random bytes for size+1 pages
    +pointer zp1, data
    ldx size+1 ; num pages to fill

.next_page
    ldy #$00

.next_byte
    lda random
    sta (zp1),Y
    dey
    bne .next_byte
    inc zp1+1
    dex
    bne .next_page

    rts
}

echo !zone echo {
    timeout = $02
    jsr wic64_init

    lda #$02
    sta z_timeout
    +pointer $a7, request

    jsr wic64_push

    lda z_timeout
    cmp #$00
    beq .timeout

    lda #$02
    sta z_timeout
    +pointer $a7, response

    jsr wic64_pull

    lda z_timeout
    cmp #$00
    beq .timeout

    jsr wic64_exit

    clc
    rts

.timeout
    sec
    rts
}

verify !zone verify {
    +pointer zp1, data
    +pointer zp2, response

    ldy #$00
    ldx #$00
.loop
    lda (zp1),y
    cmp (zp2),y
    bne .fail
    +incw zp1
    +incw zp2
    inx
    beq .next_page
    jmp .loop

.next_page
    dec size+1
    bne .loop

.success:
    clc
    rts

.fail:
    sec
    rts
}

status !zone status {
    jsr home
    +print .prefix

    lda size+1
    jsr hexprint

    +print .postfix
    rts

.prefix !text "WIC64 TEST: ECHO ($FF)", $0d, $0d, "SENDING $", $00
.postfix !text "00 BYTES OF RANDOM DATA", $0d, $0d, $00
}

!src "universal.asm"

request !text "W"
size    !byte $00, $00
id      !byte $ff
data    ; Up to 16kb of random payload data

* = * + $4000

response ; Echo'ed payload data