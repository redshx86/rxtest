/* ---------------------------------------------------------------------------------------------- */

#include "pagevis.h"

/* ---------------------------------------------------------------------------------------------- */

static pagevis_wndentry_t window_list[] =
{
	{ _T("Hann"),		NULL,						0,		WND_HANN },
	{ _T("Hamming"),	NULL,						0,		WND_HAMMING },
	{ _T("Blackman"),	NULL,						0,		WND_BLACKMAN },
	{ _T("Kaiser"),		_T("Kaiser window Beta"),	5.63,	WND_KAISER }
};

/* ---------------------------------------------------------------------------------------------- */

void pagevis_data_init(pagevis_data_t *data, rxconfig_t *rxcfg,
					   specview_cfg_t *specviewcfg, watrview_cfg_t *watrviewcfg)
{
	data->spect_ups_req = (int)(rxcfg->spect_ups_req);
	data->spect_length = (int)(rxcfg->spect_length);
	data->spect_bufcount = (int)(rxcfg->spect_bufcount);
	data->spect_wndtype = rxcfg->spect_wndtype;
	data->spect_wndarg = rxcfg->spect_wndarg;
	data->spect_magref = rxcfg->spect_magref;

	data->sv_m_0 = specviewcfg->m_0;
	data->sv_m_1 = specviewcfg->m_1;
	data->sv_scale_mode = specviewcfg->scale_mode;
	data->sv_show_ups_counter = specviewcfg->show_ups_counter;

	data->wv_scale_mode = watrviewcfg->scale_mode;
	data->wv_framediv = watrviewcfg->framediv;
	data->wv_chain_max_len = watrviewcfg->chain_max_len;
	data->wv_seg_len = watrviewcfg->seg_len;
}

/* ---------------------------------------------------------------------------------------------- */

int pagevis_data_apply(rxstate_t *rx, specview_ctx_t *specview,
					   watrview_ctx_t *watrview, pagevis_data_t *data,
					   uievent_t *event_visualcfg,
					   HWND hwndMsgbox, TCHAR *msgbuf, size_t msgbuf_size)
{
	int status = 1;
	int sv_update = 0, wv_update = 0;

	/* apply new spectrum analyzer config */
	if( (data->spect_ups_req != (int)(rx->config.spect_ups_req)) ||
		(data->spect_length != (int)(rx->config.spect_length)) ||
		(data->spect_bufcount != (int)(rx->config.spect_bufcount)) ||
		(data->spect_wndtype != rx->config.spect_wndtype) ||
		(fabs(data->spect_wndarg - rx->config.spect_wndarg) >= 1e-6) ||
		(fabs(data->spect_magref - rx->config.spect_magref) >= 1e-6) )
	{
		if( ! rx_set_spect_params(rx, data->spect_ups_req, data->spect_length,
			data->spect_bufcount, data->spect_wndtype, data->spect_wndarg,
			data->spect_magref, msgbuf, msgbuf_size) )
		{
			MessageBox(hwndMsgbox, msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			status = 0;
		}
	}

	/* apply new spectrum viewer config */
	if( (fabs(data->sv_m_0 - specview->cfg.m_0) >= 1e-6) ||
		(fabs(data->sv_m_1 - specview->cfg.m_1) >= 1e-6) )
	{
		specview->cfg.m_0 = data->sv_m_0;
		specview->cfg.m_1 = data->sv_m_1;

		if(!specview_set_mag_range(specview))
		{
			_sntprintf(msgbuf, msgbuf_size,
				_T("Can't set spectrum viewer magnitude range (%f to %f dB)."),
				data->sv_m_0, data->sv_m_1);
			MessageBox(hwndMsgbox, msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			status = 0;
		}

		sv_update = 1;
	}

	if( (specview->cfg.scale_mode != data->sv_scale_mode) ||
		(specview->cfg.show_ups_counter != data->sv_show_ups_counter) )
	{
		specview->cfg.scale_mode = data->sv_scale_mode;
		specview->cfg.show_ups_counter = data->sv_show_ups_counter;

		sv_update = 1;
	}

	/* apply new waterfall viewer config */
	if( (watrview->cfg.scale_mode != data->wv_scale_mode) ||
		(watrview->cfg.framediv != data->wv_framediv) )
	{
		watrview->cfg.scale_mode = data->wv_scale_mode;
		watrview->cfg.framediv = data->wv_framediv;

		wv_update = 1;
	}

	if( (data->wv_chain_max_len != watrview->cfg.chain_max_len) ||
		(data->wv_seg_len != watrview->cfg.seg_len) )
	{
		if(!watrview_set_len(watrview, data->wv_chain_max_len, data->wv_seg_len))
		{
			_sntprintf(msgbuf, msgbuf_size,
				_T("Can't set waterfall viewer buffer length ")
				_T("(max chain length: %d pixels, segment length: %d pixels)."),
				data->wv_chain_max_len, data->wv_seg_len);
			MessageBox(hwndMsgbox, msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			status = 0;
		}

		wv_update = 1;
	}
	
	/* send notifications */
	if(sv_update) {
		uievent_send(event_visualcfg, EVENT_VISUALCFG_SPECVIEW, NULL);
	}

	if(wv_update) {
		uievent_send(event_visualcfg, EVENT_VISUALCFG_WATRVIEW, NULL);
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

int pagevis_save(pagevis_data_t *data, HWND hwndPage)
{
	pagevis_ctx_t *ctx;

	ctx = GetWndPtr(hwndPage, GWLP_USERDATA);

	/* save spectrum analyzer config */
	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditSpectUpsReq, &(data->spect_ups_req),
		RXLIM_SPECT_UPS_REQ_MIN, RXLIM_SPECT_UPS_REQ_MAX,
		_T("Wanted spectrum analyzer refresh rate"), _T(" Hz")) )
	{
		return 0;
	}

	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditSpectLength, &(data->spect_length),
		RXLIM_SPECT_LENGTH_MIN, RXLIM_SPECT_LENGTH_MAX,
		_T("Spectrum analyzer frame length"), _T(" samples")) )
	{
		return 0;
	}

	if((data->spect_length & (data->spect_length - 1)) != 0)
	{
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Spectrum analyzer frame length should be equal some power of two. Currently %d."),
			data->spect_length);
		MessageBox(ctx->hwndSetWnd, ctx->uidata->msgbuf,
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if(ctx->i_spect_cur_wnd == -1)
	{
		MessageBox(ctx->hwndSetWnd, _T("Spectrum analyzer window type is not selected."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}
	data->spect_wndtype = window_list[ctx->i_spect_cur_wnd].window_id;

	if(window_list[ctx->i_spect_cur_wnd].param_name != NULL)
	{
		if( !ui_get_double(ctx->uidata, ctx->hwndSetWnd,
			ctx->hwndEditSpectWndArgValue, &(data->spect_wndarg),
			window_list[ctx->i_spect_cur_wnd].param_name, 0, NULL) )
		{
			return 0;
		}
	}
	else
	{
		data->spect_wndarg = 0;
	}

	if( !ui_get_double(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditSpectMagRef, &(data->spect_magref),
		_T("Magnitude reference"), 0, NULL) )
	{
		return 0;
	}

	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditSpectBufCount, &(data->spect_bufcount),
		RXLIM_SPECT_BUF_COUNT_MIN, RXLIM_SPECT_BUF_COUNT_MAX,
		_T("Spectrum analyzer buffer count"), _T("")) )
	{
		return 0;
	}

	/* save spectrum viewer config */
	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditSvMag0, &(data->sv_m_0),
		SPECVIEW_MAGN_MINABS, SPECVIEW_MAGN_MAXABS,
		_T("Lower limit of spectrum viewer magnitude range"), _T(" dB"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if( !ui_get_double_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditSvMag1, &(data->sv_m_1),
		SPECVIEW_MAGN_MINABS, SPECVIEW_MAGN_MAXABS,
		_T("Upper limit of spectrum viewer magnitude range"), _T(" dB"), 6, 1, 0, NULL) )
	{
		return 0;
	}

	if(data->sv_m_1 <= data->sv_m_0)
	{
		MessageBox(ctx->hwndSetWnd, _T("Spectrum viewer magnitude range is empty."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if(Button_GetCheck(ctx->hwndBtnSvPeak) & BST_CHECKED)
		data->sv_scale_mode = SPECSCALE_MAP_PEAK;
	if(Button_GetCheck(ctx->hwndBtnSvAvg) & BST_CHECKED)
		data->sv_scale_mode = SPECSCALE_MAP_AVG;

	data->sv_show_ups_counter =
		((Button_GetCheck(ctx->hwndBtnSvUpsCtr) & BST_CHECKED) != 0);

	/* save waterfall viewer config */
	if(Button_GetCheck(ctx->hwndBtnWvPeak) & BST_CHECKED)
		data->wv_scale_mode = SPECSCALE_MAP_PEAK;
	if(Button_GetCheck(ctx->hwndBtnWvAvg) & BST_CHECKED)
		data->wv_scale_mode = SPECSCALE_MAP_AVG;

	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditWvFrameDiv, &(data->wv_framediv),
		WATRVIEW_FRAMEDIV_MIN, WATRVIEW_FRAMEDIV_MAX,
		_T("Waterfall viewer frame skip counter"), _T("")) )
	{
		return 0;
	}

	if( !ui_get_int(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditWvChainMaxLen, &(data->wv_chain_max_len),
		_T("Waterfall viewer scroll buffer length")) )
	{
		return 0;
	}

	if( !ui_get_int_range(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditWvSegLen, &(data->wv_seg_len),
		WATRVIEW_SEGLEN_MIN, WATRVIEW_SEGLEN_MAX,
		_T("Waterfall viewer segment length"), _T(" pixels")) )
	{
		return 0;
	}

	if( (data->wv_chain_max_len != 0) &&
		((data->wv_chain_max_len < WATRVIEW_CHAINMAXLEN_MIN) || (data->wv_chain_max_len > WATRVIEW_CHAINMAXLEN_MAX)) )
	{
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Waterfall viewer scroll buffer length must be 0 (auto) or %d to %d."),
			WATRVIEW_CHAINMAXLEN_MIN, WATRVIEW_CHAINMAXLEN_MAX);
		MessageBox(ctx->hwndSetWnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagevis_set_cur_wndtype(pagevis_ctx_t *ctx, int i_wnd)
{
	int n;
	double param;

	n = sizeof(window_list) / sizeof(window_list[0]);
	if( (i_wnd < 0) || (i_wnd >= n) )
		return 0;

	Edit_SetText(ctx->hwndEditSpectWndType, window_list[i_wnd].name);

	if(window_list[i_wnd].param_name != NULL)
	{
		Static_SetText(ctx->hwndStaticSpectWndArgName, window_list[i_wnd].param_name);

		param = (i_wnd == ctx->i_spect_org_wnd) ? 
			ctx->spect_org_wnd_arg : window_list[i_wnd].param_default;
		ui_set_double(ctx->uidata, ctx->hwndEditSpectWndArgValue, param, 0, 6, 1);

		EnableWindow(ctx->hwndStaticSpectWndArgName, TRUE);
		EnableWindow(ctx->hwndEditSpectWndArgValue, TRUE);
	}
	else
	{
		Static_SetText(ctx->hwndStaticSpectWndArgName, _T("Window parameter"));
		Edit_SetText(ctx->hwndEditSpectWndArgValue, _T("0"));

		EnableWindow(ctx->hwndStaticSpectWndArgName, FALSE);
		EnableWindow(ctx->hwndEditSpectWndArgValue, FALSE);
	}

	ctx->i_spect_cur_wnd = i_wnd;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagevis_initvals(pagevis_ctx_t *ctx, pagevis_data_t *data)
{
	int i, n;

	/* init spectrum analyzer config */
	ui_set_int(ctx->uidata, ctx->hwndEditSpectUpsReq, data->spect_ups_req);
	ui_set_int(ctx->uidata, ctx->hwndEditSpectLength, data->spect_length);

	ctx->i_spect_org_wnd = -1;
	n = sizeof(window_list) / sizeof(window_list[0]);
	for(i = 0; i < n; i++)
	{
		if(data->spect_wndtype == window_list[i].window_id) {
			ctx->i_spect_org_wnd = i;
			ctx->spect_org_wnd_arg = data->spect_wndarg;
			break;
		}
	}

	ctx->i_spect_cur_wnd = -1;
	pagevis_set_cur_wndtype(ctx, ctx->i_spect_org_wnd);

	ui_set_double(ctx->uidata, ctx->hwndEditSpectWndArgValue, data->spect_wndarg, 0, 6, 1);
	ui_set_double(ctx->uidata, ctx->hwndEditSpectMagRef, data->spect_magref, 0, 6, 1);
	ui_set_int(ctx->uidata, ctx->hwndEditSpectBufCount, data->spect_bufcount);

	/* init spectrum viewer config */
	ui_set_double(ctx->uidata, ctx->hwndEditSvMag0, data->sv_m_0, 0, 6, 1);
	ui_set_double(ctx->uidata, ctx->hwndEditSvMag1, data->sv_m_1, 0, 6, 1);

	switch(data->sv_scale_mode)
	{
	case SPECSCALE_MAP_PEAK:
		Button_SetCheck(ctx->hwndBtnSvPeak, BST_CHECKED);
		break;
	case SPECSCALE_MAP_AVG:
		Button_SetCheck(ctx->hwndBtnSvAvg, BST_CHECKED);
		break;
	}

	if(data->sv_show_ups_counter) {
		Button_SetCheck(ctx->hwndBtnSvUpsCtr, BST_CHECKED);
	}

	/* init waterfall viewer config */
	switch(data->wv_scale_mode)
	{
	case SPECSCALE_MAP_PEAK:
		Button_SetCheck(ctx->hwndBtnWvPeak, BST_CHECKED);
		break;
	case SPECSCALE_MAP_AVG:
		Button_SetCheck(ctx->hwndBtnWvAvg, BST_CHECKED);
		break;
	}

	ui_set_int(ctx->uidata, ctx->hwndEditWvFrameDiv, data->wv_framediv);
	ui_set_int(ctx->uidata, ctx->hwndEditWvChainMaxLen, data->wv_chain_max_len);
	ui_set_int(ctx->uidata, ctx->hwndEditWvSegLen, data->wv_seg_len);
}

/* ---------------------------------------------------------------------------------------------- */

static int pagevis_init(pagevis_ctx_t *ctx, pagevis_data_t *data)
{
	int n, i;
	UINT ucommand;

	/* spectrum analyzer */
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		0, 0, 235, 15, _T("Spectrum analyzer parameters:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0, 
		10, 20, 140, 15, _T("Wanted refresh rate"), ID_CTL_STATIC);
	ctx->hwndEditSpectUpsReq = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 20, 45, 15, PAGEVIS_ID_SPECT_UPS_REQ);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		200, 20, 35, 15, _T("Hz"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 40, 120, 15, _T("Frame (two's pwr)"), ID_CTL_STATIC);
	ctx->hwndEditSpectLength = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		130, 40, 65, 15, PAGEVIS_ID_SPECT_LENGTH);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		200, 40, 35, 15, _T("samp"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 60, 120, 15, _T("Window type"), ID_CTL_STATIC);
	ctx->hwndEditSpectWndType = ui_crt_editse(ctx->uidata, ctx->hwndPage, ES_READONLY|WS_TABSTOP,
		130, 60, 65, 15, PAGEVIS_ID_SPECT_WNDTYPE);
	ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		200, 60, 15, 15, _T("»"), PAGEVIS_ID_SPECT_WNDSELECT);

	ctx->hwndStaticSpectWndArgName = ui_crt_static(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		10, 80, 140, 15, _T("Window parameter"), PAGEVIS_ID_SPECT_WNDARG_NAME);
	ctx->hwndEditSpectWndArgValue = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 80, 45, 15, PAGEVIS_ID_SPECT_WNDARG_VALUE);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 100, 140, 15, _T("Magnitude ref (0 dB)"), ID_CTL_STATIC);
	ctx->hwndEditSpectMagRef = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 100, 45, 15, PAGEVIS_ID_SPECT_MAGREF);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 120, 140, 15, _T("Buffer count"), ID_CTL_STATIC);
	ctx->hwndEditSpectBufCount = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 120, 45, 15, PAGEVIS_ID_SPECT_BUFCOUNT);

	ctx->hmenuSpectWndType = CreatePopupMenu();
	n = sizeof(window_list) / sizeof(window_list[0]);
	ucommand = PAGEVIS_ID_SPECT_WND_0;
	for(i = 0; i < n; i++) {
		AppendMenu(ctx->hmenuSpectWndType, 0,
			ucommand++, window_list[i].name);
	}

	/* spectrum viewer */
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		245, 0, 235, 15, _T("Spectrum viewer configuration:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 20, 100, 15, _T("Magn range"), ID_CTL_STATIC);
	ctx->hwndEditSvMag0 = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		355, 20, 40, 15, PAGEVIS_ID_SV_MAG0);
	ui_crt_static(ctx->uidata, ctx->hwndPage, SS_CENTER,
		395, 20, 15, 15, _T("–"), ID_CTL_STATIC);
	ctx->hwndEditSvMag1 = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		410, 20, 40, 15, PAGEVIS_ID_SV_MAG1);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		455, 20, 25, 15, _T("dB"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 40, 100, 15, _T("Horz scaling"), ID_CTL_STATIC);
	ctx->hwndBtnSvPeak = ui_crt_btn(ctx->uidata, ctx->hwndPage,
		BS_AUTORADIOBUTTON|WS_TABSTOP|WS_GROUP,
		355, 40, 60, 15, _T("Peak"), PAGEVIS_ID_SV_PEAK);
	ctx->hwndBtnSvAvg = ui_crt_btn(ctx->uidata, ctx->hwndPage,
		BS_AUTORADIOBUTTON|WS_TABSTOP,
		415, 40, 60, 15, _T("Avg"), PAGEVIS_ID_SV_AVG);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 60, 140, 15, _T("Update rate counter"), ID_CTL_STATIC);
	ctx->hwndBtnSvUpsCtr = ui_crt_btn(ctx->uidata, ctx->hwndPage, BS_AUTOCHECKBOX|WS_TABSTOP,
		395, 60, 80, 15, _T("enable"), PAGEVIS_ID_SV_UPSCTR);

	/* waterfall viewer */
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		245, 90, 235, 15, _T("Waterfall viewer configuration:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 110, 100, 15, _T("Horz scaling"), ID_CTL_STATIC);
	ctx->hwndBtnWvPeak = ui_crt_btn(ctx->uidata, ctx->hwndPage, BS_AUTORADIOBUTTON|WS_TABSTOP|WS_GROUP,
		355, 110, 60, 15, _T("Peak"), PAGEVIS_ID_WV_PEAK);
	ctx->hwndBtnWvAvg = ui_crt_btn(ctx->uidata, ctx->hwndPage, BS_AUTORADIOBUTTON|WS_TABSTOP,
		415, 110, 60, 15, _T("Avg"), PAGEVIS_ID_WV_AVG);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 130, 140, 15, _T("Frame skip counter"), ID_CTL_STATIC);
	ctx->hwndEditWvFrameDiv = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 130, 45, 15, PAGEVIS_ID_WV_FRAMEDIV);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 150, 140, 15, _T("Scroll len (0=auto)"), ID_CTL_STATIC);
	ctx->hwndEditWvChainMaxLen = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 150, 45, 15, PAGEVIS_ID_WV_CHAINMAXLEN);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		445, 150, 35, 15, _T("px"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		255, 170, 140, 15, _T("Segment length"), ID_CTL_STATIC);
	ctx->hwndEditWvSegLen = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 170, 45, 15, PAGEVIS_ID_WV_SEGLEN);
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		445, 170, 35, 15, _T("px"), ID_CTL_STATIC);


	pagevis_initvals(ctx, data);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagevis_cleanup(pagevis_ctx_t *ctx)
{
	if(ctx != NULL)
	{
		DestroyMenu(ctx->hmenuSpectWndType);
		free(ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void pagevis_showwndtypemenu(pagevis_ctx_t *ctx)
{
	POINT pt;

	GetCursorPos(&pt);
	TrackPopupMenu(ctx->hmenuSpectWndType, 0, pt.x, pt.y, 0, ctx->hwndPage, NULL);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK pagevis_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	pagevis_ctx_t *ctx;

	switch(umsg)
	{
	case WM_NCDESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		pagevis_cleanup(ctx);
		return 0;

	case WM_COMMAND:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(LOWORD(wp))
		{
		case PAGEVIS_ID_SPECT_WNDSELECT:
			pagevis_showwndtypemenu(ctx);
			return 0;
		}
		if( (LOWORD(wp) >= PAGEVIS_ID_SPECT_WND_0) &&
			(LOWORD(wp) <= PAGEVIS_ID_SPECT_WND_MAX) )
		{
			pagevis_set_cur_wndtype(ctx, 
				LOWORD(wp) - PAGEVIS_ID_SPECT_WND_0);
			return 0;
		}
		break;
	}

	return DefWindowProc(hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

int pagevis_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata, pagevis_data_t *data)
{
	pagevis_ctx_t *ctx;

	if( (ctx = calloc(1, sizeof(pagevis_ctx_t))) != NULL )
	{
		ctx->hwndSetWnd = hwndSetWnd;
		ctx->hwndPage = hwndPage;
		ctx->uidata = uidata;

		SetWndPtr(hwndPage, GWLP_WNDPROC, (void*)pagevis_proc);
		SetWndPtr(hwndPage, GWLP_USERDATA, ctx);

		if(pagevis_init(ctx, data))
			return 1;

		SetWndPtr(hwndPage, GWLP_USERDATA, NULL);
		free(ctx);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
