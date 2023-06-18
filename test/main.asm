!addr random = $d41b
!addr chrout = $ffd2

!addr zp1 = $22
!addr zp2 = $50

* = $0801 ; 10 SYS 2064 ($0810)
!byte $0c, $08, $0a, $00, $9e, $20, $32, $30, $36, $34, $00, $00, $00

* = $0810
jmp main

!src "util.asm"
!src "universal.asm"

!macro status .addr {
    lda #<.addr
    sta status_task
    lda #>.addr
    sta status_task+1
    jsr status
}

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

    lda #$ff
    sta iterations
    sta iterations+1

loop
    +incw iterations
    jsr randomize
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
iterations !word $0000
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
    +status .randomizing

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
.randomizing !text "GENERATING", $00
}

echo !zone echo {
    timeout = $02
    jsr wic64_init

    +status .sending

    lda #$02
    sta z_timeout
    +pointer $a7, request

    jsr wic64_push

    lda z_timeout
    cmp #$00
    beq .timeout

    +status .receiving

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

.sending !text "SENDING   ", $00
.receiving !text "RECEIVING ", $00
}

verify !zone verify {
    +status .verifying
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

.verifying !text "VERIFYING ", $00
}

status !zone status {
    sei
    lda $0400
    cmp #$20
    bne +

    jsr home
    +print .text

+   +plot 0, 2
    +wait_raster $30
    +print_indirect status_task

    +plot 12, 2
    +wait_raster $30
    lda size+1
    jsr hexprint

    +plot 4, 4
    +wait_raster $30
    lda iterations+1
    jsr hexprint

    +plot 6, 4
    +wait_raster $30
    lda iterations
    jsr hexprint

    +plot 0, 16
    cli
    rts

.text
!text "WIC64 TEST: ECHO ($FF)", $0d, $0d
!text "           $  00 BYTES OF RANDOM DATA", $0d, $0d
!text "=> $     SUCCESSFUL TRANSFERS", $0d, $0d, $0d
!text "-- THIS TEST SHOULD RUN INDEFINITELY --", $0d, $0d, $0d
!text "IF THE ESP IS RESET, THIS TEST SHOULD", $0d
!text "TIME OUT AFTER APPROX. TWO SECONDS.", $0d, $0d
!text "IF THIS TEST IS ABORTED, THE ESP SHOULD", $0d
!text "TIME OUT AFTER APPROX. ONE SECOND.", $00
status_task !16 $0000
}

request !text "W"
size    !byte $00, $00
id      !byte $ff
data    ; Up to 16kb of random payload data

* = * + $4000

response ; Echo'ed payload data