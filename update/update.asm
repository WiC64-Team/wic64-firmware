!addr zp1 = $22
!addr zp2 = $50

!addr draw_menu_header = $ce0c
!addr chrout = $ffd2

roff = $92
lowercase = $0e

* = $0801 ; 10 SYS 2064 ($0810)
!byte $0c, $08, $0a, $00, $9e, $20, $32, $30, $36, $34, $00, $00, $00

* = $0810
jmp main

key_none  !byte %00000000, %11111111
key_one   !byte %01111111, %00000001
key_two   !byte %01111111, %00001000
key_three !byte %11111101, %00000001
key_four  !byte %11111101, %00001000
key_five  !byte %11111011, %00000001
key_stop  !byte %01111111, %10000000

!src "../test/wic64.asm"
!src "../test/util.asm"

; ---------------------------------------------------------------------------

print_dec !zone print_dec {
    cmp #$00
    bne .not_zero

    lda #'0'
    jsr $ffd2
    rts

.not_zero:
   sta .value
   ldy #$01
   sta .nonzero_digit_printed

.hundreds:
    ldx #$00
-   sec
    sbc #100
    bcc .print_hundreds
    sta .value
    inx
    jmp -

.print_hundreds:
    jsr .print_digit_in_x

.tens:
    lda .value
    ldx #$00
-   sec
    sbc #10
    bcc .print_tens
    sta .value
    inx
    jmp -

.print_tens:
    jsr .print_digit_in_x

.print_ones:
    lda .value
    tax
    jsr .print_digit_in_x

    rts

.print_digit_in_x:
    cpx #$00
    bne +

    lda .nonzero_digit_printed
    beq +

    rts

+   lda .digits,x
    jsr $ffd2

    lda #$00
    sta .nonzero_digit_printed
    rts

.value: !byte $00
.nonzero_digit_printed: !byte $01
.digits: !text "0123456789"
}

; ---------------------------------------------------------------------------

!macro print_version .addr {
    lda .addr
    bne +

    +print none_text
    jmp .done

+   jsr print_dec
    jsr dot

    lda .addr+1
    jsr print_dec
    jsr dot

    lda .addr+2
    jsr print_dec

    lda .addr+3
    beq .done

    jsr dash
    lda .addr+3
    jsr print_dec

.done:
}

; ---------------------------------------------------------------------------

dot:
    lda #'.'
    jsr chrout
    rts

; ---------------------------------------------------------------------------

dash:
    lda #'-'
    jsr chrout
    rts

; ---------------------------------------------------------------------------

main:
    sei

    lda #$00
    sta $d020
    sta $d021

    lda #$05
    sta $0286

    jsr clrhome

    lda #lowercase
    jsr chrout

    jsr draw_menu_header

    +plot 0, 4
    +print versions_text

    lda #$00
    sta local_version_is_stable

    +wic64_execute local_version_request, local_version

    +plot 19, 4
    +print_version local_version

    lda local_version+3
    beq +

    sta local_version_is_stable
    +print unstable_text

+   lda #'v'
    sta remote_request_query+1

    lda #'s'
    sta remote_request_query

    +wic64_execute remote_request, stable_version

    +plot 19, 5
    +print_version stable_version

    lda #'u'
    sta remote_request_query

    +wic64_execute remote_request, unstable_version

    +plot 19, 6
    +print_version unstable_version

    +plot 0, 8
    +print options_text

    jmp *

; ---------------------------------------------------------------------------

local_version_is_stable !byte $01

versions_text:
!text roff
!text "iNSTALLED vERSION:", $0d
!text "    lATEST STABLE:", $0d
!text "  lATEST UNSTABLE:", $0d
!text $00

unstable_text:
!text " (UNSTABLE)", $00

none_text: !text "NONE", $00

options_text:
!text "1 - iNSTALL LATEST STABLE VERSION", $0d
!text "2 - iNSTALL LATEST UNSTABLE VERSION", $0d
!text "0 - eXIT", $0d
!text $00

local_version_request:
!byte "W", $04, $00, $26

local_version:
!fill 4, 0

remote_request:
remote_request_header: !byte "W"
remote_request_size: !byte <remote_request_length, >remote_request_length
remote_request_cmd: !byte $01
remote_request_url: !text "http://www.henning-liebenau.de/update/update.php?q="
remote_request_query: !text "xx"
remote_request_url_end:

remote_request_url_length = remote_request_url_end - remote_request_url
remote_request_length = remote_request_url_length + 4

stable_version:
!fill 4, 0

unstable_version:
!fill 4, 0