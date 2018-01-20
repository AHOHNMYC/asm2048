# 2048.COM

*BIOS*-interrupts based, 16-bit assembler written, power of _2_ formed logical game.

Works under:

* DOSBox
* FreeDOS, MS-DOS (tested in QEMU with SeaBIOS)

Doesn't work on:

* NTVDM
* real PC (hey, don't laugh at me, you)

*Based on 2048 originally written by Gabriele Cirulli.*
*Licensed as WTFPL software.*
*(C) AHOHNMYC, 2017*

## Compiling:

Tested with TASM 4.1 and TLINK 7.1.30.1

```
:: m - multiple passes
TASM /m 2048.ASM
:: t -  COM compilation
:: x - 'cause we do not want MAP file
TLINK /t /x 2048.OBJ
```

## Bugs:

One, but fat and complex.

* Sometimes it doesn't work.

Game doesn't want to start correctly in XP's NTVDM and on my PC with real (and strange) BIOS.

I don't know what's wrong with NTVDM and how it interacts with physical BIOS (and exists those interactions at all or not).

In case of BIOS all is simple: some INTs returns changed registers. This behavior not described in my literature and available references (HelpPC 2.10 [1991], TECH Help! [1993]).

I see single simple solution: wrapper for all calls that'll pusha/popa every time. (I know, it's ANON.FM's level, but it have to work correctly)

Something like that:

```assembly
int_wrapper	proc
	pusha
	; addressing crap
	; interruption call

	; maybe sort of self-alterating code for minimum complexity
	;'cause int call - 2 bytes: INT 20h == "CD 20" in hex codes
	; addressing, segment horror - DO NOT WANT.
	popa
	ret
int_wrapper	endp
```
## Todo:

* More variability (maybe different phrases dependent on score)
* Try to compress code further (but I at present time done some scary things about reusing high registers and somewhere ignoring high-halfs at all)
* Some easter eggs