/* ---------------------------------------------------------------------------------------------- */
/* Just buffer. FIFO, fixed size, thread safe. */

#pragma once

#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <crtdbg.h>

/* ---------------------------------------------------------------------------------------------- */

typedef struct buf_state {

	CRITICAL_SECTION csec;

	/* buffer pointers */
	size_t esize;	/* size of element */
	size_t cap;		/* buffer capacity */
	size_t idx;		/* data offset */
	size_t cnt;		/* element count */

	/* underflow mode, rdenhyst = 0 to disable */
	size_t rhyst;	/* read re-enable hysteresis */
	int rf;			/* read enabled */

	/* overflow mode, wrenhyst = 0 to disable */
	size_t whyst;	/* write re-enable hysteresis */
	int wf;			/* write enabled */

	/* data block */
	void *data;

} buf_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* initialize FIFO buffer.
 *	esize: element size, bytes,
 *	cap: buffer capacity, elements,
 *	rhyst: read hysteresis, elements,
 *	whyst: write hysteresis, elements. */
int buf_init(buf_state_t *s, size_t esize, size_t cap, size_t rhyst, size_t whyst);

/* free buffer. */
void buf_cleanup(buf_state_t *s);

/* read data from buffer.
 *	dst: destination buffer,
 *	nelem: number of elements to read.
 * returns: number of elements read. */
size_t buf_read(buf_state_t *s, void *dst, size_t nelem);

/* write data to buffer.
 *	src: source buffer,
 *	nelem: number of elements to write.
 * returns: number of elements written. */
size_t buf_write(buf_state_t *s, void *src, size_t nelem);

/* ---------------------------------------------------------------------------------------------- */
