test_get_ip !zone test_get_ip {

.restart
    jsr clrhome
    +print .text

    jsr get_ip
    bcc .success
    jmp .timeout

.success
    +print response
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
!text "WIC64 TEST: GET IP ADDRESS (GETIP $06)", $0d
!text $0d
!text "IP ADDRESS: ", $00

.paragraph
!text $0d, $0d, $00

get_ip !zone get_ip {
    lda #$06
    sta request_id

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

} // !zone test_get_ip