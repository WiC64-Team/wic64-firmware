test_post !zone test_post {

.restart
    jsr clrhome
    +print .text

    jsr .post
    bcs .timeout

    +paragraph
    jmp .prompt

.timeout
    +newline
    +paragraph
    +print timeout_error_text

.prompt
    +restart_or_return_prompt .restart

.post
    ldx #$00
    lda #$00
-   sta response,x
    dex
    bne -

    +wic64_execute .post_request, response, timeout
    +print_ascii response
    rts

.text
!text "wIc64 tEST: http post ($24)", $0d
!text $0d
!text "postING 12 BYTES OF BINARY DATA TO url", $0d
!text "INCLUDING A QUERY PARAMETER", $0d
!text $0d
!text "sHOULD RETURN \"FOOBAR\" FOR THE QUERY", $0d
!text "AND \"DEADBEEF0000CAFE00004264\" FOR", $0d
!text "THE DATA.", $0d
!text $0d
!text "sERVER RESPONSE:", $0d
!text $0d, $00

.post_request
!text "W"
!byte 4+48+1+12, $00
!byte $24
!text "http://henning-liebenau.de/post.php?query=foobar"
!byte $00
!text $de, $ad, $be, $ef, $00, $00, $ca, $fe, $00, $00, $42, $64

} // !zone test_post