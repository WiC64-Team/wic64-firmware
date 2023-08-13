test_version !zone test_version {

.restart
    jsr clrhome
    +print .text

    +print .text_string

    lda #$04
    sta request_size

    lda #$00
    sta request_size+1

    lda #$00
    sta request_id

    jsr .get_version
    bcc +
    jmp .timeout

+   +print_ascii response
    +paragraph

    lda #$26
    sta request_id

    jsr .get_version
    bcc +
    jmp .timeout

+   +print .text_major
    lda response
    jsr hexprint
    +newline

    +print .text_minor
    lda response+1
    jsr hexprint
    +newline

    +print .text_patch
    lda response+2
    jsr hexprint
    +newline

    +paragraph
    jmp .prompt

.timeout
    +newline
    +paragraph
    +print timeout_error_text

.prompt
    +restart_or_return_prompt .restart

.text
!text "wIc64 tEST: vERSION ($00, $26)", $0d
!text $0d
!text $00

.text_string
!text "sTRING: ", $00

.text_major
!text "mAJOR: $", $00

.text_minor
!text "mAJOR: $", $00

.text_patch
!text "pATCH: $", $00

.get_version !zone get_version {
    ; null response buffer
    ; REDESIGN: always include terminating nullbyte when sending strings

    ldy #$00
    lda #$00
    +pointer wic64_response_pointer, response
-   sta (wic64_response_pointer),y
    dey
    bne -

    +wic64_execute request, response, timeout
    rts
}

} // !zone test_version