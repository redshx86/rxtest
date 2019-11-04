/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct pageproc_ctx {

	HWND hwndSetWnd;
	HWND hwndPage;
	uicommon_t *uidata;

	HWND hwndEditInputHdrCount;
	HWND hwndEditInputHdrMs;

	HWND hwndEditProcBufLenMs;
	HWND hwndEditProcInWhystMs;
	HWND hwndEditProcOutRhystMs;

	HWND hwndEditProcWorkbufMs;

	HWND hwndEditBaseFsmin;
	HWND hwndEditBaseFsmax;

} pageproc_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct pageproc_data {

	int input_hdr_count;
	double input_hdr_ms;

	double proc_buf_len_ms;
	double proc_in_whyst_ms;
	double proc_out_rhyst_ms;

	double proc_workbuf_ms;

	int base_fsmin;
	int base_fsmax;

} pageproc_data_t;

/* ---------------------------------------------------------------------------------------------- */

#define PAGEPROC_ID_INPUT_HDRCNT		1001
#define PAGEPROC_ID_INPUT_HDRMS			1002

#define PAGEPROC_ID_PROC_BUFLEN_MS		1021
#define PAGEPROC_ID_PROC_IN_WHYST_MS	1022
#define PAGEPROC_ID_PROC_OUT_RHYST_MS	1023

#define PAGEPROC_ID_PROC_WORKBUF_MS		1041

#define PAGEPROC_ID_BASE_FSMIN			1061
#define PAGEPROC_ID_BASE_FSMAX			1062

/* ---------------------------------------------------------------------------------------------- */

int pageproc_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata, pageproc_data_t *data);

int pageproc_save(pageproc_data_t *data, HWND hwndPage);

void pageproc_data_init(pageproc_data_t *data, rxconfig_t *rxcfg);
void pageproc_data_apply(rxconfig_t *rxcfg, pageproc_data_t *data);

/* ---------------------------------------------------------------------------------------------- */
