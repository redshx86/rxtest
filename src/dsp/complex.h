/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <math.h>
#include <string.h>
#include "util/cpuid.h"

/* ---------------------------------------------------------------------------------------------- */

/* Note: structure used by various asm routines. */
typedef struct cpxf {
	float re;	/* real part */
	float im;	/* imaginary part */
} cpxf_t;

/* ---------------------------------------------------------------------------------------------- */

/* Set complex value to zero.
 * out = 0; */
__forceinline
void cpxf_zero(cpxf_t *out)
{
	out->re = 0;
	out->im = 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Copy real value to complex value.
 * out = {re, 0}; */
__forceinline
void cpxf_setr(cpxf_t *out, float re)
{
	out->re = re;
	out->im = 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* Copy complex value.
 * out = a; */
__forceinline
void cpxf_copy(cpxf_t *out, cpxf_t *a)
{
	memcpy(out, a, sizeof(cpxf_t));
}

/* ---------------------------------------------------------------------------------------------- */

/* Swap complex values.
 * temp = a; a = c; c = temp; */
__forceinline
void cpxf_swap(cpxf_t *c, cpxf_t *a)
{
	cpxf_t temp;

	memcpy(&temp, a, sizeof(cpxf_t));
	memcpy(a, c, sizeof(cpxf_t));
	memcpy(c, &temp, sizeof(cpxf_t));
}

/* ---------------------------------------------------------------------------------------------- */

/* Add real and complex values.
 * out = a + {b, 0}; */
__forceinline
void cpxf_addr(cpxf_t *out, cpxf_t *a, float b)
{
	out->re = a->re + b;
	out->im = a->im;
}

/* ---------------------------------------------------------------------------------------------- */

/* Subtract real value from complex value.
 * out = a - {b, 0}; */
__forceinline
void cpxf_subr(cpxf_t *out, cpxf_t *a, float b)
{
	out->re = a->re - b;
	out->im = a->im;
}

/* ---------------------------------------------------------------------------------------------- */

/* Multiply real and complex values.
 * out = a * {b, 0}; */
__forceinline
void cpxf_mulr(cpxf_t *out, cpxf_t *a, float b)
{
	out->re = a->re * b;
	out->im = a->im * b;
}

/* ---------------------------------------------------------------------------------------------- */

/* Divide complex value by real value.
 * out = a / {b, 0}; */
__forceinline
void cpxf_divr(cpxf_t *out, cpxf_t *a, float b)
{
	float temp;

	temp = 1.0f / b;
	out->re = a->re * temp;
	out->im = a->im * temp;
}

/* ---------------------------------------------------------------------------------------------- */

/* Add complex values.
 * out = a + b; */
__forceinline
void cpxf_add(cpxf_t *out, cpxf_t *a, cpxf_t *b)
{
	out->re = a->re + b->re;
	out->im = a->im + b->im;
}

/* ---------------------------------------------------------------------------------------------- */

/* Subtract complex values.
 * out = a - b; */
__forceinline
void cpxf_sub(cpxf_t *out, cpxf_t *a, cpxf_t *b)
{
	out->re = a->re - b->re;
	out->im = a->im - b->im;
}

/* ---------------------------------------------------------------------------------------------- */

/* Multiply complex values.
 * out = a * b; */
__forceinline
void cpxf_mul(cpxf_t *out, cpxf_t *a, cpxf_t *b)
{
	float prod_re, prod_im;

	prod_re = (a->re * b->re - a->im * b->im);
	prod_im = (a->im * b->re + a->re * b->im);

	out->re = prod_re;
	out->im = prod_im;
}

/* ---------------------------------------------------------------------------------------------- */

/* Divide complex values.
 * out = a / b; */
__forceinline
void cpxf_div(cpxf_t *out, cpxf_t *a, cpxf_t *b)
{
	float quot_re, quot_im, temp;

	temp = 1.0f / (b->re * b->re + b->im * b->im);
	quot_re = (a->re * b->re + a->im * b->im) * temp;
	quot_im = (a->im * b->re - a->re * b->im) * temp;

	out->re = quot_re;
	out->im = quot_im;
}

/* ---------------------------------------------------------------------------------------------- */

/* Complex conjugate.
 * out = conj(a); */
__forceinline
void cpxf_conj(cpxf_t *out, cpxf_t *a)
{
	out->re =   a->re;
	out->im = - a->im;
}

/* ---------------------------------------------------------------------------------------------- */

/* Complex value from argument.
 * out = exp(j*phi); */
__forceinline
void cpxf_expj(cpxf_t *out, double phi)
{
	out->re = (float)cos(phi);
	out->im = (float)sin(phi);
}

/* ---------------------------------------------------------------------------------------------- */

/* Complex exponent.
 * out = exp(a); */
__forceinline
void cpxf_exp(cpxf_t *out, cpxf_t *a)
{
	double re_exp;

	re_exp = exp(a->re);
	out->re = (float)(cos(a->im) * re_exp);
	out->im = (float)(sin(a->im) * re_exp);
}

/* ---------------------------------------------------------------------------------------------- */

/* Complex value modulus squared.
 * A^2 = abs(a)^2; */
__forceinline
float cpxf_modsqr(cpxf_t *a)
{
	return (a->re * a->re + a->im * a->im);
}

/* ---------------------------------------------------------------------------------------------- */

/* Complex value modulus.
 * A = abs(a); */
__forceinline
float cpxf_mod(cpxf_t *a)
{
	return (float)sqrt(a->re * a->re + a->im * a->im);
}

/* ---------------------------------------------------------------------------------------------- */

/* Complex value argument.
 * phi = arg(a); */
__forceinline
float cpxf_arg(cpxf_t *a)
{
	return (float)atan2(a->im, a->re);
}

/* ---------------------------------------------------------------------------------------------- */

/* Add complex value blocks.
 * Note: buffers must be dqword aligned. */
void cpxf_block_add(cpxf_t *dst, cpxf_t *src, size_t num);

/* Multiply complex value blocks.
 * Note: buffers must be dqword aligned. */
void cpxf_block_mul(cpxf_t *dst, cpxf_t *src, size_t num);

/* Add complex and real values blocks.
 * Note: buffers must be dqword aligned. */
void cpxf_block_addr(cpxf_t *dst, float x, size_t num);

/* Multiply complex and real values blocks.
 * Note: buffers must be dqword aligned. */
void cpxf_block_mulr(cpxf_t *dst, float x, size_t num);

/* Pack 2 real float buffers to complex float buffer. */
void cpxf_block_pack(cpxf_t *buf, float *src_re, float *src_im, size_t len);

/* Unpack complex float buffer to 2 real float buffers. */
void cpxf_block_unpack(float *dst_re, float *dst_im, cpxf_t *buf, size_t len);

/* ---------------------------------------------------------------------------------------------- */
