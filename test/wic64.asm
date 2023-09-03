!cpu 6510

!zone WiC64 {

; TODO: wic64_protect_io
; TODO: wic64_protect_cpu_regs
; TODO: optionally store response to load address

;*********************************************************
; Assembly time options
;
; Define the symbols in the following section *before*
; including this file to change the defaults.
;*********************************************************

!ifndef wic64_request_pointer {
    wic64_request_pointer = $a6
}

!ifndef wic64_response_pointer {
    wic64_response_pointer = $a8
}

!ifndef wic64_optimize_for_size {
    wic64_optimize_for_size = 0
}

;*********************************************************
; Runtime options
;
; Set the following memory locations to control runtime
; behaviour.
;*********************************************************

; wic64_timeout
;
; This value defines the time to wait for a handshake.
;
; Note that this includes waiting for the WiC64 to process
; the request before sending a response, so this value
; may need to be adjusted, e.g. when a HTTP request is sent
; to a server that is slow to respond.
;
; The minimum value is $02, which corresponds to a timeout
; of approximately two seconds. If a value lower than $02
; is specified, $02 will be used by default.
;
; If a timeout occurs, the carry flag will be set to signal
; a timeout to the calling routine.
;
; Higher values will increase the timeout in a non-linear
; fashion

wic64_timeout !byte $02

; wic64_enable_irqs
;
; Set to a nonzero value to keep irqs enabled during
; transfers.
;
; The interrupt flag will be reset to its initial state
; after a transfer has completed.
;
; The default is to disable irqs during transfer.
;
wic64_enable_irqs !byte $00

; ********************************************************

!macro wic64_execute .request, .response {
    +wic64_execute .request, .response, $02
}

!macro wic64_execute .request, .response, .timeout {
    lda #<.request
    sta wic64_request_pointer
    lda #>.request
    sta wic64_request_pointer+1

    lda #<.response
    sta wic64_response_pointer
    lda #>.response
    sta wic64_response_pointer+1

    lda #.timeout
    sta wic64_timeout

    jsr wic64_execute
}

; ********************************************************

wic64_execute
    jsr wic64_initialize

    jsr wic64_send
    bcs .execute_done

    jsr wic64_prepare_receive
    bcs .execute_done

    jsr wic64_receive

.execute_done
    jsr wic64_finalize
    rts

; ********************************************************
; Define macro .wait_for_handshake
; ********************************************************
;
; If wic64_optimize_for_size is set to a nonzero value,
; a subroutine will be defined using the code defined
; in _wic64_wait_for_handshake_code and the macro will
; be defined to simply call this subroutine.
;
; Otherwise the macro will contain the code directly.
;
; Note that optimizing for size will significantly decrease
; transfer speed by about 30% due to the jsr/rts overhead
; added for every byte transferred.

!macro _wic64_wait_for_handshake_code {

    ; wait until a handshake has been received from the ESP,
    ; e.g. the FLAG2 line on the userport has been asserted,
    ; with sets bit 4 of $dd0d. Set the carry flag to indicate
    ; a timeout if the timeout length specified in wic64_timeout
    ; has been exceeded.

    ; do a first cheap test for FLAG2 before wasting cycles
    ; setting up the counters.
    lda #$10

    bit $dd0d
    bne .success

    ; no fast response, setup timeout delay loop

    lda #$00
    sta _wic64_counter_1
    lda wic64_timeout
    sta _wic64_counter_2
    sta _wic64_counter_3

    ; keep testing for FLAG2 until all counters are zero

    lda #$10
.wait
    bit $dd0d
    bne .success

    dec _wic64_counter_1
    bne .wait

    dec _wic64_counter_2
    bne .wait

    dec _wic64_counter_3
    bne .wait

.timeout
    ; set carry flag to indicate timeout
    sec

!if (wic64_optimize_for_size != 0) {
    ; When a timeout has occurred, the rts needs to return
    ; to the user subroutine that originally called the API
    ; routines (wic64_*), where a timeout can then be detected
    ; by checking the carry flag.

    ; In case this code is part of a subroutine, the return
    ; address on the stack points to the API routine
    ; instead of the user routine, so in order to return
    ; to the user routine, the first return address on
    ; the stack needs to be skipped.
    pla
    pla
}
    rts

.success
}

!if (wic64_optimize_for_size == 0) {
    !macro .wait_for_handshake {
        +_wic64_wait_for_handshake_code
    }
} else {
    _wic64_wait_for_handshake_routine !zone _wic64_wait_for_handshake {
        +_wic64_wait_for_handshake_code
        rts
    }

    !macro .wait_for_handshake {
        jsr _wic64_wait_for_handshake_routine
    }
}

; ********************************************************

wic64_init
wic64_initialize
    ; always start with a cleared FLAG2 bit in $dd0d
    lda $dd0d

    ; make sure timeout is at least $02
    lda wic64_timeout
    cmp #$02
    bcs +

    lda #$02
    sta wic64_timeout

    ; remember current state of cpu interrupt flag
+   php
    pla
    and #!$04
    sta .user_interrupt_flag

    ; enable or disable irqs as requested
    lda wic64_enable_irqs
    bne .enable_irqs

.disable_irqs
    sei
    jmp +

.enable_irqs
    cli

    ; make sure pa2 is set to output
+   lda $dd02
	ora #$04
	sta $dd02

    rts

; ********************************************************

wic64_send
    clc ; carry will be set if send times out

    ; ask esp to switch to input
    lda $dd00
    ora #$04
    sta $dd00

    ; switch userport to output
    lda #$ff
    sta $dd03

    ; get request size, which is the size of the complete
    ; request, including the request header

    ldy #$01
    lda (wic64_request_pointer),y ; remember lowbyte
    sta .lowbyte
    iny
    lda (wic64_request_pointer),y ; highbyte to x
    tax
    bne .send_pages
    jmp .send_remaining_bytes

.send_pages
    ldy #$00
-   lda (wic64_request_pointer),y
    sta $dd01
    +.wait_for_handshake

    iny
    bne -

    inc wic64_request_pointer+1
    dex
    bne -

.send_remaining_bytes
    ldy #$00
    ldx .lowbyte
    bne +
    jmp .send_done
+
-   lda (wic64_request_pointer),y
    sta $dd01
    +.wait_for_handshake

    iny
    dex
    bne -

.send_done
    rts

; ********************************************************

wic64_prepare_receive
    ; switch userport to input
    lda #$00
    sta $dd03

    ; ask esp to switch to output by pulling PA2 low
    lda $dd00
    and #!$04
    sta $dd00

    ; esp now sends a handshake to confirm change of direction
    +.wait_for_handshake

    ; esp now expects a handshake (accessing $dd01 asserts PC2 line)
    lda $dd01

    rts

; ********************************************************

wic64_receive
    clc ; carry will be set if receive times out

    ; first read the response size (big-endian for unknown reasons)
    +.wait_for_handshake
    lda $dd01
    tax                        ; high byte now in x

    +.wait_for_handshake
    lda $dd01
    sta .lowbyte               ; low byte saved

    cpx #$00
    clc
    bne .receive_pages
    jmp .receive_remaining_bytes

.receive_pages                ; receive pages first
    ldy #$00
-   +.wait_for_handshake
    lda $dd01
    sta (wic64_response_pointer),y
    iny
    bne -

    inc wic64_response_pointer+1
    dex
    bne -

.receive_remaining_bytes       ; receive any remaining bytes
    ldx .lowbyte
    bne +
    jmp .receive_done
+
    ldy #$00
-   +.wait_for_handshake
    lda $dd01
    sta (wic64_response_pointer),y

    iny
    dex
    bne -

.receive_done
    rts

; ********************************************************

wic64_exit
wic64_finalize
    ; switch userport back to input - we want to have both sides
    ; in input mode when idle, only switch to output if necessary
    lda #$00
    sta $dd03

    ; always exit with a cleared FLAG2 bit in $dd0d as well
    lda $dd0d

    ; restore user interrupt flag and rts
    lda .user_interrupt_flag
    beq +

    cli
    rts

+   sei
    rts

    ; carry clear => transfer complete
    ; carry set => transfer timed out

.user_interrupt_flag !byte $00
.lowbyte !byte $00

; ********************************************************

; these have to be global labels due to the limited scoping
; in acme (macros don't see local labels in enclosing scope)

_wic64_counter_1 !byte $00
_wic64_counter_2 !byte $00
_wic64_counter_3 !byte $00

} ; !zone WiC64