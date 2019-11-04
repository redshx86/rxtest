/* ---------------------------------------------------------------------------------------------- */

#include "buf.h"

/* ---------------------------------------------------------------------------------------------- */

/* initialize FIFO buffer.
 *	esize: element size, bytes,
 *	cap: buffer capacity, elements,
 *	rhyst: read hysteresis, elements,
 *	whyst: write hysteresis, elements. */
int buf_init(buf_state_t *s, size_t esize, size_t cap, size_t rhyst, size_t whyst)
{
	if((esize == 0) || (cap == 0) || (rhyst > cap) || (whyst > cap))
		return 0;

	InitializeCriticalSection(&(s->csec));

	s->esize = esize;
	s->cap = cap;
	s->idx = 0;
	s->cnt = 0;

	s->rhyst = rhyst;
	s->rf = 0;

	s->whyst = whyst;
	s->wf = 0;

	if((s->data = malloc(esize * cap)) != NULL)
		return 1;

	DeleteCriticalSection(&(s->csec));
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* free buffer. */
void buf_cleanup(buf_state_t *s)
{
	free(s->data);
	DeleteCriticalSection(&(s->csec));
}

/* ---------------------------------------------------------------------------------------------- */

/* read data from buffer.
 *	dst: destination buffer,
 *	nelem: number of elements to read.
 * returns: number of elements read. */
size_t buf_read(buf_state_t *s, void *dst, size_t nelem)
{
	size_t es, chunk, rnelem = 0;

	EnterCriticalSection(&(s->csec));

	/* read re-enable */
	s->rf = s->rf || (s->cnt >= s->rhyst);

	if(s->rf)
	{
		rnelem = nelem;

		/* check underflow */
		if(rnelem > s->cnt)
		{
			rnelem = s->cnt;
			s->rf = 0;
		}

		if(rnelem != 0)
		{
			/* copy data */
			es = s->esize;
			if(s->idx + rnelem > s->cap)
			{
				chunk = s->cap - s->idx;
				memcpy(dst, (char*)(s->data) + s->idx * es, chunk * es);
				memcpy((char*)dst + chunk * es, s->data, (rnelem - chunk) * es);
			}
			else
			{
				memcpy(dst, (char*)(s->data) + s->idx * es, rnelem * es);
			}

			/* update buffer pointers */
			s->idx += rnelem;
			if(s->idx >= s->cap)
				s->idx -= s->cap;
			s->cnt -= rnelem;
		}
	}

	LeaveCriticalSection(&(s->csec));

	return rnelem;
}

/* ---------------------------------------------------------------------------------------------- */

/* write data to buffer.
 *	src: source buffer,
 *	nelem: number of elements to write.
 * returns: number of elements written. */
size_t buf_write(buf_state_t *s, void *src, size_t nelem)
{
	size_t es, avail, widx, chunk, wnelem = 0;

	EnterCriticalSection(&(s->csec));

	/* space available */
	avail = s->cap - s->cnt;

	/* write re-enable */
	s->wf = s->wf || (avail >= s->whyst);

	if(s->wf)
	{
		wnelem = nelem;

		/* check overflow */
		if(wnelem > avail)
		{
			wnelem = avail;
			s->wf = 0;
		}

		if(wnelem != 0)
		{
			/* write index */
			widx = s->idx + s->cnt;
			if(widx >= s->cap)
				widx -= s->cap;

			/* copy data */
			es = s->esize;
			if(widx + wnelem > s->cap)
			{
				chunk = s->cap - widx;
				memcpy((char*)(s->data) + widx * es, src, chunk * es);
				memcpy(s->data, (char*)src + chunk * es, (wnelem - chunk) * es);
			}
			else
			{
				memcpy((char*)(s->data) + widx * es, src, wnelem * es);
			}

			/* update buffer pointers */
			s->cnt += wnelem;
		}
	}

	LeaveCriticalSection(&(s->csec));
	return wnelem;
}

/* ---------------------------------------------------------------------------------------------- */
