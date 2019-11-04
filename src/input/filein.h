/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <mmsystem.h>
#include <malloc.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "../dsp/complex.h"
#include "../audio/wavio.h"
#include "../audio/cvtbuf.h"
#include "../ui/uicommon.h"
#include "../util/iniparse.h"
#include "../../res/resource.h"
#include "indesc.h"

/* ---------------------------------------------------------------------------------------------- */

#define FILEIN_FC_MIN			-1e12
#define FILEIN_FC_MAX			1e12

#define FILEIN_READINT_MIN		15
#define FILEIN_READINT_MAX		500

/* ---------------------------------------------------------------------------------------------- */

enum filein_error {
	FILEIN_OK,
	FILEIN_ERR_MEM,
} filein_error_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct filein_config {
	
	size_t fname_max_len;
	TCHAR *fname;
	double fc;
	int invert_iq;
	unsigned int read_interval;

	int filewnd_fc_prefix;

} filein_config_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct filein_state {

	wav_io_state_t wavin;

	unsigned int fs;
	unsigned int bps;

	size_t nsamp;
	size_t pos;

	size_t blocksize;
	size_t sampbuflen;
	void *sampbuf;

	HANDLE buf_event;
	CRITICAL_SECTION buf_csec;
	input_buf_t *buf_next_fill;
	input_buf_t *buf_last_fill;
	input_buf_t *buf_next_out;
	input_buf_t *buf_last_out;

	UINT timer_tick;
	UINT timer_interval;
	UINT timer_id;

	unsigned __int64 nsamp_frac_rem;

	int error;

	HANDLE filein_event_stop;
	HANDLE filein_event_timer;
	HANDLE filein_thread;

	double fc;
	int invert_iq;

} filein_state_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct filein_ctx {

	struct filewnd_ctx *filewnd;

	filein_config_t conf;
	filein_state_t *s;

} filein_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct filewnd_ctx {

	HWND hwnd;

	uicommon_t *uidata;
	struct filein_ctx *filein;

	HWND hwndEditFilename;
	HWND hwndBtnBrowse;
	HWND hwndEditFc;
	HWND hwndEditReadInt;
	HWND hwndBtnSwapIq;

} filewnd_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

#define FILEWND_ID_FILENAME		1001
#define FILEWND_ID_BROWSE		1002
#define FILEWND_ID_FC			1003
#define FILEWND_ID_READINT		1004
#define FILEWND_ID_SWAPIQ		1005

#define FILEWND_ID_APPLY		1021
#define FILEWND_ID_CALCEL		IDCANCEL
#define FILEWND_ID_OK			IDOK

/* ---------------------------------------------------------------------------------------------- */

#define FILEWND_W				320
#define FILEWND_H				145

/* ---------------------------------------------------------------------------------------------- */

input_module_desc_t *filein_get_desc();

int filewnd_registerclass(uicommon_t *uidata);
void filewnd_unregisterclass(HINSTANCE h_inst);

/* ---------------------------------------------------------------------------------------------- */

HWND filewnd_create(uicommon_t *uidata, HWND hwndowner, filein_ctx_t *filein);
void filewnd_setstate(filewnd_ctx_t *ctx, BOOL started);

/* ---------------------------------------------------------------------------------------------- */
