/* ---------------------------------------------------------------------------------------------- */

#include "callback.h"

/* ---------------------------------------------------------------------------------------------- */

int callback_list_init(callback_list_t *cbl, size_t initcap)
{
	InitializeCriticalSection(&(cbl->csec));

	cbl->cap = initcap;
	cbl->cnt = 0;

	if( (cbl->data = malloc(initcap * sizeof(callback_entry_t))) != NULL )
		return 1;

	DeleteCriticalSection(&(cbl->csec));
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

void callback_list_cleanup(callback_list_t *cbl)
{
	free(cbl->data);
	DeleteCriticalSection(&(cbl->csec));
}

/* ---------------------------------------------------------------------------------------------- */

static size_t callback_list_find(callback_list_t *cbl, callback_proc_t proc)
{
	size_t i;

	for(i = 0; i < cbl->cnt; i++)
	{
		if(proc == cbl->data[i].proc)
			return i;
	}
	return (size_t)(-1);
}

/* ---------------------------------------------------------------------------------------------- */

int callback_list_add(callback_list_t *cbl, void *param, callback_proc_t proc, unsigned int mask)
{
	size_t index, newcap;
	callback_entry_t *newbuf;
	int status = 1;

	EnterCriticalSection(&(cbl->csec));

	index = callback_list_find(cbl, proc);

	if(index != (size_t)(-1))
	{
		cbl->data[index].mask |= mask;
	}
	else
	{
		if(cbl->cnt == cbl->cap)
		{
			newcap = cbl->cap + 16;
			newbuf = realloc(cbl->data, newcap * sizeof(callback_entry_t));
			if(newbuf == NULL) {
				status = 0;
			} else {
				cbl->cap = newcap;
				cbl->data = newbuf;
			}
		}

		if(status)
		{
			cbl->data[cbl->cnt].param = param;
			cbl->data[cbl->cnt].mask = mask;
			cbl->data[cbl->cnt].proc = proc;
			cbl->cnt++;
		}
	}

	LeaveCriticalSection(&(cbl->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

int callback_list_remove(callback_list_t *cbl, callback_proc_t proc, unsigned int mask)
{
	size_t index, newcap;
	callback_entry_t *newbuf;
	int status = 0;

	EnterCriticalSection(&(cbl->csec));

	index = callback_list_find(cbl, proc);

	if(index != (size_t)(-1))
	{
		cbl->data[index].mask &= ~mask;

		if( (cbl->data[index].mask == 0) || (mask == 0) )
		{
			memmove(cbl->data + index, cbl->data + index + 1,
				(cbl->cnt - index - 1) * sizeof(callback_entry_t));
			cbl->cnt--;

			if(cbl->cap - cbl->cnt > 32)
			{
				newcap = cbl->cnt + 8;
				newbuf = realloc(cbl->data, newcap * sizeof(callback_entry_t));
				if(newbuf != NULL)
				{
					cbl->cap = newcap;
					cbl->data = newbuf;
				}
			}
		}

		status = 1;
	}

	LeaveCriticalSection(&(cbl->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

void callback_list_call(callback_list_t *cbl, unsigned int type, unsigned int code, void *data)
{
	size_t i;

	EnterCriticalSection(&(cbl->csec));

	for(i = 0; i < cbl->cnt; i++)
	{
		if(cbl->data[i].mask & (1 << type))
			cbl->data[i].proc(cbl->data[i].param, type, code, data);
	}

	LeaveCriticalSection(&(cbl->csec));
}

/* ---------------------------------------------------------------------------------------------- */
