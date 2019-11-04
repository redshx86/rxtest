;---------------------------------------------------------------------------------------------------

format MS COFF

;---------------------------------------------------------------------------------------------------

section '.text' code readable executable align 10h

;-------------------------------------------------------------------------------

align 10h
public _firfilt_exec_sse

; size_t __cdecl firfilt_exec_sse(
;	firfilt_state_t *filt,	[ebp+08h]
;	float *buf,				[ebp+0Ch]
;	size_t nsamp			[ebp+10h]
;	)

; typedef struct firfilt_state {
;	size_t length;	[00h]
;	size_t decimf;	[04h]
;	size_t skipcnt;	[08h]
;	float *coef;	[0Ch]
;	void *z;		[10h]
; } firfilt_state_t;

; Locals:
; src_p		[ebp-04h]
; num_out	[ebp-08h]


_firfilt_exec_sse:

	push	ebp
	mov		ebp,esp
	sub		esp,08h
	push	ebx esi edi

;---------------------------------------
; check args

	; ebx = filt
	mov		ebx,[ebp+08h]	; filt

	; src_p = buf
	mov		eax,[ebp+0Ch]	; buf
	mov		[ebp-04h],eax	; src_p

	; num_out = 0
	xor		eax,eax
	mov		[ebp-08h],eax	; num_out

;---------------------------------------
; main loop

.next:

	; any input samples left?
	; if(nsamp == 0)
	;	jumpto .done
	mov		ecx,[ebp+10h]	; nsamp
	test	ecx,ecx
	jz		.done

	; skip output samples?
	; if(filt->skipcnt == 0)
	;	jumpto .skipsamples
	mov		edx,[ebx+08h]	; filt->skipcnt
	test	edx,edx
	jnz		.skipsamples

;---------------------------------------
; compute output sample

	; clear accumulator
	xorps	xmm0,xmm0

	; number of samples in delay line
	; (filter length - 1)
	mov		edx,[ebx]		; filt->length
	dec		edx

	; prepare buffer pointers
	mov		esi,[ebx+0Ch]	; filt->coef
	mov		edi,[ebx+10h]	; filt->z

	; quad sample loop
	mov		ecx,edx
	shr		ecx,2
	jz		.out_qloop_end

.out_qloop:
	; multiply and accumulate (quad)
	; accum += *z_ptr * *coef_ptr
	; z_ptr += 4, coef_ptr += 4
	movaps	xmm1,[edi]
	add		edi,10h
	movaps	xmm2,[esi]
	add		esi,10h
	mulps	xmm2,xmm1
	addps	xmm0,xmm2
	; shift delay line
	movss	xmm2,[edi]
	movss	xmm1,xmm2
	shufps	xmm1,xmm1,39h
	movaps	[edi-10h],xmm1
	loop	.out_qloop

	; sum accumulator quad
	; accum(0,1) += accum(2,3)
	movhlps	xmm1,xmm0
	addps	xmm0,xmm1
	; accum(0) += accum(1)
	movaps	xmm1,xmm0
	shufps	xmm1,xmm1,0E1h
	addss	xmm0,xmm1

.out_qloop_end:

	; single sample loop
	mov		ecx,edx
	and		ecx,3
	jz		.out_tloop_end

.out_tloop:
	; accum(0) += *z_ptr++ * *coef_ptr++
	movss	xmm1,[edi]
	add		edi,04h
	movss	xmm2,[esi]
	add		esi,04h
	mulss	xmm2,xmm1
	addss	xmm0,xmm2
	; shift delay line
	mov		eax,[edi]
	mov		[edi-04h],eax
	loop	.out_tloop

.out_tloop_end:

	mov		eax,[ebp-04h]	; src_p
	movss	xmm1,[eax]
	add		eax,04h
	movss	[edi-04h],xmm1
	mulss	xmm1,[esi]
	addss	xmm0,xmm1
	mov		[ebp-04h],eax

	; store sample
	mov		edi,[ebp+0Ch]	; buf
	mov		eax,[ebp-08h]	; num_out
	movss	[edi+eax*4],xmm0 ; accum
	inc		eax
	mov		[ebp-08h],eax

	; decimctr
	mov		eax,[ebx+04h] ; filt->decimf
	dec		eax
	mov		[ebx+08h],eax ; filt->decimctr

	; nsamp--
	dec		dword[ebp+10h]

	; jumpto main loop
	jmp		.next

;---------------------------------------
; skip samples

.skipsamples:

	; number of samples to skip
	; n = min(filt->skipcnt, nsamp)
	cmp		edx,ecx
	jna		@f
	mov		edx,ecx
@@:

	; shift delay line
	; for(i = 0; i < filt->length - n; i++)
	;	z[i] = z[i + n];
	mov		edi,[ebx+10h]	; filt->z
	lea		esi,[edi+edx*4]	; filt->z + n
	mov		ecx,[ebx]		; filt->length
	sub		ecx,edx			; n
	dec		ecx
	rep movsd

	; copy data to delay line
	; for(i = filt->length - n; i < filt->length; i++)
	;	z[i] = *(src_p++);
	mov		esi,[ebp-04h]	; src_p
	mov		ecx,edx
	rep movsd

	; update counters
	; nsamp -= n,
	; filt->skipcnt -= n,
	; src_p += n
	sub		[ebp+10h],edx	; nsamp
	sub		[ebx+08h],edx	; filt->skipcnt
	mov		[ebp-04h],esi	; src_p

	; jumpto main loop
	jmp		.next

.done:
	mov		eax,[ebp-08h]

	pop		edi esi ebx
	mov		esp,ebp
	pop		ebp
	ret

;---------------------------------------------------------------------------------------------------

align 10h
public _firfilt_exec_cpx_sse

; size_t __cdecl firfilt_exec_cpx_sse(
;	firfilt_state_t *filt,	[ebp+08h]
;	cpxf_t *buf,			[ebp+0Ch]
;	size_t nsamp			[ebp+10h]
;	)

; typedef struct firfilt_state {
;	size_t length;	[00h]
;	size_t decimf;	[04h]
;	size_t skipcnt;	[08h]
;	float *coef;	[0Ch]
;	void *z;		[10h]
; } firfilt_state_t;

; Locals:
; src_p		[ebp-04h]
; num_out	[ebp-08h]


_firfilt_exec_cpx_sse:

	push	ebp
	mov		ebp,esp
	sub		esp,08h
	push	ebx esi edi

;---------------------------------------
; check args

	; ebx = filt
	mov		ebx,[ebp+08h]	; filt

	; src_p = buf
	mov		eax,[ebp+0Ch]	; buf
	mov		[ebp-04h],eax	; src_p

	; num_out = 0
	xor		eax,eax
	mov		[ebp-08h],eax	; num_out

;---------------------------------------
; main loop

.next:

	; any input samples left?
	; if(nsamp == 0)
	;	jumpto .done
	mov		ecx,[ebp+10h]	; nsamp
	test	ecx,ecx
	jz		.done

	; skip output samples?
	; if(filt->skipcnt == 0)
	;	jumpto .outsample
	mov		edx,[ebx+08h]	; filt->skipcnt
	test	edx,edx
	jnz		.skipsamples

;---------------------------------------
; compute output sample

	; clear accumulator
	xorps	xmm0,xmm0

	; number of samples in delay line
	; (filter length - 1)
	mov		edx,[ebx]		; filt->length
	dec		edx

	; prepare buffer pointers
	mov		esi,[ebx+0Ch]	; filt->coef
	mov		edi,[ebx+10h]	; filt->z

	; double sample loop
	mov		ecx,edx
	shr		ecx,1
	jz		.out_dloop_end

.out_dloop:
	; multiply and accumulate (double)
	; accum += *z_ptr * *coef_ptr
	; z_ptr += 2, coef_ptr += 2
	movaps	xmm1,[edi]
	add		edi,10h
	movlps	xmm2,[esi]
	add		esi,08h
	shufps	xmm2,xmm2,50h
	mulps	xmm2,xmm1
	addps	xmm0,xmm2
	; shift delay line
	movhlps	xmm1,xmm1
	movhps	xmm1,[edi]
	movaps	[edi-10h],xmm1
	loop	.out_dloop

	; add values in accumulator
	; accum(0,1) += accum(2,3)
	movhlps	xmm1,xmm0
	addps	xmm0,xmm1

.out_dloop_end:

	; last sample
	test	edx,1
	jz		.out_lsamp_skip

	; accum(0) += *z_ptr++ * *coef_ptr++
	xorps	xmm1,xmm1
	movlps	xmm1,[edi]
	add		edi,08h
	movss	xmm2,[esi]
	add		esi,04h
	shufps	xmm2,xmm2,50h
	mulps	xmm2,xmm1
	addps	xmm0,xmm2
	; shift delay line
	movlps	xmm1,[edi]
	movlps	[edi-08h],xmm1

.out_lsamp_skip:

	xorps	xmm1,xmm1
	mov		eax,[ebp-04h]	; src_p
	movlps	xmm1,[eax]
	add		eax,08h
	movlps	[edi-08h],xmm1
	movss	xmm2,[esi]
	shufps	xmm2,xmm2,50h
	mulps	xmm2,xmm1
	addps	xmm0,xmm2
	mov		[ebp-04h],eax

	; store sample
	mov		edi,[ebp+0Ch]	; buf
	mov		eax,[ebp-08h]	; num_out
	movlps	[edi+eax*8],xmm0 ; accum
	inc		eax
	mov		[ebp-08h],eax

	; decimctr
	mov		eax,[ebx+04h] ; filt->decimf
	dec		eax
	mov		[ebx+08h],eax ; filt->decimctr

	; nsamp--
	dec		dword[ebp+10h]

	; jumpto main loop
	jmp		.next

;---------------------------------------
; skip samples

.skipsamples:

	; number of samples to skip
	; n = min(filt->skipcnt, nsamp)
	cmp		edx,ecx
	jna		@f
	mov		edx,ecx
@@:

	; shift delay line
	; for(i = 0; i < filt->length - n; i++)
	;	z[i] = z[i + n];
	mov		edi,[ebx+10h]	; filt->z
	lea		esi,[edi+edx*8]	; filt->z + n
	mov		ecx,[ebx]		; filt->length
	sub		ecx,edx			; n
	lea		ecx,[ecx*2-2]
	rep movsd

	; copy data to delay line
	; for(i = filt->length - n; i < filt->length; i++)
	;	z[i] = *(src_p++);
	mov		esi,[ebp-04h]	; src_p
	lea		ecx,[edx*2]
	rep movsd

	; update counters
	; nsamp -= n,
	; filt->skipcnt -= n,
	; src_p += n
	sub		[ebp+10h],edx	; nsamp
	sub		[ebx+08h],edx	; filt->skipcnt
	mov		[ebp-04h],esi	; src_p

	; jumpto main loop
	jmp		.next

.done:
	mov		eax,[ebp-08h]

	pop		edi esi ebx
	mov		esp,ebp
	pop		ebp
	ret

;---------------------------------------------------------------------------------------------------
