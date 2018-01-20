; Part of 2048 in assembler
; (C) AHOHNMYC, 2017
; Licensed under WTFPL v2
; version 0.0a
; https://github.com/AHOHNMYC/asm2048

.model	tiny
.386
.code
org	100h
start:
	call	delay_selector
	call	restart			; middle of key_catcher proc in main.inc
exit:	mov	ax, 3
	int	10h				; reinit 80x25
	int	20h	; exit game; DOS Interrupt ! have to be replaced !
			; used here because we may want to exit from
			; subroutines with various enclosure level

;ret			; hardcode^H^H^H^Hcore exit for COM application in *DOS

delay_selector	proc
	mov	ax, 3
	int	10h				; set text color mode 80x25
; 1st string
	mov	bl, 0Ah				; green color
	mov	ah, 13h				; print string function
	mov	cx, ds_text_l			; length
	mov	dx, 10*100h+40-ds_text_l/2	; position in center
	mov	bp, offset ds_text
	int	10h				; print 1st line
; 2nd string
	mov	cl, ds_text2_l			; length
	mov	dx, 12*100h+40-ds_text2_l/2	; position in center
	mov	bp, offset ds_text2
	int	10h				; print 2nd line
; 3rd string
	mov	cl, ds_text3_l			; length
;	mov	dx, 13*100h+40-ds_text2_l/2	; column as 2nd line, so :
	inc	dh				; increase line pointer
	mov	bp, offset ds_text3
	int	10h				; print 3rd line

delay_loop:
	mov	ah, 10h				; get pressed key
	int	16h				; Keyboard API
	cmp	al, '1'
	je	d_box
	cmp	al, '2'
	je	d_qemu
	cmp	al, '0'				; invisible \
	je	d_zero				; useful for debugging
	cmp	al, ESC_key
	je	exit
	jmp	delay_loop

d_zero:	mov	delay, 0
	ret
d_box:	mov	delay, 1
	ret
d_qemu:	mov	delay, 40h
	ret
delay_selector	endp

include	main.inc	; main cycle, plates moving, random generation
			; 2048 finding, key press 
include	graphics.inc	; text print effect, rainbow effect, displaying
			; of header, table, numbers and copyright

;.data		; places all data with alignment. May eat some bytes
krosavchek	db 'You got 2048. Cool, broo!      (C)ontinue/(R)estart'
krosavchek_l	equ $ - krosavchek
copy_text	db '(C) AHOHNMYC, 2017'
copy_text_l	equ $ - copy_text
score_text	db 'Score:'
score_text_l	equ $ - score_text
wasted		db 'WASTED'
wasted_l	equ $ - wasted
header_text	db 'TWENTY FORTYEIGHT'
header_text_l	equ $ - header_text

; store this megastring more effective than generate values dynamicaly
; try to compile POWERS.ASM in "Side products" for comparsion
powers		db '    ','  2 ','  4 ','  8 ',' 16 ',' 32 ',' 64 ',' 128'
	 db ' 256',' 512','1024','2048','4096','8192','2^14','2^15','2^16'

ds_text		db 'Select your platform to set delays in game :3'
ds_text_l	equ $ - ds_text
ds_text2	db '1 - DOSBox, DOS on real PC'
ds_text2_l	equ $ - ds_text2
ds_text3	db '2 - QEMU, NTVDM'
ds_text3_l	equ $ - ds_text3

regstore_s struc
	_al db ?
	_bl db ?
	_cl db ?
	_dl db ?
regstore_s ends
regstore regstore_s <>			; temporary storage for registers

keycode		db ?
succ_moving	db ?
do_print_spaces	db ?
ignr_2048_plate	db ?
delay		db ?
score		dw ?
scorebuf	db 5 dup(?)
scorebuf_l	equ $-scorebuf
matrix		db 16 dup(?)

end start