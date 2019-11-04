/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../specscal.h"
#include <math.h>

/* ---------------------------------------------------------------------------------------------- */

typedef struct lbr_ctx {

	HWND hwnd;

	HFONT hfont;

	COLORREF cr_bkgnd;
	COLORREF cr_bar;
	COLORREF cr_peak;

	COLORREF cr_bkgnd_inv;
	COLORREF cr_bar_inv;
	COLORREF cr_peak_inv;

	int show_peak;
	HPEN hpen_peak;
	HPEN hpen_peak_inv;

	double l_min;
	double l_max;
	double l_pos;
	double l_peak;

	int cw_buf;
	int ch_buf;
	HBITMAP hbm_empty;
	HBITMAP hbm_full;
	HBITMAP hbm_temp;

} lbr_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct lbr_options {
	int show_peak;
} lbr_options_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct lbr_range {
	double l_min;
	double l_max;
} lbr_range_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct lbr_pos {
	double l_pos;
	double l_peak;
} lbr_pos_t;

/* ---------------------------------------------------------------------------------------------- */

#define LBR_SETRANGE		(WM_USER + 1)
#define LBR_SETPOS			(WM_USER + 2)

/* ---------------------------------------------------------------------------------------------- */

HWND lbr_create(HINSTANCE h_inst, DWORD style, HWND hwndOwner,
				UINT uCtrlId, RECT *rc, HFONT hfont, int show_peak);

/* ---------------------------------------------------------------------------------------------- */

void lbr_send_setrange(HWND hwnd, double l_min, double l_max, int redraw);
void lbr_send_setpos(HWND hwnd, double l_pos, double l_peak, int redraw);

/* ---------------------------------------------------------------------------------------------- */

int lbr_registerclass(HINSTANCE h_inst);
void lbr_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
