/* ---------------------------------------------------------------------------------------------- */

#include "pagecrwv.h"

/* ---------------------------------------------------------------------------------------------- */

static COLORREF pagecrwv_colordlg_customcolors[16];

/* ---------------------------------------------------------------------------------------------- */

void pagecrwv_data_init(pagecrwv_data_t *data, watrview_cfg_t *wvcfg)
{
	data->cr_bkgnd = wvcfg->cr_bkgnd;
	data->cr_label = wvcfg->cr_label;
	data->cr_ticks = wvcfg->cr_ticks;
	data->cr_scrbar = wvcfg->cr_scrbar;

	data->cr_pt_count = wvcfg->cr_pt_count;
	memcpy(data->cr_pt, wvcfg->cr_pt,
		wvcfg->cr_pt_count * sizeof(watrview_cr_map_pt_t));

	data->use_chan_crmap = wvcfg->use_chan_crmap;
}

/* ---------------------------------------------------------------------------------------------- */

int pagecrwv_data_apply(watrview_ctx_t *watrview, pagecrwv_data_t *data,
						uievent_t *event_visualcfg, HWND hwndMsgbox)
{
	int status = 1;

	/* check config changed */
	if( (data->cr_bkgnd != watrview->cfg.cr_bkgnd) ||
		(data->cr_label != watrview->cfg.cr_label) ||
		(data->cr_ticks != watrview->cfg.cr_ticks) ||
		(data->cr_scrbar != watrview->cfg.cr_scrbar) ||
		(data->use_chan_crmap != watrview->cfg.use_chan_crmap) ||
		(data->cr_pt_count != watrview->cfg.cr_pt_count) ||
		(memcmp(data->cr_pt, watrview->cfg.cr_pt,
			data->cr_pt_count * sizeof(watrview_cr_map_pt_t)) != 0) )
	{
		/* set new config */
		watrview->cfg.cr_bkgnd = data->cr_bkgnd;
		watrview->cfg.cr_label = data->cr_label;
		watrview->cfg.cr_ticks = data->cr_ticks;
		watrview->cfg.cr_scrbar = data->cr_scrbar;

		watrview->cfg.cr_pt_count = data->cr_pt_count;
		memcpy(watrview->cfg.cr_pt, data->cr_pt,
			data->cr_pt_count * sizeof(watrview_cr_map_pt_t));

		watrview->cfg.use_chan_crmap = data->use_chan_crmap;

		/* reinit waterfall viewer resources */
		if(!watrview_res_reinit(watrview))
		{
			MessageBox(hwndMsgbox,
				_T("Can't initialize waterfall viewer resources."), ui_title,
				MB_ICONEXCLAMATION|MB_OK);
			status = 0;
		}

		/* force waterfall viewer redraw */
		uievent_send(event_visualcfg, EVENT_VISUALCFG_WATRVIEW, NULL);
	}

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_choosecolor(pagecrwv_ctx_t *ctx, COLORREF *pcr)
{
	CHOOSECOLOR cc;

	memset(&cc, 0, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = ctx->hwndSetWnd;
	cc.lpCustColors = pagecrwv_colordlg_customcolors;
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

static int pagecrwv_pickcolor(pagecrwv_ctx_t *ctx, HWND hwndEdit)
{
	COLORREF cr;

	Edit_GetText(hwndEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	parse_rgb(ctx->uidata->databuf, &cr);

	if(pagecrwv_choosecolor(ctx, &cr))
		ui_set_rgb(ctx->uidata, hwndEdit, cr);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static pagecrwv_cmlist_row_data_t *pagecrwv_cmlist_getrowdata(pagecrwv_ctx_t *ctx, int row)
{
	LVITEM lvi;

	memset(&lvi, 0, sizeof(lvi));
	lvi.mask = LVIF_PARAM;
	lvi.iItem = row;
	if(!ListView_GetItem(ctx->hwndCmList, &lvi))
		return 0;

	return (void*)(lvi.lParam);
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_preview_init(uicommon_t *uidata,
								 HWND hwndPage, pagecrwv_preview_ctx_t *prev, 
								 int x, int y, int w, int h)
{
	HDC hdc;

	prev->win_x = x;
	prev->win_y = y;
	prev->win_w = w;
	prev->win_h = h;

	prev->scale_y = 9;
	prev->scale_h = h - 18;

	prev->lab_x = 2;
	prev->lab_w = 31;

	prev->tick1_x = 35;
	prev->tick1_w = 36;

	prev->grad_x = 37;
	prev->grad_w = 32;

	prev->cr_label = RGB(0, 0, 0);
	prev->cr_ticks = RGB(0, 0, 0);

	if( (hdc = GetDC(hwndPage)) != NULL )
	{
		prev->buf_norm = malloc(sizeof(DWORD) * prev->scale_h);
		prev->buf_chan = malloc(sizeof(DWORD) * prev->scale_h);

		prev->cr_pt = malloc(sizeof(watrview_cr_map_pt_t) * WATRVIEW_CR_PT_MAX);

		/*prev->hfont = ui_crt_font(_T("Verdana"), 12);*/

		prev->hfont = uidata->hfnt_viewers;
		
		prev->hbm_buf = CreateCompatibleBitmap(hdc, w, h);
		prev->hdc_buf = CreateCompatibleDC(hdc);

		if( (prev->buf_norm != NULL) && (prev->buf_chan != NULL) && (prev->cr_pt != NULL) &&
			(prev->hbm_buf != NULL) && (prev->hdc_buf != NULL) )
		{
			SelectObject(prev->hdc_buf, prev->hbm_buf);

			SelectObject(prev->hdc_buf, GetStockObject(DC_BRUSH));
			SetDCBrushColor(prev->hdc_buf, GetSysColor(COLOR_BTNFACE));

			SelectObject(prev->hdc_buf, GetStockObject(DC_PEN));

			SelectObject(prev->hdc_buf, prev->hfont);
			SetBkColor(prev->hdc_buf, GetSysColor(COLOR_BTNFACE));
			SetTextColor(prev->hdc_buf, prev->cr_label);

			prev->is_inited = 1;

			ReleaseDC(hwndPage, hdc);
			return 1;
		}

		DeleteDC(prev->hdc_buf);
		DeleteObject(prev->hbm_buf);
		/*DeleteObject(prev->hfont);*/
		free(prev->cr_pt);
		free(prev->buf_chan);
		free(prev->buf_norm);

		ReleaseDC(hwndPage, hdc);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_preview_cleanup(pagecrwv_preview_ctx_t *prev)
{
	if(prev->is_inited)
	{
		DeleteDC(prev->hdc_buf);
		DeleteObject(prev->hbm_buf);
		/*DeleteObject(prev->hfont);*/
		free(prev->cr_pt);
		free(prev->buf_chan);
		free(prev->buf_norm);
		prev->is_inited = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_preview_update(pagecrwv_ctx_t *ctx, int redraw)
{
	int i, row, pos, n, crmap_ok, j, x1, x2;
	double m, m_step;
	pagecrwv_cmlist_row_data_t *rowdata;
	HDC hdc;
	RECT rt, re;

	if(!ctx->preview.is_inited)
		return 0;

	n = ListView_GetItemCount(ctx->hwndCmList);

	ctx->preview.cr_pt_count = 0;
	for(i = 0; i < n; i++)
	{
		row = n - i - 1;

		if( (rowdata = pagecrwv_cmlist_getrowdata(ctx, row)) != NULL )
		{
			memcpy(ctx->preview.cr_pt + ctx->preview.cr_pt_count,
				&(rowdata->cr_pt), sizeof(watrview_cr_map_pt_t));
			ctx->preview.cr_pt_count++;
		}
	}

	/* map colors */
	crmap_ok = watrview_crmap_build(ctx->preview.cr_pt, ctx->preview.cr_pt_count,
		ctx->preview.buf_norm, ctx->preview.buf_chan, ctx->preview.scale_h,
		&(ctx->preview.m_0), &(ctx->preview.m_1));

	/* draw background */
	PatBlt(ctx->preview.hdc_buf, 0, 0, ctx->preview.win_w, ctx->preview.win_h, PATCOPY);

	/* draw outline */
	re.left = 0;
	re.top = 0;
	re.right = ctx->preview.win_w;
	re.bottom = ctx->preview.win_h;
	DrawEdge(ctx->preview.hdc_buf, &re, EDGE_ETCHED, BF_RECT);

	if(crmap_ok)
	{
		/* draw magnitude scale */
		if(scale_step_mag(ctx->preview.m_0, ctx->preview.m_1,
			ctx->preview.scale_h, PAGECRWV_PREVIEW_MIN_MAG_LABEL_H, &m_step))
		{
			SetDCPenColor(ctx->preview.hdc_buf, ctx->preview.cr_ticks);

			m = scale_first_mark(ctx->preview.m_0, m_step, 1e-6);
			for( ; ; )
			{
				j = scale_mark_pos(ctx->preview.m_0, ctx->preview.m_1, m, ctx->preview.scale_h, 0);
				if(j >= ctx->preview.scale_h)
					break;

				pos = ctx->preview.scale_y + ctx->preview.scale_h - j - 1;

				/* draw label */
				_stprintf(ctx->uidata->databuf, _T("%.*f"), (fabs(m) < 100) ? 1 : 0, m);

				rt.left = ctx->preview.lab_x;
				rt.right = rt.left + ctx->preview.lab_w;
				rt.top = pos - 7;
				rt.bottom = pos + 8;

				DrawText(ctx->preview.hdc_buf, 
					ctx->uidata->databuf, (int)_tcslen(ctx->uidata->databuf), 
					&rt, DT_SINGLELINE|DT_RIGHT|DT_VCENTER);

				/* draw tick */
				x1 = ctx->preview.tick1_x;
				x2 = x1 + ctx->preview.tick1_w;
				MoveToEx(ctx->preview.hdc_buf, x1, pos, NULL);
				LineTo(ctx->preview.hdc_buf, x2, pos);

				m += m_step;
			}
		}

		/* draw gradients */
		for(i = 0; i < ctx->preview.scale_h; i++)
		{
			pos = ctx->preview.scale_y + ctx->preview.scale_h - i - 1;

			/* draw background gradient */
			x1 = ctx->preview.grad_x;
			x2 = x1 + ctx->preview.grad_w / 2;
			SetDCPenColor(ctx->preview.hdc_buf, ctx->preview.buf_norm[i]);
			MoveToEx(ctx->preview.hdc_buf, x1, pos, NULL);
			LineTo(ctx->preview.hdc_buf, x2, pos);

			/* draw channel bandwidth gradient */
			x1 = x2;
			x2 = x1 + ctx->preview.grad_w / 2;
			SetDCPenColor(ctx->preview.hdc_buf, ctx->preview.buf_chan[i]);
			MoveToEx(ctx->preview.hdc_buf, x1, pos, NULL);
			LineTo(ctx->preview.hdc_buf, x2, pos);
		}
	}

	/* copy to window */
	if( redraw && ((hdc = GetDC(ctx->hwndPage)) != NULL) )
	{
		BitBlt(hdc, ctx->preview.win_x, ctx->preview.win_y,
			ctx->preview.win_w, ctx->preview.win_h,
			ctx->preview.hdc_buf, 0, 0, SRCCOPY);
		ReleaseDC(ctx->hwndPage, hdc);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_preview_redraw(pagecrwv_ctx_t *ctx, HDC hdc)
{
	if(!ctx->preview.is_inited)
		return 0;

	BitBlt(hdc, ctx->preview.win_x, ctx->preview.win_y,
		ctx->preview.win_w, ctx->preview.win_h,
		ctx->preview.hdc_buf, 0, 0, SRCCOPY);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_handle_paint(pagecrwv_ctx_t *ctx)
{
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint(ctx->hwndPage, &ps);
	pagecrwv_preview_redraw(ctx, hdc);
	EndPaint(ctx->hwndPage, &ps);
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_edit_close(pagecrwv_ctx_t *ctx, int save, int forceclose)
{
	int status = 1;
	double magn = 0;
	COLORREF cr = 0;
	pagecrwv_cmlist_row_data_t *rowdata;

	if(ctx->hwndCmListEdit == NULL)
		return 0;

	if(save)
	{
		rowdata = pagecrwv_cmlist_getrowdata(ctx, ctx->iCmListEditRow);

		if(rowdata != NULL)
		{
			Edit_GetText(ctx->hwndCmListEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));

			switch(ctx->iCmListEditCol)
			{
			case PAGECRWV_CMLIST_COL_MAGN:
				status = parse_dbl(ctx->uidata->databuf, 0, &magn, NULL);
				if( (!status) && (!forceclose) ) {
					MessageBox(ctx->hwndSetWnd, _T("Bad magnitude number format."),
						ui_title, MB_ICONEXCLAMATION|MB_OK);
				}
				break;

			case PAGECRWV_CMLIST_COL_CRNORM:
			case PAGECRWV_CMLIST_COL_CRCHAN:
				status = parse_rgb(ctx->uidata->databuf, &cr);
				if( (!status) && (!forceclose) ) {
					MessageBox(ctx->hwndSetWnd, _T("Bad color value entered."),
						ui_title, MB_ICONEXCLAMATION|MB_OK);
				}
				break;
			}

			if(status)
			{
				switch(ctx->iCmListEditCol)
				{
				case PAGECRWV_CMLIST_COL_MAGN:
					ListView_SetItemText(
						ctx->hwndCmList, ctx->iCmListEditRow, PAGECRWV_CMLIST_COL_MAGN,
						fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, magn, 0, 2, 1));
					rowdata->cr_pt.m = magn;
					break;

				case PAGECRWV_CMLIST_COL_CRNORM:
					ListView_SetItemText(
						ctx->hwndCmList, ctx->iCmListEditRow, PAGECRWV_CMLIST_COL_CRNORM,
						fmt_rgb(ctx->uidata->databuf, cr));
					rowdata->cr_pt.cr_bkgnd = cr;
					break;

				case PAGECRWV_CMLIST_COL_CRCHAN:
					ListView_SetItemText(
						ctx->hwndCmList, ctx->iCmListEditRow, PAGECRWV_CMLIST_COL_CRCHAN,
						fmt_rgb(ctx->uidata->databuf, cr));
					rowdata->cr_pt.cr_chan = cr;
					break;
				}
			}
			else
			{
				if(!forceclose)
				{
					Edit_SetSel(ctx->hwndCmListEdit, 0, -1);
					SetFocus(ctx->hwndCmListEdit);
					return 0;
				}
			}
		}
	}

	DestroyWindow(ctx->hwndCmListEdit);

	InvalidateRect(ctx->hwndCmList, &(ctx->rectCmListEdit), FALSE);
	UpdateWindow(ctx->hwndCmList);

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK pagecrwv_cmlist_edit_wndproc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	pagecrwv_ctx_t *ctx;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	switch(umsg)
	{
	case WM_KEYDOWN:
		switch(LOBYTE(wp))
		{
		case VK_RETURN:
			pagecrwv_cmlist_edit_close(ctx, 1, 0);
			return 0;
		case VK_ESCAPE:
			pagecrwv_cmlist_edit_close(ctx, 0, 0);
			return 0;
		}
		break;

	case WM_DESTROY:
		ctx->hwndCmListEdit = NULL;
		break;
	}

	return CallWindowProc(ctx->wpCmListEdit, hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_edit_getrect(pagecrwv_ctx_t *ctx, int row, int col, RECT *prc)
{
	if(col == 0) {
		if(!ListView_GetItemRect(ctx->hwndCmList, row, prc, LVIR_LABEL))
			return 0;
	} else {
		if(!ListView_GetSubItemRect(ctx->hwndCmList, row, col, LVIR_BOUNDS, prc))
			return 0;
	}

	if(col != PAGECRWV_CMLIST_COL_MAGN)
		prc->right -= 16;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_edit_open(pagecrwv_ctx_t *ctx, int row, int col)
{
	int n;

	n = ListView_GetItemCount(ctx->hwndCmList);

	if( (row < 0) || (row >= n) ||
		(col < 0) || (col >= PAGECRWV_CMLIST_COL_COUNT) ||
		(ctx->hwndCmListEdit != NULL) )
	{
		return 0;
	}

	ctx->iCmListEditRow = row;
	ctx->iCmListEditCol = col;

	if(!pagecrwv_cmlist_edit_getrect(ctx, row, col, &(ctx->rectCmListEdit)))
		return 0;

	if( (ctx->rectCmListEdit.right - ctx->rectCmListEdit.left < 12) ||
		(ctx->rectCmListEdit.bottom - ctx->rectCmListEdit.top < 6) )
	{
		return 0;
	}

	ListView_GetItemText(ctx->hwndCmList, row, col,
		ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));

	ctx->hwndCmListEdit = CreateWindow(_T("EDIT"), ctx->uidata->databuf,
		WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|ES_AUTOHSCROLL,
		ctx->rectCmListEdit.left, ctx->rectCmListEdit.top,
		ctx->rectCmListEdit.right - ctx->rectCmListEdit.left,
		ctx->rectCmListEdit.bottom - ctx->rectCmListEdit.top,
		ctx->hwndCmList, (HMENU)PAGECRWV_ID_CMLIST_EDIT, ctx->uidata->h_inst, NULL);

	if(ctx->hwndCmListEdit == NULL)
		return 0;

	SetWndPtr(ctx->hwndCmListEdit, GWLP_USERDATA, ctx);

	ctx->wpCmListEdit = (WNDPROC)SetWndPtr(ctx->hwndCmListEdit,
		GWLP_WNDPROC, (void*)pagecrwv_cmlist_edit_wndproc);

	SetWindowFont(ctx->hwndCmListEdit, ctx->uidata->hfnt_mono, FALSE);
	Edit_SetSel(ctx->hwndCmListEdit, 0, -1);
	SetFocus(ctx->hwndCmListEdit);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_cmlist_edit_adjustpos(pagecrwv_ctx_t *ctx)
{
	RECT rc;

	if( (ctx->hwndCmListEdit != NULL) &&
		pagecrwv_cmlist_edit_getrect(ctx, ctx->iCmListEditRow, ctx->iCmListEditCol, &rc) )
	{
		ui_updatewndrect(ctx->hwndCmListEdit, &(ctx->rectCmListEdit), &rc, UI_SWR_REDRAWNOERASE);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_rowbtn_getrect(pagecrwv_ctx_t *ctx, int row, int col, RECT *prc)
{
	if(col == 0) {
		if(!ListView_GetItemRect(ctx->hwndCmList, row, prc, LVIR_LABEL))
			return 0;
	} else {
		if(!ListView_GetSubItemRect(ctx->hwndCmList, row, col, LVIR_BOUNDS, prc))
			return 0;
	}

	prc->left = prc->right - 16;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_cmlist_rowbtn_adjustall(pagecrwv_ctx_t *ctx)
{
	int row, n;
	RECT rc_btn_norm, rc_btn_chan;
	pagecrwv_cmlist_row_data_t *rowdata;

	n = ListView_GetItemCount(ctx->hwndCmList);
	for(row = 0; row < n; row++)
	{
		rowdata = pagecrwv_cmlist_getrowdata(ctx, row);

		if(rowdata != NULL)
		{
			pagecrwv_cmlist_rowbtn_getrect(ctx, row, PAGECRWV_CMLIST_COL_CRNORM, &rc_btn_norm);
			ui_updatewndrect(rowdata->hwndBtnPickNorm, &(rowdata->rcBtnPickNorm),
				&rc_btn_norm, UI_SWR_REDRAWNOERASE);

			pagecrwv_cmlist_rowbtn_getrect(ctx, row, PAGECRWV_CMLIST_COL_CRCHAN, &rc_btn_chan);
			ui_updatewndrect(rowdata->hwndBtnPickChan, &(rowdata->rcBtnPickChan),
				&rc_btn_chan, UI_SWR_REDRAWNOERASE);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_rowbtn_handleclick(pagecrwv_ctx_t *ctx, UINT btn)
{
	int col, n, i, row = -1;
	pagecrwv_cmlist_row_data_t *rd, *rowdata = NULL;
	COLORREF cr;

	n = ListView_GetItemCount(ctx->hwndCmList);
	for(i = 0; i < n; i++)
	{
		rd = pagecrwv_cmlist_getrowdata(ctx, i);

		if( (rd != NULL) && (btn >= rd->uctrlgroup) &&
			(btn < rd->uctrlgroup + PAGECRWV_CMLIST_COL_COUNT) )
		{
			rowdata = rd;
			row = i;
			break;
		}
	}

	if(rowdata == NULL)
		return 0;

	col = btn - rowdata->uctrlgroup;

	if( (col == PAGECRWV_CMLIST_COL_CRNORM) || (col == PAGECRWV_CMLIST_COL_CRCHAN) )
	{
		if( (ctx->hwndCmListEdit != NULL) &&
			(row == ctx->iCmListEditRow) && (col == ctx->iCmListEditCol) )
		{
			Edit_GetText(ctx->hwndCmListEdit, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
			parse_rgb(ctx->uidata->databuf, &cr);
		}
		else
		{
			switch(col)
			{
			case PAGECRWV_CMLIST_COL_CRNORM:
				cr = rowdata->cr_pt.cr_bkgnd;
				break;
			case PAGECRWV_CMLIST_COL_CRCHAN:
				cr = rowdata->cr_pt.cr_chan;
				break;
			}
		}

		if(pagecrwv_choosecolor(ctx, &cr))
		{
			if( (ctx->hwndCmListEdit != NULL) &&
				(ctx->iCmListEditRow == row) && (ctx->iCmListEditCol == col) )
			{
				Edit_SetText(ctx->hwndCmListEdit, fmt_rgb(ctx->uidata->databuf, cr));
			}
			else
			{
				ListView_SetItemText(ctx->hwndCmList, row, col,
					fmt_rgb(ctx->uidata->databuf, cr));

				switch(col)
				{
				case PAGECRWV_CMLIST_COL_CRNORM:
					rowdata->cr_pt.cr_bkgnd = cr;
					break;
				case PAGECRWV_CMLIST_COL_CRCHAN:
					rowdata->cr_pt.cr_chan = cr;
					break;
				}
			}
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static UINT pagecrwv_cmlist_rowbtn_getbaseid(pagecrwv_ctx_t *ctx)
{
	UINT uctrlgroup;
	pagecrwv_cmlist_row_data_t *rowdata;
	int n, i, success;

	uctrlgroup = PAGECRWV_ID_CMLIST_ROWBTN_0;

	n = ListView_GetItemCount(ctx->hwndCmList);

	do {

		success = 1;

		for(i = 0; i < n; i++)
		{
			rowdata = pagecrwv_cmlist_getrowdata(ctx, i);

			if( (rowdata != NULL) &&
				(rowdata->uctrlgroup == uctrlgroup) )
			{
				uctrlgroup += PAGECRWV_CMLIST_COL_COUNT;
				success = 0;
				break;
			}

		}

	} while(!success);

	return uctrlgroup;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_insrow(pagecrwv_ctx_t *ctx, int row, watrview_cr_map_pt_t *cr_pt)
{
	LVITEM lvi;
	pagecrwv_cmlist_row_data_t *rowdata;

	if( (rowdata = calloc(1, sizeof(pagecrwv_cmlist_row_data_t))) != NULL )
	{
		memset(&lvi, 0, sizeof(lvi));
		rowdata->uctrlgroup = pagecrwv_cmlist_rowbtn_getbaseid(ctx);
		memcpy(&(rowdata->cr_pt), cr_pt, sizeof(watrview_cr_map_pt_t));

		lvi.mask = LVIF_TEXT|LVIF_PARAM;
		lvi.iItem = row;
		lvi.lParam = (LPARAM)rowdata;
		lvi.pszText = fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, cr_pt->m, 0, 2, 1);
		ListView_InsertItem(ctx->hwndCmList, &lvi);

		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = PAGECRWV_CMLIST_COL_CRNORM;
		lvi.pszText = fmt_rgb(ctx->uidata->databuf, cr_pt->cr_bkgnd);
		ListView_SetItem(ctx->hwndCmList, &lvi);

		lvi.mask = LVIF_TEXT;
		lvi.iSubItem = PAGECRWV_CMLIST_COL_CRCHAN;
		lvi.pszText = fmt_rgb(ctx->uidata->databuf, cr_pt->cr_chan);
		ListView_SetItem(ctx->hwndCmList, &lvi);

		pagecrwv_cmlist_rowbtn_getrect(ctx, row,
			PAGECRWV_CMLIST_COL_CRNORM, &(rowdata->rcBtnPickNorm));
		rowdata->hwndBtnPickNorm = ui_crt_btn(
			ctx->uidata, ctx->hwndCmList, WS_CLIPSIBLINGS,
			rowdata->rcBtnPickNorm.left, rowdata->rcBtnPickNorm.top,
			rowdata->rcBtnPickNorm.right - rowdata->rcBtnPickNorm.left,
			rowdata->rcBtnPickNorm.bottom - rowdata->rcBtnPickNorm.top,
			_T("..."), rowdata->uctrlgroup + PAGECRWV_CMLIST_COL_CRNORM);
		SetWindowFont(rowdata->hwndBtnPickNorm, ctx->uidata->hfnt_mini, FALSE);

		pagecrwv_cmlist_rowbtn_getrect(ctx, row,
			PAGECRWV_CMLIST_COL_CRCHAN, &(rowdata->rcBtnPickChan));
		rowdata->hwndBtnPickChan = ui_crt_btn(
			ctx->uidata, ctx->hwndCmList, WS_CLIPSIBLINGS,
			rowdata->rcBtnPickChan.left, rowdata->rcBtnPickChan.top,
			rowdata->rcBtnPickChan.right - rowdata->rcBtnPickChan.left,
			rowdata->rcBtnPickChan.bottom - rowdata->rcBtnPickChan.top,
			_T("..."), rowdata->uctrlgroup + PAGECRWV_CMLIST_COL_CRCHAN);
		SetWindowFont(rowdata->hwndBtnPickChan, ctx->uidata->hfnt_mini, FALSE);

		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_cmlist_delrow(pagecrwv_ctx_t *ctx, int row)
{
	pagecrwv_cmlist_row_data_t *rowdata;

	rowdata = pagecrwv_cmlist_getrowdata(ctx, row);

	if(rowdata != NULL)
	{
		DestroyWindow(rowdata->hwndBtnPickNorm);
		DestroyWindow(rowdata->hwndBtnPickChan);

		free(rowdata);
	}

	ListView_DeleteItem(ctx->hwndCmList, row);
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_cmlist_clear(pagecrwv_ctx_t *ctx)
{
	int row, n;

	n = ListView_GetItemCount(ctx->hwndCmList);
	for(row = n - 1; row >= 0; row--)
		pagecrwv_cmlist_delrow(ctx, row);
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_pt_addnew(pagecrwv_ctx_t *ctx)
{
	int n, i, rowbefore;
	watrview_cr_map_pt_t cr_pt;
	pagecrwv_cmlist_row_data_t *rd, *rd_next;

	n = ListView_GetItemCount(ctx->hwndCmList);
	
	if(n >= WATRVIEW_CR_PT_MAX)
	{
		MessageBox(ctx->hwndSetWnd, _T("Already maximum number of points."),
			ui_title, MB_ICONINFORMATION|MB_OK);
		return 0;
	}

	/* prev. row (last selected row or last row in list) */
	rowbefore = n - 1;

	for(i = n - 1; i >= 0; i--)
	{
		if(ListView_GetItemState(ctx->hwndCmList, i, LVIS_SELECTED))
		{
			rowbefore = i;
			break;
		}
	}

	/* initialize new row data */
	memset(&cr_pt, 0, sizeof(cr_pt));

	if( (rowbefore >= 0) && (rowbefore < n) &&
		(rd = pagecrwv_cmlist_getrowdata(ctx, rowbefore)) != NULL)
	{
		memcpy(&cr_pt, &(rd->cr_pt), sizeof(cr_pt));
		cr_pt.m -= 10.0;

		if( (rowbefore < n - 1) &&
			((rd_next = pagecrwv_cmlist_getrowdata(ctx, rowbefore + 1)) != NULL) )
		{
			cr_pt.m = 0.5 * (rd->cr_pt.m + rd_next->cr_pt.m);
		}
	}

	/* create new row */
	pagecrwv_cmlist_insrow(ctx, rowbefore + 1, &cr_pt);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_cmlist_pt_seldelete(pagecrwv_ctx_t *ctx)
{
	int n, i, selitem = -1;

	n = ListView_GetItemCount(ctx->hwndCmList);

	for(i = n - 1; i >= 0; i--)
	{
		if(ListView_GetItemState(ctx->hwndCmList, i, LVIS_SELECTED))
		{
			pagecrwv_cmlist_delrow(ctx, i);

			if(selitem == -1)
				selitem = i + 1;

			selitem--;
			n--;
		}
	}

	if( (selitem != -1) && (n > 0) )
	{
		/* select next item */
		if( (selitem < 0) || (selitem >= n) )
			selitem = n - 1;
		ListView_SetItemState(ctx->hwndCmList, selitem,
			LVIS_SELECTED, LVIS_SELECTED);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_pt_selmoveup(pagecrwv_ctx_t *ctx)
{
	int n, i;
	pagecrwv_cmlist_row_data_t *rd;
	watrview_cr_map_pt_t cr_pt;

	n = ListView_GetItemCount(ctx->hwndCmList);

	for(i = 0; i < n; i++)
	{
		if(ListView_GetItemState(ctx->hwndCmList, i, LVIS_SELECTED))
		{
			if(i == 0)
				return 0;

			if( (rd = pagecrwv_cmlist_getrowdata(ctx, i)) != NULL )
			{
				memcpy(&cr_pt, &(rd->cr_pt), sizeof(cr_pt));

				pagecrwv_cmlist_delrow(ctx, i);
				pagecrwv_cmlist_insrow(ctx, i - 1, &cr_pt);

				ListView_SetItemState(ctx->hwndCmList, i - 1,
					LVIS_SELECTED, LVIS_SELECTED);
			}
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_pt_selmovedown(pagecrwv_ctx_t *ctx)
{
	int n, i;
	pagecrwv_cmlist_row_data_t *rd;
	watrview_cr_map_pt_t cr_pt;

	n = ListView_GetItemCount(ctx->hwndCmList);

	for(i = n - 1; i >= 0; i--)
	{
		if(ListView_GetItemState(ctx->hwndCmList, i, LVIS_SELECTED))
		{
			if(i == n - 1)
				return 0;

			if( (rd = pagecrwv_cmlist_getrowdata(ctx, i)) != NULL )
			{
				memcpy(&cr_pt, &(rd->cr_pt), sizeof(cr_pt));

				pagecrwv_cmlist_delrow(ctx, i);
				pagecrwv_cmlist_insrow(ctx, i + 1, &cr_pt);

				ListView_SetItemState(ctx->hwndCmList, i + 1,
					LVIS_SELECTED, LVIS_SELECTED);
			}
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmlist_pt_seledit(pagecrwv_ctx_t *ctx, int col)
{
	int n, i;

	n = ListView_GetItemCount(ctx->hwndCmList);
	
	for(i = n - 1; i >= 0; i--)
	{
		if(ListView_GetItemState(ctx->hwndCmList, i, LVIS_SELECTED))
		{
			pagecrwv_cmlist_edit_open(ctx, i, col);
			return 1;
		}
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static int CALLBACK pagecrwv_cmlist_pt_compbymagn(LPARAM lp1, LPARAM lp2, LPARAM parm)
{
	pagecrwv_cmlist_row_data_t *rd1 = (void*)lp1;
	pagecrwv_cmlist_row_data_t *rd2 = (void*)lp2;

	if(rd1->cr_pt.m > rd2->cr_pt.m)
		return -1;
	if(rd1->cr_pt.m < rd2->cr_pt.m)
		return 1;
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_cmlist_handledblclk(pagecrwv_ctx_t *ctx, NMITEMACTIVATE *nm)
{
	pagecrwv_cmlist_edit_open(ctx, nm->iItem, nm->iSubItem);
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_cmlist_handleclick(pagecrwv_ctx_t *ctx, NMITEMACTIVATE *nm)
{
	pagecrwv_cmlist_edit_close(ctx, 1, 0);
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmd_initmagrange(pagecrwv_ctx_t *ctx)
{
	double m_0, m_1, m_step, m;
	int n, i, row;
	pagecrwv_cmlist_row_data_t *rowdata;

	if( !ui_get_double(ctx->uidata, ctx->hwndSetWnd, ctx->hwndEditInitMag0, &m_0,
		_T("magnitude init range lower limit"), 0, NULL) )
	{
		return 0;
	}
	
	if( !ui_get_double(ctx->uidata, ctx->hwndSetWnd, ctx->hwndEditInitMag1, &m_1,
		_T("magnitude init range upper limit"), 0, NULL) )
	{
		return 0;
	}

	if(m_1 <= m_0)
	{
		MessageBox(ctx->hwndSetWnd, _T("Entered magnitude range is empty."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	n = ListView_GetItemCount(ctx->hwndCmList);

	if(n < 2)
	{
		MessageBox(ctx->hwndSetWnd, _T("At least 2 colormap points required."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	m = m_0;
	m_step = (m_1 - m_0) / (n - 1);

	for(i = 0; i < n; i++)
	{
		row = n - i - 1;

		if( (rowdata = pagecrwv_cmlist_getrowdata(ctx, row)) != NULL )
		{
			ListView_SetItemText(ctx->hwndCmList, row, PAGECRWV_CMLIST_COL_MAGN,
				fmt_dbl(ctx->uidata->databuf, ctx->uidata->databuf_size, m, 0, 2, 1));
			rowdata->cr_pt.m = m;
		}

		m += m_step;
	}

	pagecrwv_preview_update(ctx, 1);
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_cmd_apply_colormap_preset(pagecrwv_ctx_t *ctx, int column)
{
	int count, i, type;
	COLORREF *buf;
	pagecrwv_cmlist_row_data_t *rowdata;

	type = ComboBox_GetCurSel(ctx->hwndCbPresetSel);
	if((type < 0) || (type >= COLORMAP_COUNT))
	{
		MessageBox(ctx->hwndSetWnd, _T("Colormap preset is not selected."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	count = ListView_GetItemCount(ctx->hwndCmList);
	if(count < 2)
	{
		MessageBox(ctx->hwndSetWnd, _T("At least 2 colormap points required."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	if( (buf = malloc(count * sizeof(COLORREF))) == NULL )
		return 0;

	crmap_ref(type, buf, count);

	for(i = 0; i < count; i++)
	{
		rowdata = pagecrwv_cmlist_getrowdata(ctx, count - i - 1);

		if(rowdata != NULL)
		{
			switch(column)
			{
			case PAGECRWV_CMLIST_COL_CRNORM:
				rowdata->cr_pt.cr_bkgnd = buf[i];
				break;
			case PAGECRWV_CMLIST_COL_CRCHAN:
				rowdata->cr_pt.cr_chan = buf[i];
				break;
			}

			fmt_rgb(ctx->uidata->databuf, buf[i]);
			ListView_SetItemText(ctx->hwndCmList, count - i - 1, column, ctx->uidata->databuf);
		}
	}

	free(buf);

	pagecrwv_preview_update(ctx, 1);
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_load(pagecrwv_ctx_t *ctx, pagecrwv_data_t *data)
{
	int row, pt_idx;
	ini_sect_t *sect;
	double m_0, m_1;

	for(row = 0; row < data->cr_pt_count; row++)
	{
		pt_idx = data->cr_pt_count - row - 1;
		pagecrwv_cmlist_insrow(ctx, row, data->cr_pt + pt_idx);
	}

	m_0 = -20.0;
	m_1 = 40.0;

	sect = ini_sect_get(ctx->ini, _T("setwnd"), 0);
	ini_getf(sect, _T("wv_mag_0"), &m_0);
	ini_getf(sect, _T("wv_mag_1"), &m_1);

	ui_set_double(ctx->uidata, ctx->hwndEditInitMag0, m_0, 0, 6, 1);
	ui_set_double(ctx->uidata, ctx->hwndEditInitMag1, m_1, 0, 6, 1);

	ui_set_rgb(ctx->uidata, ctx->hwndEditCrBkgnd, data->cr_bkgnd);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrLabel, data->cr_label);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrTicks, data->cr_ticks);
	ui_set_rgb(ctx->uidata, ctx->hwndEditCrScrbar, data->cr_scrbar);

	Button_SetCheck(ctx->hwndBtnUseChanCrMap,
		data->use_chan_crmap ? BST_CHECKED : BST_UNCHECKED);

	pagecrwv_preview_update(ctx, 1);
}

/* ---------------------------------------------------------------------------------------------- */

int pagecrwv_save(pagecrwv_data_t *data, HWND hwndPage)
{
	int n, i, row;
	pagecrwv_ctx_t *ctx;
	pagecrwv_cmlist_row_data_t *rowdata;
	double m_min, m_max;
	ini_sect_t *sect;

	ctx = GetWndPtr(hwndPage, GWLP_USERDATA);

	pagecrwv_cmlist_edit_close(ctx, 1, 1);

	n = ListView_GetItemCount(ctx->hwndCmList);

	for(i = 0; i < n; i++)
	{
		row = n - i - 1;
		if( (rowdata = pagecrwv_cmlist_getrowdata(ctx, row)) != NULL )
			memcpy(&(data->cr_pt[i]), &(rowdata->cr_pt), sizeof(watrview_cr_map_pt_t));
	}

	data->cr_pt_count = n;

	/* check points */
	if(data->cr_pt_count < 2) {
		MessageBox(ctx->hwndSetWnd, _T("At least 2 color map points required."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	m_min = m_max = data->cr_pt[0].m;
	for(i = 0; i < data->cr_pt_count; i++) {
		if(data->cr_pt[i].m < m_min)
			m_min = data->cr_pt[i].m;
		if(data->cr_pt[i].m > m_max)
			m_max = data->cr_pt[i].m;
	}

	if(m_max - m_min < 1e-6) {
		MessageBox(ctx->hwndSetWnd, _T("Color map magnitude range must be nonzero."),
			ui_title, MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	/* save colors */
	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrBkgnd, &(data->cr_bkgnd),
		_T("Background color")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrLabel, &(data->cr_label),
		_T("Label color")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrTicks, &(data->cr_ticks),
		_T("Ticks color")) )
	{
		return 0;
	}

	if( !ui_get_rgb(ctx->uidata, ctx->hwndSetWnd,
		ctx->hwndEditCrScrbar, &(data->cr_scrbar),
		_T("Scrollbar color")) )
	{
		return 0;
	}

	data->use_chan_crmap = (Button_GetCheck(ctx->hwndBtnUseChanCrMap) & BST_CHECKED) ? 1 : 0;

	/* save magnitude range */
	sect = ini_sect_get(ctx->ini, _T("setwnd"), 0);
	Edit_GetText(ctx->hwndEditInitMag0, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	ini_setf(sect, _T("wv_mag_0"), 6, _tstof(ctx->uidata->databuf));
	Edit_GetText(ctx->hwndEditInitMag1, ctx->uidata->databuf, (int)(ctx->uidata->databuf_size));
	ini_setf(sect, _T("wv_mag_1"), 6, _tstof(ctx->uidata->databuf));

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK pagecrwv_cmlist_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	pagecrwv_ctx_t *ctx;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	switch(umsg)
	{
	case WM_PAINT:
		pagecrwv_cmlist_edit_adjustpos(ctx);
		pagecrwv_cmlist_rowbtn_adjustall(ctx);
		break;

	case WM_KEYDOWN:
		switch(LOBYTE(wp))
		{
		case VK_F2:
			pagecrwv_cmlist_pt_seledit(ctx, PAGECRWV_CMLIST_COL_MAGN);
			return 0;
		case VK_F3:
			pagecrwv_cmlist_pt_seledit(ctx, PAGECRWV_CMLIST_COL_CRNORM);
			return 0;
		case VK_F4:
			pagecrwv_cmlist_pt_seledit(ctx, PAGECRWV_CMLIST_COL_CRCHAN);
			return 0;
		case VK_INSERT:
			pagecrwv_cmlist_pt_addnew(ctx);
			return 0;
		case VK_DELETE:
			pagecrwv_cmlist_pt_seldelete(ctx);
			return 0;
		case VK_UP:
			if(GetKeyState(VK_CONTROL) & 0x8000) {
				pagecrwv_cmlist_pt_selmoveup(ctx);
				return 0;
			}
			break;
		case VK_DOWN:
			if(GetKeyState(VK_CONTROL) & 0x8000) {
				pagecrwv_cmlist_pt_selmovedown(ctx);
				return 0;
			}
			break;
		case VK_F12:
			ListView_SortItems(ctx->hwndCmList,
				pagecrwv_cmlist_pt_compbymagn, (LPARAM)ctx);
			return 0;
		}
		break;

	case WM_COMMAND:
		if( (LOWORD(wp) >= PAGECRWV_ID_CMLIST_ROWBTN_0) &&
			(LOWORD(wp) <= PAGECRWV_ID_CMLIST_ROWBTN_MAX) )
		{
			pagecrwv_cmlist_rowbtn_handleclick(ctx, LOWORD(wp));
			return 0;
		}
		break;
	}

	return CallWindowProc(ctx->wpCmList, hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

static int pagecrwv_init(pagecrwv_ctx_t *ctx, pagecrwv_data_t *data)
{
	int i;
	LVCOLUMN lvc;

	/* create listview */
	ctx->hwndCmList = ui_crt_listview(
		ctx->uidata, ctx->hwndPage,
		LVS_REPORT|LVS_SHOWSELALWAYS|WS_TABSTOP, LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT,
		0, 0, 245, 150, PAGECRWV_ID_CMLIST);

	SetWndPtr(ctx->hwndCmList, GWLP_USERDATA, ctx);
	
	ctx->wpCmList = (WNDPROC)SetWndPtr(ctx->hwndCmList,
		GWLP_WNDPROC, (void*)pagecrwv_cmlist_proc);

	SetWindowFont(ctx->hwndCmList, ctx->uidata->hfnt_mono, FALSE);

	/* create columns */
	memset(&lvc, 0, sizeof(lvc));
	lvc.mask = LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;

	lvc.cx = 60;
	lvc.iSubItem = PAGECRWV_CMLIST_COL_MAGN;
	lvc.pszText = _T("Magn");
	ListView_InsertColumn(ctx->hwndCmList, lvc.iSubItem, &lvc);

	lvc.cx = 80;
	lvc.iSubItem = PAGECRWV_CMLIST_COL_CRNORM;
	lvc.pszText = _T("Normal");
	ListView_InsertColumn(ctx->hwndCmList, lvc.iSubItem, &lvc);

	lvc.cx = 80;
	lvc.iSubItem = PAGECRWV_CMLIST_COL_CRCHAN;
	lvc.pszText = _T("Channel");
	ListView_InsertColumn(ctx->hwndCmList, lvc.iSubItem, &lvc);

	/* point commands button row */
	ctx->hwndBtnCmPtAdd = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		0, 155, 45, 21, _T("Add"), PAGECRWV_ID_PT_ADD);
	ctx->hwndBtnCmPtDel = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		50, 155, 45, 21, _T("Del"), PAGECRWV_ID_PT_DEL);
	ctx->hwndBtnCmPtUp = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		100, 155, 45, 21, _T("Up"), PAGECRWV_ID_PT_UP);
	ctx->hwndBtnCmPtDown = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 155, 45, 21, _T("Down"), PAGECRWV_ID_PT_DOWN);
	ctx->hwndBtnCmPtSort = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		200, 155, 45, 21, _T("Sort"), PAGECRWV_ID_PT_SORT);

	/* manitude init controls */
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		0, 183, 65, 15, _T("Magnitude"), ID_CTL_STATIC);
	ctx->hwndEditInitMag0 = ui_crt_edit(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		65, 180, 45, 21, PAGECRWV_ID_INITMAG_M0);
	ui_crt_static(ctx->uidata, ctx->hwndPage, SS_CENTER,
		110, 183, 15, 15, _T("–"), ID_CTL_STATIC);
	ctx->hwndEditInitMag1 = ui_crt_edit(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		125, 180, 45, 21, PAGECRWV_ID_INITMAG_M1);
	ui_crt_static(ctx->uidata, ctx->hwndPage, SS_CENTER,
		170, 183, 30, 15, _T("dB"), ID_CTL_STATIC);
	ctx->hwndBtnInitMagExec = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		200, 180, 45, 21, _T("Init"), PAGECRWV_ID_INITMAG_EXEC);

	/* colormap presets controls */
	ctx->hwndCbPresetSel = ui_crt_combo(ctx->uidata, ctx->hwndPage, CBS_DROPDOWNLIST|WS_TABSTOP,
		0, 205, 145, 200, PAGECRWV_ID_PRESET_SEL);
	ctx->hwndBtnPresetApplyNorm = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		150, 205, 45, 21, _T("Norm"), PAGECRWV_ID_PRESET_APPLY_NORM);
	ctx->hwndBtnPresetApplyBkgnd = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		200, 205, 45, 21, _T("Chan"), PAGECRWV_ID_PRESET_APPLY_CHAN);

	for(i = 0; i < COLORMAP_COUNT; i++)
		ComboBox_AddString(ctx->hwndCbPresetSel, crmap_def[i].name);
	ComboBox_SetCurSel(ctx->hwndCbPresetSel, 0);

	/* setup preview */
	if( !pagecrwv_preview_init(ctx->uidata, ctx->hwndPage, &(ctx->preview),
		255, 0, 75, 201) )
	{
		return 0;
	}

	ctx->hwndBtnPreviewUpd = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		255, 205, 75, 21, _T("Update"), PAGECRWV_ID_PREVIEW_UPD);

	/* other color selection */
	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		345, 0, 120, 15, _T("Background color"), ID_CTL_STATIC);
	ctx->hwndEditCrBkgnd = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		355, 20, 60, 15, PAGECRWV_ID_CR_BKGND_EDIT);
	ctx->hwndBtnCrBkgndPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		420, 20, 15, 15, _T("..."), PAGECRWV_ID_CR_BKGND_PICK);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		345, 45, 120, 15, _T("Label color"), ID_CTL_STATIC);
	ctx->hwndEditCrLabel = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		355, 65, 60, 15, PAGECRWV_ID_CR_LABEL_EDIT);
	ctx->hwndBtnCrLabelPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		420, 65, 15, 15, _T("..."), PAGECRWV_ID_CR_LABEL_PICK);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		345, 90, 120, 15, _T("Ticks color"), ID_CTL_STATIC);
	ctx->hwndEditCrTicks = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		355, 110, 60, 15, PAGECRWV_ID_CR_TICKS_EDIT);
	ctx->hwndBtnCrTicksPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		420, 110, 15, 15, _T("..."), PAGECRWV_ID_CR_TICKS_PICK);

	ui_crt_static(ctx->uidata, ctx->hwndPage, 0,
		345, 135, 120, 15, _T("Scrollbar color"), ID_CTL_STATIC);
	ctx->hwndEditCrScrbar = ui_crt_editse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		355, 155, 60, 15, PAGECRWV_ID_CR_SCRBAR_EDIT);
	ctx->hwndBtnCrScrbarPick = ui_crt_btnse(ctx->uidata, ctx->hwndPage, WS_TABSTOP,
		420, 155, 15, 15, _T("..."), PAGECRWV_ID_CR_SCRBAR_PICK);

	ctx->hwndBtnUseChanCrMap = ui_crt_btn(ctx->uidata, ctx->hwndPage, BS_AUTOCHECKBOX|WS_TABSTOP,
		345, 185, 120, 15, _T("Mark channels"), PAGECRWV_ID_USE_CHAN_CRMAP);

	SetWindowFont(ctx->hwndEditCrBkgnd, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrLabel, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrTicks, ctx->uidata->hfnt_mono, FALSE);
	SetWindowFont(ctx->hwndEditCrScrbar, ctx->uidata->hfnt_mono, FALSE);

	SetWindowFont(ctx->hwndBtnCrBkgndPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrLabelPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrTicksPick, ctx->uidata->hfnt_mini, FALSE);
	SetWindowFont(ctx->hwndBtnCrScrbarPick, ctx->uidata->hfnt_mini, FALSE);


	pagecrwv_load(ctx, data);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_destroy(pagecrwv_ctx_t *ctx)
{

	pagecrwv_cmlist_clear(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static void pagecrwv_cleanup(pagecrwv_ctx_t *ctx)
{
	pagecrwv_preview_cleanup(&(ctx->preview));
	free(ctx);
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK pagecrwv_wndproc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	pagecrwv_ctx_t *ctx;

	switch(umsg)
	{
	case WM_DESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		if(ctx != NULL)
			pagecrwv_destroy(ctx);
		return 0;

	case WM_NCDESTROY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		if(ctx != NULL)
		{
			SetWndPtr(hwnd, GWLP_USERDATA, NULL);
			pagecrwv_cleanup(ctx);
		}
		return 0;

	case WM_PAINT:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		pagecrwv_handle_paint(ctx);
		return 0;

	case WM_COMMAND:

		ctx = GetWndPtr(hwnd, GWLP_USERDATA);

		switch(LOWORD(wp))
		{
		case PAGECRWV_ID_PT_ADD:
			pagecrwv_cmlist_pt_addnew(ctx);
			return 0;
		case PAGECRWV_ID_PT_DEL:
			pagecrwv_cmlist_pt_seldelete(ctx);
			return 0;
		case PAGECRWV_ID_PT_UP:
			pagecrwv_cmlist_pt_selmoveup(ctx);
			return 0;
		case PAGECRWV_ID_PT_DOWN:
			pagecrwv_cmlist_pt_selmovedown(ctx);
			return 0;
		case PAGECRWV_ID_PT_SORT:
			ListView_SortItems(ctx->hwndCmList,
				pagecrwv_cmlist_pt_compbymagn, (LPARAM)ctx);
			return 0;
		case PAGECRWV_ID_INITMAG_EXEC:
			pagecrwv_cmd_initmagrange(ctx);
			return 0;
		case PAGECRWV_ID_PRESET_APPLY_NORM:
			pagecrwv_cmd_apply_colormap_preset(ctx, PAGECRWV_CMLIST_COL_CRNORM);
			return 0;
		case PAGECRWV_ID_PRESET_APPLY_CHAN:
			pagecrwv_cmd_apply_colormap_preset(ctx, PAGECRWV_CMLIST_COL_CRCHAN);
			return 0;
		case PAGECRWV_ID_PREVIEW_UPD:
			pagecrwv_preview_update(ctx, 1);
			return 0;
		case PAGECRWV_ID_CR_BKGND_PICK:
			pagecrwv_pickcolor(ctx, ctx->hwndEditCrBkgnd);
			return 0;
		case PAGECRWV_ID_CR_LABEL_PICK:
			pagecrwv_pickcolor(ctx, ctx->hwndEditCrLabel);
			return 0;
		case PAGECRWV_ID_CR_TICKS_PICK:
			pagecrwv_pickcolor(ctx, ctx->hwndEditCrTicks);
			return 0;
		case PAGECRWV_ID_CR_SCRBAR_PICK:
			pagecrwv_pickcolor(ctx, ctx->hwndEditCrScrbar);
			return 0;
		}
		break;

	case WM_NOTIFY:
		ctx = GetWndPtr(hwnd, GWLP_USERDATA);
		switch(wp)
		{
		case PAGECRWV_ID_CMLIST:
			switch(((NMHDR*)lp)->code)
			{
			case NM_CLICK:
				pagecrwv_cmlist_handleclick(ctx, (void*)lp);
				return 0;
			case NM_DBLCLK:
				pagecrwv_cmlist_handledblclk(ctx, (void*)lp);
				return 0;
			}
			break;
		}
		break;
	}

	return DefWindowProc(hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

int pagecrwv_create(HWND hwndSetWnd, HWND hwndPage, uicommon_t *uidata,
					ini_data_t *ini, pagecrwv_data_t *data)
{
	pagecrwv_ctx_t *ctx;

	if( (ctx = calloc(1, sizeof(pagecrwv_ctx_t))) != NULL )
	{
		ctx->hwndSetWnd = hwndSetWnd;
		ctx->hwndPage = hwndPage;
		ctx->uidata = uidata;
		ctx->ini = ini;

		SetWndPtr(hwndPage, GWLP_WNDPROC, (void*)pagecrwv_wndproc);
		SetWndPtr(hwndPage, GWLP_USERDATA, ctx);

		if(pagecrwv_init(ctx, data))
			return 1;

		SetWndPtr(hwndPage, GWLP_USERDATA, NULL);

		free(ctx);
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */
