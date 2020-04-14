/* ---------------------------------------------------------------------------------------------- */

#include "rxwnd.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *rxwnd_classname = _T("rxtest_main");

static ATOM rxwnd_classatom;

/* ---------------------------------------------------------------------------------------------- */

/* hotkey table */
static ACCEL rxwnd_accel_table[] =
{
	{ FVIRTKEY,				VK_F2,		RXWND_CMD_FREQWND		},
	{ FVIRTKEY,				VK_F3,		RXWND_CMD_CHANNELS		},
	{ FVIRTKEY,				VK_F4,		RXWND_CMD_AUDCTRL		},
	{ FVIRTKEY,				VK_F9,		RXWND_CMD_SETTINGS		},
	{ FVIRTKEY,				VK_F10,		RXWND_CMD_EXIT			},

	{ FVIRTKEY,				VK_F8,		RXWND_CMD_INPUT_CONFIG	},

	{ FVIRTKEY,				VK_F5,		RXWND_CMD_START			},
	{ FVIRTKEY|FCONTROL,	VK_F5,		RXWND_CMD_RESTART		}
};

/* ---------------------------------------------------------------------------------------------- */
/* Cursor setting subroutines */

/* update cursor when mouse moves or cursor changed */
static void rxwnd_updatecursor(rxwnd_ctx_t *ctx)
{
	HCURSOR hcur_use;

	if(ctx->dragstate.hcur_drag != NULL) {
		hcur_use = ctx->dragstate.hcur_drag;
	} else if(ctx->hcur_set != NULL) {
		hcur_use = ctx->hcur_set;
	} else if(ctx->dragstate.hcur_point != NULL) {
		hcur_use = ctx->dragstate.hcur_point;
	} else {
		hcur_use = ctx->uidata->hcur_main;
	}

	SetCursor(hcur_use);
}

/* ---------------------------------------------------------------------------------------------- */

/* set custom main cursor (wait, etc.) */
static void rxwnd_setcursor(rxwnd_ctx_t *ctx, HCURSOR hcur)
{
	ctx->hcur_set = hcur;
	rxwnd_updatecursor(ctx);
}

/* ---------------------------------------------------------------------------------------------- */
/* Other windows opening/closing */

/* open/close frequency window */
static void rxwnd_cmd_showfreqwnd(rxwnd_ctx_t *ctx)
{
	if( (ctx->hwndFreqWnd == NULL) ||
		(!IsWindow(ctx->hwndFreqWnd)) )
	{
		if( (ctx->hwndFreqWnd = freqwnd_create(
			ctx->uidata, ctx->ini, &(ctx->fcfg), ctx->rx, ctx->specview, ctx->watrview,
			ctx->hwnd)) == NULL )
		{
			MessageBox(ctx->hwnd, _T("Can't create frequency window."),
				ui_title, MB_ICONEXCLAMATION|MB_OK);
		}
		else
		{
			ui_menuitemf(ctx->hmenu_wnd, RXWND_CMD_FREQWND, 1, 1);
		}
	}
	else
	{
		DestroyWindow(ctx->hwndFreqWnd);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* frequency window destroying notification */
static void rxwnd_handle_freqwnd_wndclose(rxwnd_ctx_t *ctx, HWND hwndFreqWnd)
{
	if(hwndFreqWnd == ctx->hwndFreqWnd)
	{
		ui_menuitemf(ctx->hmenu_wnd, RXWND_CMD_FREQWND, 0, 1);
		ctx->hwndFreqWnd = NULL;
		SetForegroundWindow(ctx->hwnd);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* open/close channel manager */
static void rxwnd_cmd_showchannelmgr(rxwnd_ctx_t *ctx)
{
	if( (ctx->hwndChannelMgr == NULL) ||
		(!IsWindow(ctx->hwndChannelMgr)) )
	{
		if( (ctx->hwndChannelMgr = cmwnd_create(ctx->uidata, ctx->ini, ctx->inich,
			ctx->rx, ctx->hwnd)) == NULL )
		{
			MessageBox(ctx->hwnd, _T("Can't create channel manager window."),
				ui_title, MB_ICONEXCLAMATION|MB_OK);
		}
		else
		{
			ui_menuitemf(ctx->hmenu_wnd, RXWND_CMD_CHANNELS, 1, 1);
		}
	}
	else
	{
		DestroyWindow(ctx->hwndChannelMgr);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* channel manager destroying notification */
static void rxwnd_handle_channelmgr_wndclose(rxwnd_ctx_t *ctx, HWND hwndChannelMgr)
{
	if(hwndChannelMgr == ctx->hwndChannelMgr)
	{
		ui_menuitemf(ctx->hmenu_wnd, RXWND_CMD_CHANNELS, 0, 1);
		ctx->hwndChannelMgr = NULL;
		SetForegroundWindow(ctx->hwnd);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* show/hide settings window */
static void rxwnd_cmd_showsettingswnd(rxwnd_ctx_t *ctx)
{
	if( (ctx->hwndSettings == NULL) ||
		(!IsWindow(ctx->hwndSettings)) )
	{
		if( (ctx->hwndSettings = setwnd_create(
			ctx->uidata, ctx->hwnd, ctx->ini,
			ctx->rx, ctx->specview, ctx->watrview)) == NULL )
		{
			MessageBox(ctx->hwnd, _T("Can't create settings window."),
				ui_title, MB_ICONEXCLAMATION|MB_OK);
		}
		else
		{
			ui_menuitemf(ctx->hmenu_wnd, RXWND_CMD_SETTINGS, 1, 1);
		}
	}
	else
	{
		DestroyWindow(ctx->hwndSettings);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* settings window destroying notification */
static void rxwnd_handle_settings_wndclose(rxwnd_ctx_t *ctx, HWND hwndSettings)
{
	if(hwndSettings == ctx->hwndSettings)
	{
		ui_menuitemf(ctx->hmenu_wnd, RXWND_CMD_SETTINGS, 0, 1);
		ctx->hwndSettings = NULL;
		SetForegroundWindow(ctx->hwnd);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* show/hide audio output control window */
static void rxwnd_cmd_showaudctrlwnd(rxwnd_ctx_t *ctx)
{
	if( (ctx->hwndAudCtrl == NULL) ||
		(!IsWindow(ctx->hwndAudCtrl)) )
	{
		if( (ctx->hwndAudCtrl = audctrl_create(
			ctx->uidata, ctx->ini, ctx->hwnd, ctx->rx)) == NULL )
		{
			MessageBox(ctx->hwnd, _T("Can't create audio output control window."),
				ui_title, MB_ICONEXCLAMATION|MB_OK);
		}
		else
		{
			ui_menuitemf(ctx->hmenu_wnd, RXWND_CMD_AUDCTRL, 1, 1);
		}
	}
	else
	{
		DestroyWindow(ctx->hwndAudCtrl);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* audio output control window destroying notification */
static void rxwnd_handle_audctrl_wndclose(rxwnd_ctx_t *ctx, HWND hwndAudCtrl)
{
	if(hwndAudCtrl == ctx->hwndAudCtrl)
	{
		ui_menuitemf(ctx->hmenu_wnd, RXWND_CMD_AUDCTRL, 0, 1);
		ctx->hwndAudCtrl = NULL;
		SetForegroundWindow(ctx->hwnd);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* other window closing notification */
static void rxwnd_window_close_handler(rxwnd_ctx_t *ctx, unsigned int msg, HWND hwndClosed)
{
	switch(msg)
	{
	case EVENT_WNDCLOSE_FREQWND:
		rxwnd_handle_freqwnd_wndclose(ctx, hwndClosed);
		break;
	case EVENT_WNDCLOSE_CHANNELMGR:
		rxwnd_handle_channelmgr_wndclose(ctx, hwndClosed);
		break;
	case EVENT_WNDCLOSE_SETTINGS:
		rxwnd_handle_settings_wndclose(ctx, hwndClosed);
		break;
	case EVENT_WNDCLOSE_AUDCTRL:
		rxwnd_handle_audctrl_wndclose(ctx, hwndClosed);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */
/* Input module selection */

/* create input selection submenu */
static void rxwnd_createsrcmenu(rxwnd_ctx_t *ctx)
{
	UINT ucmd;
	input_module_desc_t *desc;

	ctx->hmenu_source = CreatePopupMenu();

	AppendMenu(ctx->hmenu_source, 0, RXWND_CMD_INPUT_UNSET, _T("&Not selected"));
	ui_menuitemf(ctx->hmenu_source, RXWND_CMD_INPUT_UNSET, 1, 1);

	ucmd = RXWND_CMD_INPUT_0;
	for(desc = ctx->rx->input_mod_list.module_first; desc != NULL; desc = desc->module_next) {
		AppendMenu(ctx->hmenu_source, 0, ucmd, desc->display_name);
		ucmd++;
	}

	AppendMenu(ctx->hmenu_source, MF_SEPARATOR, 0, NULL);
	AppendMenu(ctx->hmenu_source, 0, RXWND_CMD_INPUT_CONFIG, _T("Input con&fig\tF8"));
	ui_menuitemf(ctx->hmenu_source, RXWND_CMD_INPUT_CONFIG, 0, 0);

	InsertMenu(ctx->hmenu_main, 1, MF_BYPOSITION|MF_POPUP,
		(UINT_PTR)(ctx->hmenu_source), _T("&Source"));
}

/* ---------------------------------------------------------------------------------------------- */

/* unselect input */
static void rxwnd_unsetinput(rxwnd_ctx_t *ctx)
{
	unsigned int cmd;

	if( (!ctx->rx->is_started) && (ctx->rx->input_mod_ctx != NULL) )
	{
		cmd = RXWND_CMD_INPUT_0 + ctx->rx->input_mod_desc->module_id;
		ui_menuitemf(ctx->hmenu_source, cmd, 0, 1);
		ui_menuitemf(ctx->hmenu_source, RXWND_CMD_INPUT_UNSET, 1, 1);
		ui_menuitemf(ctx->hmenu_source, RXWND_CMD_INPUT_CONFIG, 0, 0);

		rxwnd_setcursor(ctx, ctx->uidata->hcur_wait);
		rx_input_mod_unset(ctx->rx, ctx->ini);
		rxwnd_setcursor(ctx, NULL);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* select ith input */
static void rxwnd_setinput(rxwnd_ctx_t *ctx, unsigned int i)
{
	input_module_desc_t *desc;
	unsigned int cmd;
	int res;

	if(!ctx->rx->is_started)
	{
		desc = input_module_get(&(ctx->rx->input_mod_list), i);

		if( (desc != NULL) && (desc != ctx->rx->input_mod_desc) )
		{

			rxwnd_unsetinput(ctx);

			rxwnd_setcursor(ctx, ctx->uidata->hcur_wait);
			res = rx_input_mod_set(ctx->rx, desc, ctx->ini,
				ctx->uidata->msgbuf, ctx->uidata->msgbuf_size);
			rxwnd_setcursor(ctx, NULL);

			if(!res)
			{
				MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			}
			else
			{
				cmd = RXWND_CMD_INPUT_0 + desc->module_id;
				ui_menuitemf(ctx->hmenu_source, cmd, 1, 1);
				ui_menuitemf(ctx->hmenu_source, RXWND_CMD_INPUT_UNSET, 0, 1);
				ui_menuitemf(ctx->hmenu_source, RXWND_CMD_INPUT_CONFIG, 0,
					(ctx->rx->input_mod_desc->fn_config_showdialog != NULL));
			}
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* reload input selection */
static void rxwnd_reloadinput(rxwnd_ctx_t *ctx)
{
	TCHAR *modulename;
	input_module_desc_t *desc;

	modulename = ini_get(ini_sect_get(ctx->ini, _T("input"), 0), _T("module"));
	if( (modulename != NULL) && (_tcscmp(modulename, _T("")) != 0) )
	{
		desc = input_module_get_by_name(&(ctx->rx->input_mod_list), modulename);
		if(desc != NULL) {
			rxwnd_setinput(ctx, desc->module_id);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* save input selection */
static void rxwnd_saveinput(rxwnd_ctx_t *ctx)
{
	TCHAR *modulename;

	if(ctx->rx->input_mod_desc != NULL) {
		modulename = ctx->rx->input_mod_desc->name;
	} else {
		modulename = _T("");
	}

	ini_set(ini_sect_get(ctx->ini, _T("input"), 1), _T("module"), modulename);
}

/* ---------------------------------------------------------------------------------------------- */

/* start input configuration dialog */
static void rxwnd_showinputconfig(rxwnd_ctx_t *ctx)
{
	if( (ctx->rx->input_mod_ctx != NULL) &&
		(ctx->rx->input_mod_desc->fn_config_showdialog != NULL) )
	{
		ctx->rx->input_mod_desc->fn_config_showdialog(
			ctx->rx->input_mod_ctx, ctx->uidata, ctx->hwnd);
	}
}

/* ---------------------------------------------------------------------------------------------- */
/* Visualizers/viewers subroutines */

/* calculate bounding rectangles for viewers */
static int rxwnd_get_visualizers_bounds(int cw, int ch, RECT *prcspecview, RECT *prcwatrview)
{
	if((cw < RXWND_MINCLTW) || (ch < RXWND_MINCLTH))
		return 0;

	/* calculate spectrum viewer bounding rectangle */
	prcspecview->left = RXWND_SPECVIEW_OFF_LEFT;
	prcspecview->right = cw - RXWND_SPECVIEW_OFF_RIGHT;
	prcspecview->top = RXWND_SPECVIEW_OFF_TOP;
	prcspecview->bottom = RXWND_SPECVIEW_OFF_TOP + RXWND_SPECVIEW_HEIGHT;

	/* calculate waterfall viewer bounding rectangle */
	prcwatrview->left = RXWND_WATRVIEW_OFF_LEFT;
	prcwatrview->right = cw - RXWND_WATRVIEW_OFF_RIGHT;
	prcwatrview->top = RXWND_WATRVIEW_OFF_TOP;
	prcwatrview->bottom = ch - RXWND_WATRVIEW_OFF_BOTTOM;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* process spectrum frame from analyzer */
static void rxwnd_spectrumframe(rxwnd_ctx_t *ctx, float *buf, size_t bufsize, size_t input_framelen)
{
	HDC hdc;

	if( (hdc = GetDC(ctx->hwnd)) != NULL )
	{
		/* update spectrum viewer spectrum mask */
		specview_set_mask(ctx->specview,
			ctx->rx->fc_input, ctx->rx->fs_input,
			buf, bufsize);
		specview_set_sql(ctx->specview, 0, 1);
		specview_update(ctx->specview);

		/* append data to waterfall viewer */
		watrview_append(ctx->watrview, hdc,
			ctx->rx->fc_input, ctx->rx->fs_input,
			buf, bufsize, input_framelen);
		watrview_update(ctx->watrview, hdc);

		/* redraw viewers */
		if(!ctx->is_minimized)
		{
			specview_copy(ctx->specview, hdc,
				ctx->specview_rect.left, ctx->specview_rect.top);
			watrview_copy(ctx->watrview, hdc,
				ctx->watrview_rect.left, ctx->watrview_rect.top);
		}

		ReleaseDC(ctx->hwnd, hdc);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* update and redraw viewers */
static void rxwnd_visualupdate(rxwnd_ctx_t *ctx, int sv_upd, int wv_upd)
{
	HDC hdc;

	if((!sv_upd) && (!wv_upd))
		return;

	if( (hdc = GetDC(ctx->hwnd)) != NULL )
	{
		if(sv_upd)
		{
			if(specview_update(ctx->specview) <= 0)
				sv_upd = 0;
		}

		if(wv_upd)
		{
			if(watrview_update(ctx->watrview, hdc) <= 0)
				wv_upd = 0;
		}

		if(!ctx->is_minimized)
		{
			if(sv_upd)
			{
				specview_copy(ctx->specview, hdc,
					ctx->specview_rect.left, ctx->specview_rect.top);
			}

			if(wv_upd)
			{
				watrview_copy(ctx->watrview, hdc,
					ctx->watrview_rect.left, ctx->watrview_rect.top);
			}
		}

		ReleaseDC(ctx->hwnd, hdc);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize visualizers */
static int rxwnd_visualinit(rxwnd_ctx_t *ctx)
{
	HDC hdc;
	RECT rc;

	/* get viewers bounding rectangles */
	GetClientRect(ctx->hwnd, &rc);
	if(!rxwnd_get_visualizers_bounds(rc.right, rc.bottom,
		&(ctx->specview_rect), &(ctx->watrview_rect)))
	{
		return 0;
	}

	if( (hdc = GetDC(ctx->hwnd)) == NULL )
		return 0;

	/* initialize spectrum viewer */
	ctx->specview = specview_init(hdc,
		ctx->uidata->hfnt_viewers, ctx->rx,
		ctx->fcfg.disp_f_0, ctx->fcfg.disp_f_1,
		ctx->specview_rect.right - ctx->specview_rect.left,
		ctx->specview_rect.bottom - ctx->specview_rect.top,
		ini_sect_get(ctx->ini, _T("specview"), 0));

	/* initialize waterfall viewer */
	ctx->watrview = watrview_init(hdc,
		ctx->uidata->hfnt_viewers, ctx->rx,
		ctx->fcfg.disp_f_0, ctx->fcfg.disp_f_1,
		ctx->watrview_rect.right - ctx->watrview_rect.left,
		ctx->watrview_rect.bottom - ctx->watrview_rect.top,
		ini_sect_get(ctx->ini, _T("watrview"), 0));

	/* check for initialization error */
	if((ctx->specview == NULL) || (ctx->watrview == NULL))
	{
		ReleaseDC(ctx->hwnd, hdc);
		return 0;
	}

	/* update viewers */
	specview_update(ctx->specview);
	watrview_update(ctx->watrview, hdc);

	ReleaseDC(ctx->hwnd, hdc);
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* cleanup visualizers */
static void rxwnd_visualcleanup(rxwnd_ctx_t *ctx)
{
	/* free spectrum viewer */
	specview_cleanup(ctx->specview,
		ini_sect_get(ctx->ini, _T("specview"), 1));
	ctx->specview = NULL;

	/* free waterfall viewer */
	watrview_cleanup(ctx->watrview,
		ini_sect_get(ctx->ini, _T("watrview"), 1));
	ctx->watrview = NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* update viewers data when channel config changes */
static void rxwnd_procchan_handler(rxwnd_ctx_t *ctx, unsigned int msg,
								   notify_proc_list_t *channelList)
{
	switch(msg)
	{
	case EVENT_PROCCHAN_CREATE:
	case EVENT_PROCCHAN_DELETE:
	case EVENT_PROCCHAN_FREQ:
	case EVENT_PROCCHAN_FILTERCFG:
		specview_set_chmap(ctx->specview);
		specview_set_sql(ctx->specview, 0, 0);
		watrview_set_chmap(ctx->watrview);
		rxwnd_visualupdate(ctx, 1, 1);
		break;
	case EVENT_PROCCHAN_SQLCFG:
		specview_set_sql(ctx->specview, 0, 0);
		rxwnd_visualupdate(ctx, 1, 0);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* update viewers when viewers config changes */
static void rxwnd_visualcfg_handler(rxwnd_ctx_t *ctx, unsigned int msg, void *data)
{
	switch(msg)
	{
	case EVENT_VISUALCFG_SPECVIEW:
		rxwnd_visualupdate(ctx, 1, 0);
		break;
	case EVENT_VISUALCFG_WATRVIEW:
		rxwnd_visualupdate(ctx, 0, 1);
		break;
	case EVENT_VISUALCFG_FREQ_RANGE:
		rxwnd_visualupdate(ctx, 1, 1);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */
/* Mouse dragging subroutines */

/* set input module center frequency */
static int rxwnd_setinputfc(rxwnd_ctx_t *ctx, double fc_input, int notify)
{
	int res;

	if(fc_input < RXLIM_FC_MIN)
		fc_input = RXLIM_FC_MIN;
	if(fc_input > RXLIM_FC_MAX)
		fc_input = RXLIM_FC_MAX;

	res = rx_set_input_fc(ctx->rx, &fc_input, ctx->uidata->msgbuf, ctx->uidata->msgbuf_size);

	if(res == 0) {
		MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
	}

	if( (res > 0) && notify ) {
		uievent_send(ctx->event_rx_state, EVENT_RX_STATE_SET_INPUT_FC, NULL);
	}

	return res;
}

/* ---------------------------------------------------------------------------------------------- */

/* set visualizers frequency display range */
static int rxwnd_setdispfreqrange(rxwnd_ctx_t *ctx, double f_0, double f_1, int notify)
{
	int is_changed = 0;

	/* check parameters */
	if((f_0 < RXLIM_FC_MIN) || (f_1 > RXLIM_FC_MAX) || (f_1 <= f_0))
		return 0;

	/* check range actually changed */
	if( (fabs(f_0 - ctx->fcfg.disp_f_0) >= 1e-3) ||
		(fabs(f_1 - ctx->fcfg.disp_f_1) >= 1e-3) )
	{
		/* save new range */
		ctx->fcfg.disp_f_0 = f_0;
		ctx->fcfg.disp_f_1 = f_1;

		/* set spectrum viewer frequency range */
		ctx->specview->f_0 = f_0;
		ctx->specview->f_1 = f_1;
		if(specview_set_freq_range(ctx->specview) > 0)
			is_changed = 1;

		/* set waterfall viewer frequency range */
		if(watrview_set_freq_range(ctx->watrview, f_0, f_1) > 0)
			is_changed = 1;

		/* broadcast notification */
		if(is_changed)
		{
			if(notify) {
				uievent_send(ctx->event_visualcfg, EVENT_VISUALCFG_FREQ_RANGE, NULL);
			}
			return 1;
		}
	}

	return -1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set spectrum viewer hint text then update and redraw viewer */
static int rxwnd_drag_sethinttext(rxwnd_ctx_t *ctx, TCHAR *text, int x, int y)
{
	HDC hdc;
	int res;

	/* set new hint text */
	res = specview_set_text(ctx->specview, text,
		x - ctx->specview_rect.left,
		y - ctx->specview_rect.top,
		6, 6, 0, 0);
	if(res == 0)
		return 0;

	/* check hint actually updated and window is not minimized */
	if((res > 0) && (!ctx->is_minimized))
	{
		if((hdc = GetDC(ctx->hwnd)) != NULL)
		{
			/* update and redraw hint text */
			specview_update_text(ctx->specview, hdc,
				ctx->specview_rect.left, ctx->specview_rect.top);
			ReleaseDC(ctx->hwnd, hdc);
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* update hint text when dragging spectrum viewer marks */
static void rxwnd_drag_updatehint(rxwnd_ctx_t *ctx, int pos_x, int pos_y)
{
	rxproc_t *proc;

	switch(ctx->dragstate.mode)
	{
	case RXWND_DRAG_INPUT_FC:
		if(ctx->rx->is_started)
		{
			fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, ctx->rx->fc_input, 2, 3, 3);
			_stprintf(ctx->uidata->msgbuf, _T("Input: %s"), ctx->uidata->databuf);
			rxwnd_drag_sethinttext(ctx, ctx->uidata->msgbuf, pos_x, pos_y);
		}
		break;
	case RXWND_DRAG_CHAN_FC:
		if((proc = rx_proc_find(ctx->rx, ctx->dragstate.chid)) != NULL)
		{
			fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, proc->cfg.fc, 2, 3, 3);
			_stprintf(ctx->uidata->msgbuf, _T("\"%s\": %s Hz"), proc->cfg.name, ctx->uidata->databuf);
			rxwnd_drag_sethinttext(ctx, ctx->uidata->msgbuf, pos_x, pos_y);
		}
		break;
	case RXWND_DRAG_CHAN_BW_EDGE_LOWER:
	case RXWND_DRAG_CHAN_BW_EDGE_UPPER:
		if((proc = rx_proc_find(ctx->rx, ctx->dragstate.chid)) != NULL)
		{
			fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, proc->cfg.filter_fc, 0, 3, 0);
			_stprintf(ctx->uidata->msgbuf, _T("\"%s\" cutoff: %s Hz"), proc->cfg.name, ctx->uidata->databuf);
			rxwnd_drag_sethinttext(ctx, ctx->uidata->msgbuf, pos_x, pos_y);
		}
		break;
	case RXWND_DRAG_CHAN_SQL_OPEN_THRES:
		if((proc = rx_proc_find(ctx->rx, ctx->dragstate.chid)) != NULL)
		{
			fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, proc->cfg.sql.op_thres_db, 0, 6, 1);
			_stprintf(ctx->uidata->msgbuf, _T("\"%s\" sq open: %s dB"), proc->cfg.name, ctx->uidata->databuf);
			rxwnd_drag_sethinttext(ctx, ctx->uidata->msgbuf, pos_x, pos_y);
		}
		break;
	case RXWND_DRAG_CHAN_SQL_CLOSE_THRES:
		if((proc = rx_proc_find(ctx->rx, ctx->dragstate.chid)) != NULL)
		{
			fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, proc->cfg.sql.cl_thres_db, 0, 6, 1);
			_stprintf(ctx->uidata->msgbuf, _T("\"%s\" sq close: %s dB"), proc->cfg.name, ctx->uidata->databuf);
			rxwnd_drag_sethinttext(ctx, ctx->uidata->msgbuf, pos_x, pos_y);
		}
		break;
	default:
		rxwnd_drag_sethinttext(ctx, NULL, 0, 0);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* move display frequency range */
static void rxwnd_drag_specview_plot(rxwnd_ctx_t *ctx, int dx_full)
{
	double d_f, f_range, f_0, snap;

	if(specview_map_dist_x(ctx->specview, -dx_full, &d_f))
	{
		f_0 = ctx->dragstate.disp_f0_org + d_f;

		snap = ctx->fcfg.f_disp_range_snap;
		if(snap <= 0)
			snap = 1.0;

		f_0 = floor(f_0 / snap + 0.5) * snap;

		f_range = ctx->fcfg.disp_f_1 - ctx->fcfg.disp_f_0;
		rxwnd_setdispfreqrange(ctx, f_0, f_0 + f_range, 1);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* move input center frequency */
static void rxwnd_drag_input_fc(rxwnd_ctx_t *ctx, int dx_full)
{
	double fc_input, d_f, snap;

	if(!ctx->rx->is_started)
		return;

	if(!specview_map_dist_x(ctx->specview, dx_full, &d_f))
		return;

	fc_input = ctx->dragstate.fc_input_org + d_f;

	snap = ctx->fcfg.f_mark_snap;
	if(snap <= 0)
		snap = 1.0;

	fc_input = floor(fc_input / snap + 0.5) * snap;

	rxwnd_setinputfc(ctx, fc_input, 1);
}

/* ---------------------------------------------------------------------------------------------- */

/* move display frequency range and input center frequency synchronously */
static void rxwnd_drag_specview_plot_and_input_fc(rxwnd_ctx_t *ctx, int dx_full)
{
	double d_f, disp_f_range, disp_f_0, disp_f_snap, fc_input;
	int set_inputfc_res = -1, set_disprange_res = -1;

	if(specview_map_dist_x(ctx->specview, -dx_full, &d_f))
	{
		/* calculate new display range offset */
		disp_f_0 = ctx->dragstate.disp_f0_org + d_f;

		disp_f_snap = ctx->fcfg.f_disp_range_snap;
		if(disp_f_snap <= 0)
			disp_f_snap = 1.0;

		disp_f_0 = floor(disp_f_0 / disp_f_snap + 0.5) * disp_f_snap;

		/* set new input center frequency */
		fc_input = ctx->dragstate.fc_input_org +
			(disp_f_0 - ctx->dragstate.disp_f0_org);
		set_inputfc_res = rxwnd_setinputfc(ctx, fc_input, 0);

		/* set new display range */
		if(set_inputfc_res > 0)
		{
			disp_f_range = ctx->fcfg.disp_f_1 - ctx->fcfg.disp_f_0;
			set_disprange_res = rxwnd_setdispfreqrange(ctx,
				disp_f_0, disp_f_0 + disp_f_range, 0);
		}

		/* send notifications */
		if(set_inputfc_res > 0) {
			uievent_send(ctx->event_rx_state, EVENT_RX_STATE_SET_INPUT_FC, NULL);
		}

		if(set_disprange_res > 0) {
			uievent_send(ctx->event_visualcfg, EVENT_VISUALCFG_FREQ_RANGE, NULL);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* start moving channel center frequency */
static void rxwnd_drag_chan_fc_start(rxwnd_ctx_t *ctx, unsigned int chid)
{
	rxproc_t *proc;

	if( (proc = rx_proc_find(ctx->rx, chid)) != NULL )
	{
		ctx->dragstate.chan_param_org = proc->cfg.fc;
		ctx->dragstate.chan_param_cur = proc->cfg.fc;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* move channel center frequency */
static void rxwnd_drag_chan_fc(rxwnd_ctx_t *ctx, unsigned int chid, int dx_full)
{
	rxproc_t *proc;
	double d_fc, fc, snap;
	notify_proc_list_t notifychid;

	if(specview_map_dist_x(ctx->specview, dx_full, &d_fc))
	{
		fc = ctx->dragstate.chan_param_org + d_fc;

		snap = ctx->fcfg.f_mark_snap;
		if(snap <= 0)
			snap = 1.0;

		fc = floor(fc / snap + 0.5) * snap;

		if(fc < RXLIM_FC_MIN)
			fc = RXLIM_FC_MIN;
		if(fc > RXLIM_FC_MAX)
			fc = RXLIM_FC_MAX;

		if(fabs(fc - ctx->dragstate.chan_param_cur) >= 1e-3)
		{
			if( (proc = rx_proc_find(ctx->rx, chid)) != NULL )
			{
				rxproc_set_fc(proc, fc);

				notifychid.count = 1;
				notifychid.items = &chid;
				uievent_send(ctx->event_procchan, EVENT_PROCCHAN_FREQ, &notifychid);
			}

			ctx->dragstate.chan_param_cur = fc;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* start moving channel filter bandwidth */
static void rxwnd_drag_chan_bw_edge_start(rxwnd_ctx_t *ctx, unsigned int chid)
{
	rxproc_t *proc;

	if( (proc = rx_proc_find(ctx->rx, chid)) != NULL )
	{
		ctx->dragstate.chan_param_org = proc->cfg.filter_fc;
		ctx->dragstate.chan_param_cur = proc->cfg.filter_fc;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* move channel filter bandwidth */
static void rxwnd_drag_chan_bw_edge(rxwnd_ctx_t *ctx, unsigned int chid, int dx_full, int is_upper_edge)
{
	rxproc_t *proc;
	double d_fc, fc, snap;
	notify_proc_list_t notifychid;

	if(specview_map_dist_x(ctx->specview, dx_full, &d_fc))
	{
		if(is_upper_edge) {
			fc = ctx->dragstate.chan_param_org + d_fc;
		} else {
			fc = ctx->dragstate.chan_param_org - d_fc;
		}

		/*if(fc >= 20000.0)		snap = 1000.0;
		else if(fc >= 10000.0)	snap = 500.0;
		else if(fc >= 5000.0)	snap = 250.0;
		else if(fc >= 1000.0)	snap = 50.0;
		else					snap = 10.0;*/

		snap = 50.0;

		fc = floor(fc / snap + 0.5) * snap;

		if(fc < RXLIM_PROC_FILTER_FC_MIN)
			fc = RXLIM_PROC_FILTER_FC_MIN;
		if(fc > RXLIM_PROC_FILTER_FC_MAX)
			fc = RXLIM_PROC_FILTER_FC_MAX;

		if(fabs(fc - ctx->dragstate.chan_param_cur) >= 1e-3)
		{
			if( (proc = rx_proc_find(ctx->rx, chid)) != NULL )
			{
				if( !rxproc_set_filter_params(proc,
					fc, proc->cfg.filter_df, proc->cfg.filter_as,
					ctx->uidata->msgbuf, ctx->uidata->msgbuf_size) )
				{
					MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
				}
				else
				{
					notifychid.count = 1;
					notifychid.items = &chid;
					uievent_send(ctx->event_procchan, EVENT_PROCCHAN_FILTERCFG, &notifychid);
				}
			}

			ctx->dragstate.chan_param_cur = fc;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void rxwnd_drag_chan_sql_thres_start(rxwnd_ctx_t *ctx, unsigned int chid, int is_op)
{
	rxproc_t *proc;
	double thres;

	if((proc = rx_proc_find(ctx->rx, chid)) != NULL)
	{
		thres = is_op ? proc->cfg.sql.op_thres_db : proc->cfg.sql.cl_thres_db;
		ctx->dragstate.chan_param_org = thres;
		ctx->dragstate.chan_param_cur = thres;
	}

}

/* ---------------------------------------------------------------------------------------------- */

static void rxwnd_drag_chan_sql_thres(rxwnd_ctx_t *ctx, unsigned int chid, int dy_full, int is_op)
{
	rxproc_t *proc;
	double d_thres, thres_cur;
	double op_thres, cl_thres;
	double snap = 0.5;
	notify_proc_list_t notifychid;

	if( specview_map_dist_y(ctx->specview, dy_full, &d_thres) &&
		((proc = rx_proc_find(ctx->rx, ctx->dragstate.chid)) != NULL) )
	{
		thres_cur = ctx->dragstate.chan_param_org + d_thres;

		thres_cur = floor(thres_cur / snap + 0.5) * snap;

		if(thres_cur < RXSQL_THRES_DB_MIN)
			thres_cur = RXSQL_THRES_DB_MIN;
		if(thres_cur > RXSQL_THRES_DB_MAX)
			thres_cur = RXSQL_THRES_DB_MAX;

		if(is_op) {
			if(thres_cur < proc->cfg.sql.cl_thres_db)
				thres_cur = proc->cfg.sql.cl_thres_db;
		} else {
			if(thres_cur > proc->cfg.sql.op_thres_db)
				thres_cur = proc->cfg.sql.op_thres_db;
		}

		if(fabs(thres_cur - ctx->dragstate.chan_param_cur) >= 1e-3)
		{
			if(is_op) {
				op_thres = thres_cur;
				cl_thres = proc->cfg.sql.cl_thres_db;
			} else {
				op_thres = proc->cfg.sql.op_thres_db;
				cl_thres = thres_cur;
			}

			if(!rxproc_set_sql_params(proc, proc->cfg.sql.use_carr_filter,
				proc->cfg.sql.bw, proc->cfg.sql.envef_param,
				op_thres, cl_thres, proc->cfg.sql.op_dly_ms, proc->cfg.sql.cl_dly_ms,
				ctx->uidata->msgbuf, ctx->uidata->msgbuf_size))
			{
				MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			}
			else
			{
				notifychid.count = 1;
				notifychid.items = &chid;
				uievent_send(ctx->event_procchan, EVENT_PROCCHAN_SQLCFG, &notifychid);
			}

			ctx->dragstate.chan_param_cur = thres_cur;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* move waterfall viewer image up/down */
static void rxwnd_drag_watrview_img(rxwnd_ctx_t *ctx, int dy)
{
	if(watrview_scroll(ctx->watrview, -dy))
	{
		uievent_send(ctx->event_visualcfg, EVENT_VISUALCFG_WATRVIEW, NULL);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* move waterfall viewer scrollbar up/down */
static void rxwnd_drag_watrview_scrbar(rxwnd_ctx_t *ctx, int dy_full)
{
	int scrbar_pos;

	scrbar_pos = ctx->dragstate.scrbar_pos_org + dy_full;

	if(watrview_drag_scrbar(ctx->watrview, scrbar_pos))
	{
		uievent_send(ctx->event_visualcfg, EVENT_VISUALCFG_WATRVIEW, NULL);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* start moving waterfall viewer scrollbar */
static void rxwnd_drag_watrview_scrbar_start(rxwnd_ctx_t *ctx)
{
	ctx->dragstate.scrbar_pos_org = ctx->watrview->scrbar.slider_pos;
	ctx->watrview->scrbar.autoscroll_inh = 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* end moving waterfall viewer scrollbar */
static void rxwnd_drag_watrview_scrbar_end(rxwnd_ctx_t *ctx)
{
	ctx->watrview->scrbar.autoscroll_inh = 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* get dragging mode by cursor position and current modifier keys */
static void rxwnd_drag_getmode(rxwnd_ctx_t *ctx, int pos_x, int pos_y,
							   rxwnd_drag_mode_t *pmode, unsigned int *pchid,
							   HCURSOR *pcur)
{
	POINT pos_pt;
	specview_point_type_t sv_mode;
	watrview_point_type_t wv_mode;

	*pmode = RXWND_DRAG_NOTHING;
	*pcur = NULL;

	pos_pt.x = pos_x;
	pos_pt.y = pos_y;

	if(PtInRect(&(ctx->specview_rect), pos_pt))
	{
		sv_mode = specview_chmap_point(ctx->specview,
			pos_x - ctx->specview_rect.left,
			pos_y - ctx->specview_rect.top,
			pchid);

		switch(sv_mode)
		{
		case SPECVIEW_POINT_SHM:
			if((GetKeyState(VK_SHIFT) & 0x8000) != 0)
			{
				if( ctx->rx->is_started &&
					(ctx->rx->input_mod_desc->fn_set_fc != NULL) )
				{
					*pmode = RXWND_DRAG_SPECVIEW_SHM_AND_INPUT_FC;
					*pcur = ctx->uidata->hcur_sizex;
				}
			}
			else
			{
				*pmode = RXWND_DRAG_SPECVIEW_SHM;
				*pcur = ctx->uidata->hcur_sizex;
			}
			break;
		case SPECVIEW_POINT_INPUT_FC:
			if( ctx->rx->is_started &&
				(ctx->rx->input_mod_desc->fn_set_fc != NULL) )
			{
				*pmode = RXWND_DRAG_INPUT_FC;
				*pcur = ctx->uidata->hcur_sizex;
			}
			break;
		case SPECVIEW_POINT_CHAN_FC:
			*pmode = RXWND_DRAG_CHAN_FC;
			*pcur = ctx->uidata->hcur_sizex;
			break;
		case SPECVIEW_POINT_CHAN_BW_EDGE_LOWER:
			*pmode = RXWND_DRAG_CHAN_BW_EDGE_LOWER;
			*pcur = ctx->uidata->hcur_sizex;
			break;
		case SPECVIEW_POINT_CHAN_BW_EDGE_UPPER:
			*pmode = RXWND_DRAG_CHAN_BW_EDGE_UPPER;
			*pcur = ctx->uidata->hcur_sizex;
			break;
		case SPECVIEW_POINT_CHAN_SQL_OPEN_THRES:
			*pmode = RXWND_DRAG_CHAN_SQL_OPEN_THRES;
			*pcur = ctx->uidata->hcur_sizey;
			break;
		case SPECVIEW_POINT_CHAN_SQL_CLOSE_THRES:
			*pmode = RXWND_DRAG_CHAN_SQL_CLOSE_THRES;
			*pcur = ctx->uidata->hcur_sizey;
			break;
		}
	}

	if(PtInRect(&(ctx->watrview_rect), pos_pt))
	{
		wv_mode = watrview_map_point(ctx->watrview,
			pos_x - ctx->watrview_rect.left,
			pos_y - ctx->watrview_rect.top);

		switch(wv_mode)
		{
		case WATRVIEW_POINT_IMAGE:
			*pmode = RXWND_DRAG_WATRVIEW_IMG;
			*pcur = ctx->uidata->hcur_sizey;
			break;
		case WATRVIEW_POINT_SCRBAR:
			*pmode = RXWND_SCROLL_WATRVIEW;
			*pcur = ctx->uidata->hcur_sizey;
			break;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* update mouse pointer when hovering over draggable elements */
static void rxwnd_drag_setpointer(rxwnd_ctx_t *ctx, int pos_x, int pos_y)
{
	rxwnd_drag_mode_t mode;
	HCURSOR hcur;
	unsigned int chid;

	rxwnd_drag_getmode(ctx, pos_x, pos_y, &mode, &chid, &hcur);

	switch(mode)
	{
	case RXWND_DRAG_SPECVIEW_SHM:
	case RXWND_DRAG_SPECVIEW_SHM_AND_INPUT_FC:
	case RXWND_DRAG_WATRVIEW_IMG:
	case RXWND_SCROLL_WATRVIEW:
		hcur = NULL;
		break;
	}

	ctx->dragstate.hcur_point = hcur;
}

/* ---------------------------------------------------------------------------------------------- */

/* drag selected element with mouse cursor */
static void rxwnd_drag(rxwnd_ctx_t *ctx, int pos_x, int pos_y)
{
	if( (pos_x != ctx->dragstate.current_x) ||
		(pos_y != ctx->dragstate.current_y) )
	{
		switch(ctx->dragstate.mode)
		{
		case RXWND_DRAG_SPECVIEW_SHM:
			rxwnd_drag_specview_plot(ctx,
				pos_x - ctx->dragstate.origin_x);
			break;
		case RXWND_DRAG_INPUT_FC:
			rxwnd_drag_input_fc(ctx,
				pos_x - ctx->dragstate.origin_x);
			break;
		case RXWND_DRAG_SPECVIEW_SHM_AND_INPUT_FC:
			rxwnd_drag_specview_plot_and_input_fc(ctx,
				pos_x - ctx->dragstate.origin_x);
			break;
		case RXWND_DRAG_CHAN_FC:
			rxwnd_drag_chan_fc(ctx,
				ctx->dragstate.chid,
				pos_x - ctx->dragstate.origin_x);
			break;
		case RXWND_DRAG_CHAN_BW_EDGE_LOWER:
			rxwnd_drag_chan_bw_edge(ctx,
				ctx->dragstate.chid,
				pos_x - ctx->dragstate.origin_x, 0);
			break;
		case RXWND_DRAG_CHAN_BW_EDGE_UPPER:
			rxwnd_drag_chan_bw_edge(ctx,
				ctx->dragstate.chid,
				pos_x - ctx->dragstate.origin_x, 1);
			break;
		case RXWND_DRAG_CHAN_SQL_OPEN_THRES:
			rxwnd_drag_chan_sql_thres(ctx,
				ctx->dragstate.chid,
				pos_y - ctx->dragstate.origin_y, 1);
			break;
		case RXWND_DRAG_CHAN_SQL_CLOSE_THRES:
			rxwnd_drag_chan_sql_thres(ctx,
				ctx->dragstate.chid,
				pos_y - ctx->dragstate.origin_y, 0);
			break;
		case RXWND_DRAG_WATRVIEW_IMG:
			rxwnd_drag_watrview_img(ctx,
				pos_y - ctx->dragstate.current_y);
			break;
		case RXWND_SCROLL_WATRVIEW:
			rxwnd_drag_watrview_scrbar(ctx,
				pos_y - ctx->dragstate.origin_y);
			break;
		}

		ctx->dragstate.current_x = pos_x;
		ctx->dragstate.current_y = pos_y;
		rxwnd_drag_updatehint(ctx, pos_x, pos_y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* select draggable element and start dragging */
static void rxwnd_drag_start(rxwnd_ctx_t *ctx, int pos_x, int pos_y)
{
	rxwnd_drag_getmode(ctx, pos_x, pos_y,
		&(ctx->dragstate.mode), &(ctx->dragstate.chid),
		&(ctx->dragstate.hcur_drag));

	if(ctx->dragstate.mode != RXWND_DRAG_NOTHING)
	{
		switch(ctx->dragstate.mode)
		{
		case RXWND_DRAG_SPECVIEW_SHM:
			ctx->dragstate.disp_f0_org = ctx->fcfg.disp_f_0;
			break;
		case RXWND_DRAG_INPUT_FC:
			ctx->dragstate.fc_input_org = ctx->rx->fc_input;
			break;
		case RXWND_DRAG_SPECVIEW_SHM_AND_INPUT_FC:
			ctx->dragstate.disp_f0_org = ctx->fcfg.disp_f_0;
			ctx->dragstate.fc_input_org = ctx->rx->fc_input;
			break;
		case RXWND_DRAG_CHAN_FC:
			rxwnd_drag_chan_fc_start(ctx, ctx->dragstate.chid);
			break;
		case RXWND_DRAG_CHAN_BW_EDGE_LOWER:
		case RXWND_DRAG_CHAN_BW_EDGE_UPPER:
			rxwnd_drag_chan_bw_edge_start(ctx, ctx->dragstate.chid);
			break;
		case RXWND_DRAG_CHAN_SQL_OPEN_THRES:
			rxwnd_drag_chan_sql_thres_start(ctx, ctx->dragstate.chid, 1);
			break;
		case RXWND_DRAG_CHAN_SQL_CLOSE_THRES:
			rxwnd_drag_chan_sql_thres_start(ctx, ctx->dragstate.chid, 0);
			break;
		case RXWND_SCROLL_WATRVIEW:
			rxwnd_drag_watrview_scrbar_start(ctx);
			break;
		}

		ctx->dragstate.current_x = ctx->dragstate.origin_x = pos_x;
		ctx->dragstate.current_y = ctx->dragstate.origin_y = pos_y;
		rxwnd_drag_updatehint(ctx, pos_x, pos_y);
		rxwnd_updatecursor(ctx);
		SetCapture(ctx->hwnd);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* end dragging */
static void rxwnd_drag_end(rxwnd_ctx_t *ctx, int pos_x, int pos_y)
{
	switch(ctx->dragstate.mode)
	{
	case RXWND_SCROLL_WATRVIEW:
		rxwnd_drag_watrview_scrbar_end(ctx);
		break;
	}

	ctx->dragstate.mode = RXWND_DRAG_NOTHING;
	ctx->dragstate.hcur_drag = NULL;
	rxwnd_drag_updatehint(ctx, pos_x, pos_y);
	rxwnd_updatecursor(ctx);
	ReleaseCapture();
}

/* ---------------------------------------------------------------------------------------------- */
/* Mouse event handling */

/* handle left button push */
static void rxwnd_handle_lbuttondown(rxwnd_ctx_t *ctx, WPARAM mod_keys, int pos_x, int pos_y)
{
	if(ctx->dragstate.mode != RXWND_DRAG_NOTHING) {
		rxwnd_drag(ctx, pos_x, pos_y);
		rxwnd_drag_end(ctx, pos_x, pos_y);
	}

	rxwnd_drag_start(ctx, pos_x, pos_y);
}

/* ---------------------------------------------------------------------------------------------- */

/* handle left button release */
static void rxwnd_handle_lbuttonup(rxwnd_ctx_t *ctx, WPARAM mod_keys, int pos_x, int pos_y)
{
	if(ctx->dragstate.mode != RXWND_DRAG_NOTHING) {
		rxwnd_drag(ctx, pos_x, pos_y);
		rxwnd_drag_end(ctx, pos_x, pos_y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle mouse moving */
static void rxwnd_handle_mousemove(rxwnd_ctx_t *ctx, WPARAM mod_keys, int pos_x, int pos_y)
{
	if(ctx->dragstate.mode != RXWND_DRAG_NOTHING)
	{
		rxwnd_drag(ctx, pos_x, pos_y);
		if( ! (mod_keys & MK_LBUTTON) )
			rxwnd_drag_end(ctx, pos_x, pos_y);
	}

	rxwnd_drag_setpointer(ctx, pos_x, pos_y);
	rxwnd_updatecursor(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

/* handle mouse wheel over spectrum viewer */
static void rxwnd_wheel_specview(rxwnd_ctx_t *ctx, int delta)
{
	double f_0, f_range, snap;

	snap = ctx->fcfg.f_disp_range_snap;

	if(snap > 0)
	{
		f_0 = ctx->fcfg.disp_f_0 + snap * delta;
		f_0 = floor(f_0 / snap + 0.5) * snap;

		f_range = ctx->fcfg.disp_f_1 - ctx->fcfg.disp_f_0;
		rxwnd_setdispfreqrange(ctx, f_0, f_0 + f_range, 1);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle mouse wheel over waterfall viewer */
static void rxwnd_wheel_watrview(rxwnd_ctx_t *ctx, int delta, WORD fwKeys)
{
	if( ! (fwKeys & MK_SHIFT) )
		delta *= 15;

	if(watrview_scroll(ctx->watrview, -delta))
	{
		uievent_send(ctx->event_visualcfg, EVENT_VISUALCFG_WATRVIEW, NULL);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle mouse wheel */
static int rxwnd_handle_mousewheel(rxwnd_ctx_t *ctx, WORD fwKeys, int delta, int x, int y)
{
	POINT pt;

	pt.x = x;
	pt.y = y;
	if(!ScreenToClient(ctx->hwnd, &pt))
		return 0;

	if(PtInRect(&(ctx->specview_rect), pt))
		rxwnd_wheel_specview(ctx, delta / WHEEL_DELTA);

	if(PtInRect(&(ctx->watrview_rect), pt))
		rxwnd_wheel_watrview(ctx, delta / WHEEL_DELTA, fwKeys);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */
/* Receiver commands handling */

/* handle start command */
static int rxwnd_cmd_rxstart(rxwnd_ctx_t *ctx)
{
	int res;

	/* check receiver is not started already */
	if(ctx->rx->is_started)
		return 0;

	/* start receiver */
	rxwnd_setcursor(ctx, ctx->uidata->hcur_wait);
	res = rx_start(ctx->rx, ctx->uidata->msgbuf, ctx->uidata->msgbuf_size);
	rxwnd_setcursor(ctx, NULL);

	if(!res) {
		MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	/* broadacast notifications */
	uievent_send(ctx->event_rx_state, EVENT_RX_STATE_START, NULL);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* handle stop command */
static int rxwnd_cmd_rxstop(rxwnd_ctx_t *ctx)
{
	/* check receiver is not stopped already */
	if(!ctx->rx->is_started)
		return 0;

	/* stop receiver */
	rxwnd_setcursor(ctx, ctx->uidata->hcur_wait);
	rx_stop(ctx->rx);
	rxwnd_setcursor(ctx, NULL);

	/* broadcast notifications */
	uievent_send(ctx->event_rx_state, EVENT_RX_STATE_STOP, NULL);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* handle receiver starting notification */
static void rxwnd_handle_rxstart(rxwnd_ctx_t *ctx)
{
	unsigned int i, ucmd;
	int itemselected;

	ui_menuitemf(ctx->hmenu_source, RXWND_CMD_INPUT_UNSET, 0, 0);

	for(i = 0; i < ctx->rx->input_mod_list.count; i++) {
		ucmd = RXWND_CMD_INPUT_0 + i;
		itemselected = (ctx->rx->input_mod_desc != NULL) &&
			(ctx->rx->input_mod_desc->module_id == i);
		ui_menuitemf(ctx->hmenu_source, ucmd, itemselected, 0);
	}

	/* update control menu items */
	ui_menuitemf(ctx->hmenu_ctrl, RXWND_CMD_START, 1, 1);
	ui_menuitemf(ctx->hmenu_ctrl, RXWND_CMD_RESTART, 0, 1);

	/* update spectrum viewer */
	specview_set_chmap(ctx->specview);
	specview_set_sql(ctx->specview, 0, 1);
	rxwnd_visualupdate(ctx, 1, 0);
}

/* ---------------------------------------------------------------------------------------------- */

/* handle receiver stopping notification */
static void rxwnd_handle_rxstop(rxwnd_ctx_t *ctx)
{
	unsigned int i, ucmd;
	int itemselected;

	/* update input selection menu */
	ui_menuitemf(ctx->hmenu_source, RXWND_CMD_INPUT_UNSET, 0, 1);

	for(i = 0; i < ctx->rx->input_mod_list.count; i++) {
		ucmd = RXWND_CMD_INPUT_0 + i;
		itemselected = (ctx->rx->input_mod_desc != NULL) &&
			(ctx->rx->input_mod_desc->module_id == i);
		ui_menuitemf(ctx->hmenu_source, ucmd, itemselected, 1);
	}

	/* update control menu items */
	ui_menuitemf(ctx->hmenu_ctrl, RXWND_CMD_START, 0, 1);
	ui_menuitemf(ctx->hmenu_ctrl, RXWND_CMD_RESTART, 0, 0);

	/* update spectrum viewer */
	specview_clear_mask(ctx->specview);
	specview_set_chmap(ctx->specview);
	specview_set_sql(ctx->specview, 0, 1);
	rxwnd_visualupdate(ctx, 1, 0);
}

/* ---------------------------------------------------------------------------------------------- */

/* handle receiver input center frequency changing notification */
static void rxwnd_handle_rxsetinputfc(rxwnd_ctx_t *ctx)
{
	specview_set_chmap(ctx->specview);
	rxwnd_visualupdate(ctx, 1, 0);
}

/* ---------------------------------------------------------------------------------------------- */

/* handle receiver event */
static void rxwnd_rx_state_handler(rxwnd_ctx_t *ctx, unsigned int msg, void *data)
{
	switch(msg)
	{
	case EVENT_RX_STATE_START:
		rxwnd_handle_rxstart(ctx);
		break;
	case EVENT_RX_STATE_STOP:
		rxwnd_handle_rxstop(ctx);
		break;
	case EVENT_RX_STATE_SET_INPUT_FC:
		rxwnd_handle_rxsetinputfc(ctx);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */
/* Receiver callbacks handling */

/* processing channel activity callback.
 *	*note: this callback is called from processing thread context* */
void rxwnd_rxproc_act_callback(rxwnd_ctx_t *ctx, rxproc_t *proc, int act)
{
	PostMessage(ctx->hwnd, RXWND_USERMSG, RXWND_USERMSG_RXPROC_ACT_CHANGE, proc->chid);
}

/* ---------------------------------------------------------------------------------------------- */

/* processing channel activity status changed */
void rxwnd_rxproc_act_change(rxwnd_ctx_t *ctx, unsigned int chid)
{
	notify_proc_list_t chlist;

	chlist.count = 1;
	chlist.items = &chid;
	uievent_send(ctx->event_procchan, EVENT_PROCCHAN_ACTIVITY, &chlist);
}

/* ---------------------------------------------------------------------------------------------- */
/* Main window init, cleanup and mesage handling */

/* restore receiver state*/
static void rxwnd_rxstaterestore(rxwnd_ctx_t *ctx)
{
	int autostart;

	/* forbid multiple execution */
	if(ctx->is_sessionreload_executed)
		return;
	ctx->is_sessionreload_executed = 1;

	/* reload selected input */
	rxwnd_reloadinput(ctx);

	/* reload started state */
	if( (ctx->rx->input_mod_ctx != NULL) &&
		ini_getb(ini_sect_get(ctx->ini, _T("input"), 0), _T("autostart"), &autostart) &&
		autostart )
	{
		rxwnd_cmd_rxstart(ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle WM_TIMER message */
static int rxwnd_handle_timer(rxwnd_ctx_t *ctx, WPARAM wTimerId)
{
	switch(wTimerId)
	{
	case RXWND_IDT_SESSIONRELOAD:
		KillTimer(ctx->hwnd, RXWND_IDT_SESSIONRELOAD);
		rxwnd_rxstaterestore(ctx);
		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* handle WM_PAINT message */
static void rxwnd_handle_paint(rxwnd_ctx_t *ctx)
{
	RECT rc_update, rc_temp;
	HDC hdc;
	PAINTSTRUCT ps;

	if(!GetUpdateRect(ctx->hwnd, &rc_update, FALSE))
		return;

	hdc = BeginPaint(ctx->hwnd, &ps);

	if(!ctx->is_minimized)
	{
		if(IntersectRect(&rc_temp, &rc_update, &(ctx->specview_rect)))
		{
			specview_copy(ctx->specview, hdc,
				ctx->specview_rect.left, ctx->specview_rect.top);
		}

		if(IntersectRect(&rc_temp, &rc_update, &(ctx->watrview_rect)))
		{
			watrview_copy(ctx->watrview, hdc,
				ctx->watrview_rect.left, ctx->watrview_rect.top);
		}
	}

	EndPaint(ctx->hwnd, &ps);
}

/* ---------------------------------------------------------------------------------------------- */

/* handle WM_SIZE message */
static void rxwnd_handle_resize(rxwnd_ctx_t *ctx, WPARAM mode, int cw, int ch)
{
	HDC hdc;

	if( (mode == SIZE_MAXIMIZED) || (mode == SIZE_RESTORED) )
	{
		/* get viewers bounding rectangles */
		if(rxwnd_get_visualizers_bounds(cw, ch, 
			&(ctx->specview_rect), &(ctx->watrview_rect)))
		{
			if( (hdc = GetDC(ctx->hwnd)) != NULL )
			{
				/* move spectrum viewer */
				specview_set_size(ctx->specview, hdc,
					ctx->specview_rect.right - ctx->specview_rect.left,
					ctx->specview_rect.bottom - ctx->specview_rect.top);
				specview_update(ctx->specview);

				/* move waterfall viewer */
				watrview_set_size(ctx->watrview, hdc,
					ctx->watrview_rect.right - ctx->watrview_rect.left,
					ctx->watrview_rect.bottom - ctx->watrview_rect.top);
				watrview_update(ctx->watrview, hdc);

				ReleaseDC(ctx->hwnd, hdc);
			}
		}

		ctx->is_minimized = 0;
	}

	if(mode == SIZE_MINIMIZED)
		ctx->is_minimized = 1;

}

/* ---------------------------------------------------------------------------------------------- */

/* save main window size */
/* handle WM_CREATE message */
static int rxwnd_handle_init(rxwnd_ctx_t *ctx)
{
	int status = 1;

	/* show cursor */
	rxwnd_updatecursor(ctx);

	/* load frequency config */
	freqcfg_init(&(ctx->fcfg), ctx->ini);

	/* rx init */
	if( (ctx->rx = rx_init(ctx->ini, rxwnd_spectrumframe, ctx,
		rxwnd_rxproc_act_callback, ctx,
		ctx->uidata->msgbuf, ctx->uidata->msgbuf_size)) == NULL )
	{
		MessageBox(NULL, ctx->uidata->msgbuf,
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	/* initialize input selection menu */
	rxwnd_createsrcmenu(ctx);

	/* load autosave channel group */
	rx_proc_group_load(ctx->rx, ctx->inich, _T("<autosave>"),
		ctx->uidata->msgbuf, ctx->uidata->msgbuf_size);

	/* initialize visualizers */
	if(!rxwnd_visualinit(ctx))
	{
		MessageBox(NULL, _T("Can't initialize vierwers."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	/* initialize hotkey table */
	ctx->haccel = CreateAcceleratorTable(
		rxwnd_accel_table, sizeof(rxwnd_accel_table) / sizeof(ACCEL));

	/* subscribe to notifications */
	uievent_handler_add(&(ctx->uidata->event_list), EVENT_NAME_WNDCLOSE, ctx, rxwnd_window_close_handler);
	uievent_handler_add(&(ctx->uidata->event_list), EVENT_NAME_RX_STATE, ctx, rxwnd_rx_state_handler);
	uievent_handler_add(&(ctx->uidata->event_list), EVENT_NAME_PROCCHAN, ctx, rxwnd_procchan_handler);
	uievent_handler_add(&(ctx->uidata->event_list), EVENT_NAME_VISUALCFG, ctx, rxwnd_visualcfg_handler);

	/* restore receiver state after small timeout */
	SetTimer(ctx->hwnd, RXWND_IDT_SESSIONRELOAD, 15, NULL);

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* handle WM_DESTROY message */
static void rxwnd_handle_destroy(rxwnd_ctx_t *ctx)
{
	/* unsubscribe from notifications */
	uievent_handler_remove(&(ctx->uidata->event_list), EVENT_NAME_VISUALCFG, ctx, rxwnd_visualcfg_handler);
	uievent_handler_remove(&(ctx->uidata->event_list), EVENT_NAME_PROCCHAN, ctx, rxwnd_procchan_handler);
	uievent_handler_remove(&(ctx->uidata->event_list), EVENT_NAME_RX_STATE, ctx, rxwnd_rx_state_handler);
	uievent_handler_remove(&(ctx->uidata->event_list), EVENT_NAME_WNDCLOSE, ctx, rxwnd_window_close_handler);

	/* delete hotkey table */
	DestroyAcceleratorTable(ctx->haccel);

	/* receiver save and cleanup */
	if(ctx->rx != NULL)
	{
		/* save started state and stop receiver */
		ini_setb(ini_sect_get(ctx->ini, _T("input"), 1), _T("autostart"), ctx->rx->is_started);
		rx_stop(ctx->rx);

		/* save selected input and unset input */
		rxwnd_saveinput(ctx);
		rxwnd_unsetinput(ctx);

		/* save channels */
		rx_proc_group_save(ctx->rx, ctx->inich, _T("<autosave>"));

		/* cleanup receiver state */
		rx_cleanup(ctx->rx, ctx->ini);
		ctx->rx = NULL;
	}

	/* visualizer cleanup */
	rxwnd_visualcleanup(ctx);

	/* save frequency config */
	freqcfg_save(&(ctx->fcfg), ctx->ini);

	/* save window size */
	ui_savepos(ctx->hwnd, ini_sect_get(ctx->ini, _T("window"), 1),
		UI_POS_ALL, ctx->cx_frame, ctx->cy_frame);
}

/* ---------------------------------------------------------------------------------------------- */

/* handle WM_NCDESTROY message */
static void rxwnd_handle_cleanup(rxwnd_ctx_t *ctx)
{
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

/* main window wndproc */
static LRESULT CALLBACK rxwnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	rxwnd_ctx_t *ctx;

	switch(uMsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lParam)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!rxwnd_handle_init(ctx))
			return -1;
		return 0;

	case WM_DESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		if(ctx != NULL)
			rxwnd_handle_destroy(ctx);
		return 0;

	case WM_NCDESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		if(ctx != NULL)
			rxwnd_handle_cleanup(ctx);
		PostQuitMessage(0);
		return 0;

	case WM_SIZING:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		ui_sizingcheck((int)wParam, (RECT*)lParam, ctx->cx_frame,
			ctx->cy_frame, RXWND_MINCLTW, RXWND_MINCLTH, -1, -1);
		return 0;

	case WM_SIZE:
		rxwnd_handle_resize(GetWndPtr(hwnd, GWLP_USERDATA),
			wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_PAINT:
		rxwnd_handle_paint(GetWndPtr(hwnd, GWLP_USERDATA));
		return 0;

	case WM_LBUTTONDOWN:
		rxwnd_handle_lbuttondown(GetWndPtr(hwnd, GWLP_USERDATA),
			wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_LBUTTONUP:
		rxwnd_handle_lbuttonup(GetWndPtr(hwnd, GWLP_USERDATA),
			wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEMOVE:
		rxwnd_handle_mousemove(GetWndPtr(hwnd, GWLP_USERDATA),
			wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_MOUSEWHEEL:
		rxwnd_handle_mousewheel(GetWndPtr(hwnd, GWLP_USERDATA),
			GET_KEYSTATE_WPARAM(wParam), GET_WHEEL_DELTA_WPARAM(wParam),
			GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;

	case WM_TIMER:
		if(rxwnd_handle_timer(GetWndPtr(hwnd, GWLP_USERDATA), wParam))
			return 0;
		break;

	case WM_COMMAND:

		ctx = GetWndPtr(hwnd, GWLP_USERDATA);

		switch(LOWORD(wParam))
		{
		/* open/close frequency window */
		case RXWND_CMD_FREQWND:
			rxwnd_cmd_showfreqwnd(ctx);
			return 0;
		/* open/close channel window */
		case RXWND_CMD_CHANNELS:
			rxwnd_cmd_showchannelmgr(ctx);
			return 0;
		/* open/close settings window */
		case RXWND_CMD_SETTINGS:
			rxwnd_cmd_showsettingswnd(ctx);
			return 0;
		/* open/close volume control window */
		case RXWND_CMD_AUDCTRL:
			rxwnd_cmd_showaudctrlwnd(ctx);
			return 0;
		/* exit */
		case RXWND_CMD_EXIT:
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		/* unselect input */
		case RXWND_CMD_INPUT_UNSET:
			rxwnd_unsetinput(ctx);
			return 0;
		/* configure input */
		case RXWND_CMD_INPUT_CONFIG:
			rxwnd_showinputconfig(ctx);
			return 0;
		/* start/stop command */
		case RXWND_CMD_START:
			if(!ctx->rx->is_started)
				rxwnd_cmd_rxstart(ctx);
			else
				rxwnd_cmd_rxstop(ctx);
			return 0;
		/* restart command */
		case RXWND_CMD_RESTART:
			if(rxwnd_cmd_rxstop(ctx))
				rxwnd_cmd_rxstart(ctx);
			return 0;
		}

		/* select input */
		if( (LOWORD(wParam) >= RXWND_CMD_INPUT_0) &&
			(LOWORD(wParam) <= RXWND_CMD_INPUT_MAX) )
		{
			rxwnd_setinput(ctx, LOWORD(wParam) - RXWND_CMD_INPUT_0);
		}

		break;

	case WM_ACTIVATE:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(LOWORD(wParam))
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			ui_set_accel(ctx->uidata, hwnd, ctx->haccel);
			break;
		case WA_INACTIVE:
			ui_set_accel(ctx->uidata, hwnd, NULL);
			break;
		}
		return 0;

	case RXWND_USERMSG:

		ctx = GetWndPtr(hwnd, GWLP_USERDATA);

		switch(wParam)
		{
		case RXWND_USERMSG_RXPROC_ACT_CHANGE:
			rxwnd_rxproc_act_change(ctx, (unsigned int)lParam);
			break;
		}

		break;

	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/* ---------------------------------------------------------------------------------------------- */

/* create main window menu */
static void rxwnd_createmenu(rxwnd_ctx_t *ctx)
{
	ctx->hmenu_main = CreateMenu();

	ctx->hmenu_wnd = CreatePopupMenu();
	AppendMenu(ctx->hmenu_wnd, 0, RXWND_CMD_FREQWND, _T("&Frequency\tF2"));
	AppendMenu(ctx->hmenu_wnd, 0, RXWND_CMD_CHANNELS, _T("&Channel list\tF3"));
	AppendMenu(ctx->hmenu_wnd, 0, RXWND_CMD_AUDCTRL, _T("&Audio output\tF4"));
	AppendMenu(ctx->hmenu_wnd, 0, RXWND_CMD_SETTINGS, _T("&Settings\tF9"));
	AppendMenu(ctx->hmenu_wnd, MF_SEPARATOR, 0, NULL);
	AppendMenu(ctx->hmenu_wnd, 0, RXWND_CMD_EXIT, _T("E&xit\tF10"));
	AppendMenu(ctx->hmenu_main, MF_POPUP, (UINT_PTR)(ctx->hmenu_wnd), _T("&Window"));

	ctx->hmenu_ctrl = CreatePopupMenu();
	AppendMenu(ctx->hmenu_ctrl, 0, RXWND_CMD_START, _T("&Start\tF5"));
	AppendMenu(ctx->hmenu_ctrl, MF_GRAYED, RXWND_CMD_RESTART, _T("&Restart\tCtrl+F5"));
	AppendMenu(ctx->hmenu_main, MF_POPUP, (UINT_PTR)(ctx->hmenu_ctrl), _T("&Control"));
}

/* ---------------------------------------------------------------------------------------------- */

/* create main window */
HWND rxwnd_create(uicommon_t *uidata, ini_data_t *ini, ini_data_t *inich,
				  int n_show)
{
	rxwnd_ctx_t *ctx;
	HWND hwnd;
	int xpos, ypos, winw, winh;
	int sizestate;

	if( (ctx = calloc(1, sizeof(rxwnd_ctx_t))) == NULL )
		return NULL;

	ctx->uidata = uidata;
	ctx->ini = ini;
	ctx->inich = inich;

	ctx->event_rx_state = uievent_register(&(uidata->event_list), EVENT_NAME_RX_STATE);
	ctx->event_procchan = uievent_register(&(uidata->event_list), EVENT_NAME_PROCCHAN);
	ctx->event_visualcfg = uievent_register(&(uidata->event_list), EVENT_NAME_VISUALCFG);

	/* calculate window position */
	ui_frame_size(&(ctx->cx_frame), &(ctx->cy_frame),
		UI_FRAME_SIZING|UI_FRAME_CAPTION|UI_FRAME_MENU);

	ui_calcpos(ini_sect_get(ini, _T("window"), 0),
		UI_POS_ALL, ctx->cx_frame, ctx->cy_frame,
		RXWND_DEFCLTW, RXWND_DEFCLTH, RXWND_MINCLTW, RXWND_MINCLTH,
		&xpos, &ypos, &winw, &winh, &sizestate, n_show);

	/* create main menu */
	rxwnd_createmenu(ctx);

	/* create window */
	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW, MAKEINTATOM(rxwnd_classatom), ui_title, WS_OVERLAPPEDWINDOW,
		xpos, ypos, winw, winh, NULL, ctx->hmenu_main, uidata->h_inst, ctx);

	if(hwnd == NULL)
	{
		/* do not free anything */
		return NULL;
	}

	ShowWindow(hwnd, sizestate);

	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */

/* register main window class */
int rxwnd_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcl;

	if(rxwnd_classatom == 0)
	{
		memset(&wcl, 0, sizeof(wcl));
		wcl.cbSize = sizeof(wcl);
		wcl.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
		wcl.lpfnWndProc = rxwnd_proc;
		wcl.hInstance = uidata->h_inst;
		wcl.hIcon = uidata->hicn_main;
		wcl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcl.lpszClassName = rxwnd_classname;
		wcl.hIconSm = uidata->hicn_sm;

		rxwnd_classatom = RegisterClassEx(&wcl);
	}

	return (rxwnd_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

/* unregister main window class */
void rxwnd_unregisterclass(HINSTANCE h_inst)
{
	if(rxwnd_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(rxwnd_classatom), h_inst);
		rxwnd_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
