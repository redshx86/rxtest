/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../specview.h"

/* ---------------------------------------------------------------------------------------------- */

enum {
	PAGECRSV_ROW_BKGND,
	PAGECRSV_ROW_INPUT_FC,
	PAGECRSV_ROW_CHAN_FC,
	PAGECRSV_ROW_CHAN_BW,
	PAGECRSV_ROW_CHAN_BWOVL,

	PAGECRSV_ROW_COUNT
};

/* ---------------------------------------------------------------------------------------------- */

enum {
	PAGECRSV_COL_CN,
	PAGECRSV_COL_CG,
	PAGECRSV_COL_HN,
	PAGECRSV_COL_HG,

	PAGECRSV_COL_COUNT
};

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagecrsv_rowdata {

	HWND hwndBtnColorPick[PAGECRSV_COL_COUNT];
	RECT rcBtnColorPick[PAGECRSV_COL_COUNT];

} pagecrsv_rowdata_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagecrsv_ctx {

	HWND hwndSetWnd;
	HWND hwndPage;
	uicommon_t *uidata;

	HWND hwndCrList;
	WNDPROC wpCrListOriginal;

	HWND hwndCrListEdit;
	WNDPROC wpCrListEdit;
	RECT rcCrListEdit;
	int iCrListEditRow;
	int iCrListEditCol;

	pagecrsv_rowdata_t rowdata[PAGECRSV_ROW_COUNT];

	HWND hwndEditCrLabelEdit;
	HWND hwndBtnCrLabelPick;
	HWND hwndEditCrTicksEdit;
	HWND hwndBtnCrTicksPick;
	HWND hwndEditCrTextBgEdit;
	HWND hwndBtnCrTextBgPick;
	HWND hwndEditCrTextEdit;
	HWND hwndBtnCrTextPick;
	HWND hwndEditCrSqSenseOp;
	HWND hwndBtnCrSqSenseOpPick;
	HWND hwndEditCrSqSenseCl;
	HWND hwndBtnCrSqSenseClPick;
	HWND hwndEditCrSqThresOp;
	HWND hwndBtnCrSqThresOpPick;
	HWND hwndEditCrSqThresCl;
	HWND hwndBtnCrSqThresClPick;

	int samp_x;
	int samp_y;
	int samp_w;
	int samp_h;

} pagecrsv_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagecrsv_data {

	COLORREF cr_label;
	COLORREF cr_ticks;
	COLORREF cr_text;
	COLORREF cr_textbg;

	COLORREF cr_sq_sense_op;
	COLORREF cr_sq_sense_cl;
	COLORREF cr_sq_thres_op;
	COLORREF cr_sq_thres_cl;

	COLORREF cr[PAGECRSV_ROW_COUNT][PAGECRSV_COL_COUNT];

} pagecrsv_data_t;

/* ---------------------------------------------------------------------------------------------- */

#define PAGECRSV_ID_CRLIST					1001
#define PAGECRSV_ID_CRLIST_EDIT				1002

#define PAGECRSV_ID_PICKBTN_0				1021
#define PAGECRSV_ID_PICKBTN_CNT				( PAGECRSV_ROW_COUNT * PAGECRSV_COL_COUNT )
#define PAGECRSV_ID_PICKBTN_MAX				( PAGECRSV_ID_PICKBTN_0 + PAGECRSV_ID_PICKBTN_CNT - 1 )

#define PAGECRSV_ID_CR_LABEL_EDIT			1301
#define PAGECRSV_ID_CR_LABEL_PICK			1302
#define PAGECRSV_ID_CR_TICKS_EDIT			1303
#define PAGECRSV_ID_CR_TICKS_PICK			1304
#define PAGECRSV_ID_CR_TEXTBG_EDIT			1305
#define PAGECRSV_ID_CR_TEXTBG_PICK			1306
#define PAGECRSV_ID_CR_TEXT_EDIT			1307
#define PAGECRSV_ID_CR_TEXT_PICK			1308
#define PAGECRSV_ID_CR_SQ_SENSE_OP_EDIT		1309
#define PAGECRSV_ID_CR_SQ_SENSE_OP_PICK		1310
#define PAGECRSV_ID_CR_SQ_SENSE_CL_EDIT		1311
#define PAGECRSV_ID_CR_SQ_SENSE_CL_PICK		1312
#define PAGECRSV_ID_CR_SQ_THRES_OP_EDIT		1313
#define PAGECRSV_ID_CR_SQ_THRES_OP_PICK		1314
#define PAGECRSV_ID_CR_SQ_THRES_CL_EDIT		1315
#define PAGECRSV_ID_CR_SQ_THRES_CL_PICK		1316

/* ---------------------------------------------------------------------------------------------- */

int pagecrsv_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata, pagecrsv_data_t *data);

int pagecrsv_save(pagecrsv_data_t *data, HWND hwndPage);

void pagecrsv_data_init(pagecrsv_data_t *data, specview_cfg_t *svcfg);

int pagecrsv_data_apply(specview_ctx_t *specview, pagecrsv_data_t *data,
						callback_list_t *cb_list, HWND hwndMsgbox);

/* ---------------------------------------------------------------------------------------------- */
