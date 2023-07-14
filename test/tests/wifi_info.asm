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

+   +print response
    +newline

    +print .text_rssi
    lda #$11
    sta request_id

    jsr get_info
    bcc +
    jmp .timeout

+   +print response
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
!text "WIC64 TEST: WIFI INFO ($10,$11,$06,$14)", $0d
!text $0d, $00
.text_ssid
!text "SSID: ", $00
.text_rssi
!text "RSSI: ", $00
.text_ip
!text "ADDR: ", $00
.text_mac
!text "MAC:: ", $00

.paragraph
!text $0d, $0d, $00

get_info !zone wifi_info {
    ; FIXME: Firmware sometimes fails on fast succession of commands
    ldy #$00
    ldx #$00
-   dey
    bne -
    dex
    bne -

    ; need to null the response buffer
    ; because strings are always send WITHOUT
    ; a terminating nullbyte by the original
    ; firmware...

    ldy #$40  ; max length of SSID (63) + 1 nullbyte
    lda #$00
    +pointer $a7, response
-   sta ($a7),y
    dey
    bne -

    lda #$04
    sta request_size

    lda #$00
    sta request_size+1

    jsr wic64_init

    lda #timeout
    sta z_timeout
    +pointer $a7, request

    jsr wic64_push

    lda z_timeout
    cmp #$00
    beq .timeout

    lda #timeout
    sta z_timeout
    +pointer $a7, response

    jsr wic64_pull

    lda z_timeout
    cmp #$00
    beq .timeout

    jsr wic64_exit

.success
    clc
    rts

.timeout
    sec
    rts
}

} // !zone test_wifi_info