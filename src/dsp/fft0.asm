;-------------------------------------------------------------------------------

format MS COFF

;-------------------------------------------------------------------------------

section '.text' code readable executable align 10h

;-------------------------------------------------------------------------------

align 10h
public _fft_bfly_sse

; typedef struct fft_params {
;	size_t length;		[00h]
;	size_t bitnum;		[04h]
;	size_t *brmap;		[08h]
;	cpxf_t *twiddle;	[0ch]
; } fft_params_t;

; void __cdecl fft_bfly_sse(
;	fft_params_t *ctx,	[ebp+08h]
;	cpxf_t *buf			[ebp+0ch]
;	)

; [ebp-04h] seglen
; [ebp-08h] nsegs*8

_fft_bfly_sse:

	push	ebp
	mov		ebp,esp
	sub		esp,08h
	push	ebx esi edi

	; seglen = 2
	mov		dword[ebp-04h],2

	; nsegs*8 = (ctx->length / 2) * 8
	mov		eax,[ebp+08h]	; eax = ctx
	mov		ecx,[eax]		; ecx = ctx->length
	shl		ecx,2			; ecx *= 4
	mov		[ebp-08h],ecx	; nsegs*8 = ecx

	; load negation masks
	movaps	xmm2,[_neg0101]	; xmm2 = { 0.0 -0.0 0.0 -0.0 }
	movaps	xmm3,[_neg0011]	; xmm3 = { 0.0 0.0 -0.0 -0.0 }

	; loop by fft levels
	mov		edi,[eax+04h]	; level = ctx->bitnum
	or		edi,edi			; if(level == 0)
	jz		.endbfly		; 	end

.bflylvl:

	; segment loop
	xor		esi,esi			; esi = seg*8 = 0

.bflyseg:

	; ecx = seglen
	mov		ecx,[ebp-04h]	; ecx = seglen

	; eax = pevn = buf + seg * seglen; (*8 bytes)
	mov		eax,esi			; eax = seg*8
	mul		ecx				; eax *= seglen
	add		eax,[ebp+0ch]	; eax += buf

	; edx = podd = pevn + seglen / 2; (*8 bytes)
	mov		edx,ecx			; edx = seglen
	shl		edx,2			; edx *= 4
	add		edx,eax			; edx += eax

	; ebx = ctx->twiddle
	mov		ebx,[ebp+08h]	; ebx = ctx
	mov		ebx,[ebx+0ch]	; ebx = ctx->twiddle

	; ecx = seglen / 2
	shr		ecx,1

.bflyloop:

	; calculate c = { ptwid * podd }
	movlps	xmm0,[edx]		; xmm0 = {  ----       ----       o_im       o_re      }
	movlps	xmm1,[ebx]		; xmm1 = {  ----       ----       t_im       t_re      }
	add		ebx,[ebp-08h]	; ptwid += nsegs
	shufps	xmm0,xmm0,14h	; xmm0 = {  o_re       o_im       o_im       o_re      }
	shufps	xmm1,xmm1,50h	; xmm1 = {  t_im       t_im       t_re       t_re      }
	mulps	xmm0,xmm1		; xmm0 = {  o_re*t_im  o_im*t_im  o_im*t_re  o_re*t_re }
	movhlps	xmm1,xmm0		; xmm1 = {  ----       ----       o_re*t_im  o_im*t_im }
	xorps	xmm1,xmm2		; xmm1 = {  ----       ----       o_re*t_im -o_im*t_im }
	addps	xmm0,xmm1		; xmm0 = {  ----       ----       c_im       c_re      }

	; calculate { pevn + c }, { pevn - c }
	movlps	xmm1,[eax]		; xmm1 = {  ----       ----       e_im       e_re      }
	movlhps	xmm0,xmm0		; xmm0 = {  c_im       c_re       c_im       c_re      }
	movlhps	xmm1,xmm1		; xmm1 = {  e_im       e_re       e_im       e_re      }
	xorps	xmm0,xmm3		; xmm0 = {  c_im       c_re      -c_im       -c_re     }
	addps	xmm1,xmm0		; xmm1 = {  e_im+c_im  e_re+c_re  e_im-c_im  e_re-c_re }

	; store { pevn - c } to podd
	movlps	[edx],xmm1		; *podd = low_qword(xmm1)
	add		edx,08h			; podd++

	; store { pevn + c } to pevn
	movhps	[eax],xmm1		; *pevn = high_qword(xmm1)
	add		eax,08h			; podd++

	loop	.bflyloop		; bfly loop control

	; segment loop control
	add		esi,8			; seg*8 += 8
	cmp		esi,[ebp-08h]	; if(seg*8 < nsegs*8)
	jb		.bflyseg		;	repeat loop

	shl		dword[ebp-04h],1	; seglen *= 2
	shr		dword[ebp-08h],1	; nsegs*8 /= 2

	; level loop control
	dec		edi				; if(--level != 0)
	jnz		.bflylvl		;	repeat loop

.endbfly:

	pop		edi esi ebx
	mov		esp,ebp
	pop		ebp

	ret

;-------------------------------------------------------------------------------

section '.data' data readable writeable align 10h

;-------------------------------------------------------------------------------

align 10h
label _neg0101 dqword
	dd	80000000h ; -0.0
	dd	0
	dd	80000000h ; -0.0
	dd	0

align 10h
label _neg0011 dqword
	dd	80000000h ; -0.0
	dd	80000000h ; -0.0
	dd	0
	dd	0

;-------------------------------------------------------------------------------
