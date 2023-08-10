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
    sta iterations+2

.next_iteration
    +inc24 iterations
    jsr randomize
    jsr echo
    bcs .timed_out

    jsr verify
    bcc .next_iteration

    +print verify_error_text
    jmp .prompt

.timed_out
    +print timeout_error_text

.prompt
    +restart_or_return_prompt .restart

iterations !byte $00, $00, $00, $00

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
    +status .generating

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
.generating !text "gENERATING", $00
}

echo !zone echo {
    lda #$ff
    sta request_id

    lda #timeout
    sta wic64_timeout

    +pointer wic64_request_pointer, request
    +pointer wic64_response_pointer, response

    jsr wic64_initialize

    +status .sending

    jsr wic64_send
    bcs +

    +status .receiving

    jsr wic64_prepare_receive
    bcs +

    jsr wic64_receive

+   jsr wic64_finalize
    rts

.sending !text "sENDING   ", $00
.receiving !text "rECEIVING ", $00
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

.verifying !text "vERIFYING ", $00
}

status !zone status {
    stx .task
    sty .task+1

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
    lda iterations+2
    jsr hexprint

    +plot 3, 4
    +wait_raster $30
    lda iterations+1
    jsr hexprint

    +plot 5, 4
    +wait_raster $30
    lda iterations
    jsr hexprint

    +plot 0, 17
    rts

.text
!text "wIc64 tEST: dATA tRANSFER (eCHO $FF)", $0d
!text $0d
!text "           $  00 BYTES OF RANDOM DATA", $0d
!text $0d
!text "$       SUCCESSFUL TRANSFERS", $0d
!text $0d
!text $0d
!text "-- tHIS TEST SHOULD RUN INDEFINITELY --", $0d
!text $0d
!text $0d
!text "iF THE esp IS RESET, THIS TEST SHOULD", $0d
!text "TIME OUT AFTER APPROX. TWO SECONDS.", $0d
!text $0d
!text "iF THIS TEST IS ABORTED, THE esp SHOULD", $0d
!text "TIME OUT AFTER APPROX. ONE SECOND.", $0d, $00
.task !16 $0000
}

} // !zone echo
