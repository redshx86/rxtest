;-------------------------------------------------------------------------------

format MS COFF

;-------------------------------------------------------------------------------

section '.text' code readable executable align 10h

;-------------------------------------------------------------------------------

align 10h
public _cpxf_block_add_sse

; void __cdecl cpxf_block_add_sse(
;	cpxf_t *dst,	[ebp+08h]
;	cpxf_t *src,	[ebp+0ch]
;	size_t num		[ebp+10h]
; );

_cpxf_block_add_sse:

	push	ebp
	mov		ebp,esp

	; load arguments
	mov		edx,[ebp+08h]	; dest
	mov		eax,[ebp+0ch]	; src

	; vectored add loop
	mov		ecx,[ebp+10h]	; num
	shr		ecx,3			; num /= 8
	jz		.endvaddlp

.vaddlp:
	movaps	xmm0,[eax]
	addps	xmm0,[edx]
	movaps	[edx],xmm0
	movaps	xmm0,[eax+10h]
	addps	xmm0,[edx+10h]
	movaps	[edx+10h],xmm0
	movaps	xmm0,[eax+20h]
	addps	xmm0,[edx+20h]
	movaps	[edx+20h],xmm0
	movaps	xmm0,[eax+30h]
	addps	xmm0,[edx+30h]
	movaps	[edx+30h],xmm0
	add		eax,40h
	add		edx,40h
	loop	.vaddlp
.endvaddlp:

	; scalar add loop
	mov		ecx,[ebp+10h]	; num
	and		ecx,07h			; num &= 7
	jz		.endsaddlp

	xorps	xmm0,xmm0
	xorps	xmm1,xmm1

.saddlp:
	movlps	xmm0,[eax]
	movlps	xmm1,[edx]
	addps	xmm0,xmm1
	movlps	[edx],xmm0
	add		eax,08h
	add		edx,08h
	loop	.saddlp

.endsaddlp:

	pop		ebp
	retn

;-------------------------------------------------------------------------------

align 10h
public _cpxf_block_mul_sse

; void __cdecl cpxf_block_mul_sse(
;	cpxf_t *dst,	[ebp+08h]
;	cpxf_t *src,	[ebp+0ch]
;	size_t num		[ebp+10h]
; );

_cpxf_block_mul_sse:

	push	ebp
	mov		ebp,esp

	; load arguments
	mov		edx,[ebp+08h]	; dest
	mov		eax,[ebp+0ch]	; src

	; load negation mask
	movaps	xmm4,[_neg0101]	; xmm4 = { 0.0 -0.0 0.0 -0.0 }

	; vectored multiplication loop
	mov		ecx,[ebp+10h]	; num
	shr		ecx,2			; num /= 4
	jz		.end_mul_loop_4

.mul_loop_4:
	movaps	xmm0,[eax]		; xmm0 = { c_im c_re a_im a_re }
	movaps	xmm1,[edx]		; xmm1 = { d_im d_re b_im b_re }
	movaps	xmm2,xmm0		; xmm2 = { c_im c_re a_im a_re }
	movaps	xmm3,xmm1		; xmm3 = { d_im d_re b_im b_re }
	shufps	xmm2,xmm2,0b1h	; xmm2 = { c_re c_im a_re a_im }
	shufps	xmm3,xmm3,0f5h	; xmm3 = { d_im d_im b_im b_im }
	shufps	xmm1,xmm1,0a0h	; xmm1 = { d_re d_re b_re b_re }
	mulps	xmm0,xmm1		; xmm0 = { c_im*d_re c_re*d_re a_im*b_re a_re*b_re }
	mulps	xmm2,xmm3		; xmm1 = { c_re*d_im c_im*d_im a_re*b_im a_im*b_im }
	xorps	xmm2,xmm4		; xmm1 = { c_re*d_im -c_im*d_im a_re*b_im -a_im*b_im }
	addps	xmm0,xmm2		; xmm0 = { (c*d)_im (c*d)_re (a*b)_im (a*b)_re  }
	movaps	[edx],xmm0		; store result

	movaps	xmm0,[eax+10h]	; xmm0 = { c_im c_re a_im a_re }
	movaps	xmm1,[edx+10h]	; xmm1 = { d_im d_re b_im b_re }
	movaps	xmm2,xmm0		; xmm2 = { c_im c_re a_im a_re }
	movaps	xmm3,xmm1		; xmm3 = { d_im d_re b_im b_re }
	shufps	xmm2,xmm2,0b1h	; xmm2 = { c_re c_im a_re a_im }
	shufps	xmm3,xmm3,0f5h	; xmm3 = { d_im d_im b_im b_im }
	shufps	xmm1,xmm1,0a0h	; xmm1 = { d_re d_re b_re b_re }
	mulps	xmm0,xmm1		; xmm0 = { c_im*d_re c_re*d_re a_im*b_re a_re*b_re }
	mulps	xmm2,xmm3		; xmm1 = { c_re*d_im c_im*d_im a_re*b_im a_im*b_im }
	xorps	xmm2,xmm4		; xmm1 = { c_re*d_im -c_im*d_im a_re*b_im -a_im*b_im }
	addps	xmm0,xmm2		; xmm0 = { (c*d)_im (c*d)_re (a*b)_im (a*b)_re  }
	movaps	[edx+10h],xmm0	; store result

	add		eax,20h
	add		edx,20h

	loop	.mul_loop_4

.end_mul_loop_4:

	; scalar multiplication loop
	mov		ecx,[ebp+10h]	; num
	and		ecx,03h			; num &= 3
	jz		.end_mul_loop

.mul_loop:
	movlps	xmm0,[eax]		; xmm0 = { a_im a_re }
	movlps	xmm1,[edx]		; xmm1 = { b_im b_re }
	shufps	xmm0,xmm0,14h	; xmm0 = { a_re a_im a_im a_re }
	shufps	xmm1,xmm1,50h	; xmm1 = { b_im b_im b_re b_re }
	mulps	xmm0,xmm1		; xmm0 = { a_re*b_im a_im*b_im a_im*b_re a_re*b_re }
	movhlps	xmm1,xmm0		; xmm1 = { a_re*b_im a_im*b_im }
	xorps	xmm1,xmm4		; xmm1 = { a_re*b_im -a_im*b_im }
	addps	xmm0,xmm1		; xmm0 = { (a*b)_im (a*b)_re }
	movlps	[edx],xmm0
	add		eax,08h
	add		edx,08h
	loop	.mul_loop

.end_mul_loop:

	pop		ebp
	retn

;-------------------------------------------------------------------------------

align 10h
public _cpxf_block_addr_sse

; void __cdecl cpxf_block_addr_sse(
;	cpxf_t *buf,	[ebp+08h]
;	float x,		[ebp+0ch]
;	size_t num		[ebp+10h]
; );

_cpxf_block_addr_sse:

	push	ebp
	mov		ebp,esp

	; load arguments
	mov		eax,[ebp+08h]	; buf
	mov		edx,[ebp+10h]	; cnt

	movss	xmm0,[ebp+0ch]	; x
	movlhps	xmm0,xmm0

	; vectored add loop
	mov		ecx,edx
	shr		ecx,3
	jz		.end_add_loop_8

.add_loop_8:
	movaps	xmm1,[eax]
	addps	xmm1,xmm0
	movaps	[eax],xmm1
	movaps	xmm1,[eax+10h]
	addps	xmm1,xmm0
	movaps	[eax+10h],xmm1
	movaps	xmm1,[eax+20h]
	addps	xmm1,xmm0
	movaps	[eax+20h],xmm1
	movaps	xmm1,[eax+30h]
	addps	xmm1,xmm0
	movaps	[eax+30h],xmm1
	add		eax,40h
	loop	.add_loop_8

.end_add_loop_8:

	; scalar add loop
	mov		ecx,edx
	and		ecx,07h
	jz		.end_add_loop

.add_loop:
	movss	xmm1,[eax]
	addss	xmm1,xmm0
	movss	[eax],xmm1
	add		eax,08h
	loop	.add_loop

.end_add_loop:

	pop		ebp
	ret

;-------------------------------------------------------------------------------

align 10h
public _cpxf_block_mulr_sse

; void __cdecl cpxf_block_mulr_sse(
;	cpxf_t *buf,	[ebp+08h]
;	float x,		[ebp+0ch]
;	size_t num		[ebp+10h]
; );

_cpxf_block_mulr_sse:

	push	ebp
	mov		ebp,esp

	; load arguments
	mov		eax,[ebp+08h]	; buf
	mov		edx,[ebp+10h]	; cnt

	movss	xmm0,[ebp+0ch]	; x
	shufps	xmm0,xmm0,0

	; vectored multiplication loop
	mov		ecx,edx
	shr		ecx,3
	jz		.end_mul_loop_8

.mul_loop_8:
	movaps	xmm1,[eax]
	mulps	xmm1,xmm0
	movaps	[eax],xmm1
	movaps	xmm1,[eax+10h]
	mulps	xmm1,xmm0
	movaps	[eax+10h],xmm1
	movaps	xmm1,[eax+20h]
	mulps	xmm1,xmm0
	movaps	[eax+20h],xmm1
	movaps	xmm1,[eax+30h]
	mulps	xmm1,xmm0
	movaps	[eax+30h],xmm1
	add		eax,40h
	loop	.mul_loop_8

.end_mul_loop_8:

	; scalar multiplication loop
	mov		ecx,edx
	and		ecx,07h
	jz		.end_mul_loop

	xorps	xmm1,xmm1

.mul_loop:
	movlps	xmm1,[eax]
	mulps	xmm1,xmm0
	movlps	[eax],xmm1
	add		eax,08h
	loop	.mul_loop

.end_mul_loop:

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

;-------------------------------------------------------------------------------
