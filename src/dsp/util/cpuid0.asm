;-------------------------------------------------------------------------------

format MS COFF

;-------------------------------------------------------------------------------

section '.text' code readable executable align 10h

;-------------------------------------------------------------------------------

align 10h
public __is_cpuid_supported

__is_cpuid_supported:

	pushfd
	pop		eax
	mov		ecx,eax

	xor		eax,1 shl 21
	push	eax
	popfd

	pushfd
	pop		eax

	xor		eax,ecx
	shr		eax,21
	and		eax,1

	push	ecx
	popfd

	ret

;-------------------------------------------------------------------------------

align 10h
public __cpuid

__cpuid:

	push	ebp
	mov		ebp,esp
	push	ebx esi

	mov		eax,[ebp+0ch]
	xor		ecx,ecx
	xor		edx,edx
	xor		ebx,ebx
	cpuid

	mov		esi,[ebp+08h]
	mov		[esi],eax
	mov		[esi+04h],ecx
	mov		[esi+08h],edx
	mov		[esi+0ch],ebx

	pop		esi ebx
	mov		esp,ebp
	pop		ebp
	ret

;-------------------------------------------------------------------------------
