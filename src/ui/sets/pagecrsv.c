/* ---------------------------------------------------------------------------------------------- */

#include "pagecrsv.h"

/* ---------------------------------------------------------------------------------------------- */

static COLORREF pagecrsv_colordlg_customcolors[16];

/* ---------------------------------------------------------------------------------------------- */

static void pagecrsv_cr_get(COLORREF cr[][PAGECRSV_COL_COUNT], specview_cfg_t *cfg)
{
	cr[PAGECRSV_ROW_BKGND][PAGECRSV_COL_CN] = cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND];
	cr[PAGECRSV_ROW_BKGND][PAGECRSV_COL_CG] = cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND_GRID];
	cr[PAGECRSV_ROW_BKGND][PAGECRSV_COL_HN] = cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND];
	cr[PAGECRSV_ROW_BKGND][PAGECRSV_COL_HG] = cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND_GRID];

	cr[PAGECRSV_ROW_INPUT_FC][PAGECRSV_COL_CN] = cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC];
	cr[PAGECRSV_ROW_INPUT_FC][PAGECRSV_COL_CG] = cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC_GRID];
	cr[PAGECRSV_ROW_INPUT_FC][PAGECRSV_COL_HN] = cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC];
	cr[PAGECRSV_ROW_INPUT_FC][PAGECRSV_COL_HG] = cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC_GRID];

	cr[PAGECRSV_ROW_CHAN_FC][PAGECRSV_COL_CN] = cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC];
	cr[PAGECRSV_ROW_CHAN_FC][PAGECRSV_COL_CG] = cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC_GRID];
	cr[PAGECRSV_ROW_CHAN_FC][PAGECRSV_COL_HN] = cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC];
	cr[PAGECRSV_ROW_CHAN_FC][PAGECRSV_COL_HG] = cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC_GRID];

	cr[PAGECRSV_ROW_CHAN_BW][PAGECRSV_COL_CN] = cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW];
	cr[PAGECRSV_ROW_CHAN_BW][PAGECRSV_COL_CG] = cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW_GRID];
	cr[PAGECRSV_ROW_CHAN_BW][PAGECRSV_COL_HN] = cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW];
	cr[PAGECRSV_ROW_CHAN_BW][PAGECRSV_COL_HG] = cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW_GRID];

	cr[PAGECRSV_ROW_CHAN_BWOVL][PAGECRSV_COL_CN] = cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL];
	cr[PAGECRSV_ROW_CHAN_BWOVL][PAGECRSV_COL_CG] = cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL_GRID];
	cr[PAGECRSV_ROW_CHAN_BWOVL][PAGECRSV_COL_HN] = cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL];
	cr[PAGECRSV_ROW_CHAN_BWOVL][PAGECRSV_COL_HG] = cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL_GRID];
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrsv_cr_set(specview_cfg_t *cfg, COLORREF cr[][PAGECRSV_COL_COUNT])
{
	cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND] = cr[PAGECRSV_ROW_BKGND][PAGECRSV_COL_CN];
	cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND_GRID] = cr[PAGECRSV_ROW_BKGND][PAGECRSV_COL_CG];
	cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND] = cr[PAGECRSV_ROW_BKGND][PAGECRSV_COL_HN];
	cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND_GRID] = cr[PAGECRSV_ROW_BKGND][PAGECRSV_COL_HG];

	cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC] = cr[PAGECRSV_ROW_INPUT_FC][PAGECRSV_COL_CN];
	cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC_GRID] = cr[PAGECRSV_ROW_INPUT_FC][PAGECRSV_COL_CG];
	cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC] = cr[PAGECRSV_ROW_INPUT_FC][PAGECRSV_COL_HN];
	cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC_GRID] = cr[PAGECRSV_ROW_INPUT_FC][PAGECRSV_COL_HG];

	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC] = cr[PAGECRSV_ROW_CHAN_FC][PAGECRSV_COL_CN];
	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC_GRID] = cr[PAGECRSV_ROW_CHAN_FC][PAGECRSV_COL_CG];
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC] = cr[PAGECRSV_ROW_CHAN_FC][PAGECRSV_COL_HN];
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC_GRID] = cr[PAGECRSV_ROW_CHAN_FC][PAGECRSV_COL_HG];

	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW] = cr[PAGECRSV_ROW_CHAN_BW][PAGECRSV_COL_CN];
	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW_GRID] = cr[PAGECRSV_ROW_CHAN_BW][PAGECRSV_COL_CG];
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW] = cr[PAGECRSV_ROW_CHAN_BW][PAGECRSV_COL_HN];
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW_GRID] = cr[PAGECRSV_ROW_CHAN_BW][PAGECRSV_COL_HG];

	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL] = cr[PAGECRSV_ROW_CHAN_BWOVL][PAGECRSV_COL_CN];
	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL_GRID] = cr[PAGECRSV_ROW_CHAN_BWOVL][PAGECRSV_COL_CG];
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL] = cr[PAGECRSV_ROW_CHAN_BWOVL][PAGECRSV_COL_HN];
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL_GRID] = cr[PAGECRSV_ROW_CHAN_BWOVL][PAGECRSV_COL_HG];
}

/* ---------------------------------------------------------------------------------------------- */

void pagecrsv_data_init(pagecrsv_data_t *data, specview_cfg_t *svcfg)
{
	data->cr_label = svcfg->cr_label;
	data->cr_ticks = svcfg->cr_ticks;
	data->cr_textbg = svcfg->cr_textbg;
	data->cr_text = svcfg->cr_text;

	data->cr_sq_sense_op = svcfg->cr_sq_sense_op;
	data->cr_sq_sense_cl = svcfg->cr_sq_sense_cl;
	data->cr_sq_thres_op = svcfg->cr_sq_thres_op;
	data->cr_sq_thres_cl = svcfg->cr_sq_thres_cl;

	pagecrsv_cr_get(data->cr, svcfg);
}

/* ---------------------------------------------------------------------------------------------- */

int pagecrsv_data_apply(specview_ctx_t *specview, pagecrsv_data_t *data,
						callback_list_t *cb_list, HWND hwndMsgbox)
{
	int status = 1;
	COLORREF cr_cur[PAGECRSV_ROW_COUNT][PAGECRSV_COL_COUNT];

	pagecrsv_cr_get(cr_cur, &(specview->cfg));

	/* check spectrum viewer colors changed */
	if( (specview->cfg.cr_label != data->cr_label) ||
		(specview->cfg.cr_ticks != data->cr_ticks) ||
		(specview->cfg.cr_textbg != data->cr_textbg) ||
		(specview->cfg.cr_text != data->cr_text) ||
		(specview->cfg.cr_sq_sense_op != data->cr_sq_sense_op) ||
		(specview->cfg.cr_sq_sense_cl != data->cr_sq_sense_cl) ||
		(specview->cfg.cr_sq_thres_op != data->cr_sq_thres_op) ||
		(specview->cfg.cr_sq_thres_cl != data->cr_sq_thres_cl) ||
		(memcmp(data->cr, cr_cur, sizeof(cr_cur)) != 0) )
	{
		/* set new colors */
		specview->cfg.cr_label = data->cr_label;
		specview->cfg.cr_ticks = data->cr_ticks;
		specview->cfg.cr_textbg = data->cr_textbg;
		specview->cfg.cr_text = data->cr_text;
		specview->cfg.cr_sq_sense_op = data->cr_sq_sense_op;
		specview->cfg.cr_sq_sense_cl = data->cr_sq_sense_cl;
		specview->cfg.cr_sq_thres_op = data->cr_sq_thres_op;
		specview->cfg.cr_sq_thres_cl = data->cr_sq_thres_cl;
		pagecrsv_cr_set(&(specview->cfg), data->cr);

		/* reinitialize spectrum viewer resources */
		if(!specview_res_reinit(specview))
		{
			MessageBox(hwndMsgbox,
				_T("Can't initialize spectrum viewer resources."),
				ui_title, MB_ICONEXCLAMATION|MB_OK);
			status = 0;
		}

		/* force spectrum viewer redraw */
		callback_list_call(cb_list,
			NOTIFY_VISUALCFG, NOTIFY_VISUALCFG_SPECVIEW, NULL);
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_choosecolor(pagecrsv_ctx_t *ctx, COLORREF *pcr)
{
	CHOOSECOLOR cc;

	memset(&cc, 0, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = ctx->hwndSetWnd;
	cc.lpCustColors = pagecrsv_colordlg_customcolors;
	cc.rgbResult = *pcr;
	cc.Flags = CC_FULLOPEN|CC_RGBINIT;

	if(ChooseColor(&cc))
	{
		*pcr = cc.rgbResult;
		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_list_edit_getrect(pagecrsv_ctx_t *ctx, int row, int col, RECT *prc)
{
	if(!ListView_GetSubItemRect(ctx->hwndCrList, row, col + 1, LVIR_BOUNDS, prc))
		return 0;

	prc->right -= 16;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_list_edit_close(pagecrsv_ctx_t *ctx, int save, int forceclose)
{
	int status = 1;
	COLORREF cr;

	if(ctx->hwndCrListEdit == NULL)
		return 0;

	if(save)
	{
		Edit_GetText(ctx->hwndCrListEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));

		if(!parse_rgb(ctx->uidata->databuf, &cr))
		{
			if(!forceclose)
			{
				MessageBox(ctx->hwndSetWnd, _T("Bad color value entered."),
					ui_title, MB_ICONEXCLAMATION|MB_OK);
				SetFocus(ctx->hwndCrListEdit);
				return 0;
			}
			status = 0;
		}
		else
		{
			ListView_SetItemText(ctx->hwndCrList,
				ctx->iCrListEditRow, ctx->iCrListEditCol + 1,
				fmt_rgb(ctx->uidata->databuf, cr));
		}
	}

	DestroyWindow(ctx->hwndCrListEdit);

	InvalidateRect(ctx->hwndCrList, &(ctx->rcCrListEdit), FALSE);
	UpdateWindow(ctx->hwndCrList);

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK pagecrsv_list_edit_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	pagecrsv_ctx_t *ctx;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	switch(umsg)
	{
	case WM_KEYDOWN:
		switch(LOBYTE(wp))
		{
		case VK_RETURN:
			pagecrsv_list_edit_close(ctx, 1, 0);
			return 0;
		case VK_ESCAPE:
			pagecrsv_list_edit_close(ctx, 0, 0);
			return 0;
		}
		break;
	case WM_DESTROY:
		ctx->hwndCrListEdit = NULL;
		break;
	}

	return CallWindowProc(ctx->wpCrListEdit, hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_list_edit_open(pagecrsv_ctx_t *ctx, int row, int col)
{
	if( (row < 0) || (row >= PAGECRSV_ROW_COUNT) ||
		(col < 0) || (col >= PAGECRSV_ROW_COUNT) ||
		(ctx->hwndCrListEdit != NULL) )
	{
		return 0;
	}

	ctx->iCrListEditRow = row;
	ctx->iCrListEditCol = col;

	if(!pagecrsv_list_edit_getrect(ctx, row, col, &(ctx->rcCrListEdit)))
		return 0;

	if( (ctx->rcCrListEdit.right - ctx->rcCrListEdit.left < 12) ||
		(ctx->rcCrListEdit.bottom - ctx->rcCrListEdit.top < 6) )
	{
		return 0;
	}

	ListView_GetItemText(ctx->hwndCrList, row, col + 1,
		ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));

	ctx->hwndCrListEdit = CreateWindow(
		_T("EDIT"), ctx->uidata->databuf,
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|ES_AUTOHSCROLL, 
		ctx->rcCrListEdit.left, ctx->rcCrListEdit.top,
		ctx->rcCrListEdit.right - ctx->rcCrListEdit.left,
		ctx->rcCrListEdit.bottom - ctx->rcCrListEdit.top,
		ctx->hwndCrList, (HMENU)PAGECRSV_ID_CRLIST_EDIT, ctx->uidata->h_inst, NULL);

	if(ctx->hwndCrListEdit == NULL)
		return 0;

	SetWndPtr(ctx->hwndCrListEdit, GWLP_USERDATA, ctx);

	ctx->wpCrListEdit = (WNDPROC)SetWndPtr(ctx->hwndCrListEdit,
		GWLP_WNDPROC, (void*)pagecrsv_list_edit_proc);

	SetWindowFont(ctx->hwndCrListEdit, ctx->uidata->hfnt_mono, FALSE);
	Edit_SetSel(ctx->hwndCrListEdit, 0, -1);
	SetFocus(ctx->hwndCrListEdit);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_list_edit_updatepos(pagecrsv_ctx_t *ctx)
{
	RECT rc;

	if(ctx->hwndCrListEdit == NULL)
		return 0;

	if(!pagecrsv_list_edit_getrect(ctx, ctx->iCrListEditRow, ctx->iCrListEditCol, &rc))
		return 0;
	ui_updatewndrect(ctx->hwndCrListEdit, &(ctx->rcCrListEdit), &rc, UI_SWR_REDRAWNOERASE);
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_list_pickbtn_getrect(pagecrsv_ctx_t *ctx, int row, int col, RECT *prc)
{
	if(!ListView_GetSubItemRect(ctx->hwndCrList, row, col + 1, LVIR_BOUNDS, prc))
		return 0;

	prc->left = prc->right - 16;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrsv_list_pickbtn_updateposall(pagecrsv_ctx_t *ctx)
{
	int row, col;
	pagecrsv_rowdata_t *rowdata;
	RECT rc;

	for(row = 0; row < PAGECRSV_ROW_COUNT; row++)
	{
		rowdata = ctx->rowdata + row;
		for(col = 0; col < PAGECRSV_COL_COUNT; col++)
		{
			if( (rowdata->hwndBtnColorPick[col] != NULL) &&
				(pagecrsv_list_pickbtn_getrect(ctx, row, col, &rc)) )
			{
				ui_updatewndrect(rowdata->hwndBtnColorPick[col],
					&(rowdata->rcBtnColorPick[col]), &rc, UI_SWR_REDRAWNOERASE);
			}
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrsv_list_pickbtn_click(pagecrsv_ctx_t *ctx, int btn)
{
	int row, col;
	COLORREF cr;

	row = btn / PAGECRSV_COL_COUNT;
	col = btn % PAGECRSV_COL_COUNT;

	if( (ctx->hwndCrListEdit != NULL) &&
		(ctx->iCrListEditRow == row) && (ctx->iCrListEditCol == col) )
	{
		Edit_GetText(ctx->hwndCrListEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
		parse_rgb(ctx->uidata->databuf, &cr);
	}
	else
	{
		ListView_GetItemText(ctx->hwndCrList, row, col + 1,
			ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
		parse_rgb(ctx->uidata->databuf, &cr);
	}

	if(pagecrsv_choosecolor(ctx, &cr))
	{
		if( (ctx->hwndCrListEdit != NULL) &&
			(ctx->iCrListEditRow == row) && (ctx->iCrListEditCol == col) )
		{
			Edit_SetText(ctx->hwndCrListEdit, fmt_rgb(ctx->uidata->databuf, cr));
			Edit_SetSel(ctx->hwndCrListEdit, 0, -1);
			SetFocus(ctx->hwndCrListEdit);
		}
		else
		{
			ListView_SetItemText(ctx->hwndCrList, row, col + 1, fmt_rgb(ctx->uidata->databuf, cr));
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrsv_list_editkey(pagecrsv_ctx_t *ctx, int col)
{
	int row, n;

	n = ListView_GetItemCount(ctx->hwndCrList);

	for(row = 0; row < n; row++)
	{
		if(ListView_GetItemState(ctx->hwndCrList, row, LVIS_SELECTED))
		{
			pagecrsv_list_edit_open(ctx, row, col);
			break;
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK pagecrsv_list_wndproc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	pagecrsv_ctx_t *ctx;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	switch(umsg)
	{
	case WM_PAINT:
		pagecrsv_list_edit_updatepos(ctx);
		pagecrsv_list_pickbtn_updateposall(ctx);
		break;

	case WM_COMMAND:
		if( (LOWORD(wp) >= PAGECRSV_ID_PICKBTN_0) &&
			(LOWORD(wp) <= PAGECRSV_ID_PICKBTN_MAX) )
		{
			pagecrsv_list_pickbtn_click(ctx,
				LOWORD(wp) - PAGECRSV_ID_PICKBTN_0);
			return 0;
		}
		break;

	case WM_KEYDOWN:
		switch(LOBYTE(wp))
		{
		case VK_F3:
			pagecrsv_list_editkey(ctx, PAGECRSV_COL_CN);
			return 0;
		case VK_F4:
			pagecrsv_list_editkey(ctx, PAGECRSV_COL_CG);
			return 0;
		case VK_F5:
			pagecrsv_list_editkey(ctx, PAGECRSV_COL_HN);
			return 0;
		case VK_F6:
			pagecrsv_list_editkey(ctx, PAGECRSV_COL_HG);
			return 0;
		}
		break;
	}

	return CallWindowProc(ctx->wpCrListOriginal, hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_list_doubleclick(pagecrsv_ctx_t *ctx, NMITEMACTIVATE *nm)
{
	pagecrsv_list_edit_open(ctx, nm->iItem, nm->iSubItem - 1);
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_list_click(pagecrsv_ctx_t *ctx, NMITEMACTIVATE *nm)
{
	pagecrsv_list_edit_close(ctx, 1, 0);
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_list_insertrow(pagecrsv_ctx_t *ctx, int row, TCHAR *name, COLORREF *crp)
{
	LVITEM lvi;
	int col;
	pagecrsv_rowdata_t *rowdata;
	UINT ucommand;

	rowdata = ctx->rowdata + row;

	memset(&lvi, 0, sizeof(lvi));

	lvi.mask = LVIF_TEXT;
	lvi.iItem = row;
	lvi.pszText = name;
	ListView_InsertItem(ctx->hwndCrList, &lvi);

	lvi.mask = LVIF_TEXT;
	lvi.pszText = ctx->uidata->databuf;

	for(col = 0; col < PAGECRSV_COL_COUNT; col++)
	{
		
		lvi.iSubItem = col + 1;
		fmt_rgb(ctx->uidata->databuf, crp[col]);
		ListView_SetItem(ctx->hwndCrList, &lvi);

		if( pagecrsv_list_pickbtn_getrect(ctx, row, col, &(rowdata->rcBtnColorPick[col])) )
		{
			ucommand = PAGECRSV_ID_PICKBTN_0 + row * PAGECRSV_COL_COUNT + col;

			rowdata->hwndBtnColorPick[col] = ui_crt_btn(
				ctx->uidata, ctx->hwndCrList, WS_CLIPSIBLINGS,
				rowdata->rcBtnColorPick[col].left, rowdata->rcBtnColorPick[col].top,
				rowdata->rcBtnColorPick[col].right - rowdata->rcBtnColorPick[col].left,
				rowdata->rcBtnColorPick[col].bottom - rowdata->rcBtnColorPick[col].top,
				_T("..."), ucommand);
			SetWindowFont(rowdata->hwndBtnColorPick[col], ctx->uidata->hfnt_mini, FALSE);
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrsv_edit_color(pagecrsv_ctx_t *ctx, HWND hwndEdit)
{
	COLORREF cr;

	Edit_GetText(hwndEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	parse_rgb(ctx->uidata->databuf, &cr);

	if(pagecrsv_choosecolor(ctx, &cr))
		Edit_SetText(hwndEdit, fmt_rgb(ctx->uidata->databuf, cr));
}

/* ---------------------------------------------------------------------------------------------- */

int pagecrsv_save(pagecrsv_data_t *data, HWND hwndPage)
{
	pagecrsv_ctx_t *ctx;
	int row, col;

	ctx = GetWndPtr(hwndPage, GWLP_USERDATA);

	pagecrsv_list_edit_close(ctx, 1, 1);

	for(row = 0; row < PAGECRSV_ROW_COUNT; row++)
	{
		for(col = 0; col < PAGECRSV_COL_COUNT; col++)
		{
			ListView_GetItemText(ctx->hwndCrList, row, col + 1,
				ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
			parse_rgb(ctx->uidata->databuf, &(data->cr[row][col]));
		}
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrLabelEdit, &(data->cr_label),
		_T("Label color")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrTicksEdit, &(data->cr_ticks),
		_T("Tick color")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrTextBgEdit, &(data->cr_textbg),
		_T("Text background")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrTextEdit, &(data->cr_text),
		_T("Text color")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrSqSenseOp, &(data->cr_sq_sense_op),
		_T("Squelch level mark color (active)")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrSqSenseCl, &(data->cr_sq_sense_cl),
		_T("Squelch level mark color (closed)")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrSqThresOp, &(data->cr_sq_thres_op),
		_T("Squelch open threshold mark color")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrSqThresCl, &(data->cr_sq_thres_cl),
		_T("Squelch closed threshold mark color")) )
	{
		return 0;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrsv_init(pagecrsv_ctx_t *ctx, pagecrsv_data_t *data)
{
	LVCOLUMN lvc;

	/* create listview */
	ctx->hwndCrList = ui_crt_listview(ctx->uidata, ctx->hwndPage,
		LVS_REPORT|LVS_SHOWSELALWAYS|WS_TABSTOP, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT,
		0, 0, 480, 105, PAGECRSV_ID_CRLIST);
	SetWindowFont(ctx->hwndCrList, ctx->uidata->hfnt_mono, FALSE);

	SetWndPtr(ctx->hwndCrList, GWLP_USERDATA, ctx);
	
	ctx->wpCrListOriginal = (WNDPROC)SetWndPtr(ctx->hwndCrList,
		GWLP_WNDPROC, (void*)pagecrsv_list_wndproc);

	/* create columns */
	memset(&lvc, 0, sizeof(lvc));
	lvc.mask = LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;

	lvc.cx = 135;
	lvc.pszText = _T("Element name");
	ListView_InsertColumn(ctx->hwndCrList, lvc.iSubItem, &lvc);

	lvc.cx = 80;
	lvc.pszText = _T("Normal");
	lvc.iSubItem = PAGECRSV_COL_CN + 1;
	ListView_InsertColumn(ctx->hwndCrList, lvc.iSubItem, &lvc);

	lvc.cx = 80;
	lvc.pszText = _T("Nor grid");
	lvc.iSubItem = PAGECRSV_COL_CG + 1;
	ListView_InsertColumn(ctx->hwndCrList, lvc.iSubItem, &lvc);

	lvc.cx = 80;
	lvc.pszText = _T("Active");
	lvc.iSubItem = PAGECRSV_COL_HN + 1;
	ListView_InsertColumn(ctx->hwndCrList, lvc.iSubItem, &lvc);

	lvc.cx = 80;
	lvc.pszText = _T("Act grid");
	lvc.iSubItem = PAGECRSV_COL_HG + 1;
	ListView_InsertColumn(ctx->hwndCrList, lvc.iSubItem, &lvc);

	/* create rows */
	pagecrsv_list_insertrow(ctx, PAGECRSV_ROW_BKGND,
		_T("Background"), data->cr[PAGECRSV_ROW_BKGND]);
	pagecrsv_list_insertrow(ctx, PAGECRSV_ROW_INPUT_FC,
		_T("Input center"), data->cr[PAGECRSV_ROW_INPUT_FC]);
	pagecrsv_list_insertrow(ctx, PAGECRSV_ROW_CHAN_FC,
		_T("Channel center"), data->cr[PAGECRSV_ROW_CHAN_FC]);
	pagecrsv_list_insertrow(ctx, PAGECRSV_ROW_CHAN_BW,
		_T("Channel bandwidth"), data->cr[PAGECRSV_ROW_CHAN_BW]);
	pagecrsv_list_insertrow(ctx, PAGECRSV_ROW_CHAN_BWOVL,
		_T("Channel overlap"), data->cr[PAGECRSV_ROW_CHAN_BWOVL]);

	/* create other color controls */
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		0, 120, 150, 15, _T("Other elements:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 140, 100, 15, _T("Gauge label"), ID_CTL_STATIC);
	ctx->hwndEditCrLabelEdit = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		110, 140, 60, 15, PAGECRSV_ID_CR_LABEL_EDIT);
	ctx->hwndBtnCrLabelPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		175, 140, 15, 15, _T("..."), PAGECRSV_ID_CR_LABEL_PICK);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 160, 100, 15, _T("Gauge tick"), ID_CTL_STATIC);
	ctx->hwndEditCrTicksEdit = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		110, 160, 60, 15, PAGECRSV_ID_CR_TICKS_EDIT);
	ctx->hwndBtnCrTicksPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		175, 160, 15, 15, _T("..."), PAGECRSV_ID_CR_TICKS_PICK);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 180, 100, 15, _T("Hint text bkgnd"), ID_CTL_STATIC);
	ctx->hwndEditCrTextBgEdit = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		110, 180, 60, 15, PAGECRSV_ID_CR_TEXTBG_EDIT);
	ctx->hwndBtnCrTextBgPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		175, 180, 15, 15, _T("..."), PAGECRSV_ID_CR_TEXTBG_PICK);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		10, 200, 100, 15, _T("Hint text color"), ID_CTL_STATIC);
	ctx->hwndEditCrTextEdit = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		110, 200, 60, 15, PAGECRSV_ID_CR_TEXT_EDIT);
	ctx->hwndBtnCrTextPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		175, 200, 15, 15, _T("..."), PAGECRSV_ID_CR_TEXT_PICK);

	/* squelch-related element colors */
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		220, 120, 150, 15, _T("Squelch marks:"), ID_CTL_STATIC);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		230, 140, 100, 15, _T("Level (active)"), ID_CTL_STATIC);
	ctx->hwndEditCrSqSenseOp = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		330, 140, 60, 15, PAGECRSV_ID_CR_SQ_SENSE_OP_EDIT);
	ctx->hwndBtnCrSqSenseOpPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 140, 15, 15, _T("..."), PAGECRSV_ID_CR_SQ_SENSE_OP_PICK);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		230, 160, 100, 15, _T("Level (closed)"), ID_CTL_STATIC);
	ctx->hwndEditCrSqSenseCl = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		330, 160, 60, 15, PAGECRSV_ID_CR_SQ_SENSE_CL_EDIT);
	ctx->hwndBtnCrSqSenseClPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 160, 15, 15, _T("..."), PAGECRSV_ID_CR_SQ_SENSE_CL_PICK);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		230, 180, 100, 15, _T("Open threshold"), ID_CTL_STATIC);
	ctx->hwndEditCrSqThresOp = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		330, 180, 60, 15, PAGECRSV_ID_CR_SQ_THRES_OP_EDIT);
	ctx->hwndBtnCrSqThresOpPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 180, 15, 15, _T("..."), PAGECRSV_ID_CR_SQ_THRES_OP_PICK);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		230, 200, 100, 15, _T("Close threshold"), ID_CTL_STATIC);
	ctx->hwndEditCrSqThresCl = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		330, 200, 60, 15, PAGECRSV_ID_CR_SQ_THRES_CL_EDIT);
	ctx->hwndBtnCrSqThresClPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		395, 200, 15, 15, _T("..."), PAGECRSV_ID_CR_SQ_THRES_CL_PICK);

	SetWindowFont(ctx->hwndEditCrLabelEdit, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrTicksEdit, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrTextBgEdit, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrTextEdit, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrSqSenseOp, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrSqSenseCl, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrSqThresOp, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrSqThresCl, ctx->uidata->hfnt_mono, FALSE);

	SetWindowFont(ctx->hwndBtnCrLabelPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrTicksPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrTextBgPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrTextPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrSqSenseOpPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrSqSenseClPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrSqThresOpPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrSqThresClPick, ctx->uidata->hfnt_mini, FALSE);

	ui_set_rgb(ctx->uidata, ctx->hwndEditCrLabelEdit, data->cr_label);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrTicksEdit, data->cr_ticks);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrTextBgEdit, data->cr_textbg);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrTextEdit, data->cr_text);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrSqSenseOp, data->cr_sq_sense_op);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrSqSenseCl, data->cr_sq_sense_cl);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrSqThresOp, data->cr_sq_thres_op);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrSqThresCl, data->cr_sq_thres_cl);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrsv_cleanup(pagecrsv_ctx_t *ctx)
{
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK pagecrsv_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	pagecrsv_ctx_t *ctx;

	switch(umsg)
	{
	case WM_NCDESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);
		pagecrsv_cleanup(ctx);
		return 0;

	case WM_COMMAND:

		ctx = GetWndPtr(hwnd, GWLP_USERDATA);

		switch(LOWORD(wp))
		{
		case PAGECRSV_ID_CR_LABEL_PICK:
			pagecrsv_edit_color(ctx, ctx->hwndEditCrLabelEdit);
			return 0;
		case PAGECRSV_ID_CR_TICKS_PICK:
			pagecrsv_edit_color(ctx, ctx->hwndEditCrTicksEdit);
			return 0;
		case PAGECRSV_ID_CR_TEXTBG_PICK:
			pagecrsv_edit_color(ctx, ctx->hwndEditCrTextBgEdit);
			return 0;
		case PAGECRSV_ID_CR_TEXT_PICK:
			pagecrsv_edit_color(ctx, ctx->hwndEditCrTextEdit);
			return 0;
		case PAGECRSV_ID_CR_SQ_SENSE_OP_PICK:
			pagecrsv_edit_color(ctx, ctx->hwndEditCrSqSenseOp);
			return 0;
		case PAGECRSV_ID_CR_SQ_SENSE_CL_PICK:
			pagecrsv_edit_color(ctx, ctx->hwndEditCrSqSenseCl);
			return 0;
		case PAGECRSV_ID_CR_SQ_THRES_OP_PICK:
			pagecrsv_edit_color(ctx, ctx->hwndEditCrSqThresOp);
			return 0;
		case PAGECRSV_ID_CR_SQ_THRES_CL_PICK:
			pagecrsv_edit_color(ctx, ctx->hwndEditCrSqThresCl);
			return 0;
		}

		break;

	case WM_NOTIFY:

		ctx = GetWndPtr(hwnd, GWLP_USERDATA);

		switch(wp)
		{
		case PAGECRSV_ID_CRLIST:

			switch( ((NMHDR*)lp)->code )
			{
			case NM_CLICK:
				pagecrsv_list_click(ctx, (void*)lp);
				return 0;
			case NM_DBLCLK:
				pagecrsv_list_doubleclick(ctx, (void*)lp);
				return 0;
			}

			break;
		}

		break;
	}

	return DefWindowProc(hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

int pagecrsv_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata, pagecrsv_data_t *data)
{
	pagecrsv_ctx_t *ctx;

	if( (ctx = calloc(1, sizeof(pagecrsv_ctx_t))) != NULL )
	{
		ctx->hwndSetWnd = hwndSetWnd;
		ctx->hwndPage = hwndPage;
		ctx->uidata = uidata;

		SetWndPtr(hwndPage, GWLP_WNDPROC, (void*)pagecrsv_proc);
		SetWndPtr(hwndPage, GWLP_USERDATA, ctx);

		if(pagecrsv_init(ctx, data))
			return 1;

		SetWndPtr(hwndPage, GWLP_USERDATA, NULL);
		free(ctx);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
