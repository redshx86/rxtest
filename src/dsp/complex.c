/* ---------------------------------------------------------------------------------------------- */

#include "complex.h"

/* ---------------------------------------------------------------------------------------------- */

void __cdecl cpxf_block_add_sse(cpxf_t *dst, cpxf_t *src, size_t num);
void __cdecl cpxf_block_mul_sse(cpxf_t *dst, cpxf_t *src, size_t num);
void __cdecl cpxf_block_addr_sse(cpxf_t *buf, float x, size_t num);
void __cdecl cpxf_block_mulr_sse(cpxf_t *buf, float x, size_t num);

/* ---------------------------------------------------------------------------------------------- */

/* Add complex value blocks.
 * Note: buffers must be dqword aligned. */
void cpxf_block_add(cpxf_t *dst, cpxf_t *src, size_t num)
{
	size_t i;

	if(is_sse_enabled) {
		cpxf_block_add_sse(dst, src, num);
	} else {
		for(i = 0; i < num; i++)
			cpxf_add(dst + i, dst + i, src + i);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Multiply complex value blocks.
 * Note: buffers must be dqword aligned. */
void cpxf_block_mul(cpxf_t *dst, cpxf_t *src, size_t num)
{
	size_t i;

	if(is_sse_enabled) {
		cpxf_block_mul_sse(dst, src, num);
	} else {
		for(i = 0; i < num; i++)
			cpxf_mul(dst + i, dst + i, src + i);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Add complex and real values blocks.
 * Note: buffers must be dqword aligned. */
void cpxf_block_addr(cpxf_t *dst, float x, size_t num)
{
	size_t i;

	if(is_sse_enabled) {
		cpxf_block_addr_sse(dst, x, num);
	} else {
		for(i = 0; i < num; i++)
			cpxf_addr(dst + i, dst + i, x);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Multiply complex and real values blocks.
 * Note: buffers must be dqword aligned. */
void cpxf_block_mulr(cpxf_t *dst, float x, size_t num)
{
	size_t i;

	if(is_sse_enabled) {
		cpxf_block_mulr_sse(dst, x, num);
	} else {
		for(i = 0; i < num; i++)
			cpxf_mulr(dst + i, dst + i, x);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Pack 2 real float buffers to complex float buffer. */
void cpxf_block_pack(cpxf_t *buf, float *src_re, float *src_im, size_t len)
{
	size_t i;

	for(i = 0; i < len; i++) {
		buf[i].re = src_re[i];
		buf[i].im = src_im[i];
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Unpack complex float buffer to 2 real float buffers. */
void cpxf_block_unpack(float *dst_re, float *dst_im, cpxf_t *buf, size_t len)
{
	size_t i;

	for(i = 0; i < len; i++) {
		dst_re[i] = buf[i].re;
		dst_im[i] = buf[i].im;
	}
}

/* ---------------------------------------------------------------------------------------------- */
