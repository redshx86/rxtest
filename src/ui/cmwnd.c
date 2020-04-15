/* ---------------------------------------------------------------------------------------------- */

#include "cmwnd.h"
#include "../../res/resource.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *cmwnd_classname = _T("rxtest_channels");
static TCHAR *cmwnd_wndname = _T("Channel list");

static ATOM cmwnd_classatom;

/* ---------------------------------------------------------------------------------------------- */

/* frequency adjustment step */
static double cmwnd_freqadjstep[] =
{
	1000.0,
	2500.0,
	3125.0,
	5000.0,
	6250.0,
	10000.0,
	12500.0,
	20000.0,
	25000.0,
	50000.0,
	100000.0
};

/* ---------------------------------------------------------------------------------------------- */

/* audio output mode button titles */
static TCHAR *cmwnd_outmode_btn_text[] =
{
	_T("M"),	/* SNDMIX_INPUT_CHAN_NONE */
	_T("L"),	/* SNDMIX_INPUT_CHAN_A */
	_T("R"),	/* SNDMIX_INPUT_CHAN_B */
	_T("B")		/* SNDMIX_INPUT_CHAN_AB */
};

/* ---------------------------------------------------------------------------------------------- */

/* channel list column width */
static cmwnd_clcol_cx_t cmwnd_clcol_cx_data[CMWND_CLCOL_COUNT] =
{
	/*	def		min		max		add		name */
	{	45,		30,		75,		28,		_T("cx_name") },	/* CMWND_CLCOL_NAME */
	{	65,		55,		90,		0,		_T("cx_freq") },	/* CMWND_CLCOL_FREQ */
	{	35,		20,		40,		12,		_T("cx_filter") },	/* CMWND_CLCOL_FILTER */
	{	20,		20,		25,		24,		_T("cx_demod") },	/* CMWND_CLCOL_DEMOD */
	{	45,		30,		60,		12,		_T("cx_squelch") },	/* CMWND_CLCOL_SQUELCH */
	{	20,		20,		35,		12,		_T("cx_audio")  },	/* CMWND_CLCOL_AUDIO */
	{	70,		40,		170,	0,		_T("cx_level")  }	/* CMWND_CLCOL_LEVEL */
};

/* ---------------------------------------------------------------------------------------------- */
/* Channel events broadcasting subroutines  */

/* reset and prepare affected channel list */
static int cmwnd_chanevent_begin(cmwnd_ctx_t *ctx, int maxcount)
{
	unsigned int *newitems;

	ctx->chan_notify_list.count = 0;

	if(maxcount > ctx->chan_notify_list_capacity)
	{
		maxcount += 32; /* some allocation hysteresis */
		if( (newitems = realloc(ctx->chan_notify_list.items, sizeof(int) * maxcount)) == NULL )
			return 0;
		ctx->chan_notify_list_capacity = maxcount;
		ctx->chan_notify_list.items = newitems;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* append identifier to affected channel list */
static int cmwnd_chanevent_addchan(cmwnd_ctx_t *ctx, unsigned int chid)
{
	if(ctx->chan_notify_list.count == ctx->chan_notify_list_capacity)
		return 0;

	ctx->chan_notify_list.items[ctx->chan_notify_list.count++] = chid;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* broadcast notification about channel event (using affected channel list) */
static void cmwnd_chanevent_notify(cmwnd_ctx_t *ctx, unsigned int code)
{
	if(ctx->chan_notify_list.count != 0)
		uievent_send(ctx->event_procchan, code, &(ctx->chan_notify_list));
}

/* ---------------------------------------------------------------------------------------------- */

/* broadcast notification about channel event (for single channel) */
static void cmwnd_chanevent_notifysingle(cmwnd_ctx_t *ctx, unsigned int code, unsigned int chid)
{
	notify_proc_list_t channel;

	channel.count = 1;
	channel.items = &chid;
	uievent_send(ctx->event_procchan, code, &channel);
}

/* ---------------------------------------------------------------------------------------------- */
/* Channel list utility subroutines */

/* get rowdata structure associated with specified row */
static cmwnd_rowdata_t *cmwnd_cl_getrowdata(cmwnd_ctx_t *ctx, int row)
{
	LVITEM lvi;

	if(row < 0)
		return NULL;

	memset(&lvi, 0, sizeof(lvi));
	lvi.mask = LVIF_PARAM;
	lvi.iItem = row;

	if( !ListView_GetItem(ctx->hwndChList, &lvi) )
		return NULL;

	return (void*)(lvi.lParam);
}

/* ---------------------------------------------------------------------------------------------- */

/* get rowdata structure and channel associated with specified row */
static int cmwnd_cl_getrowchannel(cmwnd_ctx_t *ctx, int row,
								  cmwnd_rowdata_t **prowdata, rxproc_t **pproc)
{
	cmwnd_rowdata_t *rowdata;
	rxproc_t *proc;

	if( ((rowdata = cmwnd_cl_getrowdata(ctx, row)) != NULL) &&
		((proc = rx_proc_find(ctx->rx, rowdata->chid)) != NULL) )
	{
		if(prowdata != NULL)
			*prowdata = rowdata;
		if(pproc != NULL)
			*pproc = proc;
		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* get single selected row index */
static int cmwnd_cl_getselectedrowindex(cmwnd_ctx_t *ctx)
{
	int n, i;

	/* make sure single row selected */
	if(ListView_GetSelectedCount(ctx->hwndChList) != 1)
		return -1;

	/* find selected row index */
	n = ListView_GetItemCount(ctx->hwndChList);
	for(i = 0; i < n; i++) {
		if(ListView_GetItemState(ctx->hwndChList, i, LVIS_SELECTED))
			return i;
	}

	return -1;
}

/* ---------------------------------------------------------------------------------------------- */

/* preallocate rowlist to specified capacity */
static int cmwnd_cl_rowlist_alloc(cmwnd_ctx_t *ctx, int maxcount)
{
	int *newbuf;

	if(maxcount > ctx->rowlist.capacity)
	{
		maxcount += 32; /* some allocation hysteresis */
		if( (newbuf = realloc(ctx->rowlist.row, sizeof(int) * maxcount)) == NULL )
			return 0;
		ctx->rowlist.capacity = maxcount;
		ctx->rowlist.row = newbuf;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* put all row indices to rowlist */
static int cmwnd_cl_rowlist_getallrows(cmwnd_ctx_t *ctx)
{
	int rowcount, i;

	rowcount = ListView_GetItemCount(ctx->hwndChList);

	if(!cmwnd_cl_rowlist_alloc(ctx, rowcount))
		return 0;

	ctx->rowlist.count = rowcount;
	for(i = 0; i < rowcount; i++)
		ctx->rowlist.row[i] = i;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* put selected row indices to rowlist */
static int cmwnd_cl_rowlist_getselectedrows(cmwnd_ctx_t *ctx)
{
	int rowcount, i;

	rowcount = ListView_GetItemCount(ctx->hwndChList);

	if(!cmwnd_cl_rowlist_alloc(ctx, rowcount))
		return 0;

	ctx->rowlist.count = 0;
	for(i = 0; i < rowcount; i++) {
		if(ListView_GetItemState(ctx->hwndChList, i, LVIS_SELECTED))
			ctx->rowlist.row[ctx->rowlist.count++] = i;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* find row index by associated channel id */
static int cmwnd_cl_getrowindexbychan(cmwnd_ctx_t *ctx, unsigned int chid,
									  cmwnd_rowdata_t **prowdata)
{
	int i, n;
	cmwnd_rowdata_t *rowdata;

	n = ListView_GetItemCount(ctx->hwndChList);
	for(i = 0; i < n; i++)
	{
		if( ((rowdata = cmwnd_cl_getrowdata(ctx, i)) != NULL) &&
			(rowdata->chid == chid) )
		{
			*prowdata = rowdata;
			return i;
		}
	}

	*prowdata = NULL;
	return -1;
}

/* ---------------------------------------------------------------------------------------------- */
/* Channel list inline edit box handling */

/* save edited channel name */
static int cmwnd_cl_edit_savename(cmwnd_ctx_t *ctx, TCHAR *str, rxproc_t *proc, int errmsg)
{
	/* check name really changed */
	if(_tcscmp(str, proc->cfg.name) == 0)
		return 1;

	/* set new channel name */
	if(!rxproc_set_name(proc, str))
	{
		if(errmsg) {
			_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
				_T("Can't rename channel '%s' to '%s'."), proc->cfg.name, str);
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
		}
		return 0;
	}

	/* broadcast notification */
	cmwnd_chanevent_notifysingle(ctx, EVENT_PROCCHAN_NAME, proc->chid);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* change channel frequency and update cell data */
static int cmwnd_setchanfreq(cmwnd_ctx_t *ctx, rxproc_t *proc, double fc)
{
	/* check frequency actually changed */
	if(fabs(fc - proc->cfg.fc) < 1e-3)
		return 1;

	/* set new frequency */
	rxproc_set_fc(proc, fc);

	/* broadcast notification */
	cmwnd_chanevent_notifysingle(ctx, EVENT_PROCCHAN_FREQ, proc->chid);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* save edited channel frequency */
static int cmwnd_cl_edit_savefreq(cmwnd_ctx_t *ctx, TCHAR *str, rxproc_t *proc, int errmsg)
{
	double freq;

	/* parse channel frequency */
	if(!parse_dbl(str, 1, &freq, NULL))
	{
		if(errmsg)
		{
			ui_msg_double_format(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
				_T("Entered channel frequency"));
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			SetFocus(ctx->hwndEdit);
		}
		return 0;
	}

	/* check entered channel frequency value */
	if(!INRANGE_MINMAX(freq, RXLIM_FC))
	{
		if(errmsg)
		{
			ui_msg_double_range(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
				_T("Entered channel frequency"), _T(" Hz"), 3, 3, 0, 
				RXLIM_FC_MIN, RXLIM_FC_MAX);
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			SetFocus(ctx->hwndEdit);
		}
		return 0;
	}

	/* set new frequency, set cell value, send notifications */
	if(!cmwnd_setchanfreq(ctx, proc, freq))
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* save edited filter cutoff frequency */
static int cmwnd_cl_edit_savefiltercutoff(cmwnd_ctx_t *ctx, TCHAR *str, rxproc_t *proc, int errmsg)
{
	double fc;

	/* parse filter cutoff frequency */
	if(!parse_dbl(str, 1, &fc, NULL))
	{
		if(errmsg)
		{
			ui_msg_double_format(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
				_T("Entered filter cutoff frequency"));
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			SetFocus(ctx->hwndEdit);
		}
		return 0;
	}

	/* check entered channel frequency value */
	if(!INRANGE_MINMAX(fc, RXLIM_PROC_FILTER_FC))
	{
		if(errmsg)
		{
			ui_msg_double_range(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
				_T("Entered filter cutoff frequency"), _T(" Hz"), 0, 3, 0,
				RXLIM_PROC_FILTER_FC_MIN, RXLIM_PROC_FILTER_FC_MAX);
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			SetFocus(ctx->hwndEdit);
		}
		return 0;
	}

	/* check filter cutoff frequency changed */
	if(fabs(fc - proc->cfg.filter_fc) < 1e-3)
		return 1;

	/* set new frequency */
	if( !rxproc_set_filter_params(proc, fc, proc->cfg.filter_df,
		proc->cfg.filter_as, ctx->uidata->msgbuf, ctx->uidata->msgbuf_size) )
	{
		if(errmsg)
		{
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			SetFocus(ctx->hwndEdit);
		}
		return 0;
	}

	/* broadcast notification */
	cmwnd_chanevent_notifysingle(ctx, EVENT_PROCCHAN_FILTERCFG, proc->chid);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* save edited output gain */
static int cmwnd_cl_edit_saveoutputgain(cmwnd_ctx_t *ctx, TCHAR *str, rxproc_t *proc, int errmsg)
{
	double gain;

	/* parse entered output gain */
	if(!parse_dbl(str, 0, &gain, NULL))
	{
		if(errmsg)
		{
			ui_msg_double_format(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
				_T("Entered output gain"));
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			SetFocus(ctx->hwndEdit);
		}
		return 0;
	}

	/* check entered channel frequency value */
	if(!INRANGE_MINMAX(gain, RXLIM_AUDIO_GAIN))
	{
		if(errmsg)
		{
			ui_msg_double_range(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
				_T("Entered output gain"), _T(" dB"), 0, 6, 0,
				RXLIM_AUDIO_GAIN_MIN, RXLIM_AUDIO_GAIN_MAX);
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			SetFocus(ctx->hwndEdit);
		}
		return 0;
	}

	/* check output gain is changed */
	if(fabs(gain - proc->cfg.output_gain) < 1e-6)
		return 1;

	/* set new gain */
	rxproc_set_output_gain(proc, gain);

	/* broadcast notification */
	cmwnd_chanevent_notifysingle(ctx, EVENT_PROCCHAN_OUTPUTCFG, proc->chid);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* close editing control without saving changes */
static void cmwnd_cl_edit_close(cmwnd_ctx_t *ctx)
{
	/* check control now open */
	if(ctx->hwndEdit != NULL)
	{
		/* destroy control */
		DestroyWindow(ctx->hwndEdit);
		ctx->hwndEdit = NULL;

		/* redraw channel list background rectangle */
		InvalidateRect(ctx->hwndChList, &(ctx->rcEdit), TRUE);
		UpdateWindow(ctx->hwndChList);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* save changes and close control if data entered correctly */
static int cmwnd_cl_edit_end(cmwnd_ctx_t *ctx, int errmsg)
{
	rxproc_t *proc;
	int status = 1;

	/* check control now open */
	if(ctx->hwndEdit != NULL)
	{
		Edit_GetText(ctx->hwndEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));

		if(_tcscmp(ctx->uidata->databuf, _T("")) != 0)
		{
			/* get row associated channel */
			if(cmwnd_cl_getrowchannel(ctx, ctx->editRow, NULL, &proc))
			{
				switch(ctx->editCol)
				{
				/* save edited channel name */
				case CMWND_CLCOL_NAME:
					status = cmwnd_cl_edit_savename(
						ctx, ctx->uidata->databuf, proc, errmsg);
					break;

				/* save edited frequency */
				case CMWND_CLCOL_FREQ:
					status = cmwnd_cl_edit_savefreq(
						ctx, ctx->uidata->databuf, proc, errmsg);
					break;

				/* save edited filter bandwidth */
				case CMWND_CLCOL_FILTER:
					status = cmwnd_cl_edit_savefiltercutoff(
						ctx, ctx->uidata->databuf, proc, errmsg);
					break;

				/* save edited filter bandwidth */
				case CMWND_CLCOL_AUDIO:
					status = cmwnd_cl_edit_saveoutputgain(
						ctx, ctx->uidata->databuf, proc, errmsg);
					break;
				}
			}
		}

		/* close edit control */
		if( status || (!errmsg) )
			cmwnd_cl_edit_close(ctx);
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* get bounding rectangle for cell editing control */
static void cmwnd_cl_edit_getboundrect(cmwnd_ctx_t *ctx, int row, int col, RECT *prect)
{
	/* get cell bounds */
	if(col == 0) {
		ListView_GetItemRect(ctx->hwndChList, row, prect, LVIR_LABEL);
	} else {
		ListView_GetSubItemRect(ctx->hwndChList, row, col, LVIR_BOUNDS, prect);
	}

	/* adjust bounding rectangle for row controls */
	prect->right -= cmwnd_clcol_cx_data[col].cx_add;

	if(col == CMWND_CLCOL_NAME)
		prect->right += 16;
}

/* ---------------------------------------------------------------------------------------------- */

/* channel list edit box message proc */
static LRESULT CALLBACK cmwnd_cl_edit_editboxproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	cmwnd_ctx_t *stateCmwin;

	stateCmwin = GetWndPtr(hwnd, GWLP_USERDATA);

	switch(uMsg)
	{
	case WM_KEYDOWN:
		switch(wParam)
		{
		/* cancel editing on escape */
		case VK_ESCAPE:
			cmwnd_cl_edit_close(stateCmwin);
			return 0;

		/* done editing on return */
		case VK_RETURN:
			cmwnd_cl_edit_end(stateCmwin, 1);
			return 0;
		}
		break;
	}

	return CallWindowProc(stateCmwin->wndprocEdit, hwnd, uMsg, wParam, lParam);
}

/* ---------------------------------------------------------------------------------------------- */

/* create edit box at channel list cell */
static int cmwnd_cl_edit_createeditbox(cmwnd_ctx_t *ctx, TCHAR *text, int row, int col)
{
	/* save cell coordinates */
	ctx->editRow = row;
	ctx->editCol = col;

	/* get control bounding rectangle */
	cmwnd_cl_edit_getboundrect(ctx, row, col, &(ctx->rcEdit));

	if( (ctx->rcEdit.right - ctx->rcEdit.left < 12) ||
		(ctx->rcEdit.bottom - ctx->rcEdit.top < 6) )
	{
		return 0;
	}

	/* create edit control */
	ctx->hwndEdit = CreateWindow(_T("EDIT"), text,
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|ES_AUTOHSCROLL,
		ctx->rcEdit.left, ctx->rcEdit.top,
		ctx->rcEdit.right - ctx->rcEdit.left, ctx->rcEdit.bottom - ctx->rcEdit.top,
		ctx->hwndChList, (HMENU)CMWND_ID_CHLIST_EDIT, ctx->uidata->h_inst, NULL);

	if(ctx->hwndEdit == NULL)
		return 0;

	/* hook edit box window proc */
	SetWndPtr(ctx->hwndEdit, GWLP_USERDATA, ctx);

	ctx->wndprocEdit = (WNDPROC)SetWndPtr(ctx->hwndEdit,
		GWLP_WNDPROC, (void*)cmwnd_cl_edit_editboxproc);

	/* setup edit box */
	SetWindowFont(ctx->hwndEdit, ctx->uidata->hfnt_main, FALSE);
	Edit_SetSel(ctx->hwndEdit, 0, -1);
	SetFocus(ctx->hwndEdit);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* open channel list editing control */
static void cmwnd_cl_edit_startedit(cmwnd_ctx_t *ctx, int row, int col)
{
	rxproc_t *proc;

	/* check control not opened now and cell coordinates are correct */
	if( (ctx->hwndEdit == NULL) && (row >= 0) && (col < CMWND_CLCOL_COUNT) )
	{
		/* get row related channel */
		if( cmwnd_cl_getrowchannel(ctx, row, NULL, &proc) )
		{
			switch(col)
			{
			/* open channel name edit box */
			case CMWND_CLCOL_NAME:
				cmwnd_cl_edit_createeditbox(ctx, proc->cfg.name, row, CMWND_CLCOL_NAME);
				break;

			/* open channel frequency edit box */
			case CMWND_CLCOL_FREQ:
				fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size,
					proc->cfg.fc, ctx->dispUnitChanFreq,
					3, (ctx->dispUnitChanFreq >= 2) ? 3 : 0);
				cmwnd_cl_edit_createeditbox(ctx, ctx->uidata->databuf, row, CMWND_CLCOL_FREQ);
				break;

			/* open filter bandwidth edit box */
			case CMWND_CLCOL_FILTER:
				fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size,
					proc->cfg.filter_fc, ctx->dispUnitFiltCutoff, 3, 0);
				cmwnd_cl_edit_createeditbox(ctx, ctx->uidata->databuf, row, CMWND_CLCOL_FILTER);
				break;

			/* open output gain edit box */
			case CMWND_CLCOL_AUDIO:
				fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size,
					proc->cfg.output_gain, 0, 6, 1);
				cmwnd_cl_edit_createeditbox(ctx, ctx->uidata->databuf, row, CMWND_CLCOL_AUDIO);
				break;
			}
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* check/update channel editing control position */
static void cmwnd_cl_edit_updatepos(cmwnd_ctx_t *ctx)
{
	RECT rc;

	if(ctx->hwndEdit != NULL)
	{
		/* update edit position */
		cmwnd_cl_edit_getboundrect(ctx, ctx->editRow, ctx->editCol, &rc);
		ui_updatewndrect(ctx->hwndEdit, &(ctx->rcEdit), &rc, UI_SWR_REDRAWNOERASE);
	}
}

/* ---------------------------------------------------------------------------------------------- */
/* Channel list row inline controls handling */

/* find free base Id for control group */
static UINT cmwnd_cl_rowctls_getfreebaseid(cmwnd_ctx_t *ctx)
{
	cmwnd_rowdata_t *rowdata;
	UINT uIdBase;
	int n, i;
	int success;

	uIdBase = CMWND_ID_ROWCTL_0;
	n = ListView_GetItemCount(ctx->hwndChList);

	do
	{
		success = 1;

		for(i = 0; i < n; i++)
		{
			if( (rowdata = cmwnd_cl_getrowdata(ctx, i)) != NULL )
			{
				if(uIdBase == rowdata->uIdBase)
				{
					uIdBase += CMWND_ROWCTLS_NUM_IDS;
					success = 0;
					break;
				}
			}
		}

	} while(!success);

	return uIdBase;
}

/* ---------------------------------------------------------------------------------------------- */

/* get bounding rectangles for controls */
static void cmwnd_cl_rowctls_getpos(cmwnd_ctx_t *ctx, int row,
									RECT *prcBtnOptions, RECT *prcBtnFilterCfg,
									RECT *prcBtnDemodType, RECT *prcBtnDemodCfg,
									RECT *prcBtnSqlCfg, RECT *prcBtnOutMode,
									RECT *prcLevelBar)
{
	RECT rc;

	/* options button */
	ListView_GetItemRect(ctx->hwndChList, row, prcBtnOptions, LVIR_LABEL);
	prcBtnOptions->left = prcBtnOptions->right - 12;

	/* filter adjust buttons */
	ListView_GetSubItemRect(ctx->hwndChList, row, CMWND_CLCOL_FILTER, LVIR_BOUNDS, &rc);
	prcBtnFilterCfg->left = rc.right - 12;
	prcBtnFilterCfg->right = rc.right;
	prcBtnFilterCfg->top = rc.top;
	prcBtnFilterCfg->bottom = rc.bottom;

	/* demodulator type and config buttons */
	ListView_GetSubItemRect(ctx->hwndChList, row, CMWND_CLCOL_DEMOD, LVIR_BOUNDS, &rc);
	prcBtnDemodType->top = prcBtnDemodCfg->top = rc.top;
	prcBtnDemodType->bottom = prcBtnDemodCfg->bottom = rc.bottom;
	prcBtnDemodType->left = rc.right - 24;
	prcBtnDemodType->right = prcBtnDemodCfg->left = rc.right - 12;
	prcBtnDemodCfg->right = rc.right;

	/* demodulator type and config buttons */
	ListView_GetSubItemRect(ctx->hwndChList, row, CMWND_CLCOL_SQUELCH, LVIR_BOUNDS, &rc);
	prcBtnSqlCfg->left = rc.right - 12;
	prcBtnSqlCfg->right = rc.right;
	prcBtnSqlCfg->top = rc.top;
	prcBtnSqlCfg->bottom = rc.bottom;

	/* output selection button */
	ListView_GetSubItemRect(ctx->hwndChList, row, CMWND_CLCOL_AUDIO, LVIR_BOUNDS, prcBtnOutMode);
	prcBtnOutMode->left = prcBtnOutMode->right - 12;

	/* level meter */
	ListView_GetSubItemRect(ctx->hwndChList, row, CMWND_CLCOL_LEVEL, LVIR_BOUNDS, prcLevelBar);
	prcLevelBar->left += 1;
	prcLevelBar->bottom -= 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* check and update control positions */
static void cmwnd_cl_rowctls_updatepos(cmwnd_ctx_t *ctx, cmwnd_rowdata_t *rowdata, int row)
{
	RECT rcBtnOptions, rcBtnFilterCfg;
	RECT rcBtnDemodType, rcBtnDemodCfg;
	RECT rcBtnSqlCfg, rcBtnOutMode;
	RECT rcLevelBar;

	/* get correct position rectangles */
	cmwnd_cl_rowctls_getpos(ctx, row,
		&rcBtnOptions, &rcBtnFilterCfg,
		&rcBtnDemodType, &rcBtnDemodCfg,
		&rcBtnSqlCfg, &rcBtnOutMode,
		&rcLevelBar);

	ui_updatewndrect(rowdata->hwndBtnOptions, &(rowdata->rcBtnOptions),
		&rcBtnOptions, UI_SWR_REDRAWNOERASE);
	ui_updatewndrect(rowdata->hwndBtnFilterCfg, &(rowdata->rcBtnFilterCfg),
		&rcBtnFilterCfg, UI_SWR_REDRAWNOERASE);
	ui_updatewndrect(rowdata->hwndBtnDemodType, &(rowdata->rcBtnDemodType),
		&rcBtnDemodType, UI_SWR_REDRAWNOERASE);
	ui_updatewndrect(rowdata->hwndBtnDemodCfg, &(rowdata->rcBtnDemodCfg),
		&rcBtnDemodCfg, UI_SWR_REDRAWNOERASE);
	ui_updatewndrect(rowdata->hwndBtnSqlCfg, &(rowdata->rcBtnSqlCfg),
		&rcBtnSqlCfg, UI_SWR_REDRAWNOERASE);
	ui_updatewndrect(rowdata->hwndBtnOutMode, &(rowdata->rcBtnOutMode),
		&rcBtnOutMode, UI_SWR_REDRAWNOERASE);
	ui_updatewndrect(rowdata->hwndLevelBar, &(rowdata->rcLevelBar),
		&rcLevelBar, UI_SWR_REDRAWNOERASE);
}

/* ---------------------------------------------------------------------------------------------- */

/* check and update control positions in all rows */
static void cmwnd_cl_rowctls_updatepos_allrows(cmwnd_ctx_t *ctx)
{
	int n, i;
	cmwnd_rowdata_t *rowdata;

	n = ListView_GetItemCount(ctx->hwndChList);
	for(i = 0; i < n; i++) {
		if( (rowdata = cmwnd_cl_getrowdata(ctx, i)) != NULL )
			cmwnd_cl_rowctls_updatepos(ctx, rowdata, i);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* create row inline controls */
static void cmwnd_cl_rowctls_create(cmwnd_ctx_t *ctx, cmwnd_rowdata_t *rowdata, int row)
{
	/* get bounding rectangles for controls */
	cmwnd_cl_rowctls_getpos(ctx, row,
		&(rowdata->rcBtnOptions), &(rowdata->rcBtnFilterCfg),
		&(rowdata->rcBtnDemodType), &(rowdata->rcBtnDemodCfg),
		&(rowdata->rcBtnSqlCfg), &(rowdata->rcBtnOutMode),
		&(rowdata->rcLevelBar));

	/* create channel options button */
	rowdata->hwndBtnOptions = ui_crt_btn(
		ctx->uidata, ctx->hwndChList, WS_CLIPSIBLINGS,
		rowdata->rcBtnOptions.left, rowdata->rcBtnOptions.top,
		rowdata->rcBtnOptions.right - rowdata->rcBtnOptions.left,
		rowdata->rcBtnOptions.bottom - rowdata->rcBtnOptions.top,
		_T("..."), rowdata->uIdBase + CMWND_ID_ROWCTLS_OPTIONS);
	SetWindowFont(rowdata->hwndBtnOptions, ctx->uidata->hfnt_mini, FALSE);

	/* create filter config button */
	rowdata->hwndBtnFilterCfg = ui_crt_btn(
		ctx->uidata, ctx->hwndChList, WS_CLIPSIBLINGS,
		rowdata->rcBtnFilterCfg.left, rowdata->rcBtnFilterCfg.top,
		rowdata->rcBtnFilterCfg.right - rowdata->rcBtnFilterCfg.left,
		rowdata->rcBtnFilterCfg.bottom - rowdata->rcBtnFilterCfg.top,
		_T("..."), rowdata->uIdBase + CMWND_ID_ROWCTLS_FILTCFG);
	SetWindowFont(rowdata->hwndBtnFilterCfg, ctx->uidata->hfnt_mini, FALSE);

	/* create demodulator type selection button */
	rowdata->hwndBtnDemodType = ui_crt_btn(
		ctx->uidata, ctx->hwndChList, WS_CLIPSIBLINGS,
		rowdata->rcBtnDemodType.left, rowdata->rcBtnDemodType.top,
		rowdata->rcBtnDemodType.right - rowdata->rcBtnDemodType.left,
		rowdata->rcBtnDemodType.bottom - rowdata->rcBtnDemodType.top,
		_T("»"), rowdata->uIdBase + CMWND_ID_ROWCTLS_DEMODTYPE);
	SetWindowFont(rowdata->hwndBtnDemodType, ctx->uidata->hfnt_mini, FALSE);

	/* create demodulator config button */
	rowdata->hwndBtnDemodCfg = ui_crt_btn(
		ctx->uidata, ctx->hwndChList, WS_CLIPSIBLINGS,
		rowdata->rcBtnDemodCfg.left, rowdata->rcBtnDemodCfg.top,
		rowdata->rcBtnDemodCfg.right - rowdata->rcBtnDemodCfg.left,
		rowdata->rcBtnDemodCfg.bottom - rowdata->rcBtnDemodCfg.top,
		_T("..."), rowdata->uIdBase + CMWND_ID_ROWCTLS_DEMODCFG);
	SetWindowFont(rowdata->hwndBtnDemodCfg, ctx->uidata->hfnt_mini, FALSE);

	/* create squelch config button */
	rowdata->hwndBtnSqlCfg = ui_crt_btn(
		ctx->uidata, ctx->hwndChList, WS_CLIPSIBLINGS,
		rowdata->rcBtnSqlCfg.left, rowdata->rcBtnSqlCfg.top,
		rowdata->rcBtnSqlCfg.right - rowdata->rcBtnSqlCfg.left,
		rowdata->rcBtnSqlCfg.bottom - rowdata->rcBtnSqlCfg.top,
		_T("..."), rowdata->uIdBase + CMWND_ID_ROWCTLS_SQLCFG);
	SetWindowFont(rowdata->hwndBtnSqlCfg, ctx->uidata->hfnt_mini, FALSE);

	/* create output mode selection button */
	rowdata->hwndBtnOutMode = ui_crt_btn(
		ctx->uidata, ctx->hwndChList, WS_CLIPSIBLINGS,
		rowdata->rcBtnOutMode.left, rowdata->rcBtnOutMode.top,
		rowdata->rcBtnOutMode.right - rowdata->rcBtnOutMode.left,
		rowdata->rcBtnOutMode.bottom - rowdata->rcBtnOutMode.top,
		_T("?"), rowdata->uIdBase + CMWND_ID_ROWCTLS_OUTMODE);
	SetWindowFont(rowdata->hwndBtnOutMode, ctx->uidata->hfnt_mini, FALSE);

	/* create level meter */
	rowdata->hwndLevelBar = lbr_create(
		ctx->uidata->h_inst, WS_CLIPSIBLINGS, 
		ctx->hwndChList, rowdata->uIdBase + CMWND_ID_ROWCTLS_LEVELBAR,
		&(rowdata->rcLevelBar), ctx->uidata->hfnt_mini, 0);
	lbr_send_setrange(rowdata->hwndLevelBar, ctx->levelMin, ctx->levelMax, 0);
	lbr_send_setpos(rowdata->hwndLevelBar, ctx->levelMin, 0.0, 0);
}

/* ---------------------------------------------------------------------------------------------- */

/* destroy row inline controls */
static void cmwnd_cl_rowctls_destroy(cmwnd_rowdata_t *rowdata)
{
	DestroyWindow(rowdata->hwndBtnOptions);
	DestroyWindow(rowdata->hwndBtnFilterCfg);
	DestroyWindow(rowdata->hwndBtnDemodType);
	DestroyWindow(rowdata->hwndBtnDemodCfg);
	DestroyWindow(rowdata->hwndBtnSqlCfg);
	DestroyWindow(rowdata->hwndBtnOutMode);
	DestroyWindow(rowdata->hwndLevelBar);
}

/* ---------------------------------------------------------------------------------------------- */
/* Row controls command handing */

/* get frequency adjustment step */
static int cmwnd_getfreqadjstep(cmwnd_ctx_t *ctx, double *pstep)
{
	double step;

	if( !ui_get_double(ctx->uidata, ctx->hwnd, ctx->hwndComboFreqStep,
		&step, _T("Frequency adjust step"), 1, NULL) )
	{
		return 0;
	}

	if(step <= 0)
		return 0;

	*pstep = step;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* handle row inline channel misc options button */
static void cmwnd_cl_rowctls_cmd_showchanoptions(cmwnd_ctx_t *ctx, unsigned int chid)
{
	POINT pt;
	unsigned int *chidlist;

	if( (chidlist = malloc( sizeof(int) )) != NULL )
	{
		GetCursorPos(&pt);
		chidlist[0] = chid;
		chanopt_createwindow(ctx->uidata, ctx->event_procchan,
			ctx->hwnd, ctx->rx, ctx->procdefcfg, chidlist, 1, pt.x, pt.y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle row inline filter configuration button */
static void cmwnd_cl_rowctls_cmd_showfilterconfig(cmwnd_ctx_t *ctx, unsigned int chid)
{
	POINT pt;
	unsigned int *chidlist;

	if( (chidlist = malloc( sizeof(int) )) != NULL )
	{
		GetCursorPos(&pt);
		chidlist[0] = chid;
		filtcfg_createwindow(ctx->uidata, ctx->event_procchan,
			ctx->hwnd, ctx->rx, ctx->procdefcfg, chidlist, 1, pt.x, pt.y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmdwnd_showdemodconfig(cmwnd_ctx_t *ctx, rxproc_demodtype_t type,
								   unsigned int *chidlist, int chidcount, int x, int y)
{
	if((type < 0) || (type >= RXPROC_DEMOD_COUNT))
		return;

	switch(type)
	{
	case RXPROC_DEMOD_FM:
		fmcfg_createwindow(ctx->uidata, ctx->event_procchan,
			ctx->hwnd, ctx->rx, ctx->procdefcfg, chidlist, chidcount, x, y);
		break;

	default:
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("%s demodulator have no configurable parameters."),
			rxproc_demod_display_name[type]);
		MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONINFORMATION|MB_OK);
		free(chidlist);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle row inline filter configuration button */
static void cmwnd_cl_rowctls_cmd_showdemodconfig(cmwnd_ctx_t *ctx, unsigned int chid)
{
	POINT pt;
	unsigned int *chidlist;
	rxproc_t *proc;

	if( (proc = rx_proc_find(ctx->rx, chid)) != NULL )
	{
		if( (chidlist = malloc( sizeof(int) )) != NULL )
		{
			GetCursorPos(&pt);
			chidlist[0] = chid;
			cmdwnd_showdemodconfig(ctx, proc->cfg.demod_type, chidlist, 1, pt.x, pt.y);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle row inline squelch configuration button */
static void cmwnd_cl_rowctls_cmd_showsqlconfig(cmwnd_ctx_t *ctx, unsigned int chid)
{
	POINT pt;
	unsigned int *chidlist;

	if( (chidlist = malloc( sizeof(int) )) != NULL )
	{
		GetCursorPos(&pt);
		chidlist[0] = chid;
		sqlcfg_createwindow(ctx->uidata, ctx->event_procchan,
			ctx->hwnd, ctx->rx, ctx->procdefcfg, chidlist, 1, pt.x, pt.y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle row inline output mode button */
static void cmwnd_cl_rowctls_cmd_changeoptputmode(cmwnd_ctx_t *ctx, cmwnd_rowdata_t *rowdata)
{
	rxproc_t *proc;
	int output_mode;

	/* find channel */
	if( (proc = rx_proc_find(ctx->rx, rowdata->chid)) != NULL )
	{
		/* change channel output mode */
		output_mode = proc->cfg.output_mode + 1;
		if(output_mode > SNDMIX_INPUT_CHAN_AB)
			output_mode = SNDMIX_INPUT_CHAN_NONE;
		proc->cfg.output_mode = output_mode;

		/* broadcast notification */
		cmwnd_chanevent_notifysingle(ctx, EVENT_PROCCHAN_OUTPUTCFG, proc->chid);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* create demodulator type selection popup menu */
static void cmwnd_cl_createdemodpopupmenu(cmwnd_ctx_t *ctx)
{
	int i;

	ctx->hmenuDemodType = CreatePopupMenu();
	ctx->demodSelectRow = -1;

	for(i = 0; i < RXPROC_DEMOD_COUNT; i++) {
		AppendMenu(ctx->hmenuDemodType, 0,
			CMWND_ID_DEMODTYPE_0 + i, rxproc_demod_display_name[i]);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* handle row inline demodulator type selection button */
static void cmwnd_cl_rowctls_cmd_showdemodtypemenu(cmwnd_ctx_t *ctx, int row)
{
	POINT pt;

	ctx->demodSelectRow = row;

	GetCursorPos(&pt);
	TrackPopupMenu(ctx->hmenuDemodType, 0,
		pt.x, pt.y, 0, ctx->hwndChList, NULL);
}

/* ---------------------------------------------------------------------------------------------- */

/* handle demodulator type selection menu command */
static void cmwnd_cl_cmd_demodtypemenu(cmwnd_ctx_t *ctx, int demod_type)
{
	rxproc_t *proc;

	if(cmwnd_cl_getrowchannel(ctx, ctx->demodSelectRow, NULL, &proc))
	{
		/* check demodulator type really changed */
		if(demod_type == proc->cfg.demod_type)
			return;

		/* set new demodulator type */
		if(!rxproc_set_demod_type(proc, demod_type,
			ctx->uidata->msgbuf, ctx->uidata->msgbuf_size))
		{
			MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
			return;
		}

		/* broadcast notification */
		cmwnd_chanevent_notifysingle(ctx, EVENT_PROCCHAN_DEMODCFG, proc->chid);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* dispatch row control command */
static int cmwnd_cl_rowctls_command(cmwnd_ctx_t *ctx, UINT ucommand)
{
	UINT uCmdCtrlId;
	cmwnd_rowdata_t *rowdata, *tmprowdata;
	int n, row;

	/* find row by command id */
	rowdata = NULL;
	n = ListView_GetItemCount(ctx->hwndChList);
	for(row = 0; row < n; row++)
	{
		if( (tmprowdata = cmwnd_cl_getrowdata(ctx, row)) != NULL )
		{
			if( (ucommand >= tmprowdata->uIdBase) &&
				(ucommand < tmprowdata->uIdBase + CMWND_ROWCTLS_NUM_IDS) )
			{
				rowdata = tmprowdata;
				break;
			}
		}
	}

	if(rowdata != NULL)
	{
		/* control index */
		uCmdCtrlId = ucommand - rowdata->uIdBase;

		/* handle command */
		switch(uCmdCtrlId)
		{
		case CMWND_ID_ROWCTLS_OPTIONS:
			cmwnd_cl_rowctls_cmd_showchanoptions(ctx, rowdata->chid);
			break;
		case CMWND_ID_ROWCTLS_DEMODTYPE:
			cmwnd_cl_rowctls_cmd_showdemodtypemenu(ctx, row);
			break;
		case CMWND_ID_ROWCTLS_FILTCFG:
			cmwnd_cl_rowctls_cmd_showfilterconfig(ctx, rowdata->chid);
			break;
		case CMWND_ID_ROWCTLS_DEMODCFG:
			cmwnd_cl_rowctls_cmd_showdemodconfig(ctx, rowdata->chid);
			break;
		case CMWND_ID_ROWCTLS_SQLCFG:
			cmwnd_cl_rowctls_cmd_showsqlconfig(ctx, rowdata->chid);
			break;
		case CMWND_ID_ROWCTLS_OUTMODE:
			cmwnd_cl_rowctls_cmd_changeoptputmode(ctx, rowdata);
			break;
		}

		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
/* Channel list row handling */

/* insert row to channel list for exiting channel */
static int cmwnd_cl_insertrow(cmwnd_ctx_t *ctx, rxproc_t *proc, int row)
{
	LVITEM lvi;
	cmwnd_rowdata_t *rowdata;

	/* initialize row data */
	if( (rowdata = malloc(sizeof(cmwnd_rowdata_t))) == NULL )
		return 0;

	rowdata->chid = proc->chid;
	rowdata->uIdBase = cmwnd_cl_rowctls_getfreebaseid(ctx);

	rowdata->status_icon_cur = rxproc_status_query(proc);

	/* insert row to channel list */
	memset(&lvi, 0, sizeof(lvi));
	lvi.iItem = row;

	lvi.mask = LVIF_TEXT|LVIF_PARAM|LVIF_IMAGE;
	lvi.pszText = proc->cfg.name;
	lvi.lParam = (LPARAM)rowdata;
	lvi.iImage = ctx->cl_imglist_data.img_proc_status[rowdata->status_icon_cur];
	ListView_InsertItem(ctx->hwndChList, &lvi);

	fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size,
		proc->cfg.fc, ctx->dispUnitChanFreq,
		3, (ctx->dispUnitChanFreq >= 2) ? 3 : 0);
	ListView_SetItemText(ctx->hwndChList, row, CMWND_CLCOL_FREQ, ctx->uidata->databuf);

	fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size,
		proc->cfg.filter_fc, ctx->dispUnitFiltCutoff, 3, 0);
	ListView_SetItemText(ctx->hwndChList, row, CMWND_CLCOL_FILTER, ctx->uidata->databuf);

	ListView_SetItemText(ctx->hwndChList, row, CMWND_CLCOL_DEMOD,
		rxproc_demod_display_name[proc->cfg.demod_type]);

	fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, proc->cfg.output_gain, 0, 6, 1);
	ListView_SetItemText(ctx->hwndChList, row, CMWND_CLCOL_AUDIO, ctx->uidata->databuf);

	/* create row controls */
	cmwnd_cl_rowctls_create(ctx, rowdata, row);

	/* set controls data */
	Button_SetText(rowdata->hwndBtnOutMode, cmwnd_outmode_btn_text[proc->cfg.output_mode]);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* insert rows for all existing channels (window init or channel list reloading) */
static void cmwnd_cl_fillrows(cmwnd_ctx_t *ctx)
{
	int i;
	rxproc_t *proc;

	i = 0;
	for(proc = ctx->rx->proc_first; proc != NULL; proc = proc->proc_next)
		cmwnd_cl_insertrow(ctx, proc, i++);
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_levelmtr_update(cmwnd_ctx_t *ctx, int row,
									 cmwnd_rowdata_t *rowdata, rxproc_t *proc,
									 DWORD tick)
{
	double level, gain;
	rxproc_status_t status;

	status = rxproc_status_query(proc);

	if((status != RXPROC_SQLOPENED) && (status != RXPROC_SQLCLOSED))
	{
		lbr_send_setpos(rowdata->hwndLevelBar, ctx->levelMin, 0.0, 1);
		return;
	}

	if(rxproc_read_level(proc, tick, &level))
	{
		if(ctx->levelScaleToInputFc)
		{
			gain = sqrt((double)(proc->fs_input) / (double)(2.0 * proc->cfg.filter_fc));
			level *= gain;
		}

		lbr_send_setpos(rowdata->hwndLevelBar, LINM2DB(level + DBL_MIN), 0.0, 1);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_levelmtr_updateall(cmwnd_ctx_t *ctx)
{
	int i, n;
	cmwnd_rowdata_t *rowdata;
	rxproc_t *proc;
	DWORD tick;

	if(!ctx->levelIsStarted)
		return;

	tick = GetTickCount();

	n = ListView_GetItemCount(ctx->hwndChList);
	for(i = 0; i < n; i++)
	{
		if(cmwnd_cl_getrowchannel(ctx, i, &rowdata, &proc)) {
			cmwnd_cl_levelmtr_update(ctx, i, rowdata, proc, tick);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_levelmtr_start(cmwnd_ctx_t *ctx)
{
	if((ctx->rx->is_started) && (!ctx->levelIsStarted))
	{
		SetTimer(ctx->hwnd, CMWND_IDT_UPDATELEVEL, ctx->levelUpdateInt, NULL);
		ctx->levelIsStarted = 1;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_levelmtr_stop(cmwnd_ctx_t *ctx)
{
	int i, n;
	cmwnd_rowdata_t *rowdata;

	if(ctx->levelIsStarted)
	{
		n = ListView_GetItemCount(ctx->hwndChList);
		for(i = 0; i < n; i++)
		{
			if( (rowdata = cmwnd_cl_getrowdata(ctx, i)) != NULL )
				lbr_send_setpos(rowdata->hwndLevelBar, ctx->levelMin, 0.0, 1);
		}

		KillTimer(ctx->hwnd, CMWND_IDT_UPDATELEVEL);
		ctx->levelIsStarted = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_levelmtr_init(cmwnd_ctx_t *ctx)
{
	ini_sect_t *sect;
	double levelMin, levelMax;

	ctx->levelMin = -40.0;
	ctx->levelMax = 0.0;
	ctx->levelUpdateInt = 100;
	ctx->levelScaleToInputFc = 0;

	if( (sect = ini_sect_get(ctx->ini, _T("cmwnd"), 0)) != NULL )
	{
		levelMin = ctx->levelMin;
		levelMax = ctx->levelMax;

		ini_getfr(sect, _T("level_min"), &levelMin, CMCFG_LEVEL_ABS_MIN, CMCFG_LEVEL_ABS_MAX);
		ini_getfr(sect, _T("level_max"), &levelMax, CMCFG_LEVEL_ABS_MIN, CMCFG_LEVEL_ABS_MAX);

		if(levelMin < levelMax)
		{
			ctx->levelMin = levelMin;
			ctx->levelMax = levelMax;
		}

		ini_getur(sect, _T("level_upd"), &(ctx->levelUpdateInt),
			CMCFG_LEVEL_UPDATE_INT_MIN, CMCFG_LEVEL_UPDATE_INT_MAX);
		ini_getb(sect, _T("level_scale"), &(ctx->levelScaleToInputFc));
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_levelmtr_cleanup(cmwnd_ctx_t *ctx)
{
	ini_sect_t *sect;

	sect = ini_sect_get(ctx->ini, _T("cmwnd"), 1);
	ini_setf(sect, _T("level_min"), 6, ctx->levelMin);
	ini_setf(sect, _T("level_max"), 6, ctx->levelMax);
	ini_setu(sect, _T("level_upd"), ctx->levelUpdateInt);
	ini_setb(sect, _T("level_scale"), ctx->levelScaleToInputFc);
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_levelmtr_setrange(cmwnd_ctx_t *ctx, double levelMin, double levelMax)
{
	int i, num;
	cmwnd_rowdata_t *rowdata;

	if( (fabs(levelMin - ctx->levelMin) >= 1e-6) ||
		(fabs(levelMax - ctx->levelMax) >= 1e-6) )
	{
		num = ListView_GetItemCount(ctx->hwndChList);
		for(i = 0; i < num; i++)
		{
			if( (rowdata = cmwnd_cl_getrowdata(ctx, i)) != NULL )
			{
				lbr_send_setrange(rowdata->hwndLevelBar, levelMin, levelMax, 1);
				lbr_send_setpos(rowdata->hwndLevelBar, levelMin, 0.0, 1);
			}
		}

		ctx->levelMin = levelMin;
		ctx->levelMax = levelMax;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_levelmtr_setinterval(cmwnd_ctx_t *ctx, unsigned int interval)
{
	if(interval != ctx->levelUpdateInt)
	{
		if(ctx->levelIsStarted)
		{
			KillTimer(ctx->hwnd, CMWND_IDT_UPDATELEVEL);
			SetTimer(ctx->hwnd, CMWND_IDT_UPDATELEVEL, interval, NULL);
		}

		ctx->levelUpdateInt = interval;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_updatestatusicon(cmwnd_ctx_t *ctx, int row,
									  cmwnd_rowdata_t *rowdata, rxproc_t *proc)
{
	rxproc_status_t status;
	LVITEM lvi;

	status = rxproc_status_query(proc);

	if(status != rowdata->status_icon_cur)
	{
		memset(&lvi, 0, sizeof(lvi));
		lvi.mask = LVIF_IMAGE;
		lvi.iItem = row;
		lvi.iImage = ctx->cl_imglist_data.img_proc_status[status];
		ListView_SetItem(ctx->hwndChList, &lvi);

		rowdata->status_icon_cur = status;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int cmwnd_cl_updateallstatusicons(cmwnd_ctx_t *ctx)
{
	int i, status = 1;
	cmwnd_rowdata_t *rowdata;
	rxproc_t *proc;

	if(!cmwnd_cl_rowlist_getallrows(ctx))
		return 0;

	for(i = 0; i < ctx->rowlist.count; i++)
	{
		if(cmwnd_cl_getrowchannel(ctx, ctx->rowlist.row[i], &rowdata, &proc))
			cmwnd_cl_updatestatusicon(ctx, ctx->rowlist.row[i], rowdata, proc);
		else 
			status = 0;
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_updaterow(cmwnd_ctx_t *ctx, int row, cmwnd_clcol_t col,
							   cmwnd_rowdata_t *rowdata, rxproc_t *proc)
{
	switch(col)
	{
	/* update channel name */
	case CMWND_CLCOL_NAME:
		ListView_SetItemText(ctx->hwndChList, row, CMWND_CLCOL_NAME,
			proc->cfg.name);
		break;
	/* update carrier frequency */
	case CMWND_CLCOL_FREQ:
		fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size,
			proc->cfg.fc, ctx->dispUnitChanFreq, 3,
			(ctx->dispUnitChanFreq >= 2) ? 3 : 0);
		ListView_SetItemText(ctx->hwndChList, row, CMWND_CLCOL_FREQ, ctx->uidata->databuf);
		break;
	/* update filter cutoff frequency */
	case CMWND_CLCOL_FILTER:
		fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size,
			proc->cfg.filter_fc, ctx->dispUnitFiltCutoff, 3, 0);
		ListView_SetItemText(ctx->hwndChList, row, CMWND_CLCOL_FILTER, ctx->uidata->databuf);
		break;
	/* update demodulator type */
	case CMWND_CLCOL_DEMOD:
		ListView_SetItemText(ctx->hwndChList, row, CMWND_CLCOL_DEMOD,
			rxproc_demod_display_name[proc->cfg.demod_type]);
		break;
	/* update output parameters */
	case CMWND_CLCOL_AUDIO:
		/* output gain */
		fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, proc->cfg.output_gain, 0, 6, 1);
		ListView_SetItemText(ctx->hwndChList, row, CMWND_CLCOL_AUDIO, ctx->uidata->databuf);
		/* output mode */
		Button_SetText(rowdata->hwndBtnOutMode, cmwnd_outmode_btn_text[proc->cfg.output_mode]);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int cmwnd_cl_updateallrows(cmwnd_ctx_t *ctx, int col)
{
	int i, status = 1;
	cmwnd_rowdata_t *rowdata;
	rxproc_t *proc;

	if(!cmwnd_cl_rowlist_getallrows(ctx))
		return 0;

	for(i = 0; i < ctx->rowlist.count; i++)
	{
		if(cmwnd_cl_getrowchannel(ctx, ctx->rowlist.row[i], &rowdata, &proc))
			cmwnd_cl_updaterow(ctx, ctx->rowlist.row[i], col, rowdata, proc);
		else 
			status = 0;
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* remove row from channel list and free associated data (does not remove channel) */
static void cmwnd_cl_deleterow(cmwnd_ctx_t *ctx, int row)
{
	cmwnd_rowdata_t *rowdata;

	/* get associated data */
	if( (rowdata = cmwnd_cl_getrowdata(ctx, row)) != NULL )
	{
		/* destroy row inline controls */
		cmwnd_cl_rowctls_destroy(rowdata);

		/* free memory */
		free(rowdata);
	}

	/* delete row */
	ListView_DeleteItem(ctx->hwndChList, row);
}

/* ---------------------------------------------------------------------------------------------- */

/* remove all rows and free associated data (window destroying or channel list reloading) */
static void cmwnd_cl_clear(cmwnd_ctx_t *ctx)
{
	int n, i;

	n = ListView_GetItemCount(ctx->hwndChList);
	for(i = n - 1; i >= 0; i--)
		cmwnd_cl_deleterow(ctx, i);
}

/* ---------------------------------------------------------------------------------------------- */

/* update channel list data (channel notification handling) */
static void cmwnd_procchan_handler(cmwnd_ctx_t *ctx, unsigned int msg, notify_proc_list_t *chList)
{
	int index, row;
	cmwnd_rowdata_t *rowdata;
	rxproc_t *proc;

	/* channels creation event */
	if(msg == EVENT_PROCCHAN_CREATE)
	{
		/* create rows for channels */
		for(index = 0; index < chList->count; index++)
		{
			if( (proc = rx_proc_find(ctx->rx, chList->items[index])) != NULL )
				cmwnd_cl_insertrow(ctx, proc, ListView_GetItemCount(ctx->hwndChList));
		}
	}

	/* channels deletion event */
	else if(msg == EVENT_PROCCHAN_DELETE)
	{
		/* destroy associated rows */
		for(index = 0; index < chList->count; index++)
		{
			if( (row = cmwnd_cl_getrowindexbychan(ctx, chList->items[index], &rowdata)) != -1 )
				cmwnd_cl_deleterow(ctx, row);
		}
	}

	/* channel config change event */
	else
	{
		/* update data in channel list cells */
		for(index = 0; index < chList->count; index++)
		{
			/* get channel and associated list row */
			if( ((proc = rx_proc_find(ctx->rx, chList->items[index])) != NULL) &&
				((row = cmwnd_cl_getrowindexbychan(ctx, proc->chid, &rowdata)) != -1) )
			{
				switch(msg)
				{
				/* update channel name */
				case EVENT_PROCCHAN_NAME:
					cmwnd_cl_updaterow(ctx, row, CMWND_CLCOL_NAME, rowdata, proc);
					break;
				/* update carrier frequency */
				case EVENT_PROCCHAN_FREQ:
					cmwnd_cl_updaterow(ctx, row, CMWND_CLCOL_FREQ, rowdata, proc);
					break;
				/* update filter cutoff frequency */
				case EVENT_PROCCHAN_FILTERCFG:
					cmwnd_cl_updaterow(ctx, row, CMWND_CLCOL_FILTER, rowdata, proc);
					break;
				/* update demodulator type */
				case EVENT_PROCCHAN_DEMODCFG:
					cmwnd_cl_updaterow(ctx, row, CMWND_CLCOL_DEMOD, rowdata, proc);
					break;
				/* update output parameters */
				case EVENT_PROCCHAN_OUTPUTCFG:
					cmwnd_cl_updaterow(ctx, row, CMWND_CLCOL_AUDIO, rowdata, proc);
					break;
				}

				/* update status icon when any channel parameters changed
				   (channel may enter error state on config change) */
				cmwnd_cl_updatestatusicon(ctx, row, rowdata, proc);
			}
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* update channel status (rx state change handler) */
static void cmwnd_rx_state_handler(cmwnd_ctx_t *ctx, unsigned int msg, void *data)
{
	switch(msg)
	{
	case EVENT_RX_STATE_START:
		cmwnd_cl_updateallstatusicons(ctx);
		cmwnd_cl_levelmtr_start(ctx);
		break;
	case EVENT_RX_STATE_STOP:
		cmwnd_cl_updateallstatusicons(ctx);
		cmwnd_cl_levelmtr_stop(ctx);
		break;
	case EVENT_RX_STATE_SET_INPUT_FC:
		cmwnd_cl_updateallstatusicons(ctx);
		break;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* create new channel at end of list and insert row */
static int cmwnd_insertchannel(cmwnd_ctx_t *ctx)
{
	rxproc_t *proc;

	/* close current editing control if one */
	cmwnd_cl_edit_end(ctx, 0);

	/* create new channel */
	if( (proc = rx_proc_create(ctx->rx, ctx->procdefcfg,
		ctx->uidata->msgbuf, ctx->uidata->msgbuf_size)) == NULL )
	{
		MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONEXCLAMATION|MB_OK);
	}
	else
	{
		/* set channel name */
		_stprintf(ctx->uidata->databuf, _T("new %u"), ctx->rx->proc_nextchid);
		rxproc_set_name(proc, ctx->uidata->databuf);

		/* broadcast notification */
		cmwnd_chanevent_notifysingle(ctx, EVENT_PROCCHAN_CREATE, proc->chid);

		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* delete channels by row indices and delete rows */
static void cmwnd_deletechannels(cmwnd_ctx_t *ctx, int *rowlist, int count)
{
	int i;
	rxproc_t *proc;

	/* close current editing control if one */
	cmwnd_cl_edit_end(ctx, 0);

	/* prepare notification list */
	if(cmwnd_chanevent_begin(ctx, count))
	{
		/* delete channels */
		for(i = count - 1; i >= 0; i--)
		{
			if(cmwnd_cl_getrowchannel(ctx, rowlist[i], NULL, &proc))
			{
				/* append notification list */
				cmwnd_chanevent_addchan(ctx, proc->chid);

				/* delete channel */
				rx_proc_delete(proc);
			}
		}

		/* broadcast notification */
		cmwnd_chanevent_notify(ctx, EVENT_PROCCHAN_DELETE);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* delete all channels in the list */
static void cmwnd_deleteallchannels(cmwnd_ctx_t *ctx)
{
	if( cmwnd_cl_rowlist_getallrows(ctx) )
		cmwnd_deletechannels(ctx, ctx->rowlist.row, ctx->rowlist.count);
}

/* ---------------------------------------------------------------------------------------------- */
/* Commands for selected channels */

/* delete selected channels */
static void cmwnd_deleteselchan(cmwnd_ctx_t *ctx)
{
	if( cmwnd_cl_rowlist_getselectedrows(ctx) ) {
		cmwnd_deletechannels(ctx, ctx->rowlist.row, ctx->rowlist.count);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* move selected channels one row up */
static void cmwnd_moveselectedchannelsup(cmwnd_ctx_t *ctx)
{
	int i, row;
	rxproc_t *proccur, *procprev;

	/* close current editing control if one */
	cmwnd_cl_edit_end(ctx, 0);

	if(cmwnd_cl_rowlist_getselectedrows(ctx))
	{
		for(i = 0; i < ctx->rowlist.count; i++)
		{
			if( (row = ctx->rowlist.row[i]) == 0 )
				break;

			if( cmwnd_cl_getrowchannel(ctx, row, NULL, &proccur) &&
				cmwnd_cl_getrowchannel(ctx, row - 1, NULL, &procprev) )
			{
				cmwnd_cl_deleterow(ctx, row);
				cmwnd_cl_insertrow(ctx, proccur, row - 1);
				ListView_SetItemState(ctx->hwndChList, row - 1,
					LVIS_SELECTED, LVIS_SELECTED);
				rx_proc_putbefore(proccur, procprev);
			}
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* move selected channels one row down */
static void cmwnd_moveselectedchannelsdown(cmwnd_ctx_t *ctx)
{
	int i, row, totalnumrows;
	rxproc_t *proccur, *procnext;

	/* close current editing control if one */
	cmwnd_cl_edit_end(ctx, 0);

	if(cmwnd_cl_rowlist_getselectedrows(ctx))
	{
		totalnumrows = ListView_GetItemCount(ctx->hwndChList);

		for(i = ctx->rowlist.count - 1; i >= 0; i--)
		{
			if( (row = ctx->rowlist.row[i]) == totalnumrows - 1 )
				break;

			if( cmwnd_cl_getrowchannel(ctx, row, NULL, &proccur) &&
				cmwnd_cl_getrowchannel(ctx, row + 1, NULL, &procnext) )
			{
				cmwnd_cl_deleterow(ctx, row);
				cmwnd_cl_insertrow(ctx, proccur, row + 1);
				ListView_SetItemState(ctx->hwndChList, row + 1,
					LVIS_SELECTED, LVIS_SELECTED);
				rx_proc_putafter(proccur, procnext);
			}
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* adjust selected channels carrier frequency */
static void cmwnd_adjustselectedchannelsfreq(cmwnd_ctx_t *ctx, UINT uCmdId)
{
	int i;
	double step, fc;
	rxproc_t *proc;

	if( cmwnd_getfreqadjstep(ctx, &step) &&
		cmwnd_cl_rowlist_getselectedrows(ctx) &&
		cmwnd_chanevent_begin(ctx, ctx->rowlist.count) )
	{
		for(i = 0; i < ctx->rowlist.count; i++)
		{
			if(cmwnd_cl_getrowchannel(ctx, ctx->rowlist.row[i], NULL, &proc))
			{
				fc = proc->cfg.fc;

				switch(uCmdId)
				{
				case CMWND_ID_FREQSTEPUP:
					fc += step;
					break;
				case CMWND_ID_FREQSTEPDOWN:
					fc -= step;
					break;
				case CMWND_ID_FREQSTEPSNAP:
					fc = floor(fc / step + 0.5) * step;
					break;
				}

				/* restrict frequency to limit */
				if(fc < RXLIM_FC_MIN)
					fc = RXLIM_FC_MIN;
				if(fc > RXLIM_FC_MAX)
					fc = RXLIM_FC_MAX;

				/* check frequency actually changed */
				if(fabs(fc - proc->cfg.fc) >= 1e-3)
				{
					/* set new frequency */
					rxproc_set_fc(proc, fc);

					/* append channel to notification list */
					cmwnd_chanevent_addchan(ctx, proc->chid);
				}
			}
		}

		/* broadcast notifications */
		cmwnd_chanevent_notify(ctx, EVENT_PROCCHAN_FREQ);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* set selected channels output gain */
static void cmwnd_setselectedchanoutputgain(cmwnd_ctx_t *ctx, UINT ucmd)
{
	double output_gain;
	int i;
	cmwnd_rowdata_t *rowdata;
	rxproc_t *proc;

	/* select gain value */
	switch(ucmd) {
	case CMWND_ID_OUTGAIN_3:
		output_gain = 3.0;
		break;
	case CMWND_ID_OUTGAIN_0:
		output_gain = 0.0;
		break;
	case CMWND_ID_OUTGAIN__3:
		output_gain = -3.0;
		break;
	case CMWND_ID_OUTGAIN__6:
		output_gain = -6.0;
		break;
	case CMWND_ID_OUTGAIN__9:
		output_gain = -9.0;
		break;
	case CMWND_ID_OUTGAIN__12:
		output_gain = -12.0;
		break;
	case CMWND_ID_OUTGAIN__15:
		output_gain = -15.0;
		break;
	default:
		return;
	}

	/* apply to selected channels */
	if( cmwnd_cl_rowlist_getselectedrows(ctx) &&
		cmwnd_chanevent_begin(ctx, ctx->rowlist.count) )
	{
		for(i = 0; i < ctx->rowlist.count; i++)
		{
			if(cmwnd_cl_getrowchannel(ctx, ctx->rowlist.row[i], &rowdata, &proc))
			{
				if(fabs(output_gain - proc->cfg.output_gain) >= 1e-6)
				{
					/* set channel output gain */
					rxproc_set_output_gain(proc, output_gain);

					/* add to notify list */
					cmwnd_chanevent_addchan(ctx, proc->chid);
				}
			}
		}

		/* broadcast notifications */
		cmwnd_chanevent_notify(ctx, EVENT_PROCCHAN_OUTPUTCFG);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* set output mode for selected channels */
static void cmwnd_setselectedchanoutputmode(cmwnd_ctx_t *ctx, UINT ucmd)
{
	int i, output_mode;
	cmwnd_rowdata_t *rowdata;
	rxproc_t *proc;

	/* select new mode */
	switch(ucmd) {
	case CMWND_ID_OUTCHANNONE:
		output_mode = SNDMIX_INPUT_CHAN_NONE;
		break;
	case CMWND_ID_OUTCHANA:
		output_mode = SNDMIX_INPUT_CHAN_A;
		break;
	case CMWND_ID_OUTCHANB:
		output_mode = SNDMIX_INPUT_CHAN_B;
		break;
	case CMWND_ID_OUTCHANAB:
		output_mode = SNDMIX_INPUT_CHAN_AB;
		break;
	default:
		return;
	}

	/* apply to selected channels */
	if( cmwnd_cl_rowlist_getselectedrows(ctx) &&
		cmwnd_chanevent_begin(ctx, ctx->rowlist.count) )
	{
		for(i = 0; i < ctx->rowlist.count; i++)
		{
			if(cmwnd_cl_getrowchannel(ctx, ctx->rowlist.row[i], &rowdata, &proc))
			{
				if(proc->cfg.output_mode != output_mode)
				{
					/* set channel output mode */
					proc->cfg.output_mode = output_mode;

					/* add to notify list */
					cmwnd_chanevent_addchan(ctx, proc->chid);
				}
			}
		}

		/* broadcast notifications */
		cmwnd_chanevent_notify(ctx, EVENT_PROCCHAN_OUTPUTCFG);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* cycle output mode for selected channels */
static void cmwnd_cycleselectedchanoutputmode(cmwnd_ctx_t *ctx)
{
	int i, output_mode = -1;
	cmwnd_rowdata_t *rowdata;
	rxproc_t *proc;

	if( cmwnd_cl_rowlist_getselectedrows(ctx) &&
		cmwnd_chanevent_begin(ctx, ctx->rowlist.count) )
	{
		for(i = 0; i < ctx->rowlist.count; i++)
		{
			if(cmwnd_cl_getrowchannel(ctx, ctx->rowlist.row[i], &rowdata, &proc))
			{
				if(output_mode == -1)
				{
					output_mode = proc->cfg.output_mode + 1;
					if(output_mode > SNDMIX_INPUT_CHAN_AB)
						output_mode = SNDMIX_INPUT_CHAN_NONE;
				}

				if(proc->cfg.output_mode != output_mode)
				{
					/* set channel output mode */
					proc->cfg.output_mode = output_mode;

					/* add to notify list */
					cmwnd_chanevent_addchan(ctx, proc->chid);
				}
			}
		}

		/* broadcast notifications */
		cmwnd_chanevent_notify(ctx, EVENT_PROCCHAN_OUTPUTCFG);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* allocate new buffer and return selected channels identifiers */
static int cmwnd_cl_getselectedrowchids(cmwnd_ctx_t *ctx, unsigned int **pchanlist, int *pchancount)
{
	unsigned int *chanlist;
	int i, chancount;
	cmwnd_rowdata_t *rowdata;

	chancount = 0;

	if(!cmwnd_cl_rowlist_getselectedrows(ctx))
		return 0;

	if(ctx->rowlist.count == 0)
	{
		chanlist = NULL;
	}
	else
	{
		if( (chanlist = malloc(sizeof(int) * ctx->rowlist.count)) == NULL )
			return 0;

		for(i = 0; i < ctx->rowlist.count; i++)
		{
			if( (rowdata = cmwnd_cl_getrowdata(ctx, ctx->rowlist.row[i])) != NULL )
				chanlist[chancount++] = rowdata->chid;
		}

		if(chancount == 0) {
			free(chanlist);
			chanlist = NULL;
		}
	}

	*pchanlist = chanlist;
	*pchancount = chancount;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* show misc options window for selected channels */
static void cmwnd_showselectedchanoptions(cmwnd_ctx_t *ctx, POINT *ppt)
{
	POINT pt;
	int chidcount;
	unsigned int *chidlist;
	
	if(ppt == NULL)
	{
		GetCursorPos(&pt);
		ppt = &pt;
	}

	if(cmwnd_cl_getselectedrowchids(ctx, &chidlist, &chidcount))
	{
		chanopt_createwindow(ctx->uidata, ctx->event_procchan,
			ctx->hwnd, ctx->rx, ctx->procdefcfg, chidlist, chidcount, ppt->x, ppt->y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* show filter config window for selected channels */
static void cmwnd_showselectedchanfilterconfig(cmwnd_ctx_t *ctx, POINT *ppt)
{
	POINT pt;
	int chidcount;
	unsigned int *chidlist;

	if(ppt == NULL)
	{
		GetCursorPos(&pt);
		ppt = &pt;
	}

	if(cmwnd_cl_getselectedrowchids(ctx, &chidlist, &chidcount))
	{
		filtcfg_createwindow(ctx->uidata, ctx->event_procchan,
			ctx->hwnd, ctx->rx, ctx->procdefcfg, chidlist, chidcount, ppt->x, ppt->y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* show demodulator config window for selected channels */
static void cmwnd_showselectedchandemodconfig(cmwnd_ctx_t *ctx, POINT *ppt)
{
	POINT pt;
	int chidcount, i;
	unsigned int *chidlist;
	rxproc_t *proc;
	rxproc_demodtype_t type;

	if(ppt == NULL)
	{
		GetCursorPos(&pt);
		ppt = &pt;
	}

	if(cmwnd_cl_getselectedrowchids(ctx, &chidlist, &chidcount))
	{
		type = -1;

		for(i = 0; i < chidcount; i++)
		{
			if( (proc = rx_proc_find(ctx->rx, chidlist[i])) != NULL )
			{
				if(type == -1)
				{
					type = proc->cfg.demod_type;
				}
				else
				{
					if(type != proc->cfg.demod_type)
					{
						MessageBox(ctx->hwnd, _T("Multiple demodulator types selected."),
							ui_title, MB_ICONINFORMATION|MB_OK);
						free(chidlist);
						return;
					}
				}
			}
		}

		cmdwnd_showdemodconfig(ctx, type, chidlist, chidcount, ppt->x, ppt->y);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* show squelch config window for selected channels */
static void cmwnd_showselectedchansqlconfig(cmwnd_ctx_t *ctx, POINT *ppt)
{
	POINT pt;
	int chidcount;
	unsigned int *chidlist;

	if(ppt == NULL)
	{
		GetCursorPos(&pt);
		ppt = &pt;
	}

	if(cmwnd_cl_getselectedrowchids(ctx, &chidlist, &chidcount))
	{
		sqlcfg_createwindow(ctx->uidata, ctx->event_procchan,
			ctx->hwnd, ctx->rx, ctx->procdefcfg, chidlist, chidcount, ppt->x, ppt->y);
	}
}

/* ---------------------------------------------------------------------------------------------- */
/* Channel list menus */

/* create channel list popup menu */
static void cmwnd_cl_createlistpopupmenu(cmwnd_ctx_t *ctx)
{
	ctx->hmenuChListPopup = CreatePopupMenu();
	AppendMenu(ctx->hmenuChListPopup, 0, CMWND_ID_INSERTCHAN, _T("New channel\tIns"));
}

/* ---------------------------------------------------------------------------------------------- */

/* create selected channel commands popup menu */
static void cmwnd_cl_createchanpopupmenu(cmwnd_ctx_t *ctx)
{
	HMENU hmenuOutGain, hmenuOutMode;

	hmenuOutGain = CreatePopupMenu();
	AppendMenu(hmenuOutGain, 0, CMWND_ID_OUTGAIN_3, _T("+3 dB"));
	AppendMenu(hmenuOutGain, 0, CMWND_ID_OUTGAIN_0, _T("0 dB"));
	AppendMenu(hmenuOutGain, 0, CMWND_ID_OUTGAIN__3, _T("–3 dB"));
	AppendMenu(hmenuOutGain, 0, CMWND_ID_OUTGAIN__6, _T("–6 dB"));
	AppendMenu(hmenuOutGain, 0, CMWND_ID_OUTGAIN__9, _T("–9 dB"));
	AppendMenu(hmenuOutGain, 0, CMWND_ID_OUTGAIN__12, _T("–12 dB"));
	AppendMenu(hmenuOutGain, 0, CMWND_ID_OUTGAIN__15, _T("–15 dB"));

	hmenuOutMode = CreatePopupMenu();
	AppendMenu(hmenuOutMode, 0, CMWND_ID_OUTCHANNONE, _T("None"));
	AppendMenu(hmenuOutMode, 0, CMWND_ID_OUTCHANA, _T("Left"));
	AppendMenu(hmenuOutMode, 0, CMWND_ID_OUTCHANB, _T("Right"));
	AppendMenu(hmenuOutMode, 0, CMWND_ID_OUTCHANAB, _T("Both"));

	ctx->hmenuChanPopup = CreatePopupMenu();
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_MOVEUP, _T("Move up\tCtrl+Up"));
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_MOVEDOWN, _T("Move down\tCtrl+Down"));
	AppendMenu(ctx->hmenuChanPopup, MF_SEPARATOR, 0, NULL);
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_FREQSTEPUP, _T("Increase frequency\t+"));
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_FREQSTEPDOWN, _T("Decrease frequency\t–"));
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_FREQSTEPSNAP, _T("Snap frequency\tSpace"));
	AppendMenu(ctx->hmenuChanPopup, MF_SEPARATOR, 0, NULL);
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_CHANOPT, _T("Misc configuration\tShift+F2"));
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_FILTERCFG, _T("Filter configuration\tShift+F4"));
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_DEMODCFG, _T("Demodulator configuration\tShift+F5"));
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_SQLCFG, _T("Squelch configuration\tShift+F6"));
	AppendMenu(ctx->hmenuChanPopup, MF_POPUP, (UINT_PTR)hmenuOutGain, _T("Output gain\tF7"));
	AppendMenu(ctx->hmenuChanPopup, MF_POPUP, (UINT_PTR)hmenuOutMode, _T("Output mode\tShift+F7"));
	AppendMenu(ctx->hmenuChanPopup, MF_SEPARATOR, 0, NULL);
	AppendMenu(ctx->hmenuChanPopup, 0, CMWND_ID_DELETECHANS, _T("Delete\tDel"));
}

/* ---------------------------------------------------------------------------------------------- */

/* show channel list popup menu */
static void cmwnd_cl_showlistpopupmenu(cmwnd_ctx_t *ctx)
{
	POINT pt;

	GetCursorPos(&pt);
	TrackPopupMenu(ctx->hmenuChListPopup, 0,
		pt.x, pt.y, 0, ctx->hwndChList, NULL);
}

/* ---------------------------------------------------------------------------------------------- */

/* show selected channel commands popup menu */
static void cmwnd_cl_showchanpopupmenu(cmwnd_ctx_t *ctx)
{
	POINT pt;

	GetCursorPos(&pt);
	TrackPopupMenu(ctx->hmenuChanPopup, 0,
		pt.x, pt.y, 0, ctx->hwndChList, NULL);
}

/* ---------------------------------------------------------------------------------------------- */
/* Frequency display unit selection */

static void cmwnd_loaddispunits(cmwnd_ctx_t *ctx)
{
	ini_sect_t *sect;

	sect = ini_sect_get(ctx->ini, _T("cmwnd"), 0);
	ini_geti(sect, _T("unit_chan_fc"), &(ctx->dispUnitChanFreq));
	ini_geti(sect, _T("unit_chan_filt"), &(ctx->dispUnitFiltCutoff));
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_savedispunits(cmwnd_ctx_t *ctx)
{
	ini_sect_t *sect;

	sect = ini_sect_get(ctx->ini, _T("cmwnd"), 1);
	ini_seti(sect, _T("unit_chan_fc"), ctx->dispUnitChanFreq);
	ini_seti(sect, _T("unit_chan_filt"), ctx->dispUnitFiltCutoff);
}

/* ---------------------------------------------------------------------------------------------- */

/* Channel list message handling */

static void cmwnd_cl_limitheaderwidth(cmwnd_ctx_t *ctx, NMHEADER *nmhdr)
{
	cmwnd_clcol_cx_t *col_cx;
	int cx_min, cx_max;

	if( (nmhdr->pitem != NULL) && (nmhdr->pitem->mask & HDI_WIDTH) &&
		(nmhdr->iItem >= 0) && (nmhdr->iItem < CMWND_CLCOL_COUNT) )
	{
		col_cx = cmwnd_clcol_cx_data + nmhdr->iItem;

		cx_min = col_cx->cx_min + col_cx->cx_add + CMWND_CLCOL_CXMARGIN * 2;
		if(nmhdr->pitem->cxy < cx_min)
			nmhdr->pitem->cxy = cx_min;

		cx_max = col_cx->cx_max + col_cx->cx_add + CMWND_CLCOL_CXMARGIN * 2;
		if(nmhdr->pitem->cxy > cx_max)
			nmhdr->pitem->cxy = cx_max;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_dividerdblclk(cmwnd_ctx_t *ctx, NMHEADER *nmhdr)
{
	int cx_col, cx, i, n;
	cmwnd_clcol_cx_t *col_data;

	if((nmhdr->iItem < 0) || (nmhdr->iItem >= CMWND_CLCOL_COUNT))
		return;

	col_data = cmwnd_clcol_cx_data + nmhdr->iItem;

	cx_col = 0;

	n = ListView_GetItemCount(ctx->hwndChList);
	for(i = 0; i < n; i++)
	{
		ListView_GetItemText(ctx->hwndChList, i, nmhdr->iItem,
			ctx->uidata->msgbuf, (int)(ctx->uidata->msgbuf_size));
		cx = ListView_GetStringWidth(ctx->hwndChList, ctx->uidata->msgbuf);
		if(cx > cx_col)
			cx_col = cx;
	}

	if(cx_col < col_data->cx_min)
		cx_col = col_data->cx_min;
	if(cx_col > col_data->cx_max)
		cx_col = col_data->cx_max;
	
	cx_col += col_data->cx_add + CMWND_CLCOL_CXMARGIN * 2;

	ListView_SetColumnWidth(ctx->hwndChList, nmhdr->iItem, cx_col);
}

/* ---------------------------------------------------------------------------------------------- */

/* channel list VK_DELETE key press */
static void cmwnd_cl_deletekey(cmwnd_ctx_t *ctx)
{
	int rowcount, row;

	if( cmwnd_cl_rowlist_getselectedrows(ctx) &&
		(ctx->rowlist.count != 0) )
	{
		/* delete selected row and channels */
		cmwnd_deletechannels(ctx, ctx->rowlist.row, ctx->rowlist.count);

		/* select row below deleted channels */
		rowcount = ListView_GetItemCount(ctx->hwndChList);
		if(rowcount != 0)
		{
			row = ctx->rowlist.row[ctx->rowlist.count - 1] - ctx->rowlist.count + 1;
			if(row >= rowcount)
				row = rowcount - 1;
			ListView_SetItemState(ctx->hwndChList, row, LVIS_SELECTED, LVIS_SELECTED);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int cmwnd_cl_getpopupmenupos(cmwnd_ctx_t *ctx, int row, int col, POINT *ppt)
{
	int res;
	RECT rc;
	POINT pt;

	if(col == 0) {
		res = ListView_GetItemRect(ctx->hwndChList, row, &rc, LVIR_LABEL);
	} else {
		res = ListView_GetSubItemRect(ctx->hwndChList, row, col, LVIR_BOUNDS, &rc);
	}

	if(!res)
		return 0;

	pt.x = (rc.left + rc.right) / 2;
	pt.y = (rc.top + rc.bottom) / 2;

	ClientToScreen(ctx->hwndChList, &pt);

	memcpy(ppt, &pt, sizeof(POINT));
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* channel list param editing key press (VK_F2, VK_F3, etc) */
static void cmwnd_cl_editkey(cmwnd_ctx_t *ctx, int col, int shft)
{
	int row;
	POINT pt;

	row = cmwnd_cl_getselectedrowindex(ctx);

	if(!shft)
	{
		switch(col)
		{
		case CMWND_CLCOL_NAME:
		case CMWND_CLCOL_FREQ:
		case CMWND_CLCOL_FILTER:
		case CMWND_CLCOL_SQUELCH:
		case CMWND_CLCOL_AUDIO:
			cmwnd_cl_edit_startedit(ctx, row, col);
			break;
		case CMWND_CLCOL_DEMOD:
			ctx->demodSelectRow = row;
			if(cmwnd_cl_getpopupmenupos(ctx, row, CMWND_CLCOL_DEMOD, &pt))
				TrackPopupMenu(ctx->hmenuDemodType, 0, pt.x, pt.y, 0, ctx->hwndChList, NULL);
			break;
		}
	}
	else
	{
		switch(col)
		{
		case CMWND_CLCOL_NAME:
			if(cmwnd_cl_getpopupmenupos(ctx, row, CMWND_CLCOL_NAME, &pt))
				cmwnd_showselectedchanoptions(ctx, &pt);
			break;
		case CMWND_CLCOL_FREQ:
			SetFocus(ctx->hwndComboFreqStep);
			ComboBox_ShowDropdown(ctx->hwndComboFreqStep, TRUE);
			break;
		case CMWND_CLCOL_FILTER:
			if(cmwnd_cl_getpopupmenupos(ctx, row, CMWND_CLCOL_FILTER, &pt))
				cmwnd_showselectedchanfilterconfig(ctx, &pt);
			break;
		case CMWND_CLCOL_DEMOD:
			if(cmwnd_cl_getpopupmenupos(ctx, row, CMWND_CLCOL_DEMOD, &pt))
				cmwnd_showselectedchandemodconfig(ctx, &pt);
			break;
		case CMWND_CLCOL_SQUELCH:
			if(cmwnd_cl_getpopupmenupos(ctx, row, CMWND_CLCOL_SQUELCH, &pt))
				cmwnd_showselectedchansqlconfig(ctx, &pt);
			break;
		case CMWND_CLCOL_AUDIO:
			cmwnd_cycleselectedchanoutputmode(ctx);
			break;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* channel row param double click */
static void cmwnd_cl_doubleclick(cmwnd_ctx_t *ctx, NMITEMACTIVATE *nm)
{
	/* open param editing control */
	cmwnd_cl_edit_startedit(ctx, nm->iItem, nm->iSubItem);
}

/* ---------------------------------------------------------------------------------------------- */

/* channel list left button click */
static void cmwnd_cl_click(cmwnd_ctx_t *ctx, NMITEMACTIVATE *nm)
{
	/* close param editing control */
	cmwnd_cl_edit_end(ctx, 1);
}

/* ---------------------------------------------------------------------------------------------- */

/* channel list right button click */
static void cmwnd_cl_rclick(cmwnd_ctx_t *ctx, NMITEMACTIVATE *nm)
{
	/* close param editing control */
	if( cmwnd_cl_edit_end(ctx, 1) )
	{
		/* show channel popup menu */
		if(nm->iItem != -1) {
			cmwnd_cl_showchanpopupmenu(ctx);
		} else {
			cmwnd_cl_showlistpopupmenu(ctx);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* channel list WM_COMMAND message handler */
static int cmwnd_cl_command(cmwnd_ctx_t *ctx, UINT ucommand)
{
	switch(ucommand)
	{
	case CMWND_ID_INSERTCHAN:
		cmwnd_insertchannel(ctx);
		break;

	case CMWND_ID_MOVEUP:
		cmwnd_moveselectedchannelsup(ctx);
		break;

	case CMWND_ID_MOVEDOWN:
		cmwnd_moveselectedchannelsdown(ctx);
		break;

	case CMWND_ID_DELETECHANS:
		cmwnd_deleteselchan(ctx);
		break;

	case CMWND_ID_CHANOPT:
		cmwnd_showselectedchanoptions(ctx, NULL);
		break;

	case CMWND_ID_FILTERCFG:
		cmwnd_showselectedchanfilterconfig(ctx, NULL);
		break;

	case CMWND_ID_DEMODCFG:
		cmwnd_showselectedchandemodconfig(ctx, NULL);
		break;

	case CMWND_ID_SQLCFG:
		cmwnd_showselectedchansqlconfig(ctx, NULL);
		break;

	case CMWND_ID_FREQSTEPUP:
	case CMWND_ID_FREQSTEPDOWN:
	case CMWND_ID_FREQSTEPSNAP:
		cmwnd_adjustselectedchannelsfreq(ctx, ucommand);
		break;

	case CMWND_ID_OUTGAIN_3:
	case CMWND_ID_OUTGAIN_0:
	case CMWND_ID_OUTGAIN__3:
	case CMWND_ID_OUTGAIN__6:
	case CMWND_ID_OUTGAIN__9:
	case CMWND_ID_OUTGAIN__12:
	case CMWND_ID_OUTGAIN__15:
		cmwnd_setselectedchanoutputgain(ctx, ucommand);
		break;

	case CMWND_ID_OUTCHANNONE:
	case CMWND_ID_OUTCHANA:
	case CMWND_ID_OUTCHANB:
	case CMWND_ID_OUTCHANAB:
		cmwnd_setselectedchanoutputmode(ctx, ucommand);
		break;
	}

	/* demod type selection menu command */
	if( (ucommand >= CMWND_ID_DEMODTYPE_0) &&
		(ucommand <= CMWND_ID_DEMODTYPE_MAX) )
	{
		cmwnd_cl_cmd_demodtypemenu(ctx,
			ucommand - CMWND_ID_DEMODTYPE_0);
		return 1;
	}

	/* row controls command */
	if( (ucommand >= CMWND_ID_ROWCTL_0) &&
		(ucommand <= CMWND_ID_ROWCTL_MAX) )
	{
		if(cmwnd_cl_rowctls_command(ctx, ucommand))
		{
			/*SetFocus(ctx->hwndChList);*/
			return 1;
		}
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* channel list key press */
static int cmwnd_cl_keydown(cmwnd_ctx_t *ctx, NMLVKEYDOWN *nm)
{
	switch(nm->wVKey)
	{
	case VK_INSERT:
		cmwnd_insertchannel(ctx);
		return 1;

	case VK_DELETE:
		cmwnd_cl_deletekey(ctx);
		return 1;

	case VK_F2:
		cmwnd_cl_editkey(ctx, CMWND_CLCOL_NAME,
			(GetKeyState(VK_SHIFT) & 0x8000) != 0);
		return 1;

	case VK_F3:
		cmwnd_cl_editkey(ctx, CMWND_CLCOL_FREQ,
			(GetKeyState(VK_SHIFT) & 0x8000) != 0);
		return 1;

	case VK_F4:
		cmwnd_cl_editkey(ctx, CMWND_CLCOL_FILTER,
			(GetKeyState(VK_SHIFT) & 0x8000) != 0);
		return 1;

	case VK_F5:
		cmwnd_cl_editkey(ctx, CMWND_CLCOL_DEMOD,
			(GetKeyState(VK_SHIFT) & 0x8000) != 0);
		return 1;

	case VK_F6:
		cmwnd_cl_editkey(ctx, CMWND_CLCOL_SQUELCH,
			(GetKeyState(VK_SHIFT) & 0x8000) != 0);
		return 1;

	case VK_F7:
		cmwnd_cl_editkey(ctx, CMWND_CLCOL_AUDIO,
			(GetKeyState(VK_SHIFT) & 0x8000) != 0);
		return 1;

	case VK_UP:
		if(GetKeyState(VK_CONTROL) & 0x8000) {
			cmwnd_moveselectedchannelsup(ctx);
			return 1;
		}
		break;

	case VK_DOWN:
		if(GetKeyState(VK_CONTROL) & 0x8000) {
			cmwnd_moveselectedchannelsdown(ctx);
			return 1;
		}
		break;

	case VK_OEM_PLUS:
	case VK_ADD:
		cmwnd_adjustselectedchannelsfreq(ctx, CMWND_ID_FREQSTEPUP);
		break;

	case VK_OEM_MINUS:
	case VK_SUBTRACT:
		cmwnd_adjustselectedchannelsfreq(ctx, CMWND_ID_FREQSTEPDOWN);
		break;

	case VK_SPACE:
		cmwnd_adjustselectedchannelsfreq(ctx, CMWND_ID_FREQSTEPSNAP);
		break;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* channel manager window WM_NOTIFY message handler */
static int cmwnd_wmnotify(cmwnd_ctx_t *ctx, NMHDR *nm)
{
	if(nm->hwndFrom == ctx->hwndChList)
	{
		switch(nm->code)
		{
		case NM_DBLCLK:
			cmwnd_cl_doubleclick(ctx, (void*)nm);
			return 1;

		case NM_CLICK:
			cmwnd_cl_click(ctx, (void*)nm);
			return 1;

		case NM_RCLICK:
			cmwnd_cl_rclick(ctx, (void*)nm);
			return 1;

		case LVN_KEYDOWN:
			if(cmwnd_cl_keydown(ctx, (void*)nm))
				return 1;
			break;
		}
	}

	if(nm->hwndFrom == ctx->hwndChListHdr)
	{
		switch(nm->code)
		{
		case HDN_ENDTRACK:
			cmwnd_cl_limitheaderwidth(ctx, (NMHEADER*)nm);
			break;
		case HDN_DIVIDERDBLCLICK:
			cmwnd_cl_dividerdblclk(ctx, (NMHEADER*)nm);
			break;
		}
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* channel list window proc */
static LRESULT CALLBACK cmwnd_cl_wndproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	cmwnd_ctx_t *stateCmwin;

	stateCmwin = GetWndPtr(hwnd, GWLP_USERDATA);

	switch(uMsg)
	{
	case WM_PAINT:
		cmwnd_cl_edit_updatepos(stateCmwin);
		cmwnd_cl_rowctls_updatepos_allrows(stateCmwin);
		break;

	case WM_COMMAND:
		if(cmwnd_cl_command(stateCmwin, LOWORD(wParam)))
			return 0;
		break;
	}

	return CallWindowProc(stateCmwin->wndprocChList, hwnd, uMsg, wParam, lParam);
}

/* ---------------------------------------------------------------------------------------------- */
/* Channel list initialization */

/* load channel list column widths */
static void cmwnd_cl_colcx_load(cmwnd_ctx_t *ctx, int *cx_buf)
{
	ini_sect_t *sect;
	cmwnd_clcol_cx_t *col_cx;
	int i;

	sect = ini_sect_get(ctx->ini, _T("cmwnd"), 0);
	for(i = 0; i < CMWND_CLCOL_COUNT; i++)
	{
		col_cx = cmwnd_clcol_cx_data + i;
		cx_buf[i] = col_cx->cx_def;
		ini_getir(sect, col_cx->name, &(cx_buf[i]), col_cx->cx_min, col_cx->cx_max);
		cx_buf[i] += col_cx->cx_add + CMWND_CLCOL_CXMARGIN * 2;
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* save channel list column widths */
static void cmwnd_cl_colcx_save(cmwnd_ctx_t *ctx)
{
	ini_sect_t *sect;
	cmwnd_clcol_cx_t *col_cx;
	int i, cx;

	sect = ini_sect_get(ctx->ini, _T("cmwnd"), 1);
	for(i = 0; i < CMWND_CLCOL_COUNT; i++)
	{
		col_cx = cmwnd_clcol_cx_data + i;
		cx = ListView_GetColumnWidth(ctx->hwndChList, i);
		ini_seti(sect, col_cx->name, cx - (col_cx->cx_add + CMWND_CLCOL_CXMARGIN * 2));
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* create channel list columns (window init) */
static void cmwnd_cl_crtcols(cmwnd_ctx_t *ctx)
{
	LVCOLUMN lvc;
	int cx_buf[CMWND_CLCOL_COUNT];

	cmwnd_cl_colcx_load(ctx, cx_buf);

	memset(&lvc, 0, sizeof(lvc));
	lvc.mask = LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;

	lvc.cx = cx_buf[CMWND_CLCOL_NAME];
	lvc.pszText = _T("Name");
	lvc.iSubItem = CMWND_CLCOL_NAME;
	ListView_InsertColumn(ctx->hwndChList, lvc.iSubItem, &lvc);

	lvc.cx = cx_buf[CMWND_CLCOL_FREQ];
	lvc.pszText = _T("Frequency");
	lvc.iSubItem = CMWND_CLCOL_FREQ;
	ListView_InsertColumn(ctx->hwndChList, lvc.iSubItem, &lvc);

	lvc.cx = cx_buf[CMWND_CLCOL_FILTER];
	lvc.pszText = _T("Filter");
	lvc.iSubItem = CMWND_CLCOL_FILTER;
	ListView_InsertColumn(ctx->hwndChList, lvc.iSubItem, &lvc);

	lvc.cx = cx_buf[CMWND_CLCOL_DEMOD];
	lvc.pszText = _T("Demod");
	lvc.iSubItem = CMWND_CLCOL_DEMOD;
	ListView_InsertColumn(ctx->hwndChList, lvc.iSubItem, &lvc);

	lvc.cx = cx_buf[CMWND_CLCOL_SQUELCH];
	lvc.pszText = _T("Squelch");
	lvc.iSubItem = CMWND_CLCOL_SQUELCH;
	ListView_InsertColumn(ctx->hwndChList, lvc.iSubItem, &lvc);

	lvc.cx = cx_buf[CMWND_CLCOL_AUDIO];
	lvc.pszText = _T("Out");
	lvc.iSubItem = CMWND_CLCOL_AUDIO;
	ListView_InsertColumn(ctx->hwndChList, lvc.iSubItem, &lvc);

	lvc.cx = cx_buf[CMWND_CLCOL_LEVEL];
	lvc.pszText = _T("Level");
	lvc.iSubItem = CMWND_CLCOL_LEVEL;
	ListView_InsertColumn(ctx->hwndChList, lvc.iSubItem, &lvc);
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_imglist_create(cmwnd_cl_imglist_data_t *data, HINSTANCE h_inst)
{
	int i;

	data->h_img_list = ImageList_Create(16, 16, ILC_COLOR4|ILC_MASK, 8, 8);

	data->hicon_proc_status[RXPROC_STOPPED] =
		LoadIcon(h_inst, MAKEINTRESOURCE(IDI_RXPROC_STOPPED));
	data->hicon_proc_status[RXPROC_ERROR] =
		LoadIcon(h_inst, MAKEINTRESOURCE(IDI_RXPROC_ERROR));
	data->hicon_proc_status[RXPROC_RANGEUP] =
		LoadIcon(h_inst, MAKEINTRESOURCE(IDI_RXPROC_RANGE_UP));
	data->hicon_proc_status[RXPROC_RANGEDOWN] =
		LoadIcon(h_inst, MAKEINTRESOURCE(IDI_RXPROC_RANGE_DOWN));
	data->hicon_proc_status[RXPROC_SQLCLOSED] =
		LoadIcon(h_inst, MAKEINTRESOURCE(IDI_RXPROC_SQLCLOSED));
	data->hicon_proc_status[RXPROC_SQLOPENED] =
		LoadIcon(h_inst, MAKEINTRESOURCE(IDI_RXPROC_SQLOPENED));

	for(i = 0; i < RXPROC_STATUS_COUNT; i++)
	{
		data->img_proc_status[i] =
			ImageList_AddIcon(data->h_img_list, data->hicon_proc_status[i]);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_imglist_destroy(cmwnd_cl_imglist_data_t *data)
{
	int i;

	for(i = 0; i < RXPROC_STATUS_COUNT; i++)
	{
		ImageList_Remove(data->h_img_list, data->img_proc_status[i]);
		DeleteObject(data->hicon_proc_status[i]);
	}

	ImageList_Destroy(data->h_img_list);
}

/* ---------------------------------------------------------------------------------------------- */

/* create channel list */
static void cmwnd_cl_create(cmwnd_ctx_t *ctx, int cw, int ch)
{
	/* create imagelist */
	cmwnd_cl_imglist_create(&(ctx->cl_imglist_data), ctx->uidata->h_inst);

	/* create list view control */
	ctx->hwndChList = ui_crt_listview(ctx->uidata, ctx->hwnd,
		LVS_REPORT|LVS_SHOWSELALWAYS|WS_TABSTOP, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES,
		0, 0, cw, ch - CMWND_CL_OFS_BTM, CMWND_ID_CHLIST);
	ctx->hwndChListHdr = ListView_GetHeader(ctx->hwndChList);
	ListView_SetImageList(ctx->hwndChList, ctx->cl_imglist_data.h_img_list, LVSIL_SMALL);

	/* hook channel list window proc */
	SetWndPtr(ctx->hwndChList, GWLP_USERDATA, ctx);

	ctx->wndprocChList = (WNDPROC)SetWndPtr(ctx->hwndChList,
		GWLP_WNDPROC, (void*)cmwnd_cl_wndproc);

	/* start level meter timer */
	cmwnd_cl_levelmtr_start(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_cl_destroy(cmwnd_ctx_t *ctx)
{
	/* stop level meter timer */
	cmwnd_cl_levelmtr_stop(ctx);

	/* close cell edit control */
	cmwnd_cl_edit_end(ctx, 0);

	/* remove rows */
	cmwnd_cl_clear(ctx);

	/* destroy channel list */
	DestroyWindow(ctx->hwndChList);
	ctx->hwndChList = NULL;
	ctx->hwndChListHdr = NULL;

	/* destroy imagelist */
	cmwnd_cl_imglist_destroy(&(ctx->cl_imglist_data));
}

/* ---------------------------------------------------------------------------------------------- */
/* Preset storing controls handling */

/* fill preset selection combo box with preset names */
static void cmwnd_fillpresets(cmwnd_ctx_t *ctx)
{
	ini_sect_t *sect;

	for(sect = ctx->inich->sect_head; sect != NULL; sect = sect->sect_fwd)
	{
		if(_tcsncmp(sect->name, _T("proc0_"), 6) == 0)
			ComboBox_AddString(ctx->hwndComboPreset, sect->name + 6);
	}

	ComboBox_SetText(ctx->hwndComboPreset, _T("<autosave>"));
}

/* ---------------------------------------------------------------------------------------------- */

/* create preset selection controls */
static void cmwnd_createpresetctls(cmwnd_ctx_t *ctx, int cw, int ch)
{
	/* create preset selection combo box */
	ctx->hwndComboPreset = ui_crt_combo(ctx->uidata, ctx->hwnd,
		CBS_DROPDOWN|WS_TABSTOP, 50, ch - CMWND_BTM_CTLS_OFS, 120, 200, CMWND_ID_PRESET_LIST);
	cmwnd_fillpresets(ctx);

	/* create preset buttons */
	ctx->hwndBtnLoadPreset = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		170, ch - CMWND_BTM_CTLS_OFS, 50, 45, _T("Load"), CMWND_ID_PRESET_LOAD);
	ctx->hwndBtnPresetMenu = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		215, ch - CMWND_BTM_CTLS_OFS, 15, 21, _T("»"), CMWND_ID_PRESET_MENU);

	/* create preset commands menu */
	ctx->hmenuPresetCommands = CreatePopupMenu();
	AppendMenu(ctx->hmenuPresetCommands, 0, CMWND_ID_PRESET_APPEND, _T("Append"));
	AppendMenu(ctx->hmenuPresetCommands, 0, CMWND_ID_PRESET_SAVE, _T("Save"));
	AppendMenu(ctx->hmenuPresetCommands, 0, CMWND_ID_PRESET_DELETE, _T("Delete"));
}

/* ---------------------------------------------------------------------------------------------- */

/* update preset selection controls position */
static void cmwnd_updatepresetctlspos(cmwnd_ctx_t *ctx, int cw, int ch)
{
	MoveWindow(ctx->hwndComboPreset, 50, ch - CMWND_BTM_CTLS_OFS, 120, 200, TRUE);
	ComboBox_SetEditSel(ctx->hwndComboPreset, -1, 0);

	MoveWindow(ctx->hwndBtnLoadPreset, 170, ch - CMWND_BTM_CTLS_OFS, 45, 21, TRUE);
	MoveWindow(ctx->hwndBtnPresetMenu, 215, ch - CMWND_BTM_CTLS_OFS, 15, 21, TRUE);
}

/* ---------------------------------------------------------------------------------------------- */

/* get preset name from preset selection combo box */
static int cmwnd_getselpresetname(cmwnd_ctx_t *ctx, int *pIsExist)
{
	TCHAR *sectname;

	/* get group name */
	if(!ComboBox_GetText(ctx->hwndComboPreset, ctx->uidata->databuf, (int)ctx->uidata->databuf_size)) {
		MessageBox(ctx->hwnd, _T("Preset name empty."), ui_title, MB_ICONINFORMATION|MB_OK);
		return 0;
	}

	/* check group exist */
	if( (sectname = malloc( (_tcslen(ctx->uidata->databuf) + 32) * sizeof(TCHAR) )) == NULL )
		return 0;
	_stprintf(sectname, _T("proc0_%s"), ctx->uidata->databuf);
	*pIsExist = (ini_sect_get(ctx->inich, sectname, 0) != NULL);
	free(sectname);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* load selected preset (command) */
static void cmwnd_loadselectedpreset(cmwnd_ctx_t *ctx, int append)
{
	int exist;
	rxproc_t *proc;
	unsigned int chidoffset;

	if(!cmwnd_getselpresetname(ctx, &exist))
		return;

	if(!exist) {
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Preset '%s' does not exist."), ctx->uidata->databuf);
		MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONINFORMATION|MB_OK);
		return;
	}

	if(!append) {
		cmwnd_deleteallchannels(ctx);
	}

	chidoffset = ctx->rx->proc_nextchid;

	rx_proc_group_load(ctx->rx, ctx->inich, ctx->uidata->databuf,
		ctx->uidata->msgbuf, ctx->uidata->msgbuf_size);

	/* send notifications */
	if(cmwnd_chanevent_begin(ctx, rx_proc_count(ctx->rx)))
	{
		for(proc = ctx->rx->proc_first; proc != NULL; proc = proc->proc_next) {
			if(proc->chid >= chidoffset) {
				cmwnd_chanevent_addchan(ctx, proc->chid);
			}
		}
		cmwnd_chanevent_notify(ctx, EVENT_PROCCHAN_CREATE);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* save selected preset (command) */
static void cmwnd_saveselectedpreset(cmwnd_ctx_t *ctx)
{
	int exist;

	if(!cmwnd_getselpresetname(ctx, &exist))
		return;

	if(exist) {
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Overwrite preset '%s'?"), ctx->uidata->databuf);
		if(MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONQUESTION|MB_YESNO) != IDYES)
			return;
	}

	if(ComboBox_FindStringExact(ctx->hwndComboPreset, -1, ctx->uidata->databuf) == CB_ERR)
		ComboBox_AddString(ctx->hwndComboPreset, ctx->uidata->databuf);

	rx_proc_group_save(ctx->rx, ctx->inich, ctx->uidata->databuf);
}

/* ---------------------------------------------------------------------------------------------- */

/* delete selected preset (command) */
static void cmwnd_deleteselectedpreset(cmwnd_ctx_t *ctx)
{
	int exist, index;

	if(!cmwnd_getselpresetname(ctx, &exist))
		return;

	if(exist) {
		_sntprintf(ctx->uidata->msgbuf, ctx->uidata->msgbuf_size,
			_T("Delete preset '%s'?"), ctx->uidata->databuf);
		if(MessageBox(ctx->hwnd, ctx->uidata->msgbuf, ui_title, MB_ICONQUESTION|MB_YESNO) != IDYES)
			return;
	}

	if( (index = ComboBox_FindStringExact(ctx->hwndComboPreset, -1, ctx->uidata->databuf)) != CB_ERR )
		ComboBox_DeleteString(ctx->hwndComboPreset, index);

	rx_proc_group_delete(ctx->inich, ctx->uidata->databuf);
}

/* ---------------------------------------------------------------------------------------------- */

/* show preset commands menu (command) */
static void cmwnd_openpresetmenu(cmwnd_ctx_t *ctx)
{
	POINT pt;

	GetCursorPos(&pt);
	TrackPopupMenu(ctx->hmenuPresetCommands, 0, pt.x, pt.y, 0, ctx->hwnd, NULL);
}

/* ---------------------------------------------------------------------------------------------- */
/* Channel manager window frequency adjust controls handling */

/* create frequency adjust controls */
static void cmwnd_createfreqadjctls(cmwnd_ctx_t *ctx, int cw, int ch)
{
	ini_sect_t *sect;
	int n, i, pref, fstepunit;
	double fstepvalue;

	ctx->hwndComboFreqStep = ui_crt_combo(ctx->uidata, ctx->hwnd, CBS_DROPDOWN|WS_TABSTOP,
		235, ch - CMWND_BTM_CTLS_OFS, 100, 200, CMWND_ID_FREQSTEPSEL);
	ctx->hwndBtnFreqDn = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		335, ch - CMWND_BTM_CTLS_OFS, 25, 21, _T("–"), CMWND_ID_FREQSTEPDOWN);
	ctx->hwndBtnFreqUp = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		360, ch - CMWND_BTM_CTLS_OFS, 25, 21, _T("+"), CMWND_ID_FREQSTEPUP);
	ctx->hwndBtnFreqSnap = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		385, ch - CMWND_BTM_CTLS_OFS, 45, 21, _T("Snap"), CMWND_ID_FREQSTEPSNAP);

	n = sizeof(cmwnd_freqadjstep) / sizeof(double);
	for(i = 0; i < n; i++)
	{
		if((cmwnd_freqadjstep[i] + 1e-3) >= 1e6)
			pref = 2;
		else if((cmwnd_freqadjstep[i] + 1e-3) >= 1e3)
			pref = 1;
		else
			pref = 0;

		fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, cmwnd_freqadjstep[i], pref, 3, 0);
		ComboBox_AddString(ctx->hwndComboFreqStep, ctx->uidata->databuf);
	}

	/* reload frequency adjust step */
	fstepvalue = 25e3;
	fstepunit = 0;
	sect = ini_sect_get(ctx->ini, _T("cmwnd"), 0);
	ini_getf(sect, _T("freq_step_value"), &fstepvalue);
	ini_geti(sect, _T("freq_step_unit"), &fstepunit);
	fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, fstepvalue, fstepunit, 3, 0);
	ComboBox_SetText(ctx->hwndComboFreqStep, ctx->uidata->databuf);
}

/* ---------------------------------------------------------------------------------------------- */

/* save frequency adjust step */
static void cmwnd_savefreqadjstep(cmwnd_ctx_t *ctx)
{
	ini_sect_t *sect;
	double fstepvalue;
	int fstepunit;

	ComboBox_GetText(ctx->hwndComboFreqStep, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	if(parse_dbl(ctx->uidata->databuf, 1, &fstepvalue, &fstepunit)) {
		sect = ini_sect_get(ctx->ini, _T("cmwnd"), 1);
		ini_setf(sect, _T("freq_step_value"), 3, fstepvalue);
		ini_seti(sect, _T("freq_step_unit"), fstepunit);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* update frequency adjust controls position */
static void cmwnd_updatefreqadjctlspos(cmwnd_ctx_t *ctx, int cw, int ch)
{
	MoveWindow(ctx->hwndComboFreqStep, 235, ch - CMWND_BTM_CTLS_OFS, 100, 200, TRUE);
	ComboBox_SetEditSel(ctx->hwndComboFreqStep, -1, 0);

	MoveWindow(ctx->hwndBtnFreqDn, 335, ch - CMWND_BTM_CTLS_OFS, 25, 21, TRUE);
	MoveWindow(ctx->hwndBtnFreqUp, 360, ch - CMWND_BTM_CTLS_OFS, 25, 21, TRUE);
	MoveWindow(ctx->hwndBtnFreqSnap, 385, ch - CMWND_BTM_CTLS_OFS, 45, 21, TRUE);
}

/* ---------------------------------------------------------------------------------------------- */
/* Other channel manager window controls handling */

/* create misc controls */
static void cmwnd_createctls(cmwnd_ctx_t *ctx, int cw, int ch)
{
	ctx->hwndBtnClear = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		0, ch - CMWND_BTM_CTLS_OFS, 45, 21, _T("Clear"), CMWND_ID_CLEARBTN);
	ctx->hwndBtnConfig = ui_crt_btnse(ctx->uidata, ctx->hwnd, WS_TABSTOP,
		435, ch - CMWND_BTM_CTLS_OFS, 55, 21, _T("Config"), CMWND_ID_CONFIGBTN);
}

/* ---------------------------------------------------------------------------------------------- */

/* update misc controls position */
static void cmwnd_updatectlspos(cmwnd_ctx_t *ctx, int cw, int ch)
{
	MoveWindow(ctx->hwndBtnClear, 0, ch - CMWND_BTM_CTLS_OFS, 45, 21, TRUE);
	MoveWindow(ctx->hwndBtnConfig, 435, ch - CMWND_BTM_CTLS_OFS, 55, 21, TRUE);
}

/* ---------------------------------------------------------------------------------------------- */
/* channel manager config */

static void cmwnd_showconfig(cmwnd_ctx_t *ctx)
{
	RECT rc;
	HWND hwnd;
	cmcfg_data_t cfg;

	GetWindowRect(ctx->hwnd, &rc);
	hwnd = cmcfg_create(ctx->uidata, ctx->hwnd, rc.left + 15, rc.top + 10);

	if(hwnd != NULL)
	{
		cfg.dispUnitChanFreq = ctx->dispUnitChanFreq;
		cfg.dispUnitFiltCutoff = ctx->dispUnitFiltCutoff;

		cfg.levelMin = ctx->levelMin;
		cfg.levelMax = ctx->levelMax;
		cfg.levelUpdateInt = ctx->levelUpdateInt;
		cfg.levelScaleToInputFc = ctx->levelScaleToInputFc;

		cmcfg_load(hwnd, &cfg);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void cmwnd_setconfig_handler(cmwnd_ctx_t *ctx, unsigned int msg, cmcfg_data_t *cfg)
{
	if(msg == EVENT_CHANMGR_SETCONFIG)
	{
		if(cfg->dispUnitChanFreq != ctx->dispUnitChanFreq)
		{
			ctx->dispUnitChanFreq = cfg->dispUnitChanFreq;
			cmwnd_cl_updateallrows(ctx, CMWND_CLCOL_FREQ);
		}

		if(cfg->dispUnitFiltCutoff != ctx->dispUnitFiltCutoff)
		{
			ctx->dispUnitFiltCutoff = cfg->dispUnitFiltCutoff;
			cmwnd_cl_updateallrows(ctx, CMWND_CLCOL_FILTER);
		}

		cmwnd_cl_levelmtr_setrange(ctx, cfg->levelMin, cfg->levelMax);
		cmwnd_cl_levelmtr_setinterval(ctx, (unsigned int)(cfg->levelUpdateInt));
		ctx->levelScaleToInputFc = cfg->levelScaleToInputFc;
	}
}

/* ---------------------------------------------------------------------------------------------- */
/* Channel manager window message handling */

/* handle WM_COMMAND, channel manager window */
static int cmwnd_command(cmwnd_ctx_t *ctx, UINT uCtrlId, UINT uEventId)
{
	switch(uCtrlId)
	{
	case CMWND_ID_CLEARBTN:
		cmwnd_deleteallchannels(ctx);
		return 1;

	case CMWND_ID_CONFIGBTN:
		cmwnd_showconfig(ctx);
		return 1;

	case CMWND_ID_PRESET_LOAD:
		cmwnd_loadselectedpreset(ctx, 0);
		return 1;

	case CMWND_ID_PRESET_APPEND:
		cmwnd_loadselectedpreset(ctx, 1);
		return 1;

	case CMWND_ID_PRESET_SAVE:
		cmwnd_saveselectedpreset(ctx);
		return 1;

	case CMWND_ID_PRESET_DELETE:
		cmwnd_deleteselectedpreset(ctx);
		return 1;

	case CMWND_ID_PRESET_MENU:
		cmwnd_openpresetmenu(ctx);
		return 1;

	case CMWND_ID_FREQSTEPUP:
	case CMWND_ID_FREQSTEPDOWN:
	case CMWND_ID_FREQSTEPSNAP:
		cmwnd_adjustselectedchannelsfreq(ctx, uCtrlId);
		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* channel manager window WM_SIZE message handler */
static void cmwnd_resize(cmwnd_ctx_t *ctx, int mode, int cw, int ch)
{
	if(mode == SIZE_RESTORED)
	{
		/* move channel list */
		MoveWindow(ctx->hwndChList, 0, 0, cw, ch - CMWND_CL_OFS_BTM, TRUE);

		/* move controls */
		cmwnd_updatectlspos(ctx, cw, ch);
		cmwnd_updatepresetctlspos(ctx, cw, ch);
		cmwnd_updatefreqadjctlspos(ctx, cw, ch);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* Channel manager window initialization/destroying */

/* channel manager window WM_CREATE message handler */
static int cmwnd_init(cmwnd_ctx_t *ctx)
{
	RECT cltrc;
	int status = 1;

	/* load params */
	cmwnd_loaddispunits(ctx);
	cmwnd_cl_levelmtr_init(ctx);

	/* create popup menus */
	cmwnd_cl_createlistpopupmenu(ctx);
	cmwnd_cl_createchanpopupmenu(ctx);
	cmwnd_cl_createdemodpopupmenu(ctx);

	/* create controls */
	GetClientRect(ctx->hwnd, &cltrc);
	cmwnd_createctls(ctx, cltrc.right, cltrc.bottom);
	cmwnd_createpresetctls(ctx, cltrc.right, cltrc.bottom);
	cmwnd_createfreqadjctls(ctx, cltrc.right, cltrc.bottom);

	/* create channel list */
	cmwnd_cl_create(ctx, cltrc.right, cltrc.bottom);
	cmwnd_cl_crtcols(ctx);
	cmwnd_cl_fillrows(ctx);

	/* subscribe to notifications */
	uievent_handler_add(&(ctx->uidata->event_list), EVENT_NAME_RX_STATE, ctx, cmwnd_rx_state_handler);
	uievent_handler_add(&(ctx->uidata->event_list), EVENT_NAME_PROCCHAN, ctx, cmwnd_procchan_handler);
	uievent_handler_add(&(ctx->uidata->event_list), EVENT_NAME_CHANMGRCFG, ctx, cmwnd_setconfig_handler);

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* channel manager window WM_DESTROY message handler */
static void cmwnd_destroy(cmwnd_ctx_t *ctx)
{
	/* unsubscribe from notifications */
	uievent_handler_remove(&(ctx->uidata->event_list), EVENT_NAME_CHANMGRCFG, ctx, cmwnd_setconfig_handler);
	uievent_handler_remove(&(ctx->uidata->event_list), EVENT_NAME_PROCCHAN, ctx, cmwnd_procchan_handler);
	uievent_handler_remove(&(ctx->uidata->event_list), EVENT_NAME_RX_STATE, ctx, cmwnd_rx_state_handler);

	/* save params */
	cmwnd_cl_colcx_save(ctx);
	cmwnd_savefreqadjstep(ctx);
	cmwnd_savedispunits(ctx);
	cmwnd_cl_levelmtr_cleanup(ctx);

	/* clear and destroy channel list */
	cmwnd_cl_destroy(ctx);

	/* destroy popup menus */
	DestroyMenu(ctx->hmenuChListPopup);
	DestroyMenu(ctx->hmenuChanPopup);
	DestroyMenu(ctx->hmenuDemodType);
	DestroyMenu(ctx->hmenuPresetCommands);

	/* save window position */
	ui_savepos(ctx->hwnd, ini_sect_get(ctx->ini, _T("cmwnd"), 1),
		UI_POS_OFFSET|UI_POS_SIZE, ctx->cx_frame, ctx->cy_frame);

	/* send destroying notification */
	uievent_send(ctx->event_window_close, EVENT_WNDCLOSE_CHANNELMGR, ctx->hwnd);
}

/* ---------------------------------------------------------------------------------------------- */

/* channel manager window WM_NCDESTROY message handler */
static void cmwnd_cleanup(cmwnd_ctx_t *ctx)
{
	/* free memory */
	free(ctx->chan_notify_list.items);
	free(ctx->rowlist.row);
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */
/* Channel manager window proc */

static LRESULT CALLBACK cmwnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	cmwnd_ctx_t *ctx;

	switch(uMsg)
	{
	case WM_CREATE:
		ctx = ((CREATESTRUCT*)lParam)->lpCreateParams;
		ctx->hwnd = hwnd;
		SetWndPtr(hwnd, GWLP_USERDATA, ctx);
		if(!cmwnd_init(ctx))
			return -1;
		return 0;

	case WM_DESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		cmwnd_destroy(ctx);
		return 0;

	case WM_NCDESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		cmwnd_cleanup(ctx);
		return 0;

	case WM_SIZE:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		cmwnd_resize(ctx, (int)wParam, LOWORD(lParam), HIWORD(lParam));
		return 0;

	case WM_SIZING:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		ui_sizingcheck((int)wParam, (RECT*)lParam, ctx->cx_frame, ctx->cy_frame,
			CMWND_MINCLTW, CMWND_MINCLTH, CMWND_MAXCLTW, -1);
		return 0;

	case WM_NOTIFY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		if(cmwnd_wmnotify(ctx, (void*)lParam))
			return 0;
		break;

	case WM_COMMAND:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		if(cmwnd_command(ctx, LOWORD(wParam), HIWORD(wParam)))
			return 0;
		break;

	case WM_TIMER:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(wParam)
		{
		case CMWND_IDT_UPDATELEVEL:
			cmwnd_cl_levelmtr_updateall(ctx);
			return 0;
		}
		break;

	case WM_ACTIVATE:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(wParam)
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			keynav_setcurwnd(&(ctx->uidata->keynav), hwnd, 0);
			break;
		case WA_INACTIVE:
			keynav_unsetcurwnd(&(ctx->uidata->keynav), hwnd);
			break;
		}
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/* ---------------------------------------------------------------------------------------------- */
/* Create channel manager window */

HWND cmwnd_create(uicommon_t *uidata, ini_data_t *ini, ini_data_t *inich,
				  rxstate_t *rx, rxprocconfig_t *procdefcfg, HWND hwndMain)
{
	cmwnd_ctx_t *ctx;
	int winw, winh, xpos, ypos;
	int sizestate;
	HWND hwnd;

	/* alloc memory */
	if( (ctx = calloc(1, sizeof(cmwnd_ctx_t))) == NULL )
		return NULL;

	/* set params */
	ctx->hwndMain = hwndMain;

	ctx->uidata = uidata;

	ctx->ini = ini;
	ctx->inich = inich;
	ctx->rx = rx;
	ctx->procdefcfg = procdefcfg;

	/* register events */
	ctx->event_procchan = uievent_register(&(uidata->event_list), EVENT_NAME_PROCCHAN);
	ctx->event_window_close = uievent_register(&(uidata->event_list), EVENT_NAME_WNDCLOSE);

	/* window size and position */
	ui_frame_size(&(ctx->cx_frame), &(ctx->cy_frame),
		UI_FRAME_SIZING|UI_FRAME_CAPTION);

	ui_calcpos(ini_sect_get(ini, _T("cmwnd"), 0), UI_POS_OFFSET|UI_POS_SIZE,
		ctx->cx_frame, ctx->cy_frame, CMWND_DEFCLTW, CMWND_DEFCLTH,
		CMWND_MINCLTW, CMWND_MINCLTH, &xpos, &ypos, &winw, &winh, &sizestate, SW_SHOWNORMAL);

	/* create window */
	hwnd = CreateWindow(MAKEINTATOM(cmwnd_classatom), cmwnd_wndname,
		WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_THICKFRAME|WS_MINIMIZEBOX,
		xpos, ypos, winw, winh, hwndMain, NULL, uidata->h_inst, ctx);

	if(hwnd == NULL)
	{
		/* do not free anything */
		return NULL;
	}

	ShowWindow(hwnd, sizestate);

	return hwnd;
}

/* ---------------------------------------------------------------------------------------------- */
/* Register/unregister channel manager window class */

/* register channel manager window class */
int cmwnd_registerclass(uicommon_t *uidata)
{
	WNDCLASSEX wcl;

	if(cmwnd_classatom == 0)
	{
		memset(&wcl, 0, sizeof(wcl));
		wcl.cbSize = sizeof(wcl);
		wcl.lpfnWndProc = cmwnd_proc;
		wcl.hInstance = uidata->h_inst;
		wcl.hIcon = uidata->hicn_main;
		wcl.hCursor = uidata->hcur_main;
		wcl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
		wcl.lpszClassName = cmwnd_classname;
		wcl.hIconSm = uidata->hicn_sm;

		cmwnd_classatom = RegisterClassEx(&wcl);
	}

	return (cmwnd_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

/* unregister channel manager window class */
void cmwnd_unregisterclass(HINSTANCE h_inst)
{
	if(cmwnd_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(cmwnd_classatom), h_inst);
		cmwnd_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
