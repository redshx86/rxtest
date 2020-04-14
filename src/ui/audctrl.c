/* ---------------------------------------------------------------------------------------------- */

#include "audctrl.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *audctrl_classname = _T("rxtest_audio_ctrl");
static TCHAR *audctrl_wndname = _T("Audio output");

static ATOM audctrl_classatom;

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_draw_clip_ind(audctrl_ctx_t *ctx, HDC hdc, RECT *rc, int state, TCHAR *label)
{
	HBRUSH hbr_prev;
	HPEN hpen_prev;
	HFONT hfont_prev;

	hpen_prev = SelectObject(hdc, ctx->clip_ind.hpen_border);
	hfont_prev = SelectObject(hdc, ctx->uidata->hfnt_main);

	switch(state)
	{
	default:
		hbr_prev = SelectObject(hdc, ctx->clip_ind.hbr_none);
		SetBkColor(hdc, ctx->clip_ind.cr_none);
		break;
	case 1:
		hbr_prev = SelectObject(hdc, ctx->clip_ind.hbr_soft);
		SetBkColor(hdc, ctx->clip_ind.cr_soft);
		break;
	case 2:
		hbr_prev = SelectObject(hdc, ctx->clip_ind.hbr_hard);
		SetBkColor(hdc, ctx->clip_ind.cr_hard);
		break;
	}

	SetTextColor(hdc, ctx->clip_ind.cr_text);

	Rectangle(hdc, rc->left, rc->top, rc->right, rc->bottom);
	DrawText(hdc, label, (int)_tcslen(label), rc, DT_SINGLELINE|DT_VCENTER|DT_CENTER);

	SelectObject(hdc, hbr_prev);
	SelectObject(hdc, hpen_prev);
	SelectObject(hdc, hfont_prev);
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_update_clip_ind(audctrl_ctx_t *ctx, int clip_state_left, int clip_state_right)
{
	HDC hdc;

	if( (clip_state_left != ctx->clip_ind.state_left) ||
		(clip_state_right != ctx->clip_ind.state_right) )
	{
		hdc = GetDC(ctx->hwnd);

		if(clip_state_left != ctx->clip_ind.state_left)
		{
			audctrl_draw_clip_ind(ctx, hdc, &(ctx->clip_ind.rc_left), clip_state_left, _T("L"));
			ctx->clip_ind.state_left = clip_state_left;
		}

		if(clip_state_right != ctx->clip_ind.state_right)
		{
			audctrl_draw_clip_ind(ctx, hdc, &(ctx->clip_ind.rc_right), clip_state_right, _T("R"));
			ctx->clip_ind.state_right = clip_state_right;
		}

		ReleaseDC(ctx->hwnd, hdc);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_fmt_out_gain_label(TCHAR *buf, double g)
{
	_stprintf(buf, _T("%+.1f dB"), g);
	if(buf[0] == _T('-'))
		buf[0] = _T('–');
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_create_gain_trackbar(audctrl_ctx_t *ctx)
{
	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 10, 160, 15, _T("Audio output gain:"), ID_CTL_STATIC);

	ctx->hwndGainTrackBar = ui_crt_track(ctx->uidata, ctx->hwnd, TBS_HORZ|TBS_AUTOTICKS,
		5, 25, 230, 40, AUDCTRL_ID_GAINTRACKBAR);

	ctx->track_range = (int)((AUDCTRL_OUT_GAIN_MAX - AUDCTRL_OUT_GAIN_MIN) * 
		AUDCTRL_OUT_GAIN_STEPS_PER_DB + 0.5);

	ctx->track_pos = (int)((ctx->rx->config.output_gain - AUDCTRL_OUT_GAIN_MIN) * 
		AUDCTRL_OUT_GAIN_STEPS_PER_DB + 0.5);
	if(ctx->track_pos < 0)
		ctx->track_pos = 0;
	if(ctx->track_pos > ctx->track_range)
		ctx->track_pos = ctx->track_range;

	SendMessage(ctx->hwndGainTrackBar, TBM_SETTICFREQ, 10, 0);
	SendMessage(ctx->hwndGainTrackBar, TBM_SETRANGE, FALSE, MAKELONG(0, ctx->track_range));
	SendMessage(ctx->hwndGainTrackBar, TBM_SETPOS, TRUE, ctx->track_pos);

	audctrl_fmt_out_gain_label(ctx->uidata->databuf, ctx->rx->config.output_gain);
	ctx->hwndGainDisplay = ui_crt_static(ctx->uidata, ctx->hwnd, SS_RIGHT,
		170, 10, 60, 15, ctx->uidata->databuf, AUDCTRL_ID_GAINDISPLAY);
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_update_out_gain(audctrl_ctx_t *ctx)
{
	int pos;

	pos = (int)((ctx->rx->config.output_gain - AUDCTRL_OUT_GAIN_MIN) * 
		AUDCTRL_OUT_GAIN_STEPS_PER_DB + 0.5);
	if(pos < 0)
		pos = 0;
	if(pos > ctx->track_range)
		pos = ctx->track_range;

	if(pos != ctx->track_pos)
	{
		SendMessage(ctx->hwndGainTrackBar, TBM_SETPOS, TRUE, pos);
		ctx->track_pos = pos;
	}

	audctrl_fmt_out_gain_label(ctx->uidata->databuf, ctx->rx->config.output_gain);
	Static_SetText(ctx->hwndGainDisplay, ctx->uidata->databuf);
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_update_out_level(audctrl_ctx_t *ctx)
{
	unsigned int tick;
	double level[2], peak[2];
	int clip_state_left, clip_state_right;

	if(ctx->level_timer_started && ctx->rx->is_started)
	{
		tick = GetTickCount();

		if(rx_audio_output_read_level(&(ctx->rx->audioout),
			level, peak, ctx->level_prev_tick, tick))
		{
			lbr_send_setpos(ctx->hwndLevelLeft,
				LINM2DB(level[0] + DBL_MIN), LINM2DB(peak[0] + DBL_MIN), 1);
			lbr_send_setpos(ctx->hwndLevelRight,
				LINM2DB(level[1] + DBL_MIN), LINM2DB(peak[1] + DBL_MIN), 1);

			clip_state_left = ctx->clip_ind.state_left;
			if((peak[0] >= ctx->rx->config.output_lim_thres) && (clip_state_left == 0))
				clip_state_left = 1;
			if(peak[0] >= ctx->rx->config.output_lim_range)
				clip_state_left = 2;

			clip_state_right = ctx->clip_ind.state_right;
			if((peak[1] >= ctx->rx->config.output_lim_thres) && (clip_state_right == 0))
				clip_state_right = 1;
			if(peak[1] >= ctx->rx->config.output_lim_range)
				clip_state_right = 2;

			audctrl_update_clip_ind(ctx, clip_state_left, clip_state_right);
		}

		ctx->level_prev_tick = tick;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_update_level_start(audctrl_ctx_t *ctx)
{
	if((!ctx->level_timer_started) && (ctx->rx->is_started))
	{
		SetTimer(ctx->hwnd, AUDCTRL_IDT_UPDATE_LEVEL, AUDCTRL_LEVEL_UPDATE_INT, NULL);
		
		ctx->level_timer_started = 1;
		ctx->level_prev_tick = GetTickCount();
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_update_level_stop(audctrl_ctx_t *ctx)
{
	if(ctx->level_timer_started)
	{
		KillTimer(ctx->hwnd, AUDCTRL_IDT_UPDATE_LEVEL);

		lbr_send_setpos(ctx->hwndLevelLeft,
			AUDCTRL_OUT_LEVEL_MIN, AUDCTRL_OUT_LEVEL_MIN, 1);

		lbr_send_setpos(ctx->hwndLevelRight,
			AUDCTRL_OUT_LEVEL_MIN, AUDCTRL_OUT_LEVEL_MIN, 1);

		ctx->level_timer_started = 0;
		ctx->level_prev_tick = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_set_out_gain(audctrl_ctx_t *ctx)
{
	int pos;
	double gain;

	pos = (int)SendMessage(ctx->hwndGainTrackBar, TBM_GETPOS, 0, 0);

	if(pos != ctx->track_pos)
	{
		ctx->track_pos = pos;

		gain = (double)(ctx->track_pos) * 
			(1.0 / AUDCTRL_OUT_GAIN_STEPS_PER_DB) + AUDCTRL_OUT_GAIN_MIN;

		if(!rx_set_output_gain(ctx->rx, gain, ctx->uidata->msgbuf, ctx->uidata->msgbuf_size))
		{
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		uievent_send(ctx->event_rx_state, EVENT_RX_STATE_SET_OUT_GAIN, NULL);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_rx_state_handler(audctrl_ctx_t *ctx, unsigned int msg, void *data)
{
	switch(msg)
	{
	case EVENT_RX_STATE_START:
		audctrl_update_level_start(ctx);
		break;
	case EVENT_RX_STATE_STOP:
		audctrl_update_level_stop(ctx);
		break;
	case EVENT_RX_STATE_SET_OUT_GAIN:
		audctrl_update_out_gain(ctx);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_paint(audctrl_ctx_t *ctx)
{
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint(ctx->hwnd, &ps);
	audctrl_draw_clip_ind(ctx, hdc, &(ctx->clip_ind.rc_left), ctx->clip_ind.state_left, _T("L"));
	audctrl_draw_clip_ind(ctx, hdc, &(ctx->clip_ind.rc_right), ctx->clip_ind.state_right, _T("R"));
	EndPaint(ctx->hwnd, &ps);
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_left_click(audctrl_ctx_t *ctx, int x, int y)
{
	POINT pt;

	pt.x = x;
	pt.y = y;

	if(PtInRect(&(ctx->clip_ind.rc_left), pt)) {
		audctrl_update_clip_ind(ctx, 0, ctx->clip_ind.state_right);
	}

	if(PtInRect(&(ctx->clip_ind.rc_right), pt)) {
		audctrl_update_clip_ind(ctx, ctx->clip_ind.state_left, 0);
	}

}

/* ---------------------------------------------------------------------------------------------- */

static int audctrl_init(audctrl_ctx_t *ctx)
{
	RECT rc;

	audctrl_create_gain_trackbar(ctx);

	uievent_handler_add(&(ctx->uidata->event_list),
		EVENT_NAME_RX_STATE, ctx, audctrl_rx_state_handler);

	ui_crt_static(ctx->uidata, ctx->hwnd, 0,
		10, 70, 160, 15, _T("Output level meter:"), ID_CTL_STATIC);

	rc.left = 10;
	rc.right = 210;

	ctx->clip_ind.rc_left.left = ctx->clip_ind.rc_right.left = 214;
	ctx->clip_ind.rc_left.right = ctx->clip_ind.rc_right.right = 230;

	ctx->clip_ind.rc_left.top = rc.top = 90;
	ctx->clip_ind.rc_left.bottom = rc.bottom = rc.top + 16;

	ctx->hwndLevelLeft = lbr_create(ctx->uidata->h_inst, WS_BORDER,
		ctx->hwnd, AUDCTRL_ID_LEVEL_LEFT, &rc, ctx->uidata->hfnt_mini, 1);
	lbr_send_setrange(ctx->hwndLevelLeft,
		AUDCTRL_OUT_LEVEL_MIN, AUDCTRL_OUT_LEVEL_MAX, 0);
	lbr_send_setpos(ctx->hwndLevelLeft,
		AUDCTRL_OUT_LEVEL_MIN, AUDCTRL_OUT_LEVEL_MIN, 0);

	ctx->clip_ind.rc_right.top = rc.top = 110;
	ctx->clip_ind.rc_right.bottom = rc.bottom = rc.top + 16;
	
	ctx->hwndLevelRight = lbr_create(ctx->uidata->h_inst, WS_BORDER,
		ctx->hwnd, AUDCTRL_ID_LEVEL_RIGHT, &rc, ctx->uidata->hfnt_mini, 1);
	lbr_send_setrange(ctx->hwndLevelRight,
		AUDCTRL_OUT_LEVEL_MIN, AUDCTRL_OUT_LEVEL_MAX, 0);
	lbr_send_setpos(ctx->hwndLevelRight,
		AUDCTRL_OUT_LEVEL_MIN, AUDCTRL_OUT_LEVEL_MIN, 0);

	ctx->clip_ind.cr_text = RGB(0, 0, 0);

	ctx->clip_ind.cr_border = RGB(0, 0, 0);
	ctx->clip_ind.cr_none = GetSysColor(COLOR_BTNFACE);
	ctx->clip_ind.cr_soft = RGB(255, 255, 0);
	ctx->clip_ind.cr_hard = RGB(255, 0, 0);

	ctx->clip_ind.hpen_border = GetStockObject(ctx->clip_ind.cr_border);
	ctx->clip_ind.hbr_none = CreateSolidBrush(ctx->clip_ind.cr_none);
	ctx->clip_ind.hbr_soft = CreateSolidBrush(ctx->clip_ind.cr_soft);
	ctx->clip_ind.hbr_hard = CreateSolidBrush(ctx->clip_ind.cr_hard);

	audctrl_update_level_start(ctx);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_destroy(audctrl_ctx_t *ctx)
{
	audctrl_update_level_stop(ctx);

	DeleteObject(ctx->clip_ind.hpen_border);
	DeleteObject(ctx->clip_ind.hbr_none);
	DeleteObject(ctx->clip_ind.hbr_soft);
	DeleteObject(ctx->clip_ind.hbr_hard);

	uievent_send(ctx->event_window_close, EVENT_WNDCLOSE_AUDCTRL, ctx->hwnd);
	uievent_handler_remove(&(ctx->uidata->event_list),
		EVENT_NAME_RX_STATE, ctx, audctrl_rx_state_handler);

	ui_savepos(ctx->hwnd, ini_sect_get(ctx->ini, _T("audctrl"), 1), 
		UI_POS_OFFSET, ctx->cx_frame, ctx->cy_frame);
}

/* ---------------------------------------------------------------------------------------------- */

static void audctrl_cleanup(audctrl_ctx_t *ctx)
{
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK audctrl_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	audctrl_ctx_t *ctx;

	switch(umsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lp)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!audctrl_init(ctx))
			return -1;
		return 0;

	case WM_NCDESTROY:
		ctx = SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		audctrl_cleanup(ctx);
		return 0;

	case WM_DESTROY:
		audctrl_destroy(GetWndPtr(hwnd, GWLP_USERDATA));
		return 0;

	case WM_PAINT:
		audctrl_paint(GetWndPtr(hwnd, GWLP_USERDATA));
		return 0;

	case WM_LBUTTONDOWN:
		audctrl_left_click(GetWndPtr(hwnd, GWLP_USERDATA),
			GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
		return 0;

	case WM_VSCROLL:
	case WM_HSCROLL:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		if((HWND)lp == ctx->hwndGainTrackBar)
		{
			audctrl_set_out_gain(ctx);
			return 0;
		}
		break;

	case WM_TIMER:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(wp)
		{
		case AUDCTRL_IDT_UPDATE_LEVEL:
			audctrl_update_out_level(ctx);
			return 0;
		}
		break;
	}

	return DefWindowProc(hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

HWND audctrl_create(uicommon_t *uidata, ini_data_t *ini, HWND hwnd_parent, rxstate_t *rx)
{
	audctrl_ctx_t *ctx;
	int wx, wy, wcx, wcy;
	int sizestate;
	HWND hwnd;

	if( (ctx = calloc(1, sizeof(audctrl_ctx_t))) != NULL )
	{
		ctx->uidata = uidata;
		ctx->ini = ini;
		ctx->rx = rx;

		ctx->event_rx_state = uievent_register(&(uidata->event_list), EVENT_NAME_RX_STATE);
		ctx->event_window_close = uievent_register(&(uidata->event_list), EVENT_NAME_WNDCLOSE);

		ui_frame_size(&(ctx->cx_frame), &(ctx->cy_frame), UI_FRAME_FIXED|UI_FRAME_CAPTION);

		ui_calcpos(ini_sect_get(ini, _T("audctrl"), 0), UI_POS_OFFSET,
			ctx->cx_frame, ctx->cy_frame, AUDCTRL_CX, AUDCTRL_CY, AUDCTRL_CX, AUDCTRL_CY,
			&wx, &wy, &wcx, &wcy, &sizestate, SW_SHOWNORMAL);

		hwnd = CreateWindow(MAKEINTATOM(audctrl_classatom), audctrl_wndname,
			WS_DLGFRAME|WS_CAPTION|WS_SYSMENU, wx, wy, wcx, wcy,
			hwnd_parent, NULL, uidata->h_inst, ctx);

		if(hwnd != NULL)
		{
			ShowWindow(hwnd, sizestate);
			return hwnd;
		}
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

int audctrl_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcx;

	if(audctrl_classatom == 0)
	{
		memset(&wcx, 0, sizeof(wcx));
		wcx.cbSize = sizeof(wcx);
		wcx.lpfnWndProc = audctrl_proc;
		wcx.hInstance = uidata->h_inst;
		wcx.hIcon = uidata->hicn_main;
		wcx.hCursor = uidata->hcur_main;
		wcx.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcx.lpszClassName = audctrl_classname;
		wcx.hIconSm = uidata->hicn_sm;

		audctrl_classatom = RegisterClassEx(&wcx);
	}

	return (audctrl_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

void audctrl_unregisterclass(HINSTANCE h_inst)
{
	if(audctrl_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(audctrl_classatom), h_inst);
		audctrl_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
