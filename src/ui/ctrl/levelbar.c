/* ---------------------------------------------------------------------------------------------- */

#include "levelbar.h"

/* ---------------------------------------------------------------------------------------------- */

static TCHAR *lbr_classname = _T("rxtest_level_bar");
static ATOM lbr_classatom;

/* ---------------------------------------------------------------------------------------------- */

static void lbr_draw_bkgnd(lbr_ctx_t *ctx, HDC hdc, int w, int h, COLORREF cr)
{
	HBRUSH hbr, hbr_prev;

	hbr = CreateSolidBrush(cr);
	hbr_prev = SelectObject(hdc, hbr);
	PatBlt(hdc, 0, 0, w, h, PATCOPY);
	SelectObject(hdc, hbr_prev);
	DeleteObject(hbr);
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_draw_ticks(lbr_ctx_t *ctx, HDC hdc, int cw, int ch, COLORREF cr)
{
	HPEN hpen, hpen_prev;
	double step, l;
	int pos;

	hpen = CreatePen(PS_SOLID, 1, cr);
	hpen_prev = SelectObject(hdc, hpen);

	step = 5.0;
	l = scale_first_mark(ctx->l_min, step, 1e-6);

	for( ; ; )
	{
		pos = scale_mark_pos(ctx->l_min, ctx->l_max, l, cw, 0);
		if(pos >= cw)
			break;

		if(fabs(fmod(l, 10.0)) < 1e-6)
		{
			MoveToEx(hdc, pos, 0, NULL);
			LineTo(hdc, pos, 2);

			MoveToEx(hdc, pos, ch - 2, NULL);
			LineTo(hdc, pos, ch);
		}
		else
		{
			MoveToEx(hdc, pos, 0, NULL);
			LineTo(hdc, pos, 1);

			MoveToEx(hdc, pos, ch - 1, NULL);
			LineTo(hdc, pos, ch);
		}

		l += step;
	}

	SelectObject(hdc, hpen_prev);
	DeleteObject(hpen);
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_draw_labels(lbr_ctx_t *ctx, HDC hdc, int cw, int ch,
							COLORREF cr_bk, COLORREF cr_fg, int s)
{
	HFONT hfont_prev;
	double step, l;
	int pos, pos_prev;
	RECT rect;
	TCHAR buf[16];

	hfont_prev = SelectObject(hdc, ctx->hfont);
	
	SetBkColor(hdc, cr_bk);
	SetTextColor(hdc, cr_fg);

	if(ctx->l_max - ctx->l_min <= 25.0) {
		step = 5.0;
	} else {
		step = 10.0;
	}

	l = scale_first_mark(ctx->l_min, step, 1e-6);

	pos_prev = -s;

	for( ; ; )
	{
		pos = scale_mark_pos(ctx->l_min, ctx->l_max, l, cw, 0);
		if(pos >= cw - s)
			break;

		if(pos - pos_prev >= s * 2)
		{
			rect.left = (pos + 1) - s;
			rect.right = (pos + 1) + s;
			rect.top = 0;
			rect.bottom = ch;

			_stprintf(buf, _T("%.0f"), l);

			DrawText(hdc, buf, (int)_tcslen(buf), 
				&rect, DT_SINGLELINE|DT_VCENTER|DT_CENTER);

			pos_prev = pos;
		}

		l += step;
	}

	SelectObject(hdc, hfont_prev);
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_buf_free(lbr_ctx_t *ctx)
{
	ctx->cw_buf = 0;
	ctx->ch_buf = 0;

	DeleteObject(ctx->hbm_empty);
	DeleteObject(ctx->hbm_full);
	DeleteObject(ctx->hbm_temp);
}

/* ---------------------------------------------------------------------------------------------- */

static int lbr_update(lbr_ctx_t *ctx, HDC hdc_wnd, HDC hdc_buf, int cw, int ch, int force)
{
	HBITMAP hbm_prev;

	if(force || (ctx->cw_buf != cw) || (ctx->ch_buf != ch))
	{
		lbr_buf_free(ctx);

		ctx->hbm_empty = CreateCompatibleBitmap(hdc_wnd, cw, ch);
		ctx->hbm_full = CreateCompatibleBitmap(hdc_wnd, cw, ch);
		ctx->hbm_temp = CreateCompatibleBitmap(hdc_wnd, cw, ch);

		if((ctx->hbm_empty == NULL) || (ctx->hbm_full == NULL) || (ctx->hbm_temp == NULL))
			return 0;

		hbm_prev = SelectObject(hdc_buf, ctx->hbm_empty);
		lbr_draw_bkgnd(ctx, hdc_buf, cw, ch, ctx->cr_bkgnd);
		lbr_draw_labels(ctx, hdc_buf, cw, ch, ctx->cr_bkgnd, ctx->cr_bkgnd_inv, 10);
		lbr_draw_ticks(ctx, hdc_buf, cw, ch, ctx->cr_bkgnd_inv);
		SelectObject(hdc_buf, hbm_prev);

		hbm_prev = SelectObject(hdc_buf, ctx->hbm_full);
		lbr_draw_bkgnd(ctx, hdc_buf, cw, ch, ctx->cr_bar);
		lbr_draw_labels(ctx, hdc_buf, cw, ch, ctx->cr_bar, ctx->cr_bar_inv, 10);
		lbr_draw_ticks(ctx, hdc_buf, cw, ch, ctx->cr_bar_inv);
		SelectObject(hdc_buf, hbm_prev);

		ctx->cw_buf = cw;
		ctx->ch_buf = ch;
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_copy_bufs(lbr_ctx_t *ctx, HDC hdc_wnd, HDC hdc_buf, int cw, int ch)
{
	HBITMAP hbm_prev;
	int bar_pos;
	double frac_bar;

	frac_bar = (ctx->l_pos - ctx->l_min) / (ctx->l_max - ctx->l_min);
	bar_pos = (int)((double)(cw - 1) * frac_bar + 0.5);

	if(bar_pos != 0)
	{
		hbm_prev = SelectObject(hdc_buf, ctx->hbm_full);
		BitBlt(hdc_wnd, 0, 0, bar_pos, ch, hdc_buf, 0, 0, SRCCOPY);
		SelectObject(hdc_buf, hbm_prev);
	}

	if(bar_pos != cw - 1)
	{
		hbm_prev = SelectObject(hdc_buf, ctx->hbm_empty);
		BitBlt(hdc_wnd, bar_pos, 0, cw, ch, hdc_buf, bar_pos, 0, SRCCOPY);
		SelectObject(hdc_buf, hbm_prev);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_draw_peak(lbr_ctx_t *ctx, HDC hdc_wnd, int cw, int ch)
{
	double frac_peak;
	int peak_pos;
	HPEN hpen_prev;

	if(ctx->l_peak != ctx->l_min)
	{
		frac_peak = (ctx->l_peak - ctx->l_min) / (ctx->l_max - ctx->l_min);
		peak_pos = (int)((double)(cw - 1) * frac_peak + 0.5);

		if(ctx->l_peak <= ctx->l_pos) {
			hpen_prev = SelectObject(hdc_wnd, ctx->hpen_peak_inv);
		} else {
			hpen_prev = SelectObject(hdc_wnd, ctx->hpen_peak);
		}

		MoveToEx(hdc_wnd, peak_pos, 0, NULL);
		LineTo(hdc_wnd, peak_pos, ch);

		SelectObject(hdc_wnd, hpen_prev);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_draw(lbr_ctx_t *ctx, HDC hdc, int force_update)
{
	RECT rc;
	int cw, ch;
	HDC hdc_mem;

	GetClientRect(ctx->hwnd, &rc);
	cw = rc.right;
	ch = rc.bottom;

	hdc_mem = CreateCompatibleDC(hdc);

	if(lbr_update(ctx, hdc, hdc_mem, cw, ch, force_update))
	{
		lbr_copy_bufs(ctx, hdc, hdc_mem, cw, ch);

		if(ctx->show_peak) {
			lbr_draw_peak(ctx, hdc, cw, ch);
		}
	}

	DeleteDC(hdc_mem);
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_paint(HWND hwnd)
{
	lbr_ctx_t *ctx;
	PAINTSTRUCT ps;
	HDC hdc;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);
	hdc = BeginPaint(hwnd, &ps);

	lbr_draw(ctx, hdc, 0);

	EndPaint(hwnd, &ps);
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_redraw(lbr_ctx_t *ctx, int force_update)
{
	HDC hdc;

	hdc = GetDC(ctx->hwnd);
	lbr_draw(ctx, hdc, force_update);
	ReleaseDC(ctx->hwnd, hdc);
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_setrange(HWND hwnd, lbr_range_t *range, int redraw)
{
	lbr_ctx_t *ctx;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	if( (fabs(range->l_min - ctx->l_min) >= 1e-6) ||
		(fabs(range->l_max - ctx->l_max) >= 1e-6) )
	{
		ctx->l_min = range->l_min;
		ctx->l_max = range->l_max;

		if(ctx->l_pos < ctx->l_min)
			ctx->l_pos = ctx->l_min;
		if(ctx->l_pos > ctx->l_max)
			ctx->l_pos = ctx->l_max;

		if(ctx->l_peak < ctx->l_min)
			ctx->l_peak = ctx->l_min;
		if(ctx->l_peak > ctx->l_max)
			ctx->l_peak = ctx->l_max;

		if(redraw) {
			lbr_redraw(ctx, 1);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_setpos(HWND hwnd, lbr_pos_t *pos, int redraw)
{
	double l_pos, l_peak;
	lbr_ctx_t *ctx;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	l_pos = pos->l_pos;
	if(l_pos < ctx->l_min)
		l_pos = ctx->l_min;
	if(l_pos > ctx->l_max)
		l_pos = ctx->l_max;

	l_peak = pos->l_peak;
	if(l_peak < ctx->l_min)
		l_peak = ctx->l_min;
	if(l_peak > ctx->l_max)
		l_peak = ctx->l_max;

	if( (fabs(l_pos - ctx->l_pos) >= 1e-6) ||
		(fabs(l_peak - ctx->l_peak) >= 1e-6) )
	{
		ctx->l_pos = l_pos;
		ctx->l_peak = l_peak;

		if(redraw) {
			lbr_redraw(ctx, 0);
		}
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_setfont(HWND hwnd, HFONT hfont, int redraw)
{
	lbr_ctx_t *ctx;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	ctx->hfont = hfont;

	if(redraw) {
		lbr_redraw(ctx, 1);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static int lbr_init(HWND hwnd, CREATESTRUCT *cs)
{
	lbr_ctx_t *ctx;
	lbr_options_t *opt;

	opt = cs->lpCreateParams;

	if( (ctx = calloc(1, sizeof(lbr_ctx_t))) == NULL )
		return 0;
	SetWndPtr(hwnd, GWLP_USERDATA, ctx);

	ctx->hwnd = hwnd;
	
	ctx->hfont = GetStockObject(SYSTEM_FONT);
	
	ctx->cr_bkgnd = GetSysColor(COLOR_WINDOW);
	ctx->cr_bar = RGB(0, 0, 128);
	ctx->cr_peak = RGB(255, 0, 0);

	ctx->cr_bkgnd_inv = ~(ctx->cr_bkgnd) & 0xffffffUL;
	ctx->cr_bar_inv = ~(ctx->cr_bar) & 0xffffffUL;
	ctx->cr_peak_inv = ~(ctx->cr_peak) & 0xffffffUL;

	if(opt->show_peak)
	{
		ctx->show_peak = 1;
		ctx->hpen_peak = CreatePen(PS_SOLID, 1, ctx->cr_peak);
		ctx->hpen_peak_inv = CreatePen(PS_SOLID, 1, ctx->cr_peak_inv);
	}

	ctx->l_min = 0.0;
	ctx->l_max = 100.0;
	ctx->l_pos = 0.0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static void lbr_cleanup(HWND hwnd)
{
	lbr_ctx_t *ctx;

	ctx = GetWndPtr(hwnd, GWLP_USERDATA);

	if(ctx != NULL)
	{
		SetWndPtr(hwnd, GWLP_USERDATA, NULL);

		if(ctx->show_peak)
		{
			DeleteObject(ctx->hpen_peak);
			DeleteObject(ctx->hpen_peak_inv);
		}
		
		lbr_buf_free(ctx);
		free(ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

static LRESULT CALLBACK lbr_proc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp)
{
	switch(umsg)
	{
	case WM_CREATE:
		if(!lbr_init(hwnd, (void*)lp))
			return -1;
		return 0;

	case WM_NCDESTROY:
		lbr_cleanup(hwnd);
		return 0;

	case WM_PAINT:
		lbr_paint(hwnd);
		return 0;

	case WM_SETFONT:
		lbr_setfont(hwnd, (HFONT)wp, LOWORD(lp));
		return 0;

	case LBR_SETRANGE:
		lbr_setrange(hwnd, (void*)lp, (int)wp);
		return 0;

	case LBR_SETPOS:
		lbr_setpos(hwnd, (void*)lp, (int)wp);
		return 0;
	}

	return DefWindowProc(hwnd, umsg, wp, lp);
}

/* ---------------------------------------------------------------------------------------------- */

HWND lbr_create(HINSTANCE h_inst, DWORD style, HWND hwndOwner,
				UINT uCtrlId, RECT *rc, HFONT hfont, int show_peak)
{
	HWND hwnd;
	lbr_options_t opt;

	opt.show_peak = show_peak;

	hwnd = CreateWindow(MAKEINTATOM(lbr_classatom), NULL,
		WS_CHILD|WS_VISIBLE|style, 
		rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top,
		hwndOwner, (HMENU)(LONG_PTR)uCtrlId, h_inst, &opt);

	if(hwnd != NULL)
	{
		SetWindowFont(hwnd, hfont, FALSE);
		return hwnd;
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

void lbr_send_setrange(HWND hwnd, double l_min, double l_max, int redraw)
{
	lbr_range_t range;

	range.l_min = l_min;
	range.l_max = l_max;
	
	/*SendMessage(hwnd, LBR_SETRANGE, (WPARAM)redraw, (LPARAM)&range);*/
	lbr_setrange(hwnd, &range, redraw);
}

/* ---------------------------------------------------------------------------------------------- */

void lbr_send_setpos(HWND hwnd, double l_pos, double l_peak, int redraw)
{
	lbr_pos_t pos;

	pos.l_pos = l_pos;
	pos.l_peak = l_peak;

	/*SendMessage(hwnd, LBR_SETPOS, (WPARAM)redraw, (LPARAM)&pos);*/
	lbr_setpos(hwnd, &pos, redraw);
}

/* ---------------------------------------------------------------------------------------------- */

int lbr_registerclass(HINSTANCE h_inst)
{
	WNDCLASS wc;

	if(lbr_classatom == 0)
	{
		memset(&wc, 0, sizeof(wc));
		wc.lpfnWndProc = lbr_proc;
		wc.hInstance = h_inst;
		wc.lpszClassName = lbr_classname;

		lbr_classatom = RegisterClass(&wc);
	}

	return (lbr_classatom != 0);
}

/* ---------------------------------------------------------------------------------------------- */

void lbr_unregisterclass(HINSTANCE h_inst)
{
	if(lbr_classatom != 0)
	{
		UnregisterClass(MAKEINTATOM(lbr_classatom), h_inst);
		lbr_classatom = 0;
	}
}

/* ---------------------------------------------------------------------------------------------- */
