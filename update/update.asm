!addr zp1 = $22
!addr zp2 = $50

!addr chrout = $ffd2
!addr reboot = $fce2

roff = $92
lowercase = $0e

!addr draw_menu_header = $ce0c
!addr menu_header_title = $cfa3

* = $0801 ; 10 SYS 2064 ($0810)
!byte $0c, $08, $0a, $00, $9e, $20, $32, $30, $36, $34, $00, $00, $00

* = $0810
jmp main

key_none  !byte %00000000, %11111111
key_one   !byte %01111111, %00000001
key_two   !byte %01111111, %00001000
key_three !byte %11111101, %00000001
key_four  !byte %11111101, %00001000
key_f5    !byte %11111110, %01000000
key_esc   !byte %01111111, %00000010
key_stop  !byte %01111111, %10000000

!src "../test/wic64.asm"
!src "../test/util.asm"

; ---------------------------------------------------------------------------

test_menu_code_loaded:
    lda draw_menu_header
    cmp #$4c
    rts
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

!macro copy_string .src, .dst {
    +pointer zp1, .src
    +pointer zp2, .dst

    ldy #$00
-   lda (zp1),y
    sta (zp2),y
    beq .done
    iny
    jmp -

.done
}

; ---------------------------------------------------------------------------

!macro fill .addr, .byte, .len {
    +pointer zp1, .addr
    ldy #.len
    dey
    lda #.byte

-   sta (zp1),y
    dey
    bne -
    sta (zp1),y
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

green:
    lda #$05
    sta $0286
    rts

; ---------------------------------------------------------------------------

red:
    lda #$02
    sta $0286
    rts

; ---------------------------------------------------------------------------

grey:
    lda #$0b
    sta $0286
    rts

; ---------------------------------------------------------------------------

cyan:
    lda #$03
    sta $0286
    rts

; ---------------------------------------------------------------------------

compare_versions: !zone compare_versions {
    ldy #$03

-   lda (zp1),y
    cmp (zp2),y
    bne .not_equal
    dey
    bpl -

.equal:
    ldy #$04
    lda #$00
    sta (zp1),y
    clc
    rts

.not_equal:
    ldy #$04
    lda #$01
    sta (zp1),y
    sec
    rts

.done:
}

!macro compare_versions .a, .b {
    +pointer zp1, .a
    +pointer zp2, .b
    jsr compare_versions
}

; ---------------------------------------------------------------------------

!macro strlen .addr {
    ; for strlen < 255, result in X
    ldx #$00
-   lda .addr,x
    beq .done
    inx
    bne -
.done
}

; ---------------------------------------------------------------------------

wait_any_key:
-   +scan key_none
    bne -
-   +scan key_none
    beq -
    rts

; ---------------------------------------------------------------------------

!macro print_error_and_jmp .message, .addr {
    jsr red
    +print .message
    jsr green

    +print press_any_key_text
    jsr wait_any_key

    jmp .addr
}

; ---------------------------------------------------------------------------

!macro add_four_to .addr {
    lda .addr
    clc
    adc #4
    sta .addr
    bcc .done
    inc .addr+1
.done
}

; ---------------------------------------------------------------------------

!macro install .version, .query {

.ensure_version_exists:
    lda .version
    bne .ensure_version_is_not_installed_yet
    jmp .done

.ensure_version_is_not_installed_yet:
    lda .version+4
    bne .prepare_url_query

    jsr red
    +print warning_installed_prefix
    +print_version .version
    +print warning_installed_postfix
    jsr green

    +print press_any_key_text
    jsr wait_any_key

    jmp main

.prepare_url_query:
    ldx #$02
-   lda .query,x
    sta remote_request_query,x
    dex
    bpl -

.clear_url
    +fill install_request_url, $00, $ff

.execute_url_query
    +wic64_execute remote_request, install_request_url
    bcc .prepare_install_request
    +print_error_and_jmp timeout_error_text, main

.prepare_install_request
    +strlen install_request_url
    stx install_request_size
    +add_four_to install_request_size

.execute_install_request
    +wic64_execute install_request, install_response
    bcs +

    +print_error_and_jmp install_response, main

+   +print installing_text
    +print_version .version

.wait_for_wic64_to_reboot
-   jsr dot
    +wic64_execute ping_request, ping_response
    bcs -

    +print success_text

.confirm_newly_installed_version:
    +wic64_execute installed_version_request, installed_version
    bcc +
    +print_error_and_jmp timeout_error_text, main

+   jsr cyan
    +print install_successful_text
    +print_version installed_version
    +paragraph

    jsr green
    +print press_any_key_text
    jsr wait_any_key

    jmp main

.done
}

current_stable_url_query: !text "csu"
previous_stable_url_query: !text "psu"
current_unstable_url_query: !text "cuu"
previous_unstable_url_query: !text "puu"

installing_text: !text "iNSTALLING VERSION ", $00
success_text: !text "ok", $0d, $0d, $00

install_successful_text:
!text "iNSTALLED VERSION ", $00

warning_installed_prefix:
!text "vERSION ", $00

warning_installed_postfix:
!text " CURRENTLY INSTALLED", $0d, $0d, $00

press_any_key_text:
!text "=> pRESS ANY KEY TO CONTINUE", $00

press_any_key_quit_text:
!text "=> pRESS ANY KEY TO QUIT", $00

; ---------------------------------------------------------------------------

main:
    sei

    lda #$00
    sta $d020
    sta $d021

    jsr green
    jsr clrhome

    lda #lowercase
    jsr chrout

    jsr test_menu_code_loaded
    beq +

    +print title_text
    jmp ++

+   +copy_string title_text, menu_header_title
    jsr draw_menu_header
    +plot 0, 4

++  lda #roff
    jsr chrout

bail_on_legacy_firmware:
    +fill installed_version_string_response, $00, $40

    +wic64_execute installed_version_string_request, installed_version_string_response
    bcc +
    +print_error_and_jmp timeout_error_text, main

+   +strlen installed_version_string_response
    cpx #$04
    bne get_installed_version

    jsr red
    +print legacy_firmware_error_text

    jsr green
    +print legacy_firmware_help_text

    +print press_any_key_quit_text
    jsr wait_any_key
    jmp reboot

get_installed_version:
    +wic64_execute installed_version_request, installed_version
    bcc +
    +print_error_and_jmp timeout_error_text, main

+   +print installed_text
    +print_version installed_version

    lda installed_version+3
    beq +

    +print unstable_tag
    jmp ++

+   +print stable_tag

++   +paragraph

get_remote_versions:
    lda #'v'
    sta remote_request_query+2

    lda #'s'
    sta remote_request_query+1

get_current_stable_version:
    lda #'c'
    sta remote_request_query

    +wic64_execute remote_request, current_stable_version
    bcc +
    +print_error_and_jmp timeout_error_text, main

+   lda current_stable_version
    beq get_previous_stable_version

    +compare_versions current_stable_version, installed_version
    bcs +
    jsr grey

+   +print current_stable_text
    +print_version current_stable_version
    +newline
    jsr green

get_previous_stable_version:
    lda #'p'
    sta remote_request_query

    +wic64_execute remote_request, previous_stable_version
    bcc +
    +print_error_and_jmp timeout_error_text, main

+   lda previous_stable_version
    beq get_current_unstable_version

    +compare_versions previous_stable_version, installed_version
    bcs +
    jsr grey

+   +print previous_stable_text
    +print_version previous_stable_version
    +newline
    jsr green

get_current_unstable_version:
    lda #'u'
    sta remote_request_query+1

    lda #'c'
    sta remote_request_query

    +wic64_execute remote_request, current_unstable_version
    bcc +
    +print_error_and_jmp timeout_error_text, main

+   lda current_unstable_version
    beq get_previous_unstable_version

    +compare_versions current_unstable_version, installed_version
    bcs +
    jsr grey

+   +print current_unstable_text
    +print_version current_unstable_version
    +newline
    jsr green

get_previous_unstable_version:
    lda #'p'
    sta remote_request_query

    +wic64_execute remote_request, previous_unstable_version
    bcc +
    +print_error_and_jmp timeout_error_text, main

+   lda previous_unstable_version
    beq prompt

    +compare_versions previous_unstable_version, installed_version
    bcs +
    jsr grey

+   +print previous_unstable_text
    +print_version previous_unstable_version
    +newline
    jsr green

prompt:
    +newline
    +print prompt_text

scan:
    jsr wait_any_key

++  +scan key_f5
    bne +
    jmp ++

+   jmp main

++  +scan key_stop
    bne +
    jmp ++

+   jsr red
    +print return_not_implemented_text
    jsr green

++  +scan key_esc
    bne +
    jmp ++

+   jsr red
    +print return_not_implemented_text
    jsr green

++  +scan key_one
    bne +
    jmp ++

+   +install current_stable_version, current_stable_url_query
    jmp scan

++  +scan key_two
    bne +
    jmp ++

+   +install previous_stable_version, previous_stable_url_query
    jmp scan

++  +scan key_three
    bne +
    jmp ++

+   +install current_unstable_version, current_unstable_url_query
    jmp scan

++  +scan key_four
    bne +
    jmp ++

+  +install previous_unstable_version, previous_unstable_url_query

++  jmp scan

; ---------------------------------------------------------------------------

legacy_firmware_error_text:
!text "!! lEGACY FIRMWARE VERSION DETECTED !!", $0d, $0d, $00

legacy_firmware_help_text:
!text "fIRMWARE VERSION 2.0.0 OR LATER IS", $0d
!text "REQUIRED TO RUN THIS PROGRAM.", $0d
!text $0d
!text "tO UPDATE TO A NEWER VERSION, VISIT", $0d
!text $0d
!text $9f, "WWW.WIC64-TEAM.GITHUB.IO/ONLINE-FLASHER", $1e, $0d
!text $0d
!text $00

title_text:
!text "fIRMWARE uPDATE", $0d, $0d, $00

installed_text:
!text roff
!text "iNSTALLED VERSION: ", $00

installed_tag:
!text " <=", $00

current_stable_text:
!text "1. cURRENT STABLE...... ", $00

previous_stable_text:
!text "2. pREVIOUS STABLE..... ", $00

current_unstable_text:
!text "3. cURRENT UNSTABLE.... ", $00

previous_unstable_text:
!text "4. pREVIOUS UNSTABLE... ", $00

stable_tag:
!text " (STABLE)", $00

unstable_tag:
!text " (UNSTABLE)", $00

none_text: !text "none", $00

prompt_text:
!text "=> sELECT VERSION TO INSTALL", $0d
!text $0d, $00

timeout_error_text:
!text "rEQUEST TIMEOUT", $0d, $0d, $00

installed_version_request:
!byte "W", $04, $00, $26

installed_version_string_request:
!byte "W", $04, $00, $00

installed_version_string_response:
!fill 64, 0

return_not_implemented_text:
!text "rETURN TO PORTAL NOT IMPLEMENTED YET", $0d, $00

remote_request:
remote_request_header: !byte "W"
remote_request_size: !byte <remote_request_length, >remote_request_length
remote_request_cmd: !byte $01
remote_request_url: !text "http://www.henning-liebenau.de/update/update.php?q="
remote_request_query: !text "xxx"
remote_request_url_end:

remote_request_url_length = remote_request_url_end - remote_request_url
remote_request_length = remote_request_url_length + 4

installed_version:
!fill 4

current_stable_version:
!fill 4

current_stable_version_installed:
!byte $01

previous_stable_version:
!fill 4

previous_stable_version_installed:
!byte $01

current_unstable_version:
!fill 4

current_unstable_version_installed:
!byte $01

previous_unstable_version:
!fill 4

previous_unstable_version_installed:
!byte $01

install_request:
install_request_header: !byte "W"
install_request_size: !byte $00, $00
install_request_cmd: !byte $27
install_request_url: !fill 255

install_response: !fill 255

ping_request: !byte "W", $08, $00, $fe
ping_data: !text "ping"
ping_response: !byte $00, $00, $00, $00
