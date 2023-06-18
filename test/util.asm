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

hexprint !zone hexprint {
    pha

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

    pla

    and #$0f
    tay
    lda (zp2),Y
    jsr chrout

    rts

.digits
    !text "0123456789ABCDEF"
}

}