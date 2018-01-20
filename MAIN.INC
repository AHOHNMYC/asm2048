; Part of 2048 in assembler
; (C) AHOHNMYC, 2017
; Licensed under WTFPL v2
; version 0.0a
; https://github.com/AHOHNMYC/asm2048

UP   equ 48h
LEFT equ 4Bh
RGHT equ 4Dh
DOWN equ 50h
ESC_key equ 1Bh

key_catcher	proc
	mov	ah, 10h
	int	16h

	cmp	al, ESC_key	; ESC code located in AL, meanwhile others in AH
	je	exit

	cmp	ah, UP
	je	gotcha
	cmp	ah, LEFT
	je	gotcha
	cmp	ah, RGHT
	je	gotcha
	cmp	ah, DOWN
	jne	key_catcher
gotcha:
	mov	keycode, ah
	call	move_numbers
	cmp	succ_moving, 1
	jne	dont_put_number

put_und_update:
	call	put_number
dont_put_number:
	call	update_screen

	cmp	ignr_2048_plate, 1
	je	do_ignore_2048_plate
	call	check_2048_plate
	test	al, al
	jz	do_ignore_2048_plate
	call	show_winnerscreen

	cmp	al, 'r'
	je	restart
	mov	ignr_2048_plate, 1		; if chosen (C)ontinue

do_ignore_2048_plate:

	call	can_move
	test	al, al
	jz	loshara_ty_proigral
	jmp	key_catcher		; if loshara still not proigral we
					; goto Teh Endless TimeHole Loop
loshara_ty_proigral:
	call	show_loserscreen

restart:
	call	print_main		; blyadskiy borders and copyright

	xor	ax, ax			; register cleaning
	mov	score, ax		; and moving it to memory
	mov	ignr_2048_plate, al	; saves 3 our bytes
	jmp	put_und_update

	ret			; decorative role. it is infinite-loop-function
key_catcher endp		; with exit designed as jmp to 2048.asm module


fill_zero	proc
	mov	ax, offset matrix-1		; 'cause last pass with cx == 1
	mov	cx, 16				;while we have to fill matrix[0]
	xor	bx, bx
fill_zero_loop:
	mov	[eax+ecx], bl			; BL=0
	loop	fill_zero_loop

	mov	score, bx;moving register instead of const "0" saves 2 our bytes
	ret
fill_zero	endp


; proc from Zubkov's book adopted to 16-bit random number. AX - random number
get_random	proc
	push	dx
	mov	ax, seed		; считать последнее случайное число
	test	ax, ax			; проверить его, если это -1,
	js	fetch_seed		; функция еще ни разу не вызывалась
					; и надо создать начальное значение
randomize:
	mul	word ptr rand_a		; умножить на число а,
	div	word ptr rand_m		; взять остаток от деления на 231-1
	mov	ax, dx
	mov	seed, ax		; сохранить для следующих вызовов
	pop	dx
	ret

fetch_seed:
	push	ds
	push	0040h
	pop	ds
	mov	ax, ds:006Ch		; считать двойное слово из области
	pop	ds			; данных BIOS по адресу 0040:0060 -
	jmp	short randomize		; текущее число тактов таймера

rand_a	dw	271
rand_m	dw	7FFFh
seed	dw	-1
get_random	endp

put_number	proc
	call	count_zeros
	test	ax, ax
	jz	put_number_exit
	push	ax
	call	get_random		; AH - for 2-or-4 value
					; AL - for choosing free plate
	mov	cl, 1
	cmp 	ah, 22			; 23/256 == 9% probability
					; zero - 23rd number
	jg	dont_put_four_in_cell
	inc	cl			; CL == 2 that'll shown as 4 on screen
dont_put_four_in_cell:

	pop	bx			; restore count of cells with zeroes
	xor	ah, ah			; clear AH used in 2-or-4 choising
	div	bl			; AL division with remainder in AH
	shr	ax, 8			; movzx ax, ah
	inc	ax			; now AL - position to place number
	mov	ah, cl			;     AH - number to put in cell
	call	put_number_sub
put_number_exit:
	ret
put_number	endp


count_zeros	proc
	xor	ax, ax
	mov	bx, offset matrix-1	; -1 because of ecx = [1..16]
	mov	cx, 16			; we need [0..15]
count_zeros_loop1:
	mov	dl, [ebx+ecx]
	test	dl, dl
	jnz	count_zeros_continue
	inc	ax
count_zeros_continue:
	loop	count_zeros_loop1
	ret
count_zeros	endp


put_number_sub	proc
	mov	bx, offset matrix-1
	mov	cx, 16
put_number_sub_loop:
	cmp	byte ptr [ebx+ecx], 0
	jne	put_number_sub_continue
	dec	al				; position to put number
	jnz	put_number_sub_continue		; we wait it to be zero
	mov	byte ptr [ebx+ecx], ah		; AH = 1..2 (for 2 or 4 putting)
	jmp	put_number_sub_exit
put_number_sub_continue:
	loop	put_number_sub_loop
put_number_sub_exit:
	ret
put_number_sub	endp

; below - some moving routine
compress_ax_bx	proc				; if (al == 0 and bl !=0)
	test	al, al				; then al = bl;
	jnz	c_ax_bx_end			;      bl = 0;
	test	bl, bl
	jz	c_ax_bx_end
	xchg	ax, bx
	mov	succ_moving, 1			; plates had been moved today!
c_ax_bx_end:
	ret
compress_ax_bx	endp

merge_ax_bx	proc				; if (al != 0 and al == bl)
	test	al, al				; then al++;
	jz	m_ax_bx_end			;      bl = 0;
	cmp	al, bl				;      score += ax;
	jne	m_ax_bx_end
	inc	ax
	xor	bl, bl
	add	score, ax
	mov	succ_moving, 1			; plates had been moved today!
m_ax_bx_end:
	ret
merge_ax_bx	endp

compress_registers	proc
	mov	si, 3
c_registers_loop:
	call	compress_ax_bx
	xchg	ax, cx
	xchg	ax, bx
	call	compress_ax_bx
	xchg	ax, dx
	xchg	ax, bx
	call	compress_ax_bx
	xchg	ax, cx
	xchg	bx, dx

	dec	si				; 'cause in CX we have
	jnz	c_registers_loop		; 2nd plate value
	ret
compress_registers	endp

merge_registers	proc
	call	merge_ax_bx
	xchg	ax, bx
	xchg	bx, cx
	call	merge_ax_bx
	xchg	ax, bx
	xchg	bx, dx
	call	merge_ax_bx
	xchg	ax, cx
	xchg	bx, dx
	ret
merge_registers	endp

prepare_registers_to_loading proc
	mov	ax, 3	; move it with hope of DOWN/RGHT in keykode
	mov	bx, 2
	mov	cx, 1	; olso it clears high registers, that gives us possi-
	xor	dx, dx	; bility to save bytes in [preverted] addressing below

	cmp	keycode, DOWN
	je	move_load_end
	cmp	keycode, RGHT
	je	move_load_end
;move_up_left_load
	xor	al, al	; 0 ; its all
	dec	bx	; 1 ;  just
	inc	cx	; 2 ;  byte
	mov	dl, 3	; 3 ; fucking
move_load_end:

	cmp	keycode, LEFT
	je	not_vertical
	cmp	keycode, RGHT
	je	not_vertical
	shl	al, 2
	shl	bl, 2
	shl	cl, 2
	shl	dl, 2
not_vertical:
	ret
prepare_registers_to_loading endp


move_numbers	proc
	mov	di, offset matrix
	mov	cx, 4
	mov	succ_moving, 0
loop1:
	push	cx
	
	call	prepare_registers_to_loading
	mov	al, [edi+eax]
	mov	bl, [edi+ebx]
	mov	cl, [edi+ecx]
	mov	dl, [edi+edx]

	call	compress_registers
	call	merge_registers
	call	compress_registers

	mov	regstore._al, al
	mov	regstore._bl, bl
	mov	regstore._cl, cl
	mov	regstore._dl, dl

	call	prepare_registers_to_loading
	call	store_registers

	cmp	keycode, UP
	je	goto_next_cell
	cmp	keycode, DOWN
	je	goto_next_cell
	add	di, 3
goto_next_cell:
	inc	di
	pop	cx
	loop	loop1
	ret
move_numbers	endp


store_registers	proc
	push	bx
	mov	bl, regstore._al
	mov	[edi+eax], bl
	pop	bx
	mov	al, regstore._bl
	mov	[edi+ebx], al
	mov	al, regstore._cl
	mov	[edi+ecx], al
	mov	al, regstore._dl
	mov	[edi+edx], al
	ret
store_registers	endp


can_move	proc
	mov	ax, 1			; returning value
	mov	dx, offset matrix	; pointer to array
	mov	cx, 4
can_move_loop_glob:
	mov	bh, cl
	mov	cl, 4

can_move_loop:
	mov	bl, [edx]
	cmp	bl, ah		; comparing with AH equal 0, saves 1 byte
	jz	we_can_move

	cmp	cl, al	; AL=1	; +1 neighbour
	je	can_move_loop_next	; doesn't exist on matrix[3,7,11,15]
	cmp	bl, [edx+1]
	je	we_can_move

can_move_loop_next:
	cmp	bh, al	; AL=1	; +4 neighbour
	jle	can_move_loop_continue	; doesn't exist on matrix[12..]
	cmp	bl, [edx+4]
	je	we_can_move

can_move_loop_continue:
	inc	dx

	loop	can_move_loop
	mov	cl, bh
	loop	can_move_loop_glob

;we_canot_move:
	xor	ax, ax
we_can_move:
	ret
can_move	endp


check_2048_plate	proc
	xor	ax, ax
	mov	bx, offset matrix-1
	mov	cx, 16
check_2048_plate_loop:
	cmp	byte ptr [ebx+ecx], 11
	je	plate_2048_exists
	loop	check_2048_plate_loop
	jmp	check_2048_plate_end

plate_2048_exists:
	inc	ax
check_2048_plate_end:
	ret
check_2048_plate	endp
