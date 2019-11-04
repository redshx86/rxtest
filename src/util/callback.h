/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>

/* ---------------------------------------------------------------------------------------------- */

typedef void (*callback_proc_t)(void *param, unsigned int type, unsigned int code, void *data);

/* ---------------------------------------------------------------------------------------------- */

typedef struct callback_entry {
	void *param;
	unsigned int mask;
	callback_proc_t proc;
} callback_entry_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct callback_list {
	CRITICAL_SECTION csec;
	size_t cap;
	size_t cnt;
	callback_entry_t *data;
} callback_list_t;

/* ---------------------------------------------------------------------------------------------- */

int callback_list_init(callback_list_t *cbl, size_t initcap);
void callback_list_cleanup(callback_list_t *cbl);

int callback_list_add(callback_list_t *cbl, void *param, callback_proc_t proc, unsigned int mask);
int callback_list_remove(callback_list_t *cbl, callback_proc_t proc, unsigned int mask);

void callback_list_call(callback_list_t *cbl, unsigned int type, unsigned int code, void *data);

/* ---------------------------------------------------------------------------------------------- */
