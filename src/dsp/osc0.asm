;-------------------------------------------------------------------------------

format MS COFF

;-------------------------------------------------------------------------------

section '.text' code readable executable align 10h

;-------------------------------------------------------------------------------

align 10h
public _osc_generate_sse

; typedef struct osc_state {
;	cpxf_t astep;			[00h]
;	cpxf_t phasor;			[08h]
;	float gain;				[10h]
; } osc_state_t;

; void __cdecl osc_generate_sse(
;	osc_state_t *osc,		[ebp+08h]
;	cpxf_t *buf,			[ebp+0ch]
;	size_t num				[ebp+10h]
;	)

_osc_generate_sse:

	push	ebp
	mov		ebp,esp

	mov		eax,[ebp+08h]	; eax = osc
	mov		edx,[ebp+0ch]	; edx = buf
	mov		ecx,[ebp+10h]	; ecx = num

	; load state
	movlps	xmm1,[eax]		; xmm1 = { 0 0 s_im s_re }
	shufps	xmm1,xmm1,50h	; xmm1 = { s_im s_im s_re s_re }
	xorps	xmm0,xmm0		; xmm0 = { 0 0 0 0 }
	movlps	xmm0,[eax+08h]	; xmm0 = { 0 0 p_im p_re }

	movss	xmm4,[eax+10h]	; xmm4 = { 0 0 0 gain}
	shufps	xmm4,xmm4,0e0h	; xmm4 = { 0 0 gain gain }

	movaps	xmm5,[_neg0001]	; xmm5 = { 0 0 0 -0 }
	movaps	xmm6,[_agccoef]	; xmm6 = { 0 0 1.5 1.5 }

.osc_loop:

	; *buf = phasor * gain
	movaps	xmm2,xmm0		; xmm2 = { p_im p_re }
	mulps	xmm2,xmm4		; xmm2 = { p_im*sqrt(2) p_re*sqrt(2) }
	movlps	[edx],xmm2		; *buf = { p_im*sqrt(2) p_re*sqrt(2) }

	; phasor *= astep
	shufps	xmm0,xmm0,14h	; xmm1 = { p_re p_im p_im p_re  }
	mulps	xmm0,xmm1		; xmm0 = { p_re*s_im  p_im*s_im  p_im*s_re  p_re*s_re }
	movhlps	xmm2,xmm0		; xmm2 = {                       p_re*s_im  p_im*s_im }
	xorps	xmm2,xmm5		; xmm2 = {                       p_re*s_im -p_im*s_im }
	addps	xmm0,xmm2		; xmm0 = { p_im*s_re+p_re*s_im p_re*s_re-p_im*s_im }

	; phasor *= 1.5 - abssq(phasor)
	movaps	xmm2,xmm0		; xmm2 = { p_im p_re }
	mulps	xmm2,xmm2		; xmm2 = { p_im^2 p_re^2 }
	movaps	xmm3,xmm2		; xmm3 = { p_re^2 p_im^2 }
	shufps	xmm3,xmm3,0e1h	; xmm3 = { p_im^2 p_re^2 }
	addps	xmm3,xmm2		; xmm3 = { p_re^2+p_im^2 p_re^2+p_im^2 }
	movaps	xmm2,xmm6		; xmm3 = { 1.5 1.5 }
	subps	xmm2,xmm3		; xmm2 = { 1.5-p_re^2+p_im^2 1.5-p_re^2+p_im^2 }
	mulps	xmm0,xmm2		; xmm0 *= xmm2

	; buf++
	add		edx,08h

	loop	.osc_loop

	; store state
	movlps	[eax+08h],xmm0

	pop		ebp
	ret

;-------------------------------------------------------------------------------

align 10h
public _osc_mix_sse

; typedef struct osc_state {
;	cpxf_t astep;			[00h]
;	cpxf_t phasor;			[08h]
;	float gain;				[10h]
; } osc_state_t;

; void __cdecl osc_mix_sse(
;	osc_state_t *osc,		[ebp+08h]
;	cpxf_t *dst,			[ebp+0ch]
;	cpxf_t *src,			[ebp+10h]
;	size_t num				[ebp+14h]
;	)

_osc_mix_sse:

	push	ebp
	mov		ebp,esp
	push	esi edi

	mov		eax,[ebp+08h]	; eax = osc
	mov		edi,[ebp+0ch]	; edi = dst
	mov		esi,[ebp+10h]	; esi = src
	mov		ecx,[ebp+14h]	; ecx = num

	; load state
	movlps	xmm1,[eax]		; xmm1 = { 0 0 s_im s_re }
	shufps	xmm1,xmm1,50h	; xmm1 = { s_im s_im s_re s_re }
	xorps	xmm0,xmm0		; xmm0 = { 0 0 0 0 }
	movlps	xmm0,[eax+08h]	; xmm0 = { 0 0 p_im p_re }

	movss	xmm4,[eax+10h]	; xmm4 = { 0 0 0 gain}
	shufps	xmm4,xmm4,0e0h	; xmm4 = { 0 0 gain gain }

	movaps	xmm5,[_neg0001]	; xmm5 = { 0 0 0 -0 }
	movaps	xmm6,[_agccoef]	; xmm6 = { 0 0 1.5 1.5 }

.mix_loop:

	; *(dst++) = *(src++) * phasor * gain
	movaps	xmm2,xmm0		; xmm2 = { p_im p_re }
	mulps	xmm2,xmm4		; xmm2 = { p_im p_re } * gain

	shufps	xmm2,xmm2,14h	; xmm2 = { p_re p_im p_im p_re } * gain
	movlps	xmm3,[esi]		; xmm3 = { x_im x_re }
	shufps	xmm3,xmm3,50h	; xmm3 = { x_im x_im x_re x_re }
	mulps	xmm2,xmm3		; xmm2 = {  p_re*x_im  p_im*x_im  p_im*x_re  p_re*x_re } * gain
	movhlps	xmm3,xmm2		; xmm3 = {                        p_re*x_im  p_im*x_im } * gain
	xorps	xmm3,xmm5		; xmm3 = {                        p_re*x_im -p_im*x_im } * gain
	addps	xmm2,xmm3		; xmm2 = { p_im*x_re+p_re*x_im p_re*x_re-p_im*x_im } * gain
	movlps	[edi],xmm2		; *buf = { y_im y_re }

	; phasor *= astep
	shufps	xmm0,xmm0,14h	; xmm1 = { p_re p_im p_im p_re  }
	mulps	xmm0,xmm1		; xmm0 = { p_re*s_im  p_im*s_im  p_im*s_re  p_re*s_re }
	movhlps	xmm2,xmm0		; xmm2 = {                       p_re*s_im  p_im*s_im }
	xorps	xmm2,xmm5		; xmm2 = {                       p_re*s_im -p_im*s_im }
	addps	xmm0,xmm2		; xmm0 = { p_im*s_re+p_re*s_im p_re*s_re-p_im*s_im }

	; phasor *= 1.5 - abssq(phasor)
	movaps	xmm2,xmm0		; xmm2 = { p_im p_re }
	mulps	xmm2,xmm2		; xmm2 = { p_im^2 p_re^2 }
	movaps	xmm3,xmm2		; xmm3 = { p_re^2 p_im^2 }
	shufps	xmm3,xmm3,0e1h	; xmm3 = { p_im^2 p_re^2 }
	addps	xmm3,xmm2		; xmm3 = { p_re^2+p_im^2 p_re^2+p_im^2 }
	movaps	xmm2,xmm6		; xmm3 = { 1.5 1.5 }
	subps	xmm2,xmm3		; xmm2 = { 1.5-p_re^2+p_im^2 1.5-p_re^2+p_im^2 }
	mulps	xmm0,xmm2		; xmm0 *= xmm2

	; src++, dst++
	add		esi,08h
	add		edi,08h

	loop	.mix_loop

	; store state
	movlps	[eax+08h],xmm0

	pop		edi esi
	pop		ebp
	ret

;-------------------------------------------------------------------------------

section '.data' data readable writeable align 10h

;-------------------------------------------------------------------------------

; negation mask, xmm.0 dword
align 10h
label _neg0001 dqword
	dd	80000000h ; -0.0
	dd	0
	dd	0
	dd	0

; agc coefficient
align 10h
label _agccoef dqword
	dd	1.5
	dd	1.5
	dd	0.0
	dd	0.0

;-------------------------------------------------------------------------------
