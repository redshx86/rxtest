/* ---------------------------------------------------------------------------------------------- */
/* CPU features detection. */

#pragma once

#include <windows.h>
#include <math.h>

/* ---------------------------------------------------------------------------------------------- */

extern unsigned int is_mmx_enabled;
extern unsigned int is_sse_enabled;
extern unsigned int is_sse2_enabled;

/* ---------------------------------------------------------------------------------------------- */

/* Check CPUID support. */
int __cdecl _is_cpuid_supported();

/* Execute CPUID with eax=arg, ecx=0.
 * output: buf = {eax, ecx, edx, ebx} */
void __cdecl _cpuid(unsigned int *buf, unsigned int arg);

void check_cpu_features();

/* ---------------------------------------------------------------------------------------------- */
