/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../../rx/rxstate.h"
#include "../specview.h"
#include "../watrview.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagevis_wndentry {
	TCHAR *name;
	TCHAR *param_name;
	double param_default;
	int window_id;
} pagevis_wndentry_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagevis_ctx {

	HWND hwndSetWnd;
	HWND hwndPage;
	uicommon_t *uidata;

	HWND hwndEditSpectUpsReq;
	HWND hwndEditSpectLength;
	HWND hwndEditSpectWndType;
	HWND hwndStaticSpectWndArgName;
	HWND hwndEditSpectWndArgValue;
	HWND hwndEditSpectMagRef;
	HWND hwndEditSpectBufCount;
	int i_spect_cur_wnd;
	int i_spect_org_wnd;
	double spect_org_wnd_arg;
	HMENU hmenuSpectWndType;

	HWND hwndEditSvMag0;
	HWND hwndEditSvMag1;
	HWND hwndBtnSvPeak;
	HWND hwndBtnSvAvg;
	HWND hwndBtnSvUpsCtr;

	HWND hwndBtnWvPeak;
	HWND hwndBtnWvAvg;
	HWND hwndEditWvFrameDiv;
	HWND hwndEditWvChainMaxLen;
	HWND hwndEditWvSegLen;

} pagevis_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct pagevis_data {

	int spect_ups_req;
	int spect_length;
	int spect_bufcount;
	int spect_wndtype;
	double spect_wndarg;
	double spect_magref;

	double sv_m_0;
	double sv_m_1;
	int sv_scale_mode;
	int sv_show_ups_counter;

	int wv_scale_mode;
	int wv_framediv;
	int wv_chain_max_len;
	int wv_seg_len;

} pagevis_data_t;

/* ---------------------------------------------------------------------------------------------- */

#define PAGEVIS_ID_SPECT_UPS_REQ		1001
#define PAGEVIS_ID_SPECT_LENGTH			1002
#define PAGEVIS_ID_SPECT_WNDTYPE		1003
#define PAGEVIS_ID_SPECT_WNDSELECT		1004
#define PAGEVIS_ID_SPECT_WNDARG_NAME	1005
#define PAGEVIS_ID_SPECT_WNDARG_VALUE	1006
#define PAGEVIS_ID_SPECT_MAGREF			1007
#define PAGEVIS_ID_SPECT_BUFCOUNT		1008

#define PAGEVIS_ID_SPECT_WND_0			1021
#define PAGEVIS_ID_SPECT_WND_MAX		1084

#define PAGEVIS_ID_SV_MAG0				1101
#define PAGEVIS_ID_SV_MAG1				1102
#define PAGEVIS_ID_SV_PEAK				1103
#define PAGEVIS_ID_SV_AVG				1104
#define PAGEVIS_ID_SV_UPSCTR			1105

#define PAGEVIS_ID_WV_PEAK				1121
#define PAGEVIS_ID_WV_AVG				1122
#define PAGEVIS_ID_WV_FRAMEDIV			1123
#define PAGEVIS_ID_WV_CHAINMAXLEN		1124
#define PAGEVIS_ID_WV_SEGLEN			1125

/* ---------------------------------------------------------------------------------------------- */

int pagevis_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata, pagevis_data_t *data);

int pagevis_save(pagevis_data_t *data, HWND hwndPage);

void pagevis_data_init(pagevis_data_t *data,
					   rxconfig_t *rxcfg, specview_cfg_t *specviewcfg, watrview_cfg_t *watrviewcfg);

void pagevis_data_apply(rxconfig_t *rxcfg, specview_cfg_t *specviewcfg, watrview_cfg_t *watrviewcfg,
						pagevis_data_t *data);

/* ---------------------------------------------------------------------------------------------- */
