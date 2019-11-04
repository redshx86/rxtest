/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <tchar.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <search.h>
#include <process.h>
#include <crtdbg.h>
#include "indesc.h"
#include "rtlexp.h"
#include "../util/pathutil.h"
#include "../util/macro.h"

/* ---------------------------------------------------------------------------------------------- */

/* time to wait before killing reading thread */
#define RTLSDR_THREAD_WAIT_TIME		15000	/* ms */

/* frequency correction valid range */
#define RTLSDR_FREQCORR_MIN			-10000	/* ppm */
#define RTLSDR_FREQCORR_MAX			10000	/* ppm */

/* read buffer count valid range */
#define RTLSDR_BUFFER_COUNT_MIN		4
#define RTLSDR_BUFFER_COUNT_MAX		64

/* read buffer length valid range */
#define RTLSDR_BUFFER_MS_MIN		15.0	/* ms */
#define RTLSDR_BUFFER_MS_MAX		250.0	/* ms */

#define RTLSDR_DIRECTORY			_T("rtlsdr")
#define RTLSDR_MODULENAME			_T("rtlsdr.dll")

/* ---------------------------------------------------------------------------------------------- */

/* tuner center frequency valid range */
typedef struct rtlsdr_tuner_fc_range {
	unsigned int fc_start;			/* range start frequency */
	unsigned int fc_end;			/* range end frequency */
	unsigned int fc_gap_start;		/* gap start frequency (or zero) */
	unsigned int fc_gap_end;		/* gap end frequency (or zero) */
} rtlsdr_tuner_fc_range_t;

/* ---------------------------------------------------------------------------------------------- */

/* module data */
typedef struct rtlsdr_moduledata {
	HMODULE hmod;					/* rtl-sdr dll handle */
	rtlsdr_proc_t proc;				/* imported function list */
} rtlsdr_moduledata_t;

/* ---------------------------------------------------------------------------------------------- */

/* device info */
typedef struct rtlsdr_deviceinfo {

	char name[256];					/* device name */

	rtlsdr_tuner_type_t tuner_type;	/* type of tuners */

	int *tuner_gains_buf;			/* possible tuner gains, tenths of dB */
	int num_tuner_gains;			/* number of possible tuner gains */

	int is_offset_tuning_supported;

} rtlsdr_deviceinfo_t;

/* ---------------------------------------------------------------------------------------------- */

/* device confguration */
typedef struct rtlsdr_deviceconfig {

	int direct_sampling;			/* direct sampling mode, 0=off, 1=I-ADC, 2=Q-ADC */

	unsigned int fs;				/* sampling frequency, Hz */
	unsigned int fc;				/* center frequency, Hz */

	int offset_tuning_on;			/* offset tuning mode, 0=off, 1=on */
	int tuner_gain_manual;			/* manual tuner gain enabled, 0=off, 1=on */
	int rtl_agc_on;					/* RTL2832 AGC mode, 0=off, 1=on */
	int tuner_gain_value;

	int freqcorr;					/* frequency correction value, ppm */

	int buf_count;					/* number of read buffers */
	double buf_len_ms;				/* length of each read buffer, ms */

} rtlsdr_deviceconfig_t;

/* ---------------------------------------------------------------------------------------------- */

/* enumerated device data */
typedef struct rtlsdr_device_data {

	int index;						/* device index */
	int is_valid;					/* data valid flag */

	rtlsdr_deviceinfo_t info;		/* device information */
	rtlsdr_deviceconfig_t cfg;		/* device config */

} rtlsdr_device_data_t;

/* ---------------------------------------------------------------------------------------------- */

/* opened device state */
typedef struct rtlsdr_state {

	int openeddeviceindex;			/* opened device index */

	rtlsdr_deviceinfo_t *devinfo;	/* pointer to device information */
	rtlsdr_deviceconfig_t *devcfg;	/* pointer to device config */

	void *dev;						/* device handle */

	HANDLE buf_event;				/* buffer ready event */
	CRITICAL_SECTION buf_csec;		/* buffer pointers lock */
	input_buf_t *buf_next_fill;		/* current filling buffer */
	input_buf_t *buf_last_fill;		/* last buffer in filling queue */
	input_buf_t *buf_next_out;		/* next buffer to return */
	input_buf_t *buf_last_out;		/* last buffer in return queue */

	int is_started;					/* device read started */
	int error;						/* error flag */

	unsigned int threadid;			/* reading thread id */
	HANDLE hthread;					/* reading thread handle */
	int read_async_entered;			/* readint thread into read_async function */

} rtlsdr_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* rtl-sdr module context */
typedef struct rtlsdr_ctx {

	rtlsdr_proc_t *proc;			/* dll function pointers */

	int devicecount;				/* number of enumerated devices */
	rtlsdr_device_data_t *devicedata;	/* enumerated device data */

	int deviceindex;				/* selected device index */
	rtlsdr_state_t *s;				/* opened device state */

	struct rtlwnd_ctx *cfgwnd;		/* configuration window context (when opened) */

} rtlsdr_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* configuration window context */
typedef struct rtlwnd_ctx {

	HWND hwnd;						/* window handle */

	uicommon_t *uidata;				/* common ui data */
	rtlsdr_ctx_t *mod;				/* rtl-sdr module context */

	/* control handles */
	HWND hwndCbDevice;
	HWND hwndEditTunerType;
	HWND hwndCbDirectSampMode;
	HWND hwndCbSampRate;
	HWND hwndBtnOffsetTuning;
	HWND hwndBtnTunerAgc;
	HWND hwndBtnRtlAgc;
	HWND hwndStaticTunerGainOut;
	HWND hwndTbTunerGain;
	HWND hwndEditFreqCorr;
	HWND hwndEditBufCount;
	HWND hwndEditBufLenMs;

	int selected_index;				/* currently selected device index */

} rtlwnd_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* configuration window control ids */
#define RTLWND_ID_DEVICE			1001
#define RTLWND_ID_TUNERTYPE			1002
#define RTLWND_ID_SAMPMODE			1003
#define RTLWND_ID_SAMPRATE			1004
#define RTLWND_ID_OFFSETTUNING		1005
#define RTLWND_ID_TUNERAGC			1006
#define RTLWND_ID_RTLAGC			1007
#define RTLWND_ID_TUNERGAINOUT		1008
#define RTLWND_ID_TUNERGAIN			1009
#define RTLWND_ID_FREQCORR			1010
#define RTLWND_ID_BUFCOUNT			1011
#define RTLWND_ID_BUFLENMS			1012

#define RTLWND_ID_APPLY				1021
#define RTLWND_ID_CANCEL			IDCANCEL
#define RTLWND_ID_OK				IDOK

/* ---------------------------------------------------------------------------------------------- */

/* configuration window dimensions (client area) */
#define RTLWND_W					320
#define RTLWND_H					275

/* ---------------------------------------------------------------------------------------------- */

input_module_desc_t *rtlsdr_load();

int rtlwnd_registerclass(uicommon_t *uidata);
void rtlwnd_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */

int rtlsdr_updateconfig(rtlsdr_ctx_t *ctx, rtlsdr_deviceconfig_t *devcfgcur,
						   rtlsdr_deviceconfig_t *devcfgnew, void *dev,
						   rtlsdr_deviceinfo_t *devinfo, TCHAR *errbuf, size_t errbufsize);

void rtlwnd_setstarted(rtlwnd_ctx_t *ctx, BOOL started);

HWND rtlwnd_create(uicommon_t *uidata, HWND hwndowner, rtlsdr_ctx_t *cfg);

/* ---------------------------------------------------------------------------------------------- */
