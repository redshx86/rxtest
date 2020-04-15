/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../watrview.h"
#include "../specscal.h"
#include "../colormap.h"

/* ---------------------------------------------------------------------------------------------- */

enum {
	PAGECRWV_CMLIST_COL_MAGN,
	PAGECRWV_CMLIST_COL_CRNORM,
	PAGECRWV_CMLIST_COL_CRCHAN,

	PAGECRWV_CMLIST_COL_COUNT
};

#define PAGECRWV_PREVIEW_MIN_MAG_LABEL_H	15

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagecrwv_preview_ctx {

	int is_inited;

	int win_x;
	int win_y;
	int win_w;
	int win_h;

	int scale_y;
	int scale_h;

	int lab_x;
	int lab_w;

	int tick1_x;
	int tick1_w;

	int grad_x;
	int grad_w;


	COLORREF cr_label;
	COLORREF cr_ticks;

	COLORREF *buf_norm;
	COLORREF *buf_chan;

	int cr_pt_count;
	watrview_cr_map_pt_t *cr_pt;

	double m_0;
	double m_1;

	HDC hdc_buf;
	HBITMAP hbm_buf;

	HFONT hfont;

} pagecrwv_preview_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagecrwv_ctx {

	HWND hwndSetWnd;
	HWND hwndPage;
	uicommon_t *uidata;

	ini_data_t *ini;

	HWND hwndCmList;
	WNDPROC wpCmList;

	HWND hwndCmListEdit;
	WNDPROC wpCmListEdit;
	RECT rectCmListEdit;
	int iCmListEditRow;
	int iCmListEditCol;

	HWND hwndBtnCmPtAdd;
	HWND hwndBtnCmPtDel;
	HWND hwndBtnCmPtUp;
	HWND hwndBtnCmPtDown;
	HWND hwndBtnCmPtSort;

	HWND hwndEditInitMag0;
	HWND hwndEditInitMag1;
	HWND hwndBtnInitMagExec;

	HWND hwndCbPresetSel;
	HWND hwndBtnPresetApplyNorm;
	HWND hwndBtnPresetApplyBkgnd;

	HWND hwndBtnPreviewUpd;

	HWND hwndEditCrBkgnd;
	HWND hwndBtnCrBkgndPick;
	HWND hwndEditCrLabel;
	HWND hwndBtnCrLabelPick;
	HWND hwndEditCrTicks;
	HWND hwndBtnCrTicksPick;
	HWND hwndEditCrScrbar;
	HWND hwndBtnCrScrbarPick;

	HWND hwndBtnUseChanCrMap;

	pagecrwv_preview_ctx_t preview;

} pagecrwv_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagecrwv_cmlist_row_data {

	UINT uctrlgroup;

	watrview_cr_map_pt_t cr_pt;

	HWND hwndBtnPickNorm;
	RECT rcBtnPickNorm;

	HWND hwndBtnPickChan;
	RECT rcBtnPickChan;

} pagecrwv_cmlist_row_data_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagecrwv_data {

	COLORREF cr_bkgnd;
	COLORREF cr_label;
	COLORREF cr_ticks;
	COLORREF cr_scrbar;

	int cr_pt_count;
	watrview_cr_map_pt_t cr_pt[WATRVIEW_CR_PT_MAX];

	int use_chan_crmap;

} pagecrwv_data_t;

/* ---------------------------------------------------------------------------------------------- */

#define PAGECRWV_ID_CMLIST				1001
#define PAGECRWV_ID_CMLIST_EDIT			1002

#define PAGECRWV_ID_PT_ADD				1021
#define PAGECRWV_ID_PT_DEL				1022
#define PAGECRWV_ID_PT_UP				1023
#define PAGECRWV_ID_PT_DOWN				1024
#define PAGECRWV_ID_PT_SORT				1025

#define PAGECRWV_ID_INITMAG_M0			1041
#define PAGECRWV_ID_INITMAG_M1			1042
#define PAGECRWV_ID_INITMAG_EXEC		1043

#define PAGECRWV_ID_PRESET_SEL			1061
#define PAGECRWV_ID_PRESET_APPLY_NORM	1062
#define PAGECRWV_ID_PRESET_APPLY_CHAN	1063

#define PAGECRWV_ID_PREVIEW_UPD			1101

#define PAGECRWV_ID_CR_BKGND_EDIT		1121
#define PAGECRWV_ID_CR_BKGND_PICK		1122
#define PAGECRWV_ID_CR_LABEL_EDIT		1123
#define PAGECRWV_ID_CR_LABEL_PICK		1124
#define PAGECRWV_ID_CR_TICKS_EDIT		1125
#define PAGECRWV_ID_CR_TICKS_PICK		1126
#define PAGECRWV_ID_CR_SCRBAR_EDIT		1127
#define PAGECRWV_ID_CR_SCRBAR_PICK		1128

#define PAGECRWV_ID_USE_CHAN_CRMAP		1129

#define PAGECRWV_ID_CMLIST_ROWBTN_0		1141
#define PAGECRWV_ID_CMLIST_ROWBTN_CNT	( PAGECRWV_CMLIST_COL_COUNT * WATRVIEW_CR_PT_MAX )
#define PAGECRWV_ID_CMLIST_ROWBTN_MAX	\
	( PAGECRWV_ID_CMLIST_ROWBTN_0 + PAGECRWV_ID_CMLIST_ROWBTN_CNT - 1)

/* ---------------------------------------------------------------------------------------------- */

int pagecrwv_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata,
					ini_data_t *ini, pagecrwv_data_t *data);

int pagecrwv_save(pagecrwv_data_t *data, HWND hwndPage);

void pagecrwv_data_init(pagecrwv_data_t *data, watrview_cfg_t *wvcfg);

void pagecrwv_data_apply(watrview_cfg_t *wvcfg, pagecrwv_data_t *data);

/* ---------------------------------------------------------------------------------------------- */
