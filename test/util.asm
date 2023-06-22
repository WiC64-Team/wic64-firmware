!zone util {

!addr chrout = $ffd2

!macro pointer .pointer, .address {
    ldx #<.address
    stx .pointer
    ldx #>.address
    stx .pointer+1
}

!macro incw .addr {
    inc .addr
    bne .done
    inc .addr+1
.done
}

!macro decw .addr {
    dec .addr
    cmp #$ff
    bne .done
    dec .addr+1
.done
}

!macro jmp_via_rti .addr {
    ldx #$ff
    txs
    lda #>.addr
    pha
    lda #<.addr
    pha
    lda #$00
    pha
    rti
}

!macro scan .k {
    sei
    lda .k
	sta $dc00
	lda $dc01
	and .k+1
	cmp .k+1
    cli
}

!macro random_byte .floor, .ceiling {
-   lda random_byte
    cmp #.ceiling
    bcs -
    beq -
    cmp #.floor
    bcc -
}

home !zone home {
    lda #$13
    jsr chrout
    rts
}

clrhome !zone clrhome {
    lda #$93
    jsr chrout
    rts
}

!macro plot .x, .y {
    ldy #.x
    ldx #.y
    jsr $fff0
}

!macro wait_raster .line {
-   lda $d012
    cmp #.line
    bne -
}

!macro print .addr {
    ldx #<.addr
    ldy #>.addr
    jsr print
}

!macro print_indirect .ptr {
    ldx .ptr
    ldy .ptr+1
    jsr print
}

print !zone print {
    stx zp2
    sty zp2+1
    ldy #$00

.loop
    lda (zp2),y
    beq .done
    jsr chrout
    inc zp2
    bne .loop
    inc zp2+1
    jmp .loop

.done
    rts
}

!macro newline {
    lda #$0d
    jsr chrout
}

!macro paragraph {
    lda #$0d
    jsr chrout
    jsr chrout
}

!macro restart_or_return_prompt .restart_addr {
    +print restart_or_return_text

.scan_any_key
    +scan key_none
    beq .scan_any_key

.except_runstop
    +scan key_stop
    bne .scan_any_key

    jmp .restart_addr
}

hexprint !zone hexprint {
    sta .value

    lda zp2
    sta .zp2
    lda zp2+1
    sta .zp2+1

    txa
    pha
    tya
    pha

    lda .value
    ldx #<.digits
    stx zp2
    ldx #>.digits
    stx zp2+1
    lsr
    lsr
    lsr
    lsr
    tay
    lda (zp2),Y
    jsr chrout

    lda .value

    and #$0f
    tay
    lda (zp2),Y
    jsr chrout

    pla
    tay
    pla
    tax

    lda .zp2
    sta zp2
    lda .zp2+1
    sta zp2+1

    lda .value
    rts

.value !byte $00
.zp2 !word $0000
.digits
    !text "0123456789ABCDEF"
}

}