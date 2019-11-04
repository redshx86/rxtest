/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../util/list.h"
#include "indesc.h"
#include "filein.h"
#include "rtlsdr.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct input_module_list {

	input_module_desc_t *module_first;
	input_module_desc_t *module_last;

	unsigned int count;

} input_module_list_t;

/* ---------------------------------------------------------------------------------------------- */

int input_module_list_load(input_module_list_t *iml);
void input_module_list_cleanup(input_module_list_t *iml);

input_module_desc_t *input_module_get(input_module_list_t *iml, unsigned int id);
input_module_desc_t *input_module_get_by_name(input_module_list_t *iml, TCHAR *name);

/* ---------------------------------------------------------------------------------------------- */
