test_resistance !zone test_resistance {

    ; Simulate code that just blindly keeps sending random
    ; data to the ESP despite the occurrence of timeouts,
    ; like previous versions of the universal routines did.

    ; Since those versions are still out there, we need to
    ; make sure the ESP can handle such behaviour without
    ; ISR or Task watchdogs triggering and without running
    ; into exceptions or running out of memory.

    ; We'll use the global response buffer for the data.

    jsr clrhome

    lda #$ff
    sta iterations
    sta iterations+1

.loop:
    +incw iterations
    jsr .status
    jsr .randomize
    jsr .send
    jmp .loop

.randomize !zone randomize {

    ; randomize the response buffer and make sure there
    ; are some api ids ($57 'W') sprinkled into it.

    +pointer zp1, response

    ldx #$20
.next_page
    ldy #$00
.next_byte
    lda random_byte
    sta (zp1),y
    iny
    bne .next_byte

    inc zp1+1
    dex
    bne .next_page


    ldx #$10
    ldy #$00
.next
    +random_byte >response, (>response)+$20
    sta zp1+1

    lda random_byte
    sta zp1

    lda #$57
    sta (zp1),y

    dex
    bne .next

    rts
}

.send !zone send {
    ; change esp data direction to input
    lda $dd00
	ora #$04
	sta $dd00

    ; set port to output
	lda $dd0d
	lda #$ff
	sta $dd03

    +pointer zp1, response
    ldx #$20

.next_page
    ldy #$00

.next_byte
    lda (zp1),y
    sta $dd01

    ;setup timeout
    lda #$10
    sta .timeout
    lda #$00
    sta .timeout+1

    ; wait for handshake
-	lda $dd0d
	and #$10
	bne +

    +decw .timeout
    lda .timeout+1
    bne -
    lda .timeout
    bne -

+   iny
    bne .next_byte

    inc zp1+1
    dex
    bne .next_page

    ; set port back to input
	lda $dd0d
	lda #$ff
	sta $dd03

    rts
.timeout !16 $0000
}

.status !zone status {
    sei

    lda $0400
    cmp #$20
    bne +

    jsr home
    +print .text

+   +plot 1, 4
    +wait_raster $30
    lda iterations+1
    jsr hexprint

    +plot 3, 4
    +wait_raster $30
    lda iterations
    jsr hexprint

    cli
    rts

.text
!text "WIC64 TEST: NOISE RESISTANCE", $0d, $0d
!text "SENDING $2000 BYTES OF LINE NOISE", $0d
!text $0d
!text "$     TEST ITERATIONS", $0d
!text $0d
!text $0d
!text "-- THIS TEST SHOULD RUN INDEFINITELY --", $0d
!text $0d
!text $0d
!text "THE ESP SHOULD NOT RESET ITSELF DUE TO", $0d
!text "WATCHDOG TIMERS TRIGGERING, EXCEPTIONS", $0d
!text "BEING THROWN OR MEMORY ALLOCATIONS", $0d
!text "FAILING.", $0d
!text $0d
!text "THIS ALSO APPLIES WHEN THE ESP IS RESET", $0d
!text "MANUALLY WHILE THIS TEST IS RUNNING."
!text $00
}

} // !zone test_resistance
