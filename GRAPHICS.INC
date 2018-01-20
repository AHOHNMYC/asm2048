; Part of 2048 in assembler
; (C) AHOHNMYC, 2017
; Licensed under WTFPL v2
; version 0.0a
; https://github.com/AHOHNMYC/asm2048

print_text macro text, length, position
	mov	ax, 1300h
	mov	cx, length
	mov	dx, position
	mov	bp, offset text
	int	10h
endm

print_main proc
	mov	ax, 0003h
	int	10h				; go to color 80x25 mode
	xor	dx, dx
	mov	bx, 02h				; colour, green
	call	print_header
	mov	bx, 08h				; colour, gray
	call	print_inner

;	double with teh end of print_border because i moved print_header upper
	mov	ah, 02h
	mov	dx, 26*100h			; move cursor to 26th line
	int	10h				; out of teh screen

	call	rainbow		; SUCH RAINBOW! MUCH GRAFONIUM! SO PERFECT!!!111

	mov	bx, 08h				; colour, gray
	print_text	score_text, score_text_l, 6*100h+40-score_text_l
	print_text	copy_text,  copy_text_l, 23*100h+78-copy_text_l
	ret
print_main endp

print_header	proc
	push	offset header_text
	push	header_text_l
	call	text_effect
	ret
print_header	endp

show_loserscreen	proc
	mov	bx, 000Ch
	push	offset wasted
	push	wasted_l
	call	text_effect	; <- call effect and then
	mov	ax, 10h		; <- wait for keypress
	int	16h
	cmp	al, ESC_key
	je	exit
	ret
show_loserscreen	endp

show_winnerscreen	proc
	mov	bx, 000Ah
	push	offset krosavchek
	push	krosavchek_l
	call	text_effect

choice_loop:
	mov	ah, 10h
	int	16h
	cmp	al, 'r'
	je	choice_done
	cmp	al, 'c'
	je	choice_done
	cmp	al, ESC_key
	je	exit
	jmp	choice_loop
choice_done:

	push	ax				; here we store pressed key
	call	print_header	; now we have nice green header :3
	pop	ax		; which will be rewritten when new game starts
	ret
show_winnerscreen	endp

text_effect	proc
	mov	ah, 02h
	mov	dx, 3*100h+1
	int	10h		; set cursor
	mov	ax, 0A20h	; and
	mov	cx, 78		; then
	int	10h		; rewrite header line with 78 spaces (in AL)
	mov	ax, [esp+2]	; \
	shr	ax, 1		; |
	mov	dx, 3*100h+40	; | dx = 3*100h+40 - [esp+2]/2
	sub	dx, ax		; /

	mov	ah, 02h
	int	10h		; set cursor in beginning to write string
	mov	cx, [esp+2]	; cx = string length
text_effect_loop:
	mov	ax, [esp+4]	; \
	add	ax, [esp+2]	; |
	sub	ax, cx		; | al = symbol to write
	mov	al, [eax]	; /
	mov	ah, 09h
	push	cx		; push it there to not to \
	mov	cl, 1		; break [esp+2,4] addressing
	int	10h

	inc	dx

	push	dx		; store position 'cause we r in loop. we need it
	mov	ah, 02h
	int	10h		; cursor to next position
	mov	ah, 86h
	mov	cl, delay
	xor	dx, dx
	int	15h		; delay
	pop	dx

	pop	cx
	loop	text_effect_loop

	pop	ax		; temporary store returning address
	add	sp, 4		; remove two prameters from stack
	push	ax		; restore returning address

	mov	ah, 02h
	mov	dx, 26*100h	; move cursor to 26th line
	int	10h		; out of teh screen

	ret			; return using restored address
text_effect	endp

; Ÿç¥©ª¨ ¢ æ¥­âà¥ íªà ­ 
print_inner proc
	mov	ah, 02h
	mov	dh, 9
	int	10h
	mov	cx, 3
print_inner_loop:
	push	' ³'
	call	print_inner_sub
	push	'ÄÅ'
	call	print_inner_sub
	loop	print_inner_loop
	push	' ³'
	call	print_inner_sub
	ret
print_inner endp


print_inner_sub proc
	push	cx
	mov	ah, 02h
	mov	dl, 30
	int	10h
	mov	cx, 3
print_inner_sub_loop:
	push 	cx
	mov	al, [esp+7]
	mov	cx, 4
	call	print_symb		; 1st sumbol x 4 times
	mov	al, [esp+6]
	mov	cx, 1
	call	print_symb		; 2nd sumbol x 1 time

	pop	cx
	loop	print_inner_sub_loop

	mov	al, [esp+5]
	mov	cx, 4
	call	print_symb		; 1st sumbol x 4 times

	inc	dh
	pop	cx

	pop	ax		; temporary store returning address
	add	sp, 2		; remove one prameter from stack
	push	ax		; restore returning address

	ret			; return using restored address
print_inner_sub endp


print_symb proc
	mov	ah, 02h				; set cursor position
	int	10h				; Video BIOS services
	mov	ah, 09h				; write charecter
	int	10h				; Video BIOS services
	add	dl, cl				; add 1 to position index
	ret
print_symb endp


update_screen	proc
	call	display_numbers
	call	display_score
	ret
update_screen	endp


display_numbers	proc
	mov	bp, offset matrix+15	; changing, have to be stored
	mov	dh, 15			; position, line
	mov	cx, 4			; 4 lines
display_numbers_loop1:
	push	cx
	mov	dl, 45			; 4th number, position of 1st symbol
	mov	cl, 4			; 4 columns
display_numbers_loop2:
	push	cx
	push	bp
	
	movzx	bp, ds:byte ptr [bp]	; BP usually addreses as SS:BP
;	movzx	bp, cs:byte ptr [bp]	; but we DO NOT WANT

	mov	ax, bp			; color generation. Range:
	mov	bl, 6			; 0Ah..0Fh  which means
	div	bl			;      black BG colour
	mov	bx, 0Ah			; and bright FG colour in 3 low bits
	add	bl, ah			; BL = 0Ah + remainder

	shl	bp, 2			; bp = bp*4 \
	add	bp, offset powers	; because our substrings occupy 4 bytes\
	mov	ax, 1300h		; each
	mov	cl, 4			; 4 symbols to grab from [offset power]
	int	10h			; so let's print them
	
	sub	dl, 5h			; next (in fact previous) position
					; as our string - 4 and border - 1
	pop	bp
	dec	bp
	pop	cx
	loop	display_numbers_loop2
	sub	dh, 2			; move up two lines
	pop	cx
	loop	display_numbers_loop1
	ret
display_numbers	endp


display_score	proc
	mov	ax, score
	mov	bx, 10
	mov	do_print_spaces, bh	; BH=0
	mov	cx, scorebuf_l
	mov	di, offset scorebuf
	add	di, cx
display_score_loop:
	xor	dx, dx
	div	bx

	cmp	do_print_spaces, 1
	jge	another_space
	add	dl, '0'
another_space:

	dec	di
	mov	[di], dl
	
	test	ax, ax
	jnz	dddont
	inc	do_print_spaces	; we may move "1", but it costs 1 byte. So, no.
				; And as we may maximum increase it 5 times,
				; there is no owerflow in future
dddont:
	
	loop	display_score_loop

	dec	bx		; 'l be 9 - 1001 -  bright green
	print_text scorebuf, scorebuf_l, 6*100h+41
	ret
display_score	endp


rainbow	proc
	call	fill_zero

	mov	ax, offset matrix+16
	xor	bl, bl
rainbow_loop_with_counter_init:
	mov	cx, 16
rainbow_loop:
	pusha				; in fact we have to store AX, BX and CX

	sub	ax, cx
	test	bl, bl
	jz	nnumber_fill
;zzero_fill
	mov	byte ptr [eax], 0
	jmp	ffffill
nnumber_fill:
	mov	byte ptr [eax], 17
	sub	byte ptr [eax], cl
ffffill:
	call	display_numbers

	mov	ah, 86h				; delay
	movzx	cx, delay
	shl	cx, 1				; delay will be 2x
	xor	dx, dx				; delay located in CX:DX
	int	15h				; System BIOS services

	popa
	loop	rainbow_loop

	inc	bx				; here we set our flag
	cmp	bl, 1
	je	rainbow_loop_with_counter_init	;if flag wasn't already set, jmp

	ret
rainbow	endp