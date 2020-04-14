/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"

#include "../../rx/rxstate.h"
#include "../specview.h"
#include "../watrview.h"

#include "pageproc.h"
#include "pageaud.h"
#include "pagevis.h"
#include "pagecrsv.h"
#include "pagecrwv.h"

/* ---------------------------------------------------------------------------------------------- */

typedef enum {

	SETWND_PAGE_PROC,
	SETWND_PAGE_AUDIO,
	SETWND_PAGE_VIS,
	SETWND_PAGE_CRSV,
	SETWND_PAGE_CRWV,

	SETWND_PAGE_COUNT,

};

/* ---------------------------------------------------------------------------------------------- */

typedef struct setwnd_page_entry {
	TCHAR *page_name;
	TCHAR *page_title;
} setwnd_page_entry_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct setwnd_ctx {

	HWND hwnd;

	uicommon_t *uidata;

	uievent_t *event_window_close;
	uievent_t *event_visualcfg;

	ini_data_t *ini;

	rxstate_t *rx;
	specview_ctx_t *specview;
	watrview_ctx_t *watrview;

	HWND hwndPageList;
	HWND hwndPageTitle;

	HWND hwndPage;
	int i_cur_page;

	pageaud_data_t data_aud;
	pageproc_data_t data_proc;
	pagevis_data_t data_vis;
	pagecrsv_data_t data_crsv;
	pagecrwv_data_t data_crwv;

	int cx_frame;
	int cy_frame;

} setwnd_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

#define SETWND_W				625
#define SETWND_H				310

/* ---------------------------------------------------------------------------------------------- */

#define SETWND_ID_PAGELIST		1001
#define SETWND_ID_PAGETITLE		1002
#define SETWND_ID_PAGE			1003

#define SETWND_ID_CANCEL		1021
#define SETWND_ID_APPLY			1022
#define SETWND_ID_OK			1023

/* ---------------------------------------------------------------------------------------------- */

HWND setwnd_create(uicommon_t *uidata, HWND hwndMain, ini_data_t *ini,
				   rxstate_t *rx, specview_ctx_t *specview, watrview_ctx_t *watrview);

int setwnd_registerclass(uicommon_t *uidata);
void setwnd_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */
