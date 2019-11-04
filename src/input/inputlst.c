/* ---------------------------------------------------------------------------------------------- */

#include "inputlst.h"

/* ---------------------------------------------------------------------------------------------- */

int input_module_list_load(input_module_list_t *iml)
{
	input_module_desc_t *desc;

	iml->module_first = NULL;
	iml->module_last = NULL;
	iml->count = 0;

	if((desc = filein_get_desc()) != NULL)
	{
		desc->module_id = iml->count++;
		DLIST_INSBACK(desc,
			iml->module_first, iml->module_last,
			module_next, module_prev);
	}

	if((desc = rtlsdr_load()) != NULL)
	{
		desc->module_id = iml->count++;
		DLIST_INSBACK(desc,
			iml->module_first, iml->module_last,
			module_next, module_prev);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

void input_module_list_cleanup(input_module_list_t *iml)
{
	input_module_desc_t *im, *im_next;

	for(im = iml->module_first; im != NULL; im = im_next) {
		im_next = im->module_next;
		im->fn_cleanup_desc(im);
	}

	iml->module_first = NULL;
	iml->module_last = NULL;
	iml->count = 0;
}

/* ---------------------------------------------------------------------------------------------- */

input_module_desc_t *input_module_get(input_module_list_t *iml, unsigned int id)
{
	input_module_desc_t *desc;

	for(desc = iml->module_first; desc != NULL; desc = desc->module_next) {
		if(desc->module_id == id)
			return desc;
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

input_module_desc_t *input_module_get_by_name(input_module_list_t *iml, TCHAR *name)
{
	input_module_desc_t *desc;

	if(_tcscmp(name, _T("")) == 0)
		return NULL;

	for(desc = iml->module_first; desc != NULL; desc = desc->module_next) {
		if(_tcsicmp(desc->name, name) == 0)
			return desc;
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */
