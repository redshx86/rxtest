/* ---------------------------------------------------------------------------------------------- */

#include "delay.h"

/* ---------------------------------------------------------------------------------------------- */

int delaybuf_init(delaybuf_t *s, size_t esize, size_t length)
{
	if( (length == 0) || (esize == 0) )
		return 0;

	s->esize = esize;
	s->length = length;
	s->cursor = 0;

	if( (s->data = calloc(length, esize)) == NULL )
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

void delaybuf_cleanup(delaybuf_t *s)
{
	free(s->data);
}

/* ---------------------------------------------------------------------------------------------- */

void delaybuf(delaybuf_t *s, void *eout, void *ein)
{
	memcpy(eout, (char*)(s->data) + (s->cursor * s->esize), s->esize);
	memcpy((char*)(s->data) + (s->cursor * s->esize), ein, s->esize);

	s->cursor++;
	if(s->cursor == s->length)
		s->cursor = 0;
}

/* ---------------------------------------------------------------------------------------------- */
