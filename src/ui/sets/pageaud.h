/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "../uicommon.h"
#include "../../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct pageaud_ctx {

	HWND hwndSetWnd;
	HWND hwndPage;
	uicommon_t *uidata;

	HWND hwndEditStaticGain;

	HWND hwndEditResampDf;
	HWND hwndEditResampAs;

	HWND hwndEditLimThres;
	HWND hwndEditLimRange;

	HWND hwndEditDevId;
	HWND hwndBtnDevSel;
	HWND hwndEditFs;
	HWND hwndEditBps;

	HWND hwndEditHdrCnt;
	HWND hwndEditHdrMs;

	HMENU hmenuDevSel;
	int num_waveout_devs;

} pageaud_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct pageaud_data {

	double static_gain;

	int resamp_df;
	double resamp_as;

	double lim_thres;
	double lim_range;

	int device_id;
	int fs;
	int bps;

	int hdr_cnt;
	double hdr_ms;

} pageaud_data_t;

/* ---------------------------------------------------------------------------------------------- */

#define PAGEAUD_ID_STATICGAIN		1001

#define PAGEAUD_ID_RESAMP_DF		1021
#define PAGEAUD_ID_RESAMP_AS		1022

#define PAGEAUD_ID_LIM_THRES		1041
#define PAGEAUD_ID_LIM_RANGE		1042

#define PAGEAUD_ID_OUT_DEVID		1061
#define PAGEAUD_ID_OUT_DEVSEL		1062
#define PAGEAUD_ID_FS				1063
#define PAGEAUD_ID_BPS				1064

#define PAGEAUD_ID_HDR_CNT			1081
#define PAGEAUD_ID_HDR_MS			1082

#define PAGEAUD_ID_WAVEMAPPER		1101
#define PAGEAUD_ID_DEVICE_0			1102
#define PAGEAUD_ID_DEVICE_MAX		1165

/* ---------------------------------------------------------------------------------------------- */

int pageaud_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata, pageaud_data_t *data);

int pageaud_save(pageaud_data_t *data, HWND hwndPage);

void pageaud_data_init(pageaud_data_t *data, rxconfig_t *rxcfg);
void pageaud_data_apply(rxconfig_t *rxcfg, pageaud_data_t *data);

/* ---------------------------------------------------------------------------------------------- */
