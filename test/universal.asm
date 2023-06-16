; WiC64 Universalroutinen
; ACME compiler syntax
; v1.13 02.03.2023	; added handshake/flag2 reset befor sending command (lda $dd0d)
; v1.12 12.02.2022	; added load & run (URL in T$) with VIC init before loading
; v1.11 03.01.2022	; added 2 fixed adresses (com_out: $c021, pull_data: $c023) for simple ASM usage (lda #< ; ldy #> ; jsr)
; v1.10 18.12.2021	; added 2 functions that perform push and pull at once: pushpullparameter & pushpull_tstr
; v1.09 12.12.2021	; default-timeout=$11 ; wictest_getip with faster timeout;
; v1.08 29.11.2021	; load portal without CLS ; wictest inkl. data-pulling and IP-test (sys49152+24) $ab=0: no WiC ; $a6=0: no WiFi
; v1.07 31.10.2021	; petscii2ascii bugfix
			; load and run: init screen (but keep back- and foreground black) after load before run
			; portal: use of menue.prg instead of start.prg ; init SID, CIA und IRQ
; V1.06 23.10.2021 	; handshake now always with timeout - adjustable via $ab (171): 1 short timeout, 255 almost no timeout
			; $ab (z_error) = 0: timeout! > 0 (same value as before): OK, no timeout
			; if a timeout occurred, timeout ($ab) has to be re-set to a value >0!
			; INIT separated again from WiC-test
; V1.05 22.10.2021 	; added sys with parameter (push: sys w+18,src & pull: sys w+21,dest)

; zeropage usage: $a5-$ab

!zone Wic64 {

default_timeout	= $11	; $02 = ~ 1 sec.; adjustable via $ab (z_timeout)
					    ; $10 = ~ 20 sec.
tmp2 = $a5
tmp	 = $a6

data_pointer = $a7      ; $a7/$a8 adress for data
bytes_send	= $a9       ; $a9/$aa number of bytes

z_timeout	= $ab       ; length of timeout (1 short, max 255 - real loooong)
z_error		= z_timeout	; (0 means timeout occurred)

safe_area	= $0334 	; for load-routine

wic64_init	jmp u_wic64_init	; WiC64 init (set ESP in read-mode)
wic64_exit	jmp u_wic64_exit	; set userport back to normal
wic64_push	jmp u_wic64_push	; push data from C64 to WiC64 ("send command")
wic64_pull	jmp u_wic64_pull	; pull data from WiC64 to C64 ("retrieve return values")
send_tstr	jmp u_send_tstr		; send URL-LOAD command 1 (URL in t$) to WiC64
portal		jmp u_portal		; load and run WiC64 Menu
pushparameter	jmp u_pushparameter ; push data with source as parameter (basic sys49152+18,src)
pullparameter 	jmp u_pullparameter ; pull data with destination as parameter (basic sys49152+21,dest)
wictest_getip	jmp u_wictest_getip	; WiC64 init & harware check via "get ip" command (sys49152+24) $ab peek(171) =0: no WiC ; $a6 peek(166) =0: no WiFi
pushpull_tstr	jmp u_pushpull_tstr	; send URL-LOAD command 15 (URL in t$) to WiC64 and pull answer at destination: sys49152+27,dest
pushpullparameter
			jmp u_pushpullparameter	; push data with source & destination as parameter (basic sys49152+30,src,dest) - push&pull
com_out		jmp u_com_out		; simple ASM send command: lda #<com ; ldy #>com ; jsr com_out
pull_data	jmp pullthis		; simple ASM pull data: lda #<data ; ldy #>data ; jsr pull_data
load_tstr	jmp u_load_tstr		; load & run URL in t$ (sys49152+39)

u_wic64_init
		lda $dd02
		ora #$01
		sta $dd02		        ; WiC init
		lda #default_timeout	; set timeout
		sta z_timeout
		rts

getparameter
		jsr $aefd	; skipcomma
		jsr $ad9e	; evalparam
		jsr $b7f7	; convert16
		sta tmp		; switch A&Y
		tya
		ldy tmp
		rts

u_pushparameter
		jsr getparameter
		jmp com_out

u_pullparameter
		jsr getparameter
		jmp pullthis

u_wictest_getip
		jsr wic64_init
		lda #$02		; set short timeout
		sta z_timeout
		lda #<com_getip
		ldy #>com_getip
		jsr com_out		; send command "get_ip" for testing communication
		ldx #$00
		cpx z_error		; timeout = No WiC
		beq wtend
		lda #<string_to_send	; pull IP
		ldy #>string_to_send
		jsr pullthis
		ldx #$00		; test IP 0.0.0.0?
		stx tmp			; default "no IP"
		stx tmp2		; init ora
-		lda string_to_send,x
		cmp #$2e		; "."
		beq +
		ora tmp2
		sta tmp2
+		inx
		cpx bytes_send
		bne -
		lda tmp2
		cmp #$30
		beq wtend
		inc tmp  ; =1 : IP is not 0.0.0.0, WiFi connected
wtend		lda z_timeout
		beq +
		lda #default_timeout	; set default timeout if not 0
		sta z_timeout
+		rts

wic64_ESP_read				; set WiC64 ro 'read-mode'
u_wic64_exit
		lda $dd0d
		lda #$ff		; direction Port B out
		sta $dd03
		lda $dd00
		ora #$04		; set PA2 to HIGH = WiC64 ready for reading data from C64
		sta $dd00
		rts

u_com_out
		sta data_pointer	; set datapointer to lowbyte=A, highbyte=Y
		sty data_pointer+1
u_wic64_push
		sei			; disable IRQ
		jsr wic64_ESP_read
		ldy #$02
		lda (data_pointer),y	; number of bytes to send (lowbyte)
		sta bytes_send+1
		dey
		lda (data_pointer),y	; number of bytes to send (highbyte)
		tax
		beq +			; special case:		lowbyte=0
		inc bytes_send+1
+		dey			; y=0
loop_send
		lda (data_pointer),y
		jsr write_byte		; send bytes to WiC64 in loop
		iny
		bne +
		inc data_pointer+1
+		dex
		bne loop_send
		dec bytes_send+1
		bne loop_send
		cli			; enable IRQ
		rts

pullthis
		sta data_pointer
		sty data_pointer+1
u_wic64_pull
		jsr wic64_pull_strt	; init retrieving data from WiC64
		bne nonull		; check for length lobyte=0
		cmp tmp			; length highbyte=0?
		beq pull_end		; no bytes
nonull
		tax			; x: counter lowbyte
		beq loop_read		; special case lobyte=0?
		inc tmp			; +1 for faster check with dec/bne
loop_read
		jsr read_byte		;read byte
		sta (data_pointer),y
		iny
		bne +
		inc data_pointer+1
+		dex
		bne loop_read
		dec tmp
		bne loop_read		; all bytes?
pull_end
		cli
		rts

wic64_pull_strt
		sei			; init reading
		ldy #$00		; set port B to input
		sty $dd03
		lda $dd00
		and #$fb		; PA2 LOW: WiC in send-mode
		sta $dd00
		jsr read_byte		; dummy byte for triggering ESP IRQ
		jsr read_byte		; data length high byte
		sta bytes_send+1
		sta tmp			; counter Hhigh byte
		jsr read_byte		; data length low byte
		sta bytes_send+0
		rts

wait_handshake
		lda z_timeout	; handshake always with timeout
		bne +
		lda #$01		; if z_error/z_timeout = 0 (timeout accured), shorten the following handshakes
+		sta c3			; looplength for timeout
		sta c2			; z_timeout * z_timeout
-		lda $dd0d		; check handshake
		and #$10        	; wait for NMI FLAG2
		bne hs_rts 		; handshake ok - return
		dec c1			; inner loop: 256 passes
		bne -
		dec c2			; outer loops: z_timeout * z_timeout
		bne -
		dec c3
		bne -
		lda #$00		; timeout occurred!
		sta z_error		; $00=timeout, $01-$ff=OK!
hs_rts
		rts
c1		nop			; counter 1
c2		nop			; counter 2
c3		nop			; counter 3

write_byte
		sta $dd01		    ; bits 0..7 parallel to WiC64 (userport PB 0-7)
		jmp wait_handshake

read_byte
		jsr wait_handshake
		lda $dd01		    ; read byte from WiC64 (userport)
		rts

u_pushpullparameter
		jsr u_pushparameter
		jmp u_pullparameter

; send basic t$ to WiC64
u_pushpull_tstr
		lda #$0f		; command 15 ($0f) encoded (bin) url
		sta command+3
		jsr u_send_tstr
		jsr getparameter
		jsr pullthis
		lda #$01		; command 01 (back to normal)
		sta command+3
		rts

u_load_tstr
		jsr make_tstr_com
		jsr copyload
		lda #$81
		sta loadvicrst+1
		lda #<command
		ldy #>command
		jmp com_load		; load & run t$

u_send_tstr	jsr make_tstr_com
		lda #<command
		ldy #>command
		jmp com_out		; send command to WiC64

make_tstr_com	lda $2d 		; vartab
		sta data_pointer
		lda $2e
		sta data_pointer+1
loop_send_t	ldy #$00
		lda (data_pointer),y
		cmp #$54 		; search for t$ ($54, $80)
		bne no_tstr
		iny
		lda (data_pointer),y
		cmp #$80
		bne next_var		; found t$?
		iny
		lda (data_pointer),y	; length
		sta tmp
		iny
		lda (data_pointer),y	; pointer from t$
		sta bytes_send
		iny
		lda (data_pointer),y
		sta bytes_send+1
		ldy #$00
-		lda (bytes_send),y	; copy string
		jsr petscii2ascii	; convert PETSCII to ASCII
		sta string_to_send,y
		iny
		cpy tmp 		; string length?
		bne -
		tya
		clc
		adc #$04 		; add 4 bytes header (W+length+command)
		sta bytes_send
		sta command+1
		rts

no_tstr
		cmp #$00 		; end of vartab?
		beq ret 		; return without finding t$
next_var
		lda data_pointer
		clc
		adc #$07		; next string in vartab
		bcc +
		inc data_pointer+1
+		sta data_pointer
		jmp loop_send_t

petscii2ascii
		stx tmpx
		tax
		cpx #$41 ; >= c1
		bcc +
		adc #$1f ; +c = $20
+		cpx #$60
		bcc +
		sbc #$40
+		cpx #$a0
		bcc +
		adc #$1f ; +c = $20
+		cmp #$c1 ; >=
		bcc +
		sbc #$a0-$20
+		cmp #$dc ; >=
		bcc +
		adc #$9f
+		ldx tmpx
ret		rts
tmpx		!by 0

u_portal	; load/return to portal-menue
		jsr copyload

lp_load
		jsr wic64_ESP_read
		lda #<com_start
		ldy #>com_start
		jmp com_load

com_load
		jsr com_out		; send command to load portal
		jsr wic64_pull_strt	; init pull data
		tax			; x: counter low byte

		inc tmp			; +1 for faster check dec/bne
		jsr read_byte		; loadadress low byte
		cmp #33			; "!" bei http Fehler (z.B. file not found oder server busy)
		beq lp_load
		lda #$01		; to force load ,8 (at basic start)
		sta data_pointer
		jsr read_byte		; loadadress high byte
		lda #$08		; to force load ,8 (at basic start)
		sta data_pointer+1
		jmp safe_area		; load program and run

copyload
		ldx #(read_0334_end-read_0334_start)
-		lda read_0334_start,x	; copy load routine in safe area
		sta safe_area,x
		dex
		bpl -
		rts

read_0334_start
!pseudopc safe_area {
loop_read_0334
handshake_0334	lda $dd0d
		nop
		nop
		nop
		nop 			; short delay
		and #$10        	; wait for NMI FLAG2
		beq handshake_0334
		lda $dd01 		; read byte
		sta (data_pointer),y
		;sta $d020		; borderflicker while loading
		iny
		bne +
		inc data_pointer+1
+		dex
		bne loop_read_0334
		dec tmp
		bne loop_read_0334	; all bytes read?
		cli
		JSR $A659
		jsr $FDA3		; init SID, CIA und IRQ
loadvicrst
		JSR $FF40	;RTS	;JSR $FF81		; screen reset
		JMP $A7AE		; RUN
}
read_0334_end

!ifndef dev {
	!set dev = 0
}

com_getip	!text "W", $04, $00 ,$06
com_start	!text "W", $20, $00, $01
			!text "http://"

!if(dev > 0) {
	!text "t"
} else {
	!text "x"
}
!text ".wic64.net/menue.prg", 0 ; url of portal

command		!text "W", $04, $00, $01	; command header
string_to_send	!byte 0

}