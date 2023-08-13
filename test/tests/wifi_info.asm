test_wifi_info !zone test_wifi_info {

.restart
    jsr clrhome
    +print .text

    +print .text_ssid
    lda #$10
    sta request_id

    jsr get_info
    bcc +
    jmp .timeout

+   +print_ascii response
    +newline

    +print .text_rssi
    lda #$11
    sta request_id

    jsr get_info
    bcc +
    jmp .timeout

+   +print_ascii response
    +newline

    +print .text_ip
    lda #$06
    sta request_id

    jsr get_info
    bcc +
    jmp .timeout

+   +print response
    +newline

    +print .text_mac
    lda #$14
    sta request_id

    jsr get_info
    bcc +
    jmp .timeout

+   +print response
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
!text "wIc64 tEST: wIfI iNFO ($10,$11,$06,$14)", $0d
!text $0d, $00
.text_ssid
!text "ssid: ", $00
.text_rssi
!text "rssi: ", $00
.text_ip
!text "addr: ", $00
.text_mac
!text "mac : ", $00

get_info !zone wifi_info {
    ; need to null the response buffer
    ; because strings are always send WITHOUT
    ; a terminating nullbyte by the original
    ; firmware...

    ldy #$40  ; max length of SSID (63) + 1 nullbyte
    lda #$00
    +pointer wic64_response_pointer, response
-   sta (wic64_response_pointer),y
    dey
    bne -

    lda #$04
    sta request_size

    lda #$00
    sta request_size+1

    +wic64_execute request, response, timeout
    rts
}

} // !zone test_wifi_info