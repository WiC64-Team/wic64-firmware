test_post !zone test_post {

.restart:
    jsr clrhome

    lda #$ff
    sta iterations
    sta iterations+1
    sta iterations+2

    ; set post command id
    lda #$24
    sta request_id

    ; copy post url
    ldx #post_url_length
-   lda post_url,x
    sta request_data,x
    dex
    bpl -

.next_iteration
    +inc24 iterations
    jsr .randomize
    jsr .post
    bcs .timed_out

    jsr .verify
    bcc .next_iteration

    +print verify_error_text
    jmp .prompt

.timed_out
    +print timeout_error_text

.prompt
    +restart_or_return_prompt .restart

.randomize !zone randomize {
    ; calculate a random payload size up to 16kb
    lda #$04
    sta request_size
-   lda random_byte
    beq -
    cmp #$41
    bcs -
+   sta request_size+1

    ; fill post_data with random bytes for size+1 pages
    +status .generating, status_post

    +pointer zp1, post_data
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

    ; now add length of post url to request size
    clc
    lda #post_url_length
    adc request_size
    sta request_size
    bcc +
    inc request_size+1
+   rts
.generating !text "gENERATING", $00
}

.post !zone post {
    ; slightly larger timeout required since posting the data
    ; takes some more time than a simple get request
    lda #$04
    sta wic64_timeout

    +pointer wic64_request_pointer, request
    +pointer wic64_response_pointer, response

    jsr wic64_initialize

    +status .sending, status_post

    jsr wic64_send
    bcs +

    +status .receiving, status_post

    jsr wic64_prepare_receive
    bcs +

    jsr wic64_receive

+   jsr wic64_finalize
    rts

.sending !text "pOSTING   ", $00
.receiving !text "rECEIVING ", $00
}

.verify !zone verify {
    +status .verifying, status_post
    +pointer zp1, post_data
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

status_post !zone status_post {
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
!text "wIc64 tEST: http post ($24)", $0d
!text $0d
!text "           $  00 BYTES OF RANDOM DATA", $0d
!text $0d
!text "$       SUCCESSFUL POST REQUESTS", $0d
!text $0d
!text $0d
!text "-- tHIS TEST SHOULD RUN INDEFINITELY --", $0d
!text $0d
!text $0d
!text "iF THE esp IS RESET, THIS TEST SHOULD", $0d
!text "TIME OUT AFTER APPROX. FOUR SECONDS.", $0d
!text $0d
!text "iF THIS TEST IS ABORTED, THE esp SHOULD", $0d
!text "TIME OUT AFTER APPROX. ONE SECOND.", $0d
!text $00
.task !16 $0000
}

post_url !text "http://www.henning-liebenau.de/post-echo.php"
!byte $00
post_url_end

post_url_length = post_url_end - post_url
post_data = request_data + post_url_length

} // !zone test_post
