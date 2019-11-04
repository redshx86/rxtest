/* ---------------------------------------------------------------------------------------------- */

#include "pageaud.h"

/* ---------------------------------------------------------------------------------------------- */

void pageaud_data_init(pageaud_data_t *data, rxconfig_t *rxcfg)
{
	data->static_gain = rxcfg->output_static_gain;

	data->resamp_df = (int)(rxcfg->output_resamp_df);
	data->resamp_as = rxcfg->output_resamp_as;

	data->lim_thres = rxcfg->output_lim_thres;
	data->lim_range = rxcfg->output_lim_range;

	data->device_id = rxcfg->output_device_id;
	data->fs = (int)(rxcfg->output_fs);
	data->bps = (int)(rxcfg->output_bps);

	data->hdr_cnt = (int)(rxcfg->output_hdr_cnt);
	data->hdr_ms = rxcfg->output_hdr_ms;
}

/* ---------------------------------------------------------------------------------------------- */

void pageaud_data_apply(rxconfig_t *rxcfg, pageaud_data_t *data)
{
	rxcfg->output_static_gain = data->static_gain;

	rxcfg->output_resamp_df = data->resamp_df;
	rxcfg->output_resamp_as = data->resamp_as;

	rxcfg->output_lim_thres = data->lim_thres;
	rxcfg->output_lim_range = data->lim_range;

	rxcfg->output_device_id = data->device_id;
	rxcfg->output_fs = data->fs;
	rxcfg->output_bps = data->bps;

	rxcfg->output_hdr_cnt = data->hdr_cnt;
	rxcfg->output_hdr_ms = data->hdr_ms;
}

/* ---------------------------------------------------------------------------------------------- */

int pageaud_save(pageaud_data_t *data, HWND hwndPage)
{
	pageaud_ctx_t *ctx;

	ctx = GetWndPtr(hwndPage, GWLP_USERDATA);

	/* channel mixing */
	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditStaticGain, &(data->static_gain),
		RXLIM_OUTPUT_STATIC_GAIN_MIN, RXLIM_OUTPUT_STATIC_GAIN_MAX,
		_T("Static output gain"), _T(" dB"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	/* resampling */
	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditResampDf, &(data->resamp_df),
		RXLIM_OUTPUT_RESAMP_DF_MIN, RXLIM_OUTPUT_RESAMP_DF_MAX,
		_T("Audio resampler transition bandwidth"), _T(" Hz")) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditResampAs, &(data->resamp_as),
		RXLIM_OUTPUT_RESAMP_AS_MIN, RXLIM_OUTPUT_RESAMP_AS_MAX, 
		_T("Audio resampler alias attenuation"), _T(" dB"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	/* limiting */
	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditLimThres, &(data->lim_thres),
		RXLIM_OUTPUT_LIM_THRES_MIN, RXLIM_OUTPUT_LIM_THRES_MAX,
		_T("Audio limiting threshold"), _T(""), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditLimRange, &(data->lim_range),
		RXLIM_OUTPUT_LIM_RANGE_MIN, RXLIM_OUTPUT_LIM_RANGE_MAX,
		_T("Audio limiting range"), _T(""), 6, 1, 0, NULL) )
	{
		return 0;
	}

	/* device config */
	if( !ui_get_int(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditDevId, &(data->device_id),
		_T("Output device identifier")) )
	{
		return 0;
	}

	if( (data->device_id < -1) || (data->device_id >= ctx->num_waveout_devs) )
	{
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Output device identifier must be -1 (auto) or %d to %d. Currently %d."),
			0, ctx->num_waveout_devs - 1, data->device_id);
		MessageBox(ctx->hwndSetWnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditFs, &(data->fs),
		RXLIM_OUTPUT_FS_MIN, RXLIM_OUTPUT_FS_MAX,
		_T("Output sampling rate"), _T(" Hz")) )
	{
		return 0;
	}

	if( !ui_get_int(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditBps, &(data->bps), 
		_T("Output resolution")) )
	{
		return 0;
	}

	if( (data->bps != 8) && (data->bps != 16) )
	{
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Output resolution must be 8 or 16 bits. Currently %d bits."),
			data->bps);
		MessageBox(ctx->hwndSetWnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	/* output buffer */
	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditHdrCnt, &(data->hdr_cnt),
		RXLIM_OUTPUT_HDR_CNT_MIN, RXLIM_OUTPUT_HDR_CNT_MAX,
		_T("Number of output buffers"), _T("")) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditHdrMs, &(data->hdr_ms),
		RXLIM_OUTPUT_HDR_MS_MIN, RXLIM_OUTPUT_HDR_MS_MAX,
		_T("Length of output buffer"), _T(" ms"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pageaud_initdata(pageaud_ctx_t *ctx, pageaud_data_t *data)
{
	ui_set_double(ctx->uidata, ctx->hwndEditStaticGain, data->static_gain, 0, 6, 1);

	ui_set_int(ctx->uidata, ctx->hwndEditResampDf, data->resamp_df);
	ui_set_double(ctx->uidata, ctx->hwndEditResampAs, data->resamp_as, 0, 6, 1);

	ui_set_double(ctx->uidata, ctx->hwndEditLimThres, data->lim_thres, 0, 6, 1);
	ui_set_double(ctx->uidata, ctx->hwndEditLimRange, data->lim_range, 0, 6, 1);

	ui_set_int(ctx->uidata, ctx->hwndEditDevId, data->device_id);
	ui_set_int(ctx->uidata, ctx->hwndEditFs, data->fs);
	ui_set_int(ctx->uidata, ctx->hwndEditBps, data->bps);

	ui_set_int(ctx->uidata, ctx->hwndEditHdrCnt, data->hdr_cnt);
	ui_set_double(ctx->uidata, ctx->hwndEditHdrMs, data->hdr_ms, 0, 6, 1);
}

/* ---------------------------------------------------------------------------------------------- */

static int pageaud_init(pageaud_ctx_t *ctx, pageaud_data_t *data)
{
	int i;
	UINT uid;
	WAVEOUTCAPS woc;

	/* create controls */
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		0, 0, 235, 15, _T("Channel mixing parameters:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 20, 140, 15, _T("Static output gain"), ID_CTL_STATIC);
	ctx->hwndEditStaticGain = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 20, 45, 15, PAGEAUD_ID_STATICGAIN);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		200, 20, 35, 15, _T("dB"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		0, 50, 235, 15, _T("Audio resampling parameters:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 70, 140, 15, _T("Transition bandwidth"), ID_CTL_STATIC);
	ctx->hwndEditResampDf = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 70, 45, 15, PAGEAUD_ID_RESAMP_DF);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		200, 70, 35, 15, _T("Hz"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 90, 140, 15, _T("Alias attenuation"), ID_CTL_STATIC);
	ctx->hwndEditResampAs = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 90, 45, 15, PAGEAUD_ID_RESAMP_AS);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		200, 90, 35, 15, _T("dB"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		0, 120, 235, 15, _T("Audio range limiting paramteters:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 140, 140, 15, _T("Limiting threshold"), ID_CTL_STATIC);
	ctx->hwndEditLimThres = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 140, 45, 15, PAGEAUD_ID_LIM_THRES);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 160, 140, 15, _T("Limiting range"), ID_CTL_STATIC);
	ctx->hwndEditLimRange = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 160, 45, 15, PAGEAUD_ID_LIM_RANGE);


	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		245, 0, 235, 15, _T("Waveout device parameters:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 20, 140, 15, _T("Device identifier"), ID_CTL_STATIC);
	ctx->hwndEditDevId = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 20, 45, 15, PAGEAUD_ID_OUT_DEVID);
	ctx->hwndBtnDevSel = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		445, 20, 15, 15, _T("»"), PAGEAUD_ID_OUT_DEVSEL);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 40, 140, 15, _T("Output sampling rate"), ID_CTL_STATIC);
	ctx->hwndEditFs = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 40, 45, 15, PAGEAUD_ID_FS);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		445, 40, 35, 15, _T("Hz"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 60, 140, 15, _T("Output resolution"), ID_CTL_STATIC);
	ctx->hwndEditBps = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 60, 45, 15, PAGEAUD_ID_BPS);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		445, 60, 35, 15, _T("bits"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		245, 90, 235, 15, _T("Output buffers:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 110, 140, 15, _T("Number of buffers"), ID_CTL_STATIC);
	ctx->hwndEditHdrCnt = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 110, 45, 15, PAGEAUD_ID_HDR_CNT);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 130, 140, 15, _T("Length of each buffer"), ID_CTL_STATIC);
	ctx->hwndEditHdrMs = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 130, 45, 15, PAGEAUD_ID_HDR_MS);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		445, 130, 35, 15, _T("ms"), ID_CTL_STATIC);

	/* initialize device selection menu */
	ctx->hmenuDevSel = CreatePopupMenu();
	AppendMenu(ctx->hmenuDevSel, 0, PAGEAUD_ID_WAVEMAPPER, _T("Wave mapper"));
	AppendMenu(ctx->hmenuDevSel, MF_SEPARATOR, 0, NULL);

	ctx->num_waveout_devs = waveOutGetNumDevs();
	if(ctx->num_waveout_devs == 0)
	{
		AppendMenu(ctx->hmenuDevSel, MF_GRAYED,
			PAGEAUD_ID_DEVICE_0, _T("No devices found"));
	}
	else
	{
		uid = PAGEAUD_ID_DEVICE_0;
		for(i = 0; i < ctx->num_waveout_devs; i++)
		{
			waveOutGetDevCaps(i, &woc, sizeof(woc));
			AppendMenu(ctx->hmenuDevSel, 0, uid++, woc.szPname);
		}
	}

	pageaud_initdata(ctx, data);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pageaud_cleanup(pageaud_ctx_t *ctx)
{
	if(ctx != NULL)
	{
		DestroyMenu(ctx->hmenuDevSel);
		free(ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void pageaud_deviceselmenu(pageaud_ctx_t *ctx)
{
	POINT pt;

	GetCursorPos(&pt);
	TrackPopupMenu(ctx->hmenuDevSel, 0, pt.x, pt.y, 0, ctx->hwndPage, NULL);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK pageaud_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	pageaud_ctx_t *ctx;

	switch(umsg)
	{
	case WM_NCDESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		pageaud_cleanup(ctx);
		return 0;

	case WM_COMMAND:

		ctx = GetWndPtr(hwnd, GWLP_USERDATA);

		switch(LOWORD(wp))
		{
		case PAGEAUD_ID_OUT_DEVSEL:
			pageaud_deviceselmenu(ctx);
			return 0;

		case PAGEAUD_ID_WAVEMAPPER:
			Edit_SetText(ctx->hwndEditDevId, _T("-1"));
			return 0;
		}

		if( (LOWORD(wp) >= PAGEAUD_ID_DEVICE_0) &&
			(LOWORD(wp) <= PAGEAUD_ID_DEVICE_MAX) )
		{
			_itot(LOWORD(wp) - PAGEAUD_ID_DEVICE_0, ctx->uidata->databuf, 10);
			Edit_SetText(ctx->hwndEditDevId, ctx->uidata->databuf);
			return 0;
		}

		break;
	}

	return DefWindowProc(hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

int pageaud_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata, pageaud_data_t *data)
{
	pageaud_ctx_t *ctx;

	if( (ctx = calloc(1, sizeof(pageaud_ctx_t))) != NULL )
	{
		ctx->hwndSetWnd = hwndSetWnd;
		ctx->hwndPage = hwndPage;
		ctx->uidata = uidata;

		SetWndPtr(hwndPage, GWLP_WNDPROC, (void*)pageaud_proc);
		SetWndPtr(hwndPage, GWLP_USERDATA, ctx);

		if(pageaud_init(ctx, data))
			return 1;

		SetWndPtr(hwndPage, GWLP_USERDATA, NULL);
		free(ctx);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
