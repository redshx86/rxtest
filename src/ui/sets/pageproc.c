/* ---------------------------------------------------------------------------------------------- */

#include "pageproc.h"

/* ---------------------------------------------------------------------------------------------- */

void pageproc_data_init(pageproc_data_t *data, rxconfig_t *rxcfg)
{
	data->input_hdr_count = (int)(rxcfg->input_hdr_count);
	data->input_hdr_ms = rxcfg->input_hdr_ms;

	data->proc_buf_len_ms = rxcfg->proc_buf_len_ms;
	data->proc_in_whyst_ms = rxcfg->proc_in_whyst_ms;
	data->proc_out_rhyst_ms = rxcfg->proc_out_rhyst_ms;

	data->proc_workbuf_ms = (int)(rxcfg->proc_workbuf_ms);

	data->base_fsmin = (int)(rxcfg->base_fsmin);
	data->base_fsmax = (int)(rxcfg->base_fsmax);
}

/* ---------------------------------------------------------------------------------------------- */

void pageproc_data_apply(rxconfig_t *rxcfg, pageproc_data_t *data)
{
	rxcfg->input_hdr_count = data->input_hdr_count;
	rxcfg->input_hdr_ms = data->input_hdr_ms;

	rxcfg->proc_buf_len_ms = data->proc_buf_len_ms;
	rxcfg->proc_in_whyst_ms = data->proc_in_whyst_ms;
	rxcfg->proc_out_rhyst_ms = data->proc_out_rhyst_ms;

	rxcfg->proc_workbuf_ms = data->proc_workbuf_ms;

	rxcfg->base_fsmin = data->base_fsmin;
	rxcfg->base_fsmax = data->base_fsmax;
}

/* ---------------------------------------------------------------------------------------------- */

int pageproc_save(pageproc_data_t *data, HWND hwndPage)
{
	pageproc_ctx_t *ctx;

	ctx = GetWndPtr(hwndPage, GWLP_USERDATA);

	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditInputHdrCount, &(data->input_hdr_count),
		RXLIM_INPUT_HDR_COUNT_MIN, RXLIM_INPUT_HDR_COUNT_MAX,
		_T("Number of input buffers"), _T("")) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditInputHdrMs, &(data->input_hdr_ms),
		RXLIM_INPUT_HDR_MS_MIN, RXLIM_INPUT_HDR_MS_MAX,
		_T("Length of input buffer"), _T(" ms"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditProcBufLenMs, &(data->proc_buf_len_ms),
		RXLIM_PROC_BUF_LEN_MS_MIN, RXLIM_PROC_BUF_LEN_MS_MAX,
		_T("Processing channel input and output buffers length"), _T(" ms"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditProcInWhystMs, &(data->proc_in_whyst_ms),
		0, data->proc_buf_len_ms,
		_T("Processing channel input buffer overflow hysteresis"), _T(" ms"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditProcOutRhystMs, &(data->proc_out_rhyst_ms),
		0, data->proc_buf_len_ms,
		_T("Processing channel output buffer underflow hysteresis"), _T(" ms"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditProcWorkbufMs, &(data->proc_workbuf_ms),
		RXLIM_PROC_WORKBUF_MS_MIN, RXLIM_PROC_WORKBUF_MS_MAX,
		_T("Processing channel working buffer length"), _T(" ms"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditBaseFsmin, &(data->base_fsmin),
		RXLIM_BASE_FS_MINABS, RXLIM_BASE_FS_MAXABS,
		_T("Minimum possible baseband sampling frequency"), _T(" Hz")) )
	{
		return 0;
	}

	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditBaseFsmax, &(data->base_fsmax),
		RXLIM_BASE_FS_MINABS, RXLIM_BASE_FS_MAXABS,
		_T("Maximum possible baseband sampling frequency"), _T(" Hz")) )
	{
		return 0;
	}

	if(data->base_fsmax < data->base_fsmin)
	{
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Possible baseband sampling frequency range is empty."));
		MessageBox(ctx->hwndSetWnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pageproc_initdata(pageproc_ctx_t *ctx, pageproc_data_t *data)
{
	ui_set_int(ctx->uidata, ctx->hwndEditInputHdrCount, data->input_hdr_count);
	ui_set_double(ctx->uidata, ctx->hwndEditInputHdrMs, data->input_hdr_ms, 0, 6, 1);
	ui_set_double(ctx->uidata, ctx->hwndEditProcBufLenMs, data->proc_buf_len_ms, 0, 6, 1);
	ui_set_double(ctx->uidata, ctx->hwndEditProcInWhystMs, data->proc_in_whyst_ms, 0, 6, 1);
	ui_set_double(ctx->uidata, ctx->hwndEditProcOutRhystMs, data->proc_out_rhyst_ms, 0, 6, 1);
	ui_set_double(ctx->uidata, ctx->hwndEditProcWorkbufMs, data->proc_workbuf_ms, 0, 6, 1);
	ui_set_int(ctx->uidata, ctx->hwndEditBaseFsmin, data->base_fsmin);
	ui_set_int(ctx->uidata, ctx->hwndEditBaseFsmax, data->base_fsmax);
}

/* ---------------------------------------------------------------------------------------------- */

static int pageproc_init(pageproc_ctx_t *ctx, pageproc_data_t *data)
{
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		0, 0, 235, 15, _T("Device input buffers:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 20, 140, 15, _T("Number of buffers"), ID_CTL_STATIC);
	ctx->hwndEditInputHdrCount = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 20, 45, 15, PAGEPROC_ID_INPUT_HDRCNT);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 40, 140, 15, _T("Length of each buffer"), ID_CTL_STATIC);
	ctx->hwndEditInputHdrMs = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 40, 45, 15, PAGEPROC_ID_INPUT_HDRMS);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		200, 40, 35, 15, _T("ms"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		0, 70, 235, 15, _T("Processing channel I/O buffers:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 90, 140, 30, _T("Input and output\nbuffers length"), ID_CTL_STATIC);
	ctx->hwndEditProcBufLenMs = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 98, 45, 15, PAGEPROC_ID_PROC_BUFLEN_MS);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		200, 98, 35, 15, _T("ms"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 120, 140, 30, _T("Input buffer\noverflow hysteresis"), ID_CTL_STATIC);
	ctx->hwndEditProcInWhystMs = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 128, 45, 15, PAGEPROC_ID_PROC_IN_WHYST_MS);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		200, 128, 35, 15, _T("ms"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0, 
		10, 150, 140, 30, _T("Output buffer\nunderflow hysteresis"), ID_CTL_STATIC);
	ctx->hwndEditProcOutRhystMs = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 158, 45, 15, PAGEPROC_ID_PROC_OUT_RHYST_MS);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		200, 158, 35, 15, _T("ms"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		245, 0, 235, 15, _T("Processing channel working buffer:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 20, 140, 15, _T("Buffer length"), ID_CTL_STATIC);
	ctx->hwndEditProcWorkbufMs = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 20, 45, 15, PAGEPROC_ID_PROC_WORKBUF_MS);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		445, 20, 35, 15, _T("ms"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0, 
		245, 50, 235, 15, _T("Baseband sampling rate:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0, 
		255, 70, 130, 15, _T("Minimum possible"), ID_CTL_STATIC);
	ctx->hwndEditBaseFsmin = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		385, 70, 55, 15, PAGEPROC_ID_BASE_FSMIN);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		445, 70, 35, 15, _T("Hz"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 90, 130, 15, _T("Maximum possible"), ID_CTL_STATIC);
	ctx->hwndEditBaseFsmax = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		385, 90, 55, 15, PAGEPROC_ID_BASE_FSMAX);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		445, 90, 35, 15, _T("Hz"), ID_CTL_STATIC);

	pageproc_initdata(ctx, data);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pageproc_cleanup(pageproc_ctx_t *ctx)
{
	if(ctx != NULL)
	{
		free(ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK pageproc_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	pageproc_ctx_t *ctx;

	switch(umsg)
	{
	case WM_NCDESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		pageproc_cleanup(ctx);
		return 0;

	case WM_COMMAND:
		break;
	}

	return DefWindowProc(hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

int pageproc_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata, pageproc_data_t *data)
{
	pageproc_ctx_t *ctx;

	if( (ctx = calloc(1, sizeof(pageproc_ctx_t))) != NULL )
	{
		ctx->hwndSetWnd = hwndSetWnd;
		ctx->hwndPage = hwndPage;
		ctx->uidata = uidata;

		SetWndPtr(hwndPage, GWLP_WNDPROC, (void*)pageproc_proc);
		SetWndPtr(hwndPage, GWLP_USERDATA, ctx);

		if(pageproc_init(ctx, data))
			return 1;

		SetWndPtr(hwndPage, GWLP_USERDATA, NULL);
		free(ctx);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
