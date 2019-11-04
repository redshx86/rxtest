/* ---------------------------------------------------------------------------------------------- */

#include "specview.h"

/* ---------------------------------------------------------------------------------------------- */

/* draw window background */
static int specview_draw_window_bkgnd(specview_ctx_t *ctx)
{
	HGDIOBJ hbr_prev; 

	/* check resources */
	if((!ctx->buf.is_inited) || (!ctx->res.is_inited))
		return 0;

	/* fill background */
	hbr_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hbr_window);
	PatBlt(ctx->buf.hdc_buf, 0, 0, ctx->buf.win_w, ctx->buf.win_h, PATCOPY);
	SelectObject(ctx->buf.hdc_buf, hbr_prev);

	/* invalidate buffer contents */
	ctx->mag.is_drawn = 0;
	ctx->freq.is_drawn = 0;
	ctx->buf.is_shm_copied = 0;
	ctx->sql.is_copied = 0;
	ctx->text.is_drawn = 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* draw magnitude scale */
static int specview_draw_mag_scale(specview_ctx_t *ctx)
{
	HGDIOBJ hpen_prev, hfnt_prev;
	int pos_y, j;
	double m;
	RECT tr;
	TCHAR buf[16];

	/* check resources */
	if((!ctx->buf.is_inited) || (!ctx->res.is_inited) || (!ctx->mag.is_inited))
		return 0;

	/* select objects */
	hpen_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hpen_ticks);
	hfnt_prev = SelectObject(ctx->buf.hdc_buf, ctx->hfont);

	SetTextColor(ctx->buf.hdc_buf, ctx->cfg.cr_label);
	SetBkColor(ctx->buf.hdc_buf, ctx->res.cr_window);

	/* draw magnitude labels */
	m = scale_first_mark(ctx->mag.m_0, ctx->mag.m_step, 1e-6);
	for( ; ; )
	{
		/* calculate label position */
		j = scale_mark_pos(ctx->mag.m_0, ctx->mag.m_1, m, ctx->mag.height, 0);
		if(j >= ctx->mag.height)
			break;
		pos_y = (ctx->buf.shm_y + ctx->buf.shm_h - 1) - j;

		/* format label text */
		_stprintf(buf, _T("%.*f"), (fabs(m) < 100.0) ? 1 : 0, m);

		/* draw label */
		tr.left = 0;
		tr.right = ctx->buf.shm_x - 3;
		tr.top = pos_y - 7;
		tr.bottom = pos_y + 8;

		DrawText(ctx->buf.hdc_buf, buf, (int)_tcslen(buf),
			&tr, DT_SINGLELINE|DT_RIGHT|DT_VCENTER);

		/* to next label */
		m += ctx->mag.m_step;
	}

	/* restore previous objects */
	SelectObject(ctx->buf.hdc_buf, hpen_prev);
	SelectObject(ctx->buf.hdc_buf, hfnt_prev);

	/* mark magnitude scale valid */
	ctx->mag.is_drawn = 1;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* draw frequency scale */
static int specview_draw_freq_scale(specview_ctx_t *ctx)
{
	HGDIOBJ hpen_prev, hfnt_prev;
	double f, f_coarse_value;
	int j, j_prev, j_next;
	RECT tr;
	TCHAR buf[16], *p;

	/* check resources */
	if((!ctx->buf.is_inited) || (!ctx->res.is_inited) || (!ctx->freq.is_inited))
		return 0;

	/* select objects */
	hpen_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hpen_ticks);
	hfnt_prev = SelectObject(ctx->buf.hdc_buf, ctx->hfont);

	SetTextColor(ctx->buf.hdc_buf, ctx->cfg.cr_label);
	SetBkColor(ctx->buf.hdc_buf, ctx->res.cr_window);

	/* draw coarse interval scale */
	f = scale_first_mark(ctx->freq.f_0, ctx->freq.f_coarse_step, 1e-3);
	j_prev = 0;
	for( ; ; )
	{
		/* calculate interval delimiter position */
		j = scale_mark_pos(ctx->freq.f_0, ctx->freq.f_1, f, ctx->freq.width, 0);

		/* draw interval label */
		j_next = min(j, (ctx->freq.width - 1));
		if(j_next - j_prev >= 60)
		{
			/* calculate label value */
			p = buf;
			f_coarse_value = f - ctx->freq.f_coarse_step;
			if(f_coarse_value < 0) {
				f_coarse_value += ctx->freq.f_coarse_step;
				if(f_coarse_value == 0)
					*(p++) = _T('-'); /* -0.xxx */
			}

			/* format label text */
			switch(ctx->freq.f_scale_mode)
			{
			case SPECVIEW_FREQSCALE_HZ_KHZ:
				_stprintf(p, _T("%.03f"), 1e-6 * f_coarse_value);
				break;
			case SPECVIEW_FREQSCALE_KHZ_MHZ:
				_stprintf(p, _T("%.0f"), 1e-6 * f_coarse_value);
				break;
			case SPECVIEW_FREQSCALE_MHZ_GHZ:
				_stprintf(p, _T("%.0f"), 1e-9 * f_coarse_value);
				break;
			}

			/* draw label */
			tr.left = ctx->buf.shm_x + j_prev;
			tr.right = ctx->buf.shm_x + j_next + 1;
			tr.top = ctx->buf.shm_y + ctx->buf.shm_h + 1;
			tr.bottom = tr.top + 15;

			DrawText(ctx->buf.hdc_buf, buf, (int)_tcslen(buf), &tr,
				DT_SINGLELINE|DT_CENTER|DT_TOP);
		}

		/* draw interval delimiter */
		if(j < ctx->freq.width)
		{
			MoveToEx(ctx->buf.hdc_buf, ctx->buf.shm_x + j,
				ctx->buf.shm_y + ctx->buf.shm_h + 2, NULL);
			LineTo(ctx->buf.hdc_buf, ctx->buf.shm_x + j,
				ctx->buf.shm_y + ctx->buf.shm_h + 12);
		}

		/* check for end of scale */
		if(j >= ctx->freq.width)
			break;

		/* go to next interval */
		j_prev = j_next;
		f += ctx->freq.f_coarse_step;
	}

	/* draw fine scale */
	MoveToEx(ctx->buf.hdc_buf, ctx->buf.shm_x,
		ctx->buf.shm_y + ctx->buf.shm_h + 15, NULL);
	LineTo(ctx->buf.hdc_buf, ctx->buf.shm_x + ctx->buf.shm_w,
		ctx->buf.shm_y + ctx->buf.shm_h + 15);

	f = scale_first_mark(ctx->freq.f_0, ctx->freq.f_step, 1e-3);
	for( ; ; )
	{
		/* calculate fine scale label and tick position */
		j = scale_mark_pos(ctx->freq.f_0, ctx->freq.f_1, f, ctx->freq.width, 0);

		/* check for end of scale */
		if(j >= ctx->freq.width)
			break;

		/* format label text */
		switch(ctx->freq.f_scale_mode)
		{
		case SPECVIEW_FREQSCALE_HZ_KHZ:
			_stprintf(buf, _T("%03.0f"), fmod(fabs(f), 1e3));
			break;
		case SPECVIEW_FREQSCALE_KHZ_MHZ:
			_stprintf(buf, _T(".%03.0f"), fmod(1e-3 * fabs(f), 1e3));
			break;
		case SPECVIEW_FREQSCALE_MHZ_GHZ:
			_stprintf(buf, _T(".%03.0f"), fmod(1e-6 * fabs(f), 1e3));
			break;
		}

		/* draw label */
		tr.left = ctx->buf.shm_x + j - 15;
		tr.right = ctx->buf.shm_x + j + 16;
		tr.top = ctx->buf.shm_y + ctx->buf.shm_h + 17;
		tr.bottom = tr.top + 15;

		DrawText(ctx->buf.hdc_buf, buf, (int)_tcslen(buf), &tr,
			DT_SINGLELINE|DT_CENTER|DT_TOP);

		/* draw tick */
		MoveToEx(ctx->buf.hdc_buf, ctx->buf.shm_x + j,
			ctx->buf.shm_y + ctx->buf.shm_h + 14, NULL);
		LineTo(ctx->buf.hdc_buf, ctx->buf.shm_x + j,
			ctx->buf.shm_y + ctx->buf.shm_h + 17);

		/* go to next tick and label */
		f += ctx->freq.f_step;
	}

	/* restore previous objects */
	SelectObject(ctx->buf.hdc_buf, hpen_prev);
	SelectObject(ctx->buf.hdc_buf, hfnt_prev);

	/* mark frequency scale valid */
	ctx->freq.is_drawn = 1;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* update schematic source images */
static int specview_update_shm_src(specview_ctx_t *ctx)
{
	HGDIOBJ hbr_prev, hpen_prev;
	int x_start, x, i, y;
	double m, m_step, f, f_step;
	unsigned char br;

	/* check required resources */
	if( (!ctx->buf.is_inited) || (!ctx->buf.is_chmap_inited) ||
		(!ctx->res.is_inited) || (!ctx->mag.is_inited))
	{
		return 0;
	}

	/* prepare background */
	if(!ctx->buf.is_shm_src_prepared)
	{
		/* erase 'cold' schematic background */
		hbr_prev = SelectObject(ctx->buf.hdc_shm_src_cold, ctx->res.hbr_bkgnd_cold);
		PatBlt(ctx->buf.hdc_shm_src_cold, 0, 0, ctx->buf.shm_w, ctx->buf.shm_h, PATCOPY);
		SelectObject(ctx->buf.hdc_shm_src_cold, hbr_prev);

		/* erase 'hot' schematic background */
		hbr_prev = SelectObject(ctx->buf.hdc_shm_src_hot, ctx->res.hbr_bkgnd_hot);
		PatBlt(ctx->buf.hdc_shm_src_hot, 0, 0, ctx->buf.shm_w, ctx->buf.shm_h, PATCOPY);
		SelectObject(ctx->buf.hdc_shm_src_hot, hbr_prev);

		/* reset actual state to all-background */
		memset(ctx->buf.shm_cr_map_actual, SPECVIEW_COLOR_BKGND, ctx->buf.shm_w);
	}

	/* update schematic vertical bars */
	for(i = 0; i < ctx->buf.shm_w; i++)
	{
		/* is schematic bar should be redrawn? */
		if(ctx->buf.shm_cr_map[i] != ctx->buf.shm_cr_map_actual[i])
		{
			/* redraw 'cold' schematic bar */
			hpen_prev = SelectObject(ctx->buf.hdc_shm_src_hot,
				ctx->res.hpen_hot[ctx->buf.shm_cr_map[i]]);
			MoveToEx(ctx->buf.hdc_shm_src_hot, i, 0, NULL);
			LineTo(ctx->buf.hdc_shm_src_hot, i, ctx->buf.shm_h);
			SelectObject(ctx->buf.hdc_shm_src_hot, hpen_prev);

			/* redraw 'hot' schematic bar */
			hpen_prev = SelectObject(ctx->buf.hdc_shm_src_cold,
				ctx->res.hpen_cold[ctx->buf.shm_cr_map[i]]);
			MoveToEx(ctx->buf.hdc_shm_src_cold, i, 0, NULL);
			LineTo(ctx->buf.hdc_shm_src_cold, i, ctx->buf.shm_h);
			SelectObject(ctx->buf.hdc_shm_src_cold, hpen_prev);
		}
	}

	/* update frequency grid */
	f_step = 0.5 * ctx->freq.f_step; /* double gridline density */
	f = scale_first_mark(ctx->freq.f_0, f_step, 1e-3);
	for( ; ; )
	{
		/* calculate gridline x-position */
		x = scale_mark_pos(ctx->freq.f_0, ctx->freq.f_1, f, ctx->freq.width, 0);

		/* check for schematic end */
		if(x >= ctx->freq.width)
			break;

		if(x >= 0)
		{
			/* is gridline should be redrawn? */
			if( (ctx->buf.shm_cr_map[x] != ctx->buf.shm_cr_map_actual[x]) ||
				(!ctx->buf.is_shm_src_prepared) )
			{
				/* redraw frequency gridline ('cold' schematic) */
				hpen_prev = SelectObject(ctx->buf.hdc_shm_src_hot,
					ctx->res.hpen_hot[ctx->buf.shm_cr_map[x] + SPECVIEW_COLOR_BKGND_GRID]);
				MoveToEx(ctx->buf.hdc_shm_src_hot, x, 0, NULL);
				LineTo(ctx->buf.hdc_shm_src_hot, x, ctx->buf.shm_h);
				SelectObject(ctx->buf.hdc_shm_src_hot, hpen_prev);

				/* redraw frequency gridline ('hot' schematic) */
				hpen_prev = SelectObject(ctx->buf.hdc_shm_src_cold,
					ctx->res.hpen_cold[ctx->buf.shm_cr_map[x] + SPECVIEW_COLOR_BKGND_GRID]);
				MoveToEx(ctx->buf.hdc_shm_src_cold, x, 0, NULL);
				LineTo(ctx->buf.hdc_shm_src_cold, x, ctx->buf.shm_h);
				SelectObject(ctx->buf.hdc_shm_src_cold, hpen_prev);
			}
		}

		/* next gridline frequency */
		f += f_step;
	}

	/* update magnitude grid */
	m_step = 1.0 * ctx->mag.m_step;
	m = scale_first_mark(ctx->mag.m_0, m_step, 1e-6);
	for( ; ; )
	{
		/* calculate gridline y-position */
		y = scale_mark_pos(ctx->mag.m_0, ctx->mag.m_1, m, ctx->mag.height, 1);

		/* check for end of grid */
		if(y < 0)
			break;

		/* update grid line by segment */
		x = 0;
		while(x < ctx->buf.shm_w)
		{
			/* skip already valid segments */
			if(ctx->buf.is_shm_src_prepared)
			{
				/* increment x-coordinate until segment end or end of schematic */
				while( (x < ctx->buf.shm_w) &&
					(ctx->buf.shm_cr_map[x] == ctx->buf.shm_cr_map_actual[x]) )
				{
					x++;
				}

				/* check for schematic end */
				if(x >= ctx->buf.shm_w)
					break;
			}

			/* calculate segment length to be redrawn */
			x_start = x;
			br = ctx->buf.shm_cr_map[x];
			while( (x < ctx->buf.shm_w) && (ctx->buf.shm_cr_map[x] == br) &&
				((ctx->buf.shm_cr_map_actual[x] != br) || (!ctx->buf.is_shm_src_prepared)) )
			{
				x++;
			}

			/* redraw gridline segment ('cold' schematic) */
			hpen_prev = SelectObject(ctx->buf.hdc_shm_src_hot,
				ctx->res.hpen_hot[br + SPECVIEW_COLOR_BKGND_GRID]);
			MoveToEx(ctx->buf.hdc_shm_src_hot, x_start, y, NULL);
			LineTo(ctx->buf.hdc_shm_src_hot, x, y);
			SelectObject(ctx->buf.hdc_shm_src_hot, hpen_prev);

			/* redraw gridline segment ('hot' schematic) */
			hpen_prev = SelectObject(ctx->buf.hdc_shm_src_cold,
				ctx->res.hpen_cold[br + SPECVIEW_COLOR_BKGND_GRID]);
			MoveToEx(ctx->buf.hdc_shm_src_cold, x_start, y, NULL);
			LineTo(ctx->buf.hdc_shm_src_cold, x, y);
			SelectObject(ctx->buf.hdc_shm_src_cold, hpen_prev);
		}

		/* next gridline magnitude */
		m += m_step;
	}

	/* mark schematic source images valid */
	ctx->buf.is_shm_src_prepared = 1;
	ctx->buf.is_shm_src_updated = 1;

	/* save current schematic state */
	memcpy(ctx->buf.shm_cr_map_actual, ctx->buf.shm_cr_map, ctx->buf.shm_w);

	/* mark schematic for copy */
	ctx->buf.is_shm_copied = 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* copy schematic to main buffer (partially) */
static void specview_copy_shm_part(specview_ctx_t *ctx, int copy_x, int copy_y,
								   int copy_w, int copy_h)
{
	/* copy cold schematic to main buffer */
	BitBlt(ctx->buf.hdc_buf,
		ctx->buf.shm_x + copy_x, ctx->buf.shm_y + copy_y, copy_w, copy_h,
		ctx->buf.hdc_shm_src_cold, copy_x, copy_y, SRCCOPY);

	/* copy hot schematic to main buffer with spectrum mask applied */
	MaskBlt(ctx->buf.hdc_buf,
		ctx->buf.shm_x + copy_x, ctx->buf.shm_y + copy_y, copy_w, copy_h,
		ctx->buf.hdc_shm_src_hot, copy_x, copy_y,
		ctx->buf.hbm_shm_mask, copy_x, copy_x, MAKEROP4(SRCCOPY, 0x00AA0029));
}

/* ---------------------------------------------------------------------------------------------- */

/* copy schematic to main buffer (full) */
static int specview_copy_shm(specview_ctx_t *ctx)
{
	/* check for buffer inited */
	if(!ctx->buf.is_inited)
		return 0;

	/* copy schematic image */
	specview_copy_shm_part(ctx, 0, 0, ctx->buf.shm_w, ctx->buf.shm_h);

	/* mark schematic copied */
	ctx->buf.is_shm_copied = 1;

	/* invalidate upper layers */
	ctx->sql.is_copied = 0;
	ctx->text.is_drawn = 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* draw marks from single sql entry */
static int specview_draw_sql_ent(specview_ctx_t *ctx, specview_sql_entry_t *ent,
								 int update, int limit_left, int limit_right)
{
	int center, left, right, tmp_left, tmp_right;
	int draw_op_thres_mark, draw_cl_thres_mark, draw_sense_mark;
	HPEN hpen_prev, hpen_mask_prev;

	center = ent->pos;
	left = center - SPECVIEW_SQL_MARK_WIDTH / 2;
	right = center + SPECVIEW_SQL_MARK_WIDTH / 2 + 1;

	/* clip marks to limiting range */
	if((right < limit_left) || (left > limit_right))
		return 1;

	if(center < limit_left)
		center = limit_left;
	if(center > limit_right)
		center = limit_right;

	if(left < limit_left)
		left = limit_left;
	if(right > limit_right)
		right = limit_right;

	if(left == right)
		return 1;

	/* only draw marks within window height */
	draw_op_thres_mark = ((ent->thres_op >= 0) && (ent->thres_op < ctx->buf.shm_h));
	draw_cl_thres_mark = ((ent->thres_cl >= 0) && (ent->thres_cl < ctx->buf.shm_h));
	draw_sense_mark = ((ent->sense >= 0) && (ent->sense < ctx->buf.shm_h));

	/* dont draw threshold marks covered by sense mark */
	if(ent->thres_op == ent->sense)
		draw_op_thres_mark = 0;
	if(ent->thres_cl == ent->sense)
		draw_cl_thres_mark = 0;

	/* dont draw half marks outside window width */
	if(ent->thres_op == ent->thres_cl)
	{
		if(left == center)
			draw_op_thres_mark = 0;
		if(right == center)
			draw_cl_thres_mark = 0;
	}

	/* draw only changed marks when updating */
	if(update && (ent->pos == ent->pos_drawn))
	{
		if( (ent->thres_op == ent->thres_op_drawn) &&
			(ent->thres_op != ent->thres_cl_drawn) &&
			(ent->thres_op != ent->sense_drawn) )
		{
			draw_op_thres_mark = 0;
		}

		if( (ent->thres_cl == ent->thres_cl_drawn) &&
			(ent->thres_cl != ent->thres_op_drawn) &&
			(ent->thres_cl != ent->sense_drawn) )
		{
			draw_cl_thres_mark = 0;
		}

		if( (ent->is_open == ent->is_open_drawn) && 
			(ent->sense == ent->sense_drawn) )
		{
			draw_sense_mark = 0;
		}
	}

	if(!ent->draw_sense)
		draw_sense_mark = 0;

	/* draw open threshold mark */
	if(draw_op_thres_mark)
	{
		if(ent->thres_op == ent->thres_cl) {
			tmp_right = center;
		} else {
			tmp_right = right;
		}

		hpen_prev = SelectObject(ctx->buf.hdc_layer_sq, ctx->res.hpen_sq_thres_op);
		MoveToEx(ctx->buf.hdc_layer_sq, left, ent->thres_op, NULL);
		LineTo(ctx->buf.hdc_layer_sq, tmp_right, ent->thres_op);
		SelectObject(ctx->buf.hdc_layer_sq, hpen_prev);

		hpen_mask_prev = SelectObject(ctx->buf.hdc_mask_sq, ctx->res.hpen_stock_white);
		MoveToEx(ctx->buf.hdc_mask_sq, left, ent->thres_op, NULL);
		LineTo(ctx->buf.hdc_mask_sq, tmp_right, ent->thres_op);
		SelectObject(ctx->buf.hdc_mask_sq, hpen_mask_prev);
	}

	/* draw close threshold mark */
	if(draw_cl_thres_mark)
	{
		if(ent->thres_cl == ent->thres_op) {
			tmp_left = center;
		} else {
			tmp_left = left;
		}

		hpen_prev = SelectObject(ctx->buf.hdc_layer_sq, ctx->res.hpen_sq_thres_cl);
		MoveToEx(ctx->buf.hdc_layer_sq, tmp_left, ent->thres_cl, NULL);
		LineTo(ctx->buf.hdc_layer_sq, right, ent->thres_cl);
		SelectObject(ctx->buf.hdc_layer_sq, hpen_prev);

		hpen_mask_prev = SelectObject(ctx->buf.hdc_mask_sq, ctx->res.hpen_stock_white);
		MoveToEx(ctx->buf.hdc_mask_sq, tmp_left, ent->thres_cl, NULL);
		LineTo(ctx->buf.hdc_mask_sq, right, ent->thres_cl);
		SelectObject(ctx->buf.hdc_mask_sq, hpen_mask_prev);
	}

	/* draw sense value mark */
	if(draw_sense_mark)
	{
		if(ent->is_open) {
			hpen_prev = SelectObject(ctx->buf.hdc_layer_sq, ctx->res.hpen_sq_sense_op);
		} else {
			hpen_prev = SelectObject(ctx->buf.hdc_layer_sq, ctx->res.hpen_sq_sense_cl);
		}

		MoveToEx(ctx->buf.hdc_layer_sq, left, ent->sense, NULL);
		LineTo(ctx->buf.hdc_layer_sq, right, ent->sense);

		SelectObject(ctx->buf.hdc_layer_sq, hpen_prev);

		hpen_mask_prev = SelectObject(ctx->buf.hdc_mask_sq, ctx->res.hpen_stock_white);
		MoveToEx(ctx->buf.hdc_mask_sq, left, ent->sense, NULL);
		LineTo(ctx->buf.hdc_mask_sq, right, ent->sense);
		SelectObject(ctx->buf.hdc_mask_sq, hpen_mask_prev);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* erase marks from single sql entry */
static int specview_erase_sql_ent(specview_ctx_t *ctx, specview_sql_entry_t *ent,
								  int update, int limit_left, int limit_right)
{
	int center, left, right, tmp_left, tmp_right;
	int erase_op_thres_mark, erase_cl_thres_mark, erase_sense_mark;
	HPEN hpen_mask_prev;

	center = ent->pos_drawn;
	left = center - SPECVIEW_SQL_MARK_WIDTH / 2;
	right = center + SPECVIEW_SQL_MARK_WIDTH / 2 + 1;

	/* clip marks to limiting range */
	if((right < limit_left) || (left > limit_right))
		return 1;

	if(center < limit_left)
		center = limit_left;
	if(center > limit_right)
		center = limit_right;

	if(left < limit_left)
		left = limit_left;
	if(right > limit_right)
		right = limit_right;

	if(left == right)
		return 1;

	/* only erase marks within window height */
	erase_op_thres_mark = ((ent->thres_op_drawn >= 0) && (ent->thres_op_drawn < ctx->buf.shm_h));
	erase_cl_thres_mark = ((ent->thres_cl_drawn >= 0) && (ent->thres_cl_drawn < ctx->buf.shm_h));
	erase_sense_mark = ((ent->sense_drawn >= 0) && (ent->sense_drawn < ctx->buf.shm_h));

	/* dont erase threshold marks covered by sense mark */
	if(ent->thres_op_drawn == ent->sense_drawn)
		erase_op_thres_mark = 0;
	if(ent->thres_cl_drawn == ent->sense_drawn)
		erase_cl_thres_mark = 0;

	/* dont erase half marks outside window width */
	if(ent->thres_op_drawn == ent->thres_cl_drawn)
	{
		if(left == center)
			erase_op_thres_mark = 0;
		if(right == center)
			erase_cl_thres_mark = 0;
	}

	/* erase only changed marks when updating */
	if(update && (ent->pos == ent->pos_drawn))
	{
		if( (ent->thres_op == ent->thres_op_drawn) &&
			(ent->thres_op != ent->thres_cl_drawn) &&
			(ent->thres_op != ent->sense_drawn) )
		{
			erase_op_thres_mark = 0;
		}

		if( (ent->thres_cl == ent->thres_cl_drawn) &&
			(ent->thres_cl != ent->thres_op_drawn) &&
			(ent->thres_cl != ent->sense_drawn) )
		{
			erase_cl_thres_mark = 0;
		}

		if( ent->draw_sense &&
			(ent->is_open == ent->is_open_drawn) && 
			(ent->sense == ent->sense_drawn) )
		{
			erase_sense_mark = 0;
		}
	}

	if(!ent->sense_is_drawn)
		erase_sense_mark = 0;

	/* erase open threshold mark */
	if(erase_op_thres_mark)
	{
		if(ent->thres_op_drawn == ent->thres_cl_drawn) {
			tmp_right = center;
		} else {
			tmp_right = right;
		}

		hpen_mask_prev = SelectObject(ctx->buf.hdc_mask_sq, ctx->res.hpen_stock_black);
		MoveToEx(ctx->buf.hdc_mask_sq, left, ent->thres_op_drawn, NULL);
		LineTo(ctx->buf.hdc_mask_sq, tmp_right, ent->thres_op_drawn);
		SelectObject(ctx->buf.hdc_mask_sq, hpen_mask_prev);
	}

	/* erase close threshold mark */
	if(erase_cl_thres_mark)
	{
		if(ent->thres_cl_drawn == ent->thres_op_drawn) {
			tmp_left = center;
		} else {
			tmp_left = left;
		}

		hpen_mask_prev = SelectObject(ctx->buf.hdc_mask_sq, ctx->res.hpen_stock_black);
		MoveToEx(ctx->buf.hdc_mask_sq, tmp_left, ent->thres_cl_drawn, NULL);
		LineTo(ctx->buf.hdc_mask_sq, right, ent->thres_cl_drawn);
		SelectObject(ctx->buf.hdc_mask_sq, hpen_mask_prev);
	}

	/* erase sense value mark */
	if(erase_sense_mark)
	{
		hpen_mask_prev = SelectObject(ctx->buf.hdc_mask_sq, ctx->res.hpen_stock_black);
		MoveToEx(ctx->buf.hdc_mask_sq, left, ent->sense_drawn, NULL);
		LineTo(ctx->buf.hdc_mask_sq, right, ent->sense_drawn);
		SelectObject(ctx->buf.hdc_mask_sq, hpen_mask_prev);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* redraw all sql marks at specified range */
static int specview_draw_sql_range(specview_ctx_t *ctx, specview_sql_entry_t *ent_ignore,
								   int limit_left, int limit_right)
{
	specview_sql_entry_t *ent;
	int ent_left, ent_right;

	/* clip limiting range to buffer width */
	if(limit_left < 0)
		limit_left = 0;
	if(limit_right > ctx->buf.shm_w)
		limit_right = ctx->buf.shm_w;

	for(ent = ctx->sql.l_first; ent != NULL; ent = ent->l_next)
	{
		if(ent != ent_ignore)
		{
			ent_left = ent->pos - SPECVIEW_SQL_MARK_WIDTH / 2;
			ent_right = ent->pos + SPECVIEW_SQL_MARK_WIDTH / 2 + 1;

			if((ent_left < limit_right) && (ent_right > limit_left))
				specview_draw_sql_ent(ctx, ent, 0, limit_left, limit_right);
		}
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* update all sql marks */
static int specview_update_sql(specview_ctx_t *ctx)
{
	specview_sql_entry_t *ent, *ent_next;
	int ent_drawn_left, ent_drawn_right, ent_is_changed;
	unsigned int hash;

	/* check resources */
	if((!ctx->sql.is_inited) || (!ctx->buf.is_inited) || (!ctx->res.is_inited))
		return 0;

	/* erase mask when doing full redraw */
	if(ctx->sql.use_full_redraw) {
		PatBlt(ctx->buf.hdc_mask_sq, 0, 0, ctx->buf.shm_w, ctx->buf.shm_h, BLACKNESS);
	}

	/* process sql mark entries */
	for(ent = ctx->sql.l_first; ent != NULL; ent = ent_next)
	{
		ent_next = ent->l_next;

		if(!ent->is_deleted)
		{
			if((!ctx->sql.use_full_redraw) && ent->is_drawn)
			{
				/* check entry is changed */
				ent_is_changed = 
					(ent->pos_drawn != ent->pos) ||
					(ent->sense_drawn != ent->sense) ||
					(ent->thres_op_drawn != ent->thres_op) ||
					(ent->thres_cl_drawn != ent->thres_cl) ||
					(ent->sense_is_drawn != ent->draw_sense) ||
					(ent->is_open_drawn != ent->is_open);

				if(ent_is_changed)
				{
					/* erase changed marks */
					specview_erase_sql_ent(ctx, ent, 1, 0, ctx->buf.shm_w);

					/* restore background marks if overlapping occurs */
					ent_drawn_left = ent->pos_drawn - SPECVIEW_SQL_MARK_WIDTH / 2;
					ent_drawn_right = ent->pos_drawn + SPECVIEW_SQL_MARK_WIDTH / 2 + 1;
					specview_draw_sql_range(ctx, ent, ent_drawn_left, ent_drawn_right);

					/* redraw changed marks */
					specview_draw_sql_ent(ctx, ent, 1, 0, ctx->buf.shm_w);
				}
			}
			else
			{
				/* draw marks */
				specview_draw_sql_ent(ctx, ent, 0, 0, ctx->buf.shm_w);
			}

			/* save state of marks */
			ent->pos_drawn = ent->pos;
			ent->sense_drawn = ent->sense;
			ent->thres_op_drawn = ent->thres_op;
			ent->thres_cl_drawn = ent->thres_cl;
			ent->is_open_drawn = ent->is_open;
			ent->sense_is_drawn = ent->draw_sense;
			ent->is_drawn = 1;
		}
		else
		{
			if((!ctx->sql.use_full_redraw) && ent->is_drawn)
			{
				/* erase deleted marks */
				specview_erase_sql_ent(ctx, ent, 0, 0, ctx->buf.shm_w);

				/* restore background marks if overlapping occurs */
				ent_drawn_left = ent->pos_drawn - SPECVIEW_SQL_MARK_WIDTH / 2;
				ent_drawn_right = ent->pos_drawn + SPECVIEW_SQL_MARK_WIDTH / 2 + 1;
				specview_draw_sql_range(ctx, ent, ent_drawn_left, ent_drawn_right);
			}

			/* delete entry */
			hash = ent->hash;
			DLIST_REMOVE(ent, ctx->sql.l_first, ctx->sql.l_last, l_next, l_prev);
			DLIST_REMOVE(ent, ctx->sql.h_firsts[hash], ctx->sql.h_lasts[hash], h_next, h_prev);
			free(ent);
		}
	}

	/* set all sql marks updated */
	ctx->sql.is_updated = 1;
	ctx->sql.use_full_redraw = 0;

	/* select image for copying */
	ctx->sql.is_copied = 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* copy sql marks image part */
static int specview_copy_sql_layer_part(specview_ctx_t *ctx, int x, int y, int w, int h)
{
	if(!ctx->buf.is_inited)
		return 0;

	MaskBlt(ctx->buf.hdc_buf,
		ctx->buf.shm_x + x, ctx->buf.shm_y + y, w, h,
		ctx->buf.hdc_layer_sq, x, y,
		ctx->buf.hbm_mask_sq, x, y,
		MAKEROP4(SRCCOPY, 0x00AA0029));

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* copy sql marks image full */
static int specview_copy_sql_layer(specview_ctx_t *ctx)
{
	if(!specview_copy_sql_layer_part(ctx, 0, 0, ctx->buf.shm_w, ctx->buf.shm_h))
		return 0;

	/* mark image copied */
	ctx->sql.is_copied = 1;

	/* invalidate upper layers */
	ctx->text.is_drawn = 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set new sql marks data (reads from receiver state) */
int specview_set_sql(specview_ctx_t *ctx, int use_full_redraw, int only_sense)
{
	int status = 1;
	rxproc_t *proc;
	specview_sql_entry_t *ent;
	unsigned int hash, tick;
	double freq, sense;

	if((!ctx->buf.is_inited) || (!ctx->freq.is_inited) || (!ctx->mag.is_inited))
		return 0;

	tick = GetTickCount();

	EnterCriticalSection(&(ctx->csec));

	/* mark all entries erased and set delete flags */
	for(ent = ctx->sql.l_first; ent != NULL; ent = ent->l_next)
	{
		if(use_full_redraw)
			ent->is_drawn = 0;
		if(!only_sense)
			ent->is_deleted = 1;
	}

	/* process channels */
	for(proc = ctx->rx->proc_first; proc != NULL; proc = proc->proc_next)
	{
		freq = proc->cfg.fc;

		if((freq >= ctx->freq.f_0) && (freq <= ctx->freq.f_1))
		{
			hash = proc->chid % SPECVIEW_SQL_HASHSIZE;
			
			/* lookup existing entry */
			for(ent = ctx->sql.h_firsts[hash]; ent != NULL; ent = ent->h_next)
			{
				if(ent->chid == proc->chid)
					break;
			}

			if((!only_sense) || (ent != NULL))
			{
				/* create new entry for channel */
				if(ent == NULL)
				{
					if( (ent = calloc(1, sizeof(specview_sql_entry_t))) == NULL ) {
						status = 0;
						break;
					}

					ent->hash = hash;
					ent->chid = proc->chid;

					DLIST_INSBACK(ent, ctx->sql.l_first, ctx->sql.l_last, l_next, l_prev);
					DLIST_INSBACK(ent, ctx->sql.h_firsts[hash], ctx->sql.h_lasts[hash], h_next, h_prev);
				}

				/* set entry */
				if(!only_sense)
				{
					ent->pos = scale_mark_pos(ctx->freq.f_0, ctx->freq.f_1, freq, ctx->buf.shm_w, 0);

					ent->thres_op = scale_mark_pos(ctx->mag.m_0, ctx->mag.m_1,
						proc->cfg.sql.op_thres_db, ctx->buf.shm_h, 1);
					ent->thres_cl = scale_mark_pos(ctx->mag.m_0, ctx->mag.m_1,
						proc->cfg.sql.cl_thres_db, ctx->buf.shm_h, 1);

					/* reset deleted flags on found entries */
					ent->is_deleted = 0;
				}

				/* update sense level */
				if(rxproc_read_sql_sense(proc, tick, &sense))
				{
					ent->sense = scale_mark_pos(ctx->mag.m_0, ctx->mag.m_1,
						LINM2DB(sense + DBL_MIN), ctx->buf.shm_h, 1);
					ent->is_open = proc->sql.is_opn;
					ent->draw_sense = 1;
				}
				else
				{
					ent->sense = 0;
					ent->is_open = 0;
					ent->draw_sense = 0;
				}
			}
		}
	}

	ctx->sql.is_inited = status;
	ctx->sql.is_updated = 0;
	ctx->sql.use_full_redraw = use_full_redraw;

	LeaveCriticalSection(&(ctx->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

static void specview_sql_cleanup(specview_ctx_t *ctx)
{
	specview_sql_entry_t *ent, *ent_next;

	for(ent = ctx->sql.l_first; ent != NULL; ent = ent_next)
	{
		ent_next = ent->l_next;
		free(ent);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* erase text and update window directly */
static int specview_erase_text(specview_ctx_t *ctx, HDC hdc_copy, int copy_x, int copy_y)
{
	/* check for resources */
	if((!ctx->buf.is_inited) || (!ctx->text.is_drawn))
		return 0;

	/* restore graphics under text */
	specview_copy_shm_part(ctx, ctx->text.cur_x, ctx->text.cur_y,
		ctx->text.cur_w, ctx->text.cur_h);
	specview_copy_sql_layer_part(ctx, ctx->text.cur_x, ctx->text.cur_y,
		ctx->text.cur_w, ctx->text.cur_h);

	/* mark text erased */
	ctx->text.is_drawn = 0;

	/* copy changes to window directly */
	if((hdc_copy != NULL) && (copy_x >= 0) && (copy_y >= 0))
	{
		BitBlt(hdc_copy,
			copy_x + ctx->buf.shm_x + ctx->text.cur_x,
			copy_y + ctx->buf.shm_y + ctx->text.cur_y,
			ctx->text.cur_w, ctx->text.cur_h,
			ctx->buf.hdc_buf,
			ctx->buf.shm_x + ctx->text.cur_x,
			ctx->buf.shm_y + ctx->text.cur_y,
			SRCCOPY);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* draw text and update window directly */
static int specview_draw_text(specview_ctx_t *ctx, HDC hdc_copy, int copy_x, int copy_y)
{
	/* check for resources */
	if((!ctx->buf.is_inited) || (!ctx->text.is_enabled))
		return 0;

	/* copy text from text layer buffer to main buffer */
	BitBlt(ctx->buf.hdc_buf,
		ctx->buf.shm_x + ctx->text.text_x, ctx->buf.shm_y + ctx->text.text_y,
		ctx->text.text_w, ctx->text.text_h,
		ctx->buf.hdc_layer_text, ctx->text.text_x, ctx->text.text_y, SRCCOPY);

	/* mark text updated */
	ctx->text.is_changed = 0;

	/* save current text rect */
	ctx->text.cur_x = ctx->text.text_x;
	ctx->text.cur_y = ctx->text.text_y;
	ctx->text.cur_w = ctx->text.text_w;
	ctx->text.cur_h = ctx->text.text_h;
	ctx->text.is_drawn = 1;

	/* copy changes to window directly */
	if((hdc_copy != NULL) && (copy_x >= 0) && (copy_y >= 0))
	{
		BitBlt(hdc_copy,
			copy_x + ctx->buf.shm_x + ctx->text.cur_x,
			copy_y + ctx->buf.shm_y + ctx->text.cur_y,
			ctx->text.cur_w, ctx->text.cur_h,
			ctx->buf.hdc_buf,
			ctx->buf.shm_x + ctx->text.cur_x,
			ctx->buf.shm_y + ctx->text.cur_y,
			SRCCOPY);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set or delete text */
int specview_set_text(specview_ctx_t *ctx, TCHAR *text, int x, int y, int xm, int ym, int w, int h)
{
	HGDIOBJ hfnt_prev, hbr_prev;
	RECT rt;
	int res;

	/* delete and disable text */
	if(text == NULL)
	{
		if(!ctx->text.is_enabled)
			return -1;
		EnterCriticalSection(&(ctx->csec));
		ctx->text.is_enabled = 0;
		LeaveCriticalSection(&(ctx->csec));
		return 1;
	}

	/* check for resources */
	if((!ctx->buf.is_inited) || (!ctx->res.is_inited))
		return 0;

	/* translate window corrdinates to schematic coordinates */
	x -= ctx->buf.shm_x;
	y -= ctx->buf.shm_y;

	/* enforce coordinates to schematic rectangle */
	if(x < 0)
		x = 0;
	if(x >= ctx->buf.shm_w)
		x = ctx->buf.shm_w - 1;
	if(y < 0)
		y = 0;
	if(y >= ctx->buf.shm_h)
		y = ctx->buf.shm_h - 1;

	/* is update actually needed? */
	if( ctx->text.is_enabled &&
		(x == ctx->text.init_x) && (y == ctx->text.init_y) &&
		(_tcsncmp(text, ctx->text.buf, SPECVIEW_TEXT_MAXLEN) == 0) )
	{
		return -1;
	}

	EnterCriticalSection(&(ctx->csec));

	/* mark text disabled */
	ctx->text.is_enabled = 0;

	/* save new text */
	_tcsncpy(ctx->text.buf, text, SPECVIEW_TEXT_MAXLEN);
	ctx->text.buf[SPECVIEW_TEXT_MAXLEN - 1] = 0;

	/* save new text coordinates */
	ctx->text.init_x = x;
	ctx->text.init_y = y;

	/* calculate new text rectangle */
	rt.left = 0;
	rt.top = 0;
	rt.right = w;
	rt.bottom = h;

	hfnt_prev = SelectObject(ctx->buf.hdc_layer_text, ctx->hfont);
	res = DrawText(ctx->buf.hdc_layer_text, text, (int)_tcslen(text), &rt, DT_CALCRECT);
	SelectObject(ctx->buf.hdc_layer_text, hfnt_prev);

	/* check for error */
	if(res <= 0)
	{
		LeaveCriticalSection(&(ctx->csec));
		return 0;
	}

	/* calculate actual output coordinates */
	ctx->text.text_w = rt.right - rt.left;
	ctx->text.text_h = rt.bottom - rt.top;

	if(x + ctx->text.text_w + xm <= ctx->buf.shm_w)
	{
		ctx->text.text_x = x + xm;
	}
	else
	{
		ctx->text.text_x = x - ctx->text.text_w - xm;
		if(ctx->text.text_x < 0)
		{
			LeaveCriticalSection(&(ctx->csec));
			return 0;
		}
	}

	if(y + ctx->text.text_h + ym <= ctx->buf.shm_h)
	{
		ctx->text.text_y = y + ym;
	}
	else
	{
		ctx->text.text_y = y - ctx->text.text_h - ym;
		if(ctx->text.text_y < 0)
		{
			LeaveCriticalSection(&(ctx->csec));
			return 0;
		}
	}

	/* erase text background */
	hbr_prev = SelectObject(ctx->buf.hdc_layer_text, ctx->res.hbr_textbg);
	PatBlt(ctx->buf.hdc_layer_text,
		ctx->text.text_x, ctx->text.text_y,
		ctx->text.text_w, ctx->text.text_h, PATCOPY);
	SelectObject(ctx->buf.hdc_layer_text, hbr_prev);

	/* draw text */
	SetBkColor(ctx->buf.hdc_layer_text, ctx->cfg.cr_textbg);
	SetTextColor(ctx->buf.hdc_layer_text, ctx->cfg.cr_text);

	rt.left = ctx->text.text_x;
	rt.top = ctx->text.text_y;
	rt.right = ctx->text.text_x + ctx->text.text_w;
	rt.bottom = ctx->text.text_y + ctx->text.text_h;

	hfnt_prev = SelectObject(ctx->buf.hdc_layer_text, ctx->hfont);
	res = DrawText(ctx->buf.hdc_layer_text, text, (int)_tcslen(text), &rt, 0);
	SelectObject(ctx->buf.hdc_layer_text, hfnt_prev);

	/* check for error */
	if(res <= 0)
	{
		LeaveCriticalSection(&(ctx->csec));
		return 0;
	}

	/* enable text and mark for update */
	ctx->text.is_enabled = 1;
	ctx->text.is_changed = 1;

	LeaveCriticalSection(&(ctx->csec));

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* update text and copy changes directly to window */
int specview_update_text(specview_ctx_t *ctx, HDC hdc_copy, int copy_x, int copy_y)
{
	int is_updated = 0;

	EnterCriticalSection(&(ctx->csec));

	/* erase current text */
	if(ctx->text.is_drawn && ((!ctx->text.is_enabled) || ctx->text.is_changed))
	{
		if(!specview_erase_text(ctx, hdc_copy, copy_x, copy_y))
		{
			LeaveCriticalSection(&(ctx->csec));
			return 0;
		}
		is_updated = 1;
	}

	/* or draw new text */
	if(ctx->text.is_enabled && ((!ctx->text.is_drawn) || ctx->text.is_changed))
	{
		if(!specview_draw_text(ctx, hdc_copy, copy_x, copy_y))
		{
			LeaveCriticalSection(&(ctx->csec));
			return 0;
		}
		is_updated = 1;
	}

	LeaveCriticalSection(&(ctx->csec));

	return (is_updated ? 1 : -1);
}

/* ---------------------------------------------------------------------------------------------- */

/* update everything */
int specview_update(specview_ctx_t *ctx)
{
	int error = 0, is_updated = 0, res;

	EnterCriticalSection(&(ctx->csec));

	/* update schematic source images */
	if(!ctx->buf.is_shm_src_updated)
	{
		if(!specview_update_shm_src(ctx))
			error = 1;
		is_updated = 1;
	}

	/* update sql marks layer */
	if(!ctx->sql.is_updated)
	{
		if(!specview_update_sql(ctx))
			error = 1;
		is_updated = 1;
	}

	/* draw background and scales */
	if((!ctx->mag.is_drawn) || (!ctx->freq.is_drawn))
	{
		if(!specview_draw_window_bkgnd(ctx))
			error = 1;
		if(!specview_draw_mag_scale(ctx))
			error = 1;
		if(!specview_draw_freq_scale(ctx))
			error = 1;
		is_updated = 1;
	}

	/* copy schematic */
	if((!ctx->buf.is_shm_copied) || (!ctx->sql.is_copied))
	{
		if(!specview_copy_shm(ctx))
			error = 1;
		is_updated = 1;
	}

	/* copy sql mark layer */
	if(!ctx->sql.is_copied)
	{
		if(!specview_copy_sql_layer(ctx))
			error = 1;
		is_updated = 1;
	}

	/* draw or erase text */
	res = specview_update_text(ctx, NULL, 0, 0);
	if(res == 0)
		error = 1;
	if(res >= 0)
		is_updated = 1;

	LeaveCriticalSection(&(ctx->csec));

	return (error ? 0 : (is_updated ? 1 : -1));
}

/* ---------------------------------------------------------------------------------------------- */

static int specview_freq2pos(specview_ctx_t *ctx, double freq, int *posp)
{
	double dpos;

	if((!ctx->freq.is_inited) || (!ctx->buf.is_inited))
		return 0;

	if( (freq >= (ctx->freq.f_0 - 1e-3)) &&
		(freq <= (ctx->freq.f_1 + 1e-3)) )
	{
		if(freq < ctx->freq.f_0)
			freq = ctx->freq.f_0;
		if(freq > ctx->freq.f_1)
			freq = ctx->freq.f_1;

		dpos = (double)(ctx->buf.shm_w - 1) / (ctx->freq.f_1 - ctx->freq.f_0);
		*posp = (int)((freq - ctx->freq.f_0) * dpos + 0.5);

		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* set new channel map (reads channels from receiver state) */
int specview_set_chmap(specview_ctx_t *ctx)
{
	rxproc_t *proc;
	double f_start, f_end, dj;
	double f_start_clip, f_end_clip;
	int j_start, j_end, j;

	/* check required resources */
	if((!ctx->freq.is_inited) || (!ctx->buf.is_inited))
		return 0;

	EnterCriticalSection(&(ctx->csec));

	/* erase channel map */
	memset(ctx->buf.chmap_buf, 0, ctx->buf.shm_w * sizeof(specview_chmap_point_t));
	memset(ctx->buf.shm_cr_map, SPECVIEW_COLOR_BKGND, ctx->buf.shm_w);

	/* calculate pixel coordinate delta per hz */
	dj = (double)(ctx->buf.shm_w - 1) / (ctx->freq.f_1 - ctx->freq.f_0);

	/* mark channels bandwidth */
	for(proc = ctx->rx->proc_first; proc != NULL; proc = proc->proc_next)
	{
		/* start and end of channel */
		f_start = proc->cfg.fc - proc->cfg.filter_fc;
		f_end = proc->cfg.fc + proc->cfg.filter_fc;

		if((f_start <= ctx->freq.f_1) && (f_end >= ctx->freq.f_0))
		{
			f_start_clip = (f_start >= ctx->freq.f_0) ? f_start : ctx->freq.f_0;
			f_end_clip = (f_end <= ctx->freq.f_1) ? f_end : ctx->freq.f_1;

			/* mark lower bandwidth edge at channel map */
			if(f_start >= (ctx->freq.f_0 - 1e-3))
			{
				j = (int)((f_start_clip - ctx->freq.f_0) * dj + 0.5);
				ctx->buf.chmap_buf[j].type = SPECVIEW_MAP_CHAN_BW_EDGE_LOWER;
				ctx->buf.chmap_buf[j].chid = proc->chid;
			}

			/* mark upper bandwidth edge at channel map */
			if(f_end <= (ctx->freq.f_1 + 1e-3))
			{
				j = (int)((f_end_clip - ctx->freq.f_0) * dj + 0.5);
				ctx->buf.chmap_buf[j].type = SPECVIEW_MAP_CHAN_BW_EDGE_UPPER;
				ctx->buf.chmap_buf[j].chid = proc->chid;
			}

			/* mark channel bandwidth at schematic color map */
			if((proc->cfg.filter_fc + 1e-3) * dj >= 3.0)
			{
				j_start = (int)((f_start_clip - ctx->freq.f_0) * dj + 0.5);
				j_end = (int)((f_end_clip - ctx->freq.f_0) * dj + 0.5);

				for(j = j_start; j <= j_end; j++)
				{
					if(ctx->buf.shm_cr_map[j] == SPECVIEW_COLOR_BKGND) {
						ctx->buf.shm_cr_map[j] = SPECVIEW_COLOR_CHAN_BW;
					} else if(ctx->buf.shm_cr_map[j] == SPECVIEW_COLOR_CHAN_BW) {
						ctx->buf.shm_cr_map[j] = SPECVIEW_COLOR_CHAN_BWOVL;
					}
				}
			}
		}
	}

	/* mark channel centers */
	for(proc = ctx->rx->proc_first; proc != NULL; proc = proc->proc_next)
	{
		if(specview_freq2pos(ctx, proc->cfg.fc, &j))
		{
			/* mark channel center at channel map */
			ctx->buf.chmap_buf[j].type = SPECVIEW_MAP_CHAN_FC;
			ctx->buf.chmap_buf[j].chid = proc->chid;

			/* mark channel center at schematic color map */
			ctx->buf.shm_cr_map[j] = SPECVIEW_COLOR_CHAN_FC;
		}
	}

	/* check input center in viewer range with margin */
	if( ctx->rx->is_started &&
		specview_freq2pos(ctx, ctx->rx->fc_input, &j) )
	{
		/* mark input center at channel map */
		ctx->buf.chmap_buf[j].type = SPECVIEW_MAP_INPUT_FC;

		/* mark input center at schematic color map */
		ctx->buf.shm_cr_map[j] = SPECVIEW_COLOR_INPUT_FC;
	}

	/* mark channel map updated */
	ctx->buf.is_chmap_inited = 1;

	/* is schematic source images update required? */
	if( ctx->buf.is_shm_src_updated &&
		(memcmp(ctx->buf.shm_cr_map, ctx->buf.shm_cr_map_actual, ctx->buf.shm_w) == 0) )
	{
		LeaveCriticalSection(&(ctx->csec));
		return -1;
	}

	/* mark schematic source images for update */
	ctx->buf.is_shm_src_updated = 0;

	LeaveCriticalSection(&(ctx->csec));
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* reset refresh rate counter */
static void specview_ups_reset(specview_ctx_t *ctx)
{
	memset(ctx->ups_buf, 0, SPECVIEW_UPS_BUFSIZE * sizeof(DWORD));
	ctx->ups_ptr = 0;
	ctx->ups = 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* update refresh rate counter */
static void specview_ups_count(specview_ctx_t *ctx)
{
	int n, ptr, count;
	DWORD tick;
	double delta;

	tick = GetTickCount();

	count = 0;
	delta = 0;

	for(n = 0; n < SPECVIEW_UPS_BUFSIZE; n++)
	{
		ptr = ctx->ups_ptr - n - 1;
		if(ptr < 0)
			ptr += SPECVIEW_UPS_BUFSIZE;

		if(ctx->ups_buf[ptr] == 0)
			break;

		delta = (double)(tick - ctx->ups_buf[ptr]) / 1000.0;
		count++;

		if(delta > SPECVIEW_UPS_MAXDELTA)
			break;
	}

	if( (count == 0) || (delta == 0) ) {
		ctx->ups = 0;
	} else {
		ctx->ups = count / delta;
	}

	ctx->ups_buf[ctx->ups_ptr++] = tick;
	if(ctx->ups_ptr >= SPECVIEW_UPS_BUFSIZE)
		ctx->ups_ptr = 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* set new spectrum mask */
int specview_set_mask(specview_ctx_t *ctx, double fc_input, unsigned int fs_input,
					  float *fft_pwr_buf, size_t fft_pwr_buf_length)
{
	HGDIOBJ hpen_prev;
	int x, y;
	double df_in, f_in_0, f_in_1;

	if((fs_input == 0) || (fft_pwr_buf_length == 0))
		return 0;

	/* check required resources initialized */
	if( (!ctx->buf.is_inited) || (!ctx->freq.is_inited) || (!ctx->mag.is_inited) )
		return 0;

	EnterCriticalSection(&(ctx->csec));

	/* count update rate */
	specview_ups_count(ctx);

	/* horizontal rescale spectrum buffer */
	df_in = (double)fs_input / (double)fft_pwr_buf_length;
	f_in_0 = fc_input - df_in * (double)(fft_pwr_buf_length / 2 - 1);
	f_in_1 = fc_input + df_in * (double)(fft_pwr_buf_length / 2);

	scale_pwr_spect(
		ctx->buf.spec_db_buf, ctx->buf.shm_w, ctx->freq.f_0, ctx->freq.f_1,
		fft_pwr_buf, (int)fft_pwr_buf_length, f_in_0, f_in_1, ctx->cfg.scale_mode);

	/* vertical rescale spectrum buffer */
	scale_mag(
		ctx->buf.spec_j_buf, ctx->mag.height, ctx->mag.m_0, ctx->mag.m_1,
		ctx->buf.spec_db_buf, ctx->buf.shm_w);

	/* erase mask background */
	PatBlt(ctx->buf.hdc_shm_mask, 0, 0, ctx->buf.shm_w, ctx->buf.shm_h, BLACKNESS);

	/* redraw mask */
	hpen_prev = SelectObject(ctx->buf.hdc_shm_mask, ctx->res.hpen_stock_white);

	for(x = 0; x < ctx->buf.shm_w; x++)
	{
		if(ctx->buf.spec_j_buf[x] != -1)
		{
			y = (ctx->buf.shm_h - 1) - ctx->buf.spec_j_buf[x];

			MoveToEx(ctx->buf.hdc_shm_mask, x, y, NULL);
			LineTo(ctx->buf.hdc_shm_mask, x, ctx->buf.shm_h);
		}
	}

	SelectObject(ctx->buf.hdc_shm_mask, hpen_prev);

	/* mark schametic for copy */
	ctx->buf.is_shm_copied = 0;

	LeaveCriticalSection(&(ctx->csec));
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* clear spectrum mask */
int specview_clear_mask(specview_ctx_t *ctx)
{
	/* check buffer initialized */
	if(!ctx->buf.is_inited)
		return 0;

	EnterCriticalSection(&(ctx->csec));

	/* reset update counter */
	specview_ups_reset(ctx);

	/* erase mask */
	PatBlt(ctx->buf.hdc_shm_mask, 0, 0, ctx->buf.shm_w, ctx->buf.shm_h, BLACKNESS);

	/* mark schametic for copy */
	ctx->buf.is_shm_copied = 0;

	LeaveCriticalSection(&(ctx->csec));
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set new frequency range (get end frequencies from config) */
int specview_set_freq_range(specview_ctx_t *ctx)
{
	/* check resources and config */
	if( (!ctx->buf.is_inited) || (ctx->f_1 <= ctx->f_0) ||
		(ctx->f_0 < RXLIM_FC_MIN) || (ctx->f_1 > RXLIM_FC_MAX) )
	{
		return 0;
	}

	/* check update actually needed */
	if( ctx->freq.is_inited &&
		(ctx->freq.width == ctx->buf.shm_w) &&
		(fabs(ctx->freq.f_0 - ctx->f_0) < 1e-3) &&
		(fabs(ctx->freq.f_1 - ctx->f_1) < 1e-3) )
	{
		return -1;
	}

	EnterCriticalSection(&(ctx->csec));

	/* invalidate frequency scale */
	ctx->freq.is_inited = 0;
	ctx->freq.is_drawn = 0;

	/* initialize frequency scale */
	ctx->freq.f_0 = ctx->f_0;
	ctx->freq.f_1 = ctx->f_1;
	ctx->freq.width = ctx->buf.shm_w;

	/* select frequency label step */
	if(!scale_step_freq(ctx->f_0, ctx->f_1,
		ctx->freq.width, SPECVIEW_FREQ_LABEL_MIN_W, &(ctx->freq.f_step)))
	{
		LeaveCriticalSection(&(ctx->csec));
		return 0;
	}

	/* choose scale mode */
	if((ctx->freq.f_step + 1e-3) < 1e3)
	{
		/* fine scale: Hz, coarse scale: kHz */
		ctx->freq.f_coarse_step = 1e3;
		ctx->freq.f_scale_mode = SPECVIEW_FREQSCALE_HZ_KHZ;
	}
	else if((ctx->freq.f_step + 1e-3) < 1e6)
	{
		/* fine scale: kHz, coarse scale: MHz */
		ctx->freq.f_coarse_step = 1e6;
		ctx->freq.f_scale_mode = SPECVIEW_FREQSCALE_KHZ_MHZ;
	}
	else
	{
		/* fine scale: MHz, coarse scale: GHz */
		ctx->freq.f_coarse_step = 1e9;
		ctx->freq.f_scale_mode = SPECVIEW_FREQSCALE_MHZ_GHZ;
	}

	/* validate frequency scale */
	ctx->freq.is_inited = 1;

	/* mark schematic source images for full redraw */
	ctx->buf.is_shm_src_prepared = 0;
	ctx->buf.is_shm_src_updated = 0;

	specview_set_chmap(ctx);
	specview_set_sql(ctx, 1, 0);

	LeaveCriticalSection(&(ctx->csec));
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* set new magnitude range (get end magnitudes from config) */
int specview_set_mag_range(specview_ctx_t *ctx)
{
	/* check required resources and config */
	if( (!ctx->buf.is_inited) || (ctx->cfg.m_1 <= ctx->cfg.m_0) ||
		(ctx->cfg.m_0 < SPECVIEW_MAGN_MINABS) || (ctx->cfg.m_0 > SPECVIEW_MAGN_MAXABS) ||
		(ctx->cfg.m_1 < SPECVIEW_MAGN_MINABS) || (ctx->cfg.m_1 > SPECVIEW_MAGN_MAXABS) )
	{
		return 0;
	}

	/* check update actually needed */
	if( ctx->mag.is_inited &&
		(ctx->mag.height == ctx->buf.shm_h) &&
		(fabs(ctx->mag.m_0 - ctx->cfg.m_0) < 1e-6) &&
		(fabs(ctx->mag.m_1 - ctx->cfg.m_1) < 1e-6) )
	{
		return -1;
	}

	EnterCriticalSection(&(ctx->csec));

	/* invalidate magnitude scale */
	ctx->mag.is_inited = 0;
	ctx->mag.is_drawn = 0;

	/* initialize magnitude scale */
	ctx->mag.m_0 = ctx->cfg.m_0;
	ctx->mag.m_1 = ctx->cfg.m_1;
	ctx->mag.height = ctx->buf.shm_h;

	/* calculate magnitude scale label step */
	if(!scale_step_mag(ctx->cfg.m_0, ctx->cfg.m_1,
		ctx->mag.height, SPECVIEW_MAG_LABEL_MIN_H, &(ctx->mag.m_step)))
	{
		LeaveCriticalSection(&(ctx->csec));
		return 0;
	}

	/* validate magnitude scale */
	ctx->mag.is_inited = 1;

	/* mark schematic source images for full redraw */
	ctx->buf.is_shm_src_prepared = 0;
	ctx->buf.is_shm_src_updated = 0;

	specview_set_sql(ctx, 1, 0);

	LeaveCriticalSection(&(ctx->csec));
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* imprint refresh rate to main image buffer */
static int specview_ups_imprint(specview_ctx_t *ctx)
{
	HFONT hfnt_prev;
	TCHAR buf[64];
	RECT rt;

	/* check required resources and counter state */
	if( (!ctx->cfg.show_ups_counter) || (!ctx->buf.is_inited) ||
		(!ctx->res.is_inited) || (ctx->ups == 0) )
	{
		return 0;
	}

	/* draw update rate counter */
	_stprintf(buf, _T("UPS %4.1f"), ctx->ups);

	hfnt_prev = SelectObject(ctx->buf.hdc_buf, ctx->hfont);
	SetBkColor(ctx->buf.hdc_buf, RGB(0, 0, 0));
	SetTextColor(ctx->buf.hdc_buf, RGB(255, 255, 255));

	rt.right = ctx->buf.shm_x + ctx->buf.shm_w;
	rt.left = rt.right - 60;
	rt.top = ctx->buf.shm_y;
	rt.bottom = rt.top + 15;

	DrawText(ctx->buf.hdc_buf, buf, (int)_tcslen(buf), &rt,
		DT_RIGHT|DT_TOP|DT_SINGLELINE);

	SelectObject(ctx->buf.hdc_buf, hfnt_prev);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* copy main buffer to window */
int specview_copy(specview_ctx_t *ctx, HDC hdc, int x, int y)
{
	if( (!ctx->buf.is_inited) || (x < 0) || (y < 0) )
		return 0;

	EnterCriticalSection(&(ctx->csec));

	/* draw update rate counter to main buffer  */
	specview_ups_imprint(ctx);

	/* copy main buffer to window */
	BitBlt(hdc, x, y, ctx->buf.win_w, ctx->buf.win_h,
		ctx->buf.hdc_buf, 0, 0, SRCCOPY);

	LeaveCriticalSection(&(ctx->csec));
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* map pixel coordinates to element type and channel id */
specview_point_type_t specview_chmap_point(specview_ctx_t *ctx, int x, int y, unsigned int *pchid)
{
	int jx, jy;
	specview_point_type_t type;
	specview_sql_entry_t *sql_ent;
	int sql_left, sql_right;
	int check_op, check_cl;

	/* check config and parameters */
	if( (!ctx->buf.is_inited) || (!ctx->sql.is_inited) ||
		(x < 0) || (x >= ctx->buf.win_w) ||
		(y < 0) || (y >= ctx->buf.win_h) )
	{
		return SPECVIEW_POINT_OUTOFRANGE;
	}

	/* is coordinates within schematic rectangle? */
	if( (x >= ctx->buf.shm_x) && (x < ctx->buf.shm_x + ctx->buf.shm_w) &&
		(y >= ctx->buf.shm_y) && (y < ctx->buf.shm_y + ctx->buf.shm_h) )
	{
		/* translate coordinate to schematic rectangle */
		jx = x - ctx->buf.shm_x;
		jy = y - ctx->buf.shm_y;

		/* check sql map */
		for(sql_ent = ctx->sql.l_first; sql_ent != NULL; sql_ent = sql_ent->l_next)
		{
			sql_left = sql_ent->pos - SPECVIEW_SQL_MARK_WIDTH / 2;
			sql_right = sql_ent->pos + SPECVIEW_SQL_MARK_WIDTH / 2;

			if((jx >= sql_left) && (jx <= sql_right))
			{
				check_op = (sql_ent->thres_op >= jy - 2) && (sql_ent->thres_op <= jy + 2);
				check_cl = (sql_ent->thres_cl >= jy - 2) && (sql_ent->thres_cl <= jy + 2);

				if(check_op)
				{
					*pchid = sql_ent->chid;
					if(check_cl && (jx >= sql_ent->pos))
						return SPECVIEW_POINT_CHAN_SQL_CLOSE_THRES;
					return SPECVIEW_POINT_CHAN_SQL_OPEN_THRES;
				}

				if(check_cl)
				{
					*pchid = sql_ent->chid;
					return SPECVIEW_POINT_CHAN_SQL_CLOSE_THRES;
				}
			}
		}

		/* channel map aiming helper */
		if(ctx->buf.chmap_buf[jx].type == SPECVIEW_MAP_NOTHING)
		{
			if( (jx - 1 >= 0) &&
				(ctx->buf.chmap_buf[jx - 1].type != SPECVIEW_MAP_NOTHING) )
			{
				jx--;
			}
			else if( (jx + 1 < ctx->buf.shm_w) && 
				(ctx->buf.chmap_buf[jx + 1].type != SPECVIEW_MAP_NOTHING) )
			{
				jx++;
			}
		}

		/* get element type from channel map */
		switch(ctx->buf.chmap_buf[jx].type)
		{
		case SPECVIEW_MAP_INPUT_FC:
			type = SPECVIEW_POINT_INPUT_FC;
			break;
		case SPECVIEW_MAP_CHAN_FC:
			type = SPECVIEW_POINT_CHAN_FC;
			break;
		case SPECVIEW_MAP_CHAN_BW_EDGE_LOWER:
			type = SPECVIEW_POINT_CHAN_BW_EDGE_LOWER;
			break;
		case SPECVIEW_MAP_CHAN_BW_EDGE_UPPER:
			type = SPECVIEW_POINT_CHAN_BW_EDGE_UPPER;
			break;
		default:
			type = SPECVIEW_POINT_SHM;
			break;
		}

		/* get channel id */
		*pchid = ctx->buf.chmap_buf[jx].chid;

		return type;
	}

	return SPECVIEW_POINT_FRAME;
}

/* ---------------------------------------------------------------------------------------------- */

/* map pixel distance to frequency offset */
int specview_map_dist_x(specview_ctx_t *ctx, int dx, double *out_df)
{
	if(!ctx->freq.is_inited)
		return 0;

	*out_df = dx * (ctx->freq.f_1 - ctx->freq.f_0) / (ctx->freq.width - 1);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* map pixel distance to magnitude offset */
int specview_map_dist_y(specview_ctx_t *ctx, int dy, double *out_dm)
{
	if(!ctx->mag.is_inited)
		return 0;

	*out_dm = -dy * (ctx->mag.m_1 - ctx->mag.m_0) / (ctx->mag.height - 1);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* main buffer cleanup */
static void specview_buf_free(specview_buf_ctx_t *buf)
{
	DeleteDC(buf->hdc_buf);
	DeleteDC(buf->hdc_shm_src_cold);
	DeleteDC(buf->hdc_shm_src_hot);
	DeleteDC(buf->hdc_shm_mask);
	DeleteDC(buf->hdc_layer_text);
	DeleteDC(buf->hdc_layer_sq);
	DeleteDC(buf->hdc_mask_sq);

	DeleteObject(buf->hbm_buf);
	DeleteObject(buf->hbm_shm_src_cold);
	DeleteObject(buf->hbm_shm_src_hot);
	DeleteObject(buf->hbm_shm_mask);
	DeleteObject(buf->hbm_layer_text);
	DeleteObject(buf->hbm_layer_sq);
	DeleteObject(buf->hbm_mask_sq);

	free(buf->chmap_buf);
	free(buf->shm_cr_map);
	free(buf->shm_cr_map_actual);

	free(buf->spec_db_buf);
	free(buf->spec_j_buf);
}

/* ---------------------------------------------------------------------------------------------- */

/* main buffer (re)init */
static int specview_buf_init(specview_buf_ctx_t *buf, HDC hdc, int win_w, int win_h)
{
	/* check for buffer already initialized */
	if(buf->is_inited)
	{
		/* check for reinit actually needed */
		if((win_w == buf->win_w) && (win_h == buf->win_h))
			return -1;

		/* free current buffer data */
		specview_buf_free(buf);
		buf->is_inited = 0;
	}

	/* initialize buffer dimensions */
	buf->win_w = win_w;
	buf->win_h = win_h;

	buf->shm_x = SPECVIEW_SHM_OFS_LEFT;
	buf->shm_y = SPECVIEW_SHM_OFS_TOP;
	buf->shm_w = (win_w - SPECVIEW_FRAME_W);
	buf->shm_h = (win_h - SPECVIEW_FRAME_H);

	/* allocate resources and memory */
	buf->hbm_buf = CreateCompatibleBitmap(hdc, buf->win_w, buf->win_h);
	buf->hbm_shm_src_cold = CreateCompatibleBitmap(hdc, buf->shm_w, buf->shm_h);
	buf->hbm_shm_src_hot = CreateCompatibleBitmap(hdc, buf->shm_w, buf->shm_h);
	buf->hbm_shm_mask = CreateBitmap(buf->shm_w, buf->shm_h, 1, 1, NULL);
	buf->hbm_layer_text = CreateCompatibleBitmap(hdc, buf->shm_w, buf->shm_h);
	buf->hbm_layer_sq = CreateCompatibleBitmap(hdc, buf->shm_w, buf->shm_h);
	buf->hbm_mask_sq = CreateBitmap(buf->shm_w, buf->shm_h, 1, 1, NULL);

	buf->hdc_buf = CreateCompatibleDC(hdc);
	buf->hdc_shm_src_cold = CreateCompatibleDC(hdc);
	buf->hdc_shm_src_hot = CreateCompatibleDC(hdc);
	buf->hdc_shm_mask = CreateCompatibleDC(hdc);
	buf->hdc_layer_text = CreateCompatibleDC(hdc);
	buf->hdc_layer_sq = CreateCompatibleDC(hdc);
	buf->hdc_mask_sq = CreateCompatibleDC(hdc);

	buf->chmap_buf = malloc(buf->shm_w * sizeof(specview_chmap_point_t));
	buf->shm_cr_map = malloc(buf->shm_w);
	buf->shm_cr_map_actual = malloc(buf->shm_w);

	buf->spec_db_buf = malloc(buf->shm_w * sizeof(float));
	buf->spec_j_buf = malloc(buf->shm_w * sizeof(int));

	/* check for allocation errors */
	if( (buf->hbm_buf != NULL) && (buf->hdc_buf != NULL) &&
		(buf->hbm_shm_src_cold != NULL) && (buf->hdc_shm_src_cold != NULL) && 
		(buf->hbm_shm_src_hot != NULL) && (buf->hdc_shm_src_hot != NULL) &&
		(buf->hbm_layer_text != NULL) && (buf->hdc_layer_text != NULL) &&
		(buf->hbm_layer_sq != NULL) && (buf->hdc_layer_sq != NULL) &&
		(buf->hbm_mask_sq != NULL) && (buf->hdc_mask_sq != NULL) &&
		(buf->shm_cr_map != NULL) && (buf->shm_cr_map_actual != NULL) &&
		(buf->spec_db_buf != NULL) && (buf->spec_j_buf != NULL) )
	{
		/* select bitmaps to memory dcs */
		if( (SelectObject(buf->hdc_buf, buf->hbm_buf) != NULL) &&
			(SelectObject(buf->hdc_shm_src_cold, buf->hbm_shm_src_cold) != NULL) &&
			(SelectObject(buf->hdc_shm_src_hot, buf->hbm_shm_src_hot) != NULL) &&
			(SelectObject(buf->hdc_shm_mask, buf->hbm_shm_mask) != NULL) && 
			(SelectObject(buf->hdc_layer_text, buf->hbm_layer_text) != NULL) &&
			(SelectObject(buf->hdc_layer_sq, buf->hbm_layer_sq) != NULL) &&
			(SelectObject(buf->hdc_mask_sq, buf->hbm_mask_sq) != NULL) )
		{
			/* mark buffer inited */
			buf->is_inited = 1;

			/* invalidate buffer contents */
			buf->is_chmap_inited = 0;
			buf->is_shm_src_prepared = 0;
			buf->is_shm_src_updated = 0;
			buf->is_shm_copied = 0;

			return 1;
		}
	}

	/* cleanup */
	specview_buf_free(buf);

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* set new spectrum viewer size */
int specview_set_size(specview_ctx_t *ctx, HDC hdc, int win_w, int win_h)
{
	int status;

	if( (win_w < SPECVIEW_MIN_W) || (win_w > SPECVIEW_MAX_W) ||
		(win_h < SPECVIEW_MIN_H) || (win_h > SPECVIEW_MAX_H) )
	{
		return 0;
	}

	if(ctx->buf.is_inited && (win_w == ctx->buf.win_w) && (win_h == ctx->buf.win_h))
		return -1;

	EnterCriticalSection(&(ctx->csec));

	/* reinitialize buffer size */
	status = specview_buf_init(&(ctx->buf), hdc, win_w, win_h);

	if(status <= 0)
	{
		LeaveCriticalSection(&(ctx->csec));
		return status;
	}

	/* invalidate data dependent of window size */
	ctx->mag.is_inited = 0;
	ctx->freq.is_inited = 0;
	ctx->sql.is_inited = 0;

	/* invalidate text buffer */
	ctx->text.is_enabled = 0;
	ctx->text.is_drawn = 0;
	ctx->sql.is_copied = 0;

	/* update data dependent of window size */
	specview_set_mag_range(ctx);
	specview_set_freq_range(ctx);
	specview_set_chmap(ctx);
	specview_set_sql(ctx, 1, 0);

	LeaveCriticalSection(&(ctx->csec));

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize spectrum viewer configuration */
static void specview_cfg_reset(specview_cfg_t *cfg)
{
	cfg->m_0 = -20.0;
	cfg->m_1 = 40.0;

	cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND]				= RGB(48, 48, 48);
	cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC]			= RGB(0, 0, 128);
	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC]			= RGB(128, 0, 0);
	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW]			= RGB(192, 192, 192);
	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL]			= RGB(192, 192, 192);

	cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND_GRID]			= RGB(0, 0, 0);
	cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC_GRID]		= RGB(0, 0, 128);
	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC_GRID]		= RGB(128, 0, 0);
	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW_GRID]		= RGB(128, 128, 128);
	cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL_GRID]	= RGB(128, 128, 128);

	cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND]				= RGB(192, 192, 192);
	cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC]			= RGB(0, 0, 255);
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC]				= RGB(255, 0, 0);
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW]				= RGB(255, 255, 255);
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL]			= RGB(255, 255, 255);

	cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND_GRID]			= RGB(128, 128, 128);
	cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC_GRID]		= RGB(0, 0, 255);
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC_GRID]		= RGB(255, 0, 0);
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW_GRID]		= RGB(192, 192, 192);
	cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL_GRID]		= RGB(192, 192, 192);

	cfg->cr_ticks = RGB(0, 0, 0);
	cfg->cr_label = RGB(0, 0, 0);
	cfg->cr_text = RGB(255, 255, 0);
	cfg->cr_textbg = RGB(0, 0, 128);
	
	cfg->cr_sq_sense_op = RGB(255, 255, 255);
	cfg->cr_sq_sense_cl = RGB(64, 64, 64);
	cfg->cr_sq_thres_op = RGB(0, 255, 0);
	cfg->cr_sq_thres_cl = RGB(255, 0, 0);

	cfg->scale_mode = SPECSCALE_MAP_PEAK;

	cfg->show_ups_counter = 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* load spectrum viewer configuration */
static void specview_cfg_load(specview_cfg_t *cfg, ini_sect_t *sect)
{
	double m_0, m_1;

	if(sect != NULL)
	{
		m_0 = cfg->m_0;
		m_1 = cfg->m_1;

		if( ini_getfr(sect, _T("m_0"), &m_0, SPECVIEW_MAGN_MINABS, SPECVIEW_MAGN_MAXABS) &&
			ini_getfr(sect, _T("m_1"), &m_1, SPECVIEW_MAGN_MINABS, SPECVIEW_MAGN_MAXABS) &&
			(m_1 > m_0) )
		{
			cfg->m_0 = m_0;
			cfg->m_1 = m_1;
		}

		ini_getcr(sect, _T("cr_bkgnd_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND]));
		ini_getcr(sect, _T("cr_input_fc_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC]));
		ini_getcr(sect, _T("cr_chan_fc_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC]));
		ini_getcr(sect, _T("cr_chan_bw_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW]));
		ini_getcr(sect, _T("cr_chan_bwovl_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL]));

		ini_getcr(sect, _T("cr_bkgnd_grid_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND_GRID]));
		ini_getcr(sect, _T("cr_input_fc_grid_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC_GRID]));
		ini_getcr(sect, _T("cr_chan_fc_grid_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC_GRID]));
		ini_getcr(sect, _T("cr_chan_bw_grid_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW_GRID]));
		ini_getcr(sect, _T("cr_chan_bwovl_grid_0"), &(cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL_GRID]));

		ini_getcr(sect, _T("cr_bkgnd_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND]));
		ini_getcr(sect, _T("cr_input_fc_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC]));
		ini_getcr(sect, _T("cr_chan_fc_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC]));
		ini_getcr(sect, _T("cr_chan_bw_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW]));
		ini_getcr(sect, _T("cr_chan_bwovl_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL]));

		ini_getcr(sect, _T("cr_bkgnd_grid_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND_GRID]));
		ini_getcr(sect, _T("cr_input_fc_grid_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC_GRID]));
		ini_getcr(sect, _T("cr_chan_fc_grid_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC_GRID]));
		ini_getcr(sect, _T("cr_chan_bw_grid_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW_GRID]));
		ini_getcr(sect, _T("cr_chan_bwovl_grid_1"), &(cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL_GRID]));

		ini_getcr(sect, _T("cr_ticks"), &(cfg->cr_ticks));
		ini_getcr(sect, _T("cr_label"), &(cfg->cr_label));
		ini_getcr(sect, _T("cr_text"), &(cfg->cr_text));
		ini_getcr(sect, _T("cr_textbg"), &(cfg->cr_textbg));

		ini_getcr(sect, _T("cr_sq_sense_op"), &(cfg->cr_sq_sense_op));
		ini_getcr(sect, _T("cr_sq_sense_cl"), &(cfg->cr_sq_sense_cl));
		ini_getcr(sect, _T("cr_sq_thres_op"), &(cfg->cr_sq_thres_op));
		ini_getcr(sect, _T("cr_sq_thres_cl"), &(cfg->cr_sq_thres_cl));

		/*cfg->fontsize = ini_geti(sect, _T("fontsize"), cfg->fontsize, &usedef);
		ini_copys(sect, _T("fontname"), cfg->fontname, LF_FACESIZE, cfg->fontname, &usedef);*/

		ini_gete(sect, _T("scale_mode"), &(cfg->scale_mode), SPECSCAL_MAP_MODE_COUNT);

		ini_getb(sect, _T("ups_display"), &(cfg->show_ups_counter));
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* save specturm viewer configuration */
static void specview_cfg_save(specview_cfg_t *cfg, ini_sect_t *sect)
{
	if(sect != NULL)
	{
		ini_setf(sect, _T("m_0"), 6, cfg->m_0);
		ini_setf(sect, _T("m_1"), 6, cfg->m_1);

		ini_setcr(sect, _T("cr_bkgnd_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND]);
		ini_setcr(sect, _T("cr_input_fc_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC]);
		ini_setcr(sect, _T("cr_chan_fc_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC]);
		ini_setcr(sect, _T("cr_chan_bw_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW]);
		ini_setcr(sect, _T("cr_chan_bwovl_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL]);

		ini_setcr(sect, _T("cr_bkgnd_grid_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND_GRID]);
		ini_setcr(sect, _T("cr_input_fc_grid_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_INPUT_FC_GRID]);
		ini_setcr(sect, _T("cr_chan_fc_grid_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_FC_GRID]);
		ini_setcr(sect, _T("cr_chan_bw_grid_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BW_GRID]);
		ini_setcr(sect, _T("cr_chan_bwovl_grid_0"), cfg->cr_shm_cold[SPECVIEW_COLOR_CHAN_BWOVL_GRID]);

		ini_setcr(sect, _T("cr_bkgnd_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND]);
		ini_setcr(sect, _T("cr_input_fc_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC]);
		ini_setcr(sect, _T("cr_chan_fc_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC]);
		ini_setcr(sect, _T("cr_chan_bw_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW]);
		ini_setcr(sect, _T("cr_chan_bwovl_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL]);

		ini_setcr(sect, _T("cr_bkgnd_grid_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND_GRID]);
		ini_setcr(sect, _T("cr_input_fc_grid_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_INPUT_FC_GRID]);
		ini_setcr(sect, _T("cr_chan_fc_grid_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_FC_GRID]);
		ini_setcr(sect, _T("cr_chan_bw_grid_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BW_GRID]);
		ini_setcr(sect, _T("cr_chan_bwovl_grid_1"), cfg->cr_shm_hot[SPECVIEW_COLOR_CHAN_BWOVL_GRID]);

		ini_setcr(sect, _T("cr_ticks"), cfg->cr_ticks);
		ini_setcr(sect, _T("cr_label"), cfg->cr_label);
		ini_setcr(sect, _T("cr_text"), cfg->cr_text);
		ini_setcr(sect, _T("cr_textbg"), cfg->cr_textbg);

		ini_setcr(sect, _T("cr_sq_sense_op"), cfg->cr_sq_sense_op);
		ini_setcr(sect, _T("cr_sq_sense_cl"), cfg->cr_sq_sense_cl);
		ini_setcr(sect, _T("cr_sq_thres_op"), cfg->cr_sq_thres_op);
		ini_setcr(sect, _T("cr_sq_thres_cl"), cfg->cr_sq_thres_cl);

		/*ini_seti(sect, _T("fontsize"), cfg->fontsize);
		ini_set(sect, _T("fontname"), cfg->fontname);*/

		ini_seti(sect, _T("scale_mode"), cfg->scale_mode);

		ini_setb(sect, _T("ups_display"), cfg->show_ups_counter);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* gdi resources cleanup */
static void specview_res_free(specview_res_ctx_t *res)
{
	int i;

	DeleteObject(res->hpen_ticks);

	DeleteObject(res->hpen_sq_sense_op);
	DeleteObject(res->hpen_sq_sense_cl);
	DeleteObject(res->hpen_sq_thres_op);
	DeleteObject(res->hpen_sq_thres_cl);
	
	for(i = 0; i < SPECVIEW_COLOR_COUNT; i++) {
		DeleteObject(res->hpen_hot[i]);
		DeleteObject(res->hpen_cold[i]);
	}

	DeleteObject(res->hbr_window);
	DeleteObject(res->hbr_textbg);
	DeleteObject(res->hbr_bkgnd_cold);
	DeleteObject(res->hbr_bkgnd_hot);
}

/* ---------------------------------------------------------------------------------------------- */

/* gdi resources (re)init */
static int specview_res_init(specview_res_ctx_t *res, specview_cfg_t *cfg)
{
	int i, status = 1;

	/* clean up existing resources if needed */
	if(res->is_inited) {
		specview_res_free(res);
		res->is_inited = 0;
	}

	res->hpen_stock_null = GetStockObject(NULL_PEN);
	res->hpen_stock_black = GetStockObject(BLACK_PEN);
	res->hpen_stock_white = GetStockObject(WHITE_PEN);

	res->hpen_ticks = CreatePen(PS_SOLID, 1, cfg->cr_ticks);

	res->hpen_sq_sense_op = CreatePen(PS_SOLID, 1, cfg->cr_sq_sense_op);
	res->hpen_sq_sense_cl = CreatePen(PS_SOLID, 1, cfg->cr_sq_sense_cl);
	res->hpen_sq_thres_op = CreatePen(PS_SOLID, 1, cfg->cr_sq_thres_op);
	res->hpen_sq_thres_cl = CreatePen(PS_SOLID, 1, cfg->cr_sq_thres_cl);

	if( (res->hpen_ticks == NULL) ||
		(res->hpen_sq_sense_op == NULL) || (res->hpen_sq_sense_cl == NULL) ||
		(res->hpen_sq_thres_op == NULL) || (res->hpen_sq_thres_cl == NULL) )
	{
		status = 0;
	}

	for(i = 0; i < SPECVIEW_COLOR_COUNT; i++) {
		res->hpen_hot[i] = CreatePen(PS_SOLID, 1, cfg->cr_shm_hot[i]);
		res->hpen_cold[i] = CreatePen(PS_SOLID, 1, cfg->cr_shm_cold[i]);
		if( (res->hpen_hot[i] == NULL) || (res->hpen_cold[i] == NULL) )
			status = 0;
	}

	res->cr_window = GetSysColor(COLOR_BTNFACE);
	res->hbr_window = CreateSolidBrush(res->cr_window);

	res->hbr_textbg = CreateSolidBrush(cfg->cr_textbg);

	res->hbr_bkgnd_cold = CreateSolidBrush(cfg->cr_shm_cold[SPECVIEW_COLOR_BKGND]);
	res->hbr_bkgnd_hot = CreateSolidBrush(cfg->cr_shm_hot[SPECVIEW_COLOR_BKGND]);

	if( (res->hbr_window == NULL) || (res->hbr_textbg == NULL) ||
		(res->hbr_bkgnd_cold == NULL) || (res->hbr_bkgnd_hot == NULL) )
	{
		status = 0;
	}

	if(!status) {
		specview_res_free(res);
		return 0;
	}

	res->is_inited = 1;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* reinit gdi resources after config change */
int specview_res_reinit(specview_ctx_t *ctx)
{
	int status = 0;

	EnterCriticalSection(&(ctx->csec));

	/* reinit gdi resources */
	status = specview_res_init(&(ctx->res), &(ctx->cfg));

	/* force graphics redraw */
	ctx->buf.is_shm_src_prepared = 0;
	ctx->buf.is_shm_src_updated = 0;
	ctx->mag.is_drawn = 0;
	ctx->freq.is_drawn = 0;
	ctx->sql.is_updated = 0;
	ctx->sql.use_full_redraw = 1;
	ctx->text.is_drawn = 0;

	LeaveCriticalSection(&(ctx->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize spectrum viewer */
specview_ctx_t *specview_init(HDC hdc, HFONT hfont,
							  rxstate_t *rx, double f_0, double f_1,
							  int win_w, int win_h, ini_sect_t *sect)
{
	specview_ctx_t *ctx;

	/* check parameters */
	if( (win_w < SPECVIEW_MIN_W) || (win_w > SPECVIEW_MAX_W) ||
		(win_h < SPECVIEW_MIN_H) || (win_h > SPECVIEW_MAX_H) ||
		(f_0 < RXLIM_FC_MIN) || (f_1 > RXLIM_FC_MAX) || (f_1 <= f_0) )
	{
		return 0;
	}

	/* allocate memory */
	if( (ctx = calloc(1, sizeof(specview_ctx_t))) != NULL )
	{
		InitializeCriticalSection(&(ctx->csec));
		ctx->ups_buf = calloc(SPECVIEW_UPS_BUFSIZE, sizeof(DWORD));
		ctx->text.buf = malloc(SPECVIEW_TEXT_MAXLEN * sizeof(TCHAR));
		ctx->text.buf[SPECVIEW_TEXT_MAXLEN - 1] = 0;

		/* check for allocation errors */
		if(ctx->ups_buf != NULL)
		{
			/* initialize structure */
			ctx->rx = rx;
			ctx->f_0 = f_0;
			ctx->f_1 = f_1;
			ctx->hfont = hfont;

			/* initialize configuration */
			specview_cfg_reset(&(ctx->cfg));
			specview_cfg_load(&(ctx->cfg), sect);

			/* allocate gdi resources */
			if(specview_res_init(&(ctx->res), &(ctx->cfg)))
			{
				/* allocate main image buffer */
				if(specview_buf_init(&(ctx->buf), hdc, win_w, win_h))
				{
					/* update all data */
					specview_set_mag_range(ctx);
					specview_set_freq_range(ctx);
					specview_set_chmap(ctx);
					specview_set_sql(ctx, 1, 0);

					return ctx;
				}

				specview_res_free(&(ctx->res));
			}
		}

		/* cleanup */
		free(ctx->text.buf);
		free(ctx->ups_buf);
		DeleteCriticalSection(&(ctx->csec));
		free(ctx);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* clean up spectrum viewer */
void specview_cleanup(specview_ctx_t *ctx, ini_sect_t *sect)
{
	if(ctx != NULL)
	{
		specview_cfg_save(&(ctx->cfg), sect);

		if(ctx->buf.is_inited) {
			specview_buf_free(&(ctx->buf));
		}

		if(ctx->res.is_inited) {
			specview_res_free(&(ctx->res));
		}

		specview_sql_cleanup(ctx);

		free(ctx->text.buf);
		free(ctx->ups_buf);
		DeleteCriticalSection(&(ctx->csec));
		free(ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */
