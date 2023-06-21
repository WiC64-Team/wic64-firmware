test_echo !zone test_echo {

!macro status .addr {
    ldx #<.addr
    ldy #>.addr
    jsr status
}

.restart:
    jsr clrhome

    lda #$ff
    sta iterations
    sta iterations+1

.next_iteration
    +incw iterations
    jsr randomize
    jsr echo
    bcs .timeout

    jsr verify
    bcc .next_iteration

    +print .verify_error
    jmp .prompt

.timeout
    +print .timeout_error

.prompt
    +print .press_any_key

.scan_any_key
    +scan key_none
    beq .scan_any_key

    jmp .restart

.verify_error !text red, "=> VERIFY ERROR", green, $0d, $00
.timeout_error !text red, "=> TRANSFER TIMEOUT", green, $0d, $00

.press_any_key
!text $0d, "  -- PRESS ANY KEY TO RESTART TEST --", $0d, $0d
!text      " -- PRESS RESTORE TO RETURN TO MENU --", $00
iterations !word $0000

randomize !zone randomize {
    ; calculate a random payload size up to 16kb
    lda #$04
    sta request_size
-   lda random_byte
    beq -
    cmp #$41
    bcs -
+   sta request_size+1

    ; fill input buffer with random bytes for size+1 pages
    +status .randomizing

    +pointer zp1, request_data
    ldx request_size+1 ; num pages to fill

.next_page
    ldy #$00

.next_byte
    lda random_byte
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
    +pointer zp1, request_data
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
    dec request_size+1
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
    stx .task
    sty .task+1

    sei
    lda $0400
    cmp #$20
    bne +

    jsr home
    +print .text

+   +plot 0, 2
    +wait_raster $30
    +print_indirect .task

    +plot 12, 2
    +wait_raster $30
    lda request_size+1
    jsr hexprint

    +plot 1, 4
    +wait_raster $30
    lda iterations+1
    jsr hexprint

    +plot 3, 4
    +wait_raster $30
    lda iterations
    jsr hexprint

    +plot 0, 17
    cli
    rts

.text
!text "WIC64 TEST: DATA TRANSFER (ECHO $FF)", $0d
!text $0d
!text "           $  00 BYTES OF RANDOM DATA", $0d
!text $0d
!text "$     SUCCESSFUL TRANSFERS", $0d
!text $0d
!text $0d
!text "-- THIS TEST SHOULD RUN INDEFINITELY --", $0d
!text $0d
!text $0d
!text "IF THE ESP IS RESET, THIS TEST SHOULD", $0d
!text "TIME OUT AFTER APPROX. TWO SECONDS.", $0d
!text $0d
!text "IF THIS TEST IS ABORTED, THE ESP SHOULD", $0d
!text "TIME OUT AFTER APPROX. ONE SECOND.", $00
.task !16 $0000
}

} // !zone echo
