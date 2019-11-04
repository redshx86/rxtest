/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <tchar.h>
#include "../util/macro.h"

/* ---------------------------------------------------------------------------------------------- */

typedef enum colormap_type {

	COLORMAP_STANDARD_NORM,
	COLORMAP_STANDARD_CHAN,
	COLORMAP_GREY,
	COLORMAP_MATLAB_JET,

	COLORMAP_COUNT

} colormap_type_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct colormap_def {
	TCHAR *name;
	COLORREF *ref_pt_arr;
	size_t ref_pt_num;
} colormap_def_t;

/* ---------------------------------------------------------------------------------------------- */

colormap_def_t crmap_def[COLORMAP_COUNT];

/* ---------------------------------------------------------------------------------------------- */

void crmap_ref(colormap_type_t type, COLORREF *buf, size_t count);

COLORREF cr_mix(COLORREF cr_a, COLORREF cr_b, double f);

/* ---------------------------------------------------------------------------------------------- */
