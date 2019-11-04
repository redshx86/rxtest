/* ---------------------------------------------------------------------------------------------- */
/* Delay buffer - FIFO */

#pragma once

#include <windows.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <crtdbg.h>


/* ---------------------------------------------------------------------------------------------- */

typedef struct delaybuf {

	size_t esize;
	size_t length;
	size_t cursor;
	void *data;

} delaybuf_t;

/* ---------------------------------------------------------------------------------------------- */

int delaybuf_init(delaybuf_t *s, size_t esize, size_t length);
void delaybuf_cleanup(delaybuf_t *s);
void delaybuf(delaybuf_t *s, void *eout, void *ein);

/* ---------------------------------------------------------------------------------------------- */
