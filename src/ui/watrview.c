/* ---------------------------------------------------------------------------------------------- */

#include "watrview.h"

/* ---------------------------------------------------------------------------------------------- */

/* create new waterfall image segment and insert at chain head */
static watrview_seg_t *watrview_seg_create(watrview_seg_buf_ctx_t *segbuf, HDC hdc,
										   int seg_w, int seg_len, double f_0, double f_1)
{
	watrview_seg_t *seg;

	/* check parameters */
	if((seg_w <= 0) || (seg_len <= 0) || (f_1 <= f_0))
		return NULL;

	/* allocate memory */
	if( (seg = calloc(1, sizeof(watrview_seg_t))) != NULL )
	{
		seg->segbuf = segbuf;

		/* set segment length and data offset */
		seg->len = seg_len;

		/* initialize source image buffer */
		seg->src_w = seg_w;
		seg->hbm_src = CreateCompatibleBitmap(hdc, seg_w, seg_len);

		/* initialize output image buffer */
		seg->out_w = seg_w;
		seg->out_f_0 = f_0;
		seg->out_f_1 = f_1;
		seg->hbm_out = CreateCompatibleBitmap(hdc, seg_w, seg_len);

		/* initialize source image per-line data buffer */
		seg->src_data = malloc(sizeof(watrview_line_data_t) * seg_len);

		/* check for allocation errors */
		if((seg->hbm_src != NULL) && (seg->hbm_out != NULL) && (seg->src_data != NULL))
		{
			/* insert segment to segment chain head  */
			DLIST_INSFRONT(seg, segbuf->head, segbuf->tail, next, prev);

			return seg;
		}

		/* free image buffers */
		DeleteObject(seg->hbm_out);
		DeleteObject(seg->hbm_src);

		/* free memory */
		free(seg->src_data);
		free(seg);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* delete waterfall image segment and all next (older) segments */
static void watrview_seg_delete(watrview_seg_buf_ctx_t *segbuf, watrview_seg_t *seg_first)
{
	watrview_seg_t *seg, *seg_next;

	/* process segment list */
	for(seg = seg_first; seg != NULL; seg = seg_next)
	{
		seg_next = seg->next;

		/* remove segment from chain */
		DLIST_REMOVE(seg, segbuf->head, segbuf->tail, next, prev);

		/* delete segment buffers */
		DeleteObject(seg->hbm_out);
		DeleteObject(seg->hbm_src);

		/* free memory */
		free(seg->src_data);
		free(seg);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* returns first inactive (out of scroll range) segment from segment chain */
static watrview_seg_t *watrview_seg_firstinactive(watrview_seg_buf_ctx_t *segbuf)
{
	watrview_seg_t *seg;
	int len_sum;

	/* walk through segment chain until scroll range reached */
	len_sum = 0;
	for(seg = segbuf->head; seg != NULL; seg = seg->next)
	{
		/* check total length of segments */
		if(len_sum >= segbuf->chain_max_len)
			break;
		len_sum += seg->data_len;
	}

	return seg;
}

/* ---------------------------------------------------------------------------------------------- */

/* rebuild segment buffer to match displayed frequency range and window width */
static int watrview_seg_rebuild(watrview_seg_t *seg, watrview_res_ctx_t *res, HDC hdc,
								double out_f_0, double out_f_1, int out_w)
{
	watrview_seg_buf_ctx_t *segbuf;
	HGDIOBJ hbr_prev, hbm_src_prev, hbm_out_prev;
	HBITMAP hbm_out_new;
	int data_len, block_len, data_ptr;
	double src_f_0 = 0, src_f_1 = 0;
	double f_0, f_1, src_df, dst_df;
	int src_j_0, src_j_1, src_w;
	int dst_j_0, dst_j_1, dst_w;
	int copy_org;
	int status = 1;

	/* check resources and parameters */
	if((!res->is_inited) || (out_f_1 <= out_f_0) || (out_w <= 0))
		return 0;

	/* is rebuild actually needed? */
	if( (out_w == seg->out_w) &&
		(fabs(out_f_0 - seg->out_f_0) < 1e-3) &&
		(fabs(out_f_1 - seg->out_f_1) < 1e-3) )
	{
		return -1;
	}

	/* reallocate output buffer if displayed width changed */
	if(out_w != seg->out_w)
	{
		/* allocate new output buffer */
		hbm_out_new = CreateCompatibleBitmap(hdc, out_w, seg->len);
		if(hbm_out_new == NULL)
			return 0;

		/* replace output buffer */
		DeleteObject(seg->hbm_out);
		seg->hbm_out = hbm_out_new;

		seg->out_w = out_w;
	}

	/* select image buffers */
	segbuf = seg->segbuf;
	hbm_src_prev = SelectObject(segbuf->hdc_src, seg->hbm_src);
	hbm_out_prev = SelectObject(segbuf->hdc_out, seg->hbm_out);

	/* erase output buffer background */
	hbr_prev = SelectObject(segbuf->hdc_out, res->hbr_bkgnd);
	PatBlt(segbuf->hdc_out, 0, 0, seg->out_w, seg->len, PATCOPY);
	SelectObject(segbuf->hdc_out, hbr_prev);

	/* remaining data length to transfer */
	data_len = seg->data_len;

	/* transfer data from source buffer */
	for( ; ; )
	{
		/* check for end of transfer */
		if(data_len <= 0)
			break;

		/* calculate length of block to transfer */
		for(block_len = 0; block_len < data_len; block_len++)
		{
			data_ptr = seg->len - data_len + block_len;

			if(block_len == 0)
			{
				src_f_0 = seg->src_data[data_ptr].f_0;
				src_f_1 = seg->src_data[data_ptr].f_1;
			}
			else
			{
				if( (fabs(src_f_0 - seg->src_data[data_ptr].f_0) >= 1e-3) ||
					(fabs(src_f_1 - seg->src_data[data_ptr].f_1) >= 1e-3) )
				{
					break;
				}
			}
		}

		/* check for frequency range intersection */
		if((src_f_0 <= out_f_1) && (src_f_1 >= out_f_0))
		{
			/* get intersection lower limit */
			f_0 = out_f_0;
			if(f_0 < src_f_0)
				f_0 = src_f_0;

			/* get intersection upper limit */
			f_1 = out_f_1;
			if(f_1 > src_f_1)
				f_1 = src_f_1;

			/* map frequency intersection to source buffer x-coordinates */
			src_df = (src_f_1 - src_f_0) / (seg->src_w - 1);
			src_j_0 = (int)( (f_0 - src_f_0) / src_df + 0.5 );
			src_j_1 = (int)( (f_1 - src_f_0) / src_df + 0.5 );
			src_w = src_j_1 - src_j_0 + 1;

			/* map frequency intersection to output buffer x-coordinates */
			dst_df = (out_f_1 - out_f_0) / (seg->out_w - 1);
			dst_j_0 = (int)( (f_0 - out_f_0) / dst_df + 0.5 );
			dst_j_1 = (int)( (f_1 - out_f_0) / dst_df + 0.5 );
			dst_w = dst_j_1 - dst_j_0 + 1;

			/* calculate copy y-offset */
			copy_org = seg->len - data_len;

			/* is stretching needed? */
			if(dst_w == src_w)
			{
				BitBlt(
					segbuf->hdc_out, dst_j_0, copy_org, dst_w, block_len,
					segbuf->hdc_src, src_j_0, copy_org, SRCCOPY);
			}
			else
			{
				StretchBlt(
					segbuf->hdc_out, dst_j_0, copy_org, dst_w, block_len,
					segbuf->hdc_src, src_j_0, copy_org, src_w, block_len, SRCCOPY);
			}
		}

		/* update remaining data length */
		data_len -= block_len;
	}

	/* save new output frequency range */
	seg->out_f_0 = out_f_0;
	seg->out_f_1 = out_f_1;

	/* deselect image buffers */
	SelectObject(segbuf->hdc_src, hbm_src_prev);
	SelectObject(segbuf->hdc_out, hbm_out_prev);

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* check segment to suitability for storing new data */
static int watrview_seg_check(watrview_seg_t *seg, int img_w, int seg_len_req)
{
	return ((seg != NULL) && (seg->src_w == img_w) && (seg->len == seg_len_req));
}

/* ---------------------------------------------------------------------------------------------- */

/* reuse old inactive segment instead of creating new one */
static watrview_seg_t *watrview_seg_reuse(watrview_seg_buf_ctx_t *segbuf, int img_w)
{
	watrview_seg_t *seg;

	/* get first inactive segment from chain */
	seg = watrview_seg_firstinactive(segbuf);

	/* is segment valid for storing new data? */
	if(watrview_seg_check(seg, img_w, segbuf->seg_len))
	{
		/* reset segment data counter */
		seg->data_len = 0;

		/* move segment to chain head */
		DLIST_REMOVE(seg, segbuf->head, segbuf->tail, next, prev);
		DLIST_INSFRONT(seg, segbuf->head, segbuf->tail, next, prev);

		return seg;
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* get current image segment or create new with specified size for storing new data */
static watrview_seg_t *watrview_seg_getforappend(watrview_seg_buf_ctx_t *segbuf, watrview_res_ctx_t *res,
												 HDC hdc, int img_w, double f_0, double f_1)
{
	watrview_seg_t *seg_cur, *seg;

	/* check parameters */
	if((!res->is_inited) || (img_w <= 0) || (f_1 <= f_0))
		return NULL;

	/* can current segment hold new data? */
	seg_cur = segbuf->head;
	if((seg_cur != NULL) && (seg_cur->src_w == img_w) && (seg_cur->data_len < seg_cur->len))
	{
		/* rebuild current segment output buffer if needed */
		if(!watrview_seg_rebuild(seg_cur, res, hdc, f_0, f_1, img_w))
			return NULL;
		/* use current segment */
		return seg_cur;
	}

	/* try reusing old inactive segment instead of creating new one */
	if((seg = watrview_seg_reuse(segbuf, img_w)) != NULL)
		return seg;

	/* create new segment */
	if((seg = watrview_seg_create(segbuf, hdc, img_w, segbuf->seg_len, f_0, f_1)) != NULL)
		return seg;

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* copy waterfall image to window buffer */
static int watrview_img_copy(watrview_ctx_t *ctx, HDC hdc)
{
	HGDIOBJ hbr_prev, hbm_out_prev;
	int dst_pos, dst_len, skip_len, src_pos, copy_len;
	watrview_seg_t *seg;
	int status = 1;

	/* check resources */
	if((!ctx->buf.is_inited) || (!ctx->res.is_inited))
		return 0;

	/* erase window buffer with background color */
	hbr_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hbr_bkgnd);
	PatBlt(
		ctx->buf.hdc_buf, ctx->buf.img_x, ctx->buf.img_y, ctx->buf.img_w, ctx->buf.img_h,
		PATCOPY);
	SelectObject(ctx->buf.hdc_buf, hbr_prev);

	/* destination buffer pointer and data length */
	dst_pos = 0;
	dst_len = min(ctx->buf.img_h, ctx->segbuf.chain_max_len);

	/* data length to skip */
	skip_len = ctx->segbuf.scroll_pos;

	for(seg = ctx->segbuf.head; seg != NULL; seg = seg->next)
	{
		/* is destination buffer filled up? */
		if(dst_pos >= dst_len)
			break;

		/* is current segment to be fully skipped? */
		if(skip_len < seg->data_len)
		{
			/* calculate block length to transfer */
			copy_len = seg->data_len - skip_len;
			if(dst_pos + copy_len > dst_len)
				copy_len = dst_len - dst_pos;

			/* rebuild segment output image buffer if required */
			if(watrview_seg_rebuild(seg, &(ctx->res), hdc,
				ctx->f_0, ctx->f_1, ctx->buf.img_w))
			{
				/* segment data offset for forward copy */
				src_pos = seg->len - seg->data_len + skip_len;

				hbm_out_prev = SelectObject(ctx->segbuf.hdc_out, seg->hbm_out);

				BitBlt(
					ctx->buf.hdc_buf, ctx->buf.img_x, ctx->buf.img_y + dst_pos,
					seg->out_w, copy_len,
					ctx->segbuf.hdc_out, 0, src_pos, SRCCOPY);

				SelectObject(ctx->segbuf.hdc_out, hbm_out_prev);
			}
			else
			{
				status = 0;
			}

			/* update destination buffer position */
			dst_pos += copy_len;

			/* update skip length */
			skip_len = 0;
		}
		else
		{
			/* remove skipped segment from skip length */
			skip_len -= seg->data_len;
		}
	}

	/* validate data copied to main buffer */
	ctx->buf.is_img_copied = 1;

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* erase main buffer background */
static int watrview_draw_bkgnd(watrview_ctx_t *ctx)
{
	HBRUSH hbr_prev;

	/* check resources */
	if((!ctx->buf.is_inited) || (!ctx->res.is_inited))
		return 0;

	/* fill main buffer with window brush */
	hbr_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hbr_window);
	PatBlt(ctx->buf.hdc_buf, 0, 0, ctx->buf.win_w, ctx->buf.win_h, PATCOPY);
	SelectObject(ctx->buf.hdc_buf, hbr_prev);

	/* validate background */
	ctx->buf.is_background_erased = 1;

	/* invalidate main buffer graphics */
	ctx->time_scale.is_drawn = 0;
	ctx->scrbar.is_drawn = 0;
	ctx->buf.is_img_copied = 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* draw time scale to main buffer */
static int watrview_draw_time_scale(watrview_ctx_t *ctx)
{
	HGDIOBJ hbr_prev, hpen_prev, hfnt_prev;
	int i, m, s;
	RECT rt;
	TCHAR buf[32];

	/* check resources */
	if((!ctx->buf.is_inited) || (!ctx->res.is_inited) || (!ctx->time_scale.is_inited))
		return 0;

	/* erase time scale background */
	hbr_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hbr_window);
	PatBlt(ctx->buf.hdc_buf, ctx->buf.ts_x, ctx->buf.ts_y, ctx->buf.ts_w, ctx->buf.ts_h, PATCOPY);
	SelectObject(ctx->buf.hdc_buf, hbr_prev);

	/* select objects */
	hpen_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hpen_ticks);
	hfnt_prev = SelectObject(ctx->buf.hdc_buf, ctx->hfont);

	SetBkColor(ctx->buf.hdc_buf, ctx->res.cr_window);
	SetTextColor(ctx->buf.hdc_buf, ctx->cfg.cr_label);

	/* draw time scale labels */
	for(i = 0; i < ctx->time_scale.label_count; i++)
	{
		/* get label position */
		rt.left = ctx->buf.ts_x;
		rt.right = rt.left + ctx->buf.ts_w - 5;
		rt.top = ctx->buf.ts_y + ctx->time_scale.label[i].position - 7;
		rt.bottom = ctx->buf.ts_y + ctx->time_scale.label[i].position + 8;

		/* check label position */
		if( (rt.top >= ctx->buf.ts_y) &&
			(rt.bottom <= ctx->buf.ts_y + ctx->buf.ts_h) )
		{
			/* format label text */
			m = ctx->time_scale.label[i].value / 60;
			s = ctx->time_scale.label[i].value % 60;
			_stprintf(buf, _T("%d:%02d"), m % 10, s);

			/* draw label */
			DrawText(ctx->buf.hdc_buf, buf, (int)_tcslen(buf),
				&rt, DT_SINGLELINE|DT_RIGHT|DT_VCENTER);

			/* draw tick */
			MoveToEx(ctx->buf.hdc_buf, ctx->buf.ts_x + ctx->buf.ts_w - 3,
				ctx->time_scale.label[i].position, NULL);
			LineTo(ctx->buf.hdc_buf, ctx->buf.ts_x + ctx->buf.ts_w,
				ctx->time_scale.label[i].position);

		}
	}

	/* restore objects */
	SelectObject(ctx->buf.hdc_buf, hpen_prev);
	SelectObject(ctx->buf.hdc_buf, hfnt_prev);

	/* mark time scale drawn */
	ctx->time_scale.is_drawn = 1;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* draw scrollbar to main buffer */
static int watrview_draw_scrbar(watrview_ctx_t *ctx)
{
	HGDIOBJ hbr_prev, hpen_prev;
	int left, right, center;
	int top, bottom;

	/* check resources */
	if((!ctx->buf.is_inited) || (!ctx->res.is_inited) || (!ctx->scrbar.is_inited))
		return 0;

	left = ctx->buf.scr_x;
	right = left + ctx->buf.scr_w;
	center = left + ctx->buf.scr_w / 2;

	top = ctx->buf.scr_y;
	bottom = top + ctx->buf.scr_h;

	/* fill scrollbar background with window brush */
	hbr_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hbr_window);
	PatBlt(ctx->buf.hdc_buf, ctx->buf.scr_x, ctx->buf.scr_y,
		ctx->buf.scr_w, ctx->buf.scr_h, PATCOPY);
	SelectObject(ctx->buf.hdc_buf, hbr_prev);

	/* draw slider guide */
	hpen_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hpen_scrbar);
	MoveToEx(ctx->buf.hdc_buf, center, top, NULL);
	LineTo(ctx->buf.hdc_buf, center, bottom);
	MoveToEx(ctx->buf.hdc_buf, left, top, NULL);
	LineTo(ctx->buf.hdc_buf, right, top);
	MoveToEx(ctx->buf.hdc_buf, left, bottom - 1, NULL);
	LineTo(ctx->buf.hdc_buf, right, bottom - 1);
	SelectObject(ctx->buf.hdc_buf, hpen_prev);

	/* draw slider */
	if(ctx->scrbar.slider_len > 0)
	{
		hbr_prev = SelectObject(ctx->buf.hdc_buf, ctx->res.hbr_scrbar);
		PatBlt(ctx->buf.hdc_buf, ctx->buf.scr_x, ctx->scrbar.slider_pos,
			ctx->buf.scr_w, ctx->scrbar.slider_len, PATCOPY);
		SelectObject(ctx->buf.hdc_buf, hbr_prev);
	}

	/* validate scrollbar graphics */
	ctx->scrbar.is_drawn = 1;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* update time scale */
static int watrview_update_time_scale(watrview_ctx_t *ctx)
{
	watrview_seg_t *seg;
	int seg_offset, i, pos, pos_prev, pos_cl, value, data_pos;
	double t_cur, t_frame;
	int oldlabelcount, newlabelcap;
	void *newlabelbuf;

	ctx->time_scale.is_inited = 0;

	seg_offset = 0;
	pos = 0;
	pos_prev = 0;
	t_cur = 0;

	oldlabelcount = ctx->time_scale.label_count;
	ctx->time_scale.label_count = 0;

	for(seg = ctx->segbuf.head; seg != NULL; seg = seg->next)
	{
		for(i = 0; i < seg->data_len; i++)
		{
			data_pos = seg->len - seg->data_len + i;
			t_frame = seg->src_data[data_pos].t_frame;

			pos = seg_offset + i;

			if( (pos - pos_prev >= WATRVIEW_TIME_GAUGE_MIN_LABEL_H) &&
				(floor(t_cur + t_frame) - floor(t_cur) >= 0.5) )
			{
				pos_cl = pos - ctx->segbuf.scroll_pos + 1;
				value = (int)(floor(t_cur) + 0.5) + 1;

				if( (pos_cl >= 0) && (pos_cl < ctx->buf.ts_h) )
				{
					if(ctx->time_scale.label_count == ctx->time_scale.label_capacity)
					{
						newlabelcap = ctx->time_scale.label_capacity + 100;
						newlabelbuf = realloc(ctx->time_scale.label, 
							newlabelcap * sizeof(watrview_time_scale_label_t));
						if(newlabelbuf == NULL)
							return 0;
						ctx->time_scale.label_capacity = newlabelcap;
						ctx->time_scale.label = newlabelbuf;
					}

					if( (ctx->time_scale.label_count >= oldlabelcount) ||
						(ctx->time_scale.label[ctx->time_scale.label_count].position != pos_cl) ||
						(ctx->time_scale.label[ctx->time_scale.label_count].value != value) )
					{
						ctx->time_scale.label[ctx->time_scale.label_count].position = pos_cl;
						ctx->time_scale.label[ctx->time_scale.label_count].value = value;

						ctx->time_scale.is_drawn = 0;
					}

					ctx->time_scale.label_count++;
				}

				pos_prev = pos;
			}

			t_cur += t_frame;
		}

		seg_offset += seg->data_len;
	}

	if(ctx->time_scale.label_count != oldlabelcount)
		ctx->time_scale.is_drawn = 0;

	ctx->time_scale.is_inited = 1;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* update scrollbar data */
static int watrview_update_scrbar(watrview_ctx_t *ctx)
{
	int slider_pos, slider_len;

	/* calculate slider length */
	if(ctx->segbuf.chain_max_len == 0)
		return 0;
	slider_len = ctx->buf.img_h * ctx->buf.scr_h / ctx->segbuf.chain_max_len;
	if(slider_len < 2)
		slider_len = 2;
	if(slider_len > ctx->buf.scr_h)
		slider_len = ctx->buf.scr_h;

	/* calculate slider position */
	if(ctx->segbuf.chain_max_len <= ctx->buf.img_h) {
		slider_pos = 0;
	} else {
		slider_pos = ctx->segbuf.scroll_pos * (ctx->buf.scr_h - slider_len) /
			(ctx->segbuf.chain_max_len - ctx->buf.img_h);
		if(slider_pos < 0)
			slider_pos = 0;
		if(slider_pos + slider_len > ctx->buf.scr_h)
			slider_pos = ctx->buf.scr_h - slider_len;
	}

	/* check slider actually moved */
	if( ctx->scrbar.is_inited && (ctx->scrbar.height == ctx->buf.scr_h) &&
		(ctx->scrbar.slider_pos == slider_pos) && (ctx->scrbar.slider_len == slider_len) )
	{
		return -1;
	}

	/* set new slider data */
	ctx->scrbar.height = ctx->buf.scr_h;
	ctx->scrbar.slider_pos = slider_pos;
	ctx->scrbar.slider_len = slider_len;
	ctx->scrbar.is_inited = 1;
	ctx->scrbar.is_drawn = 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* autoscroll buffer down when data appended */
static void watrview_autoscroll(watrview_ctx_t *ctx)
{
	int scroll_pos_max, scroll_pos;

	/* calculate maximum scroll position */
	scroll_pos_max = ctx->segbuf.chain_max_len - ctx->buf.img_h;
	if(scroll_pos_max < 0)
		scroll_pos_max = 0;

	/* get scroll position */
	scroll_pos = ctx->segbuf.scroll_pos + 1;

	/* update scrollbar */
	if(scroll_pos <= scroll_pos_max)
	{
		ctx->segbuf.scroll_pos = scroll_pos;
		watrview_update_scrbar(ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */

/* set new buffer scroll position */
static int watrview_set_scroll_pos(watrview_ctx_t *ctx, int scroll_pos)
{
	int scroll_pos_max;

	/* get maximum scroll position */
	scroll_pos_max = ctx->segbuf.chain_max_len - ctx->buf.img_h;
	if(scroll_pos_max < 0)
		scroll_pos_max = 0;

	/* limit scroll position */
	if(scroll_pos < 0)
		scroll_pos = 0;
	if(scroll_pos > scroll_pos_max)
		scroll_pos = scroll_pos_max;

	/* check scroll position changed */
	if(scroll_pos == ctx->segbuf.scroll_pos)
		return 0;

	/* save new scroll position */
	ctx->segbuf.scroll_pos = scroll_pos;

	/* update scrolled graphics */
	watrview_update_time_scale(ctx);
	watrview_update_scrbar(ctx);
	ctx->buf.is_img_copied = 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* scroll viewer by specified pixel count */
int watrview_scroll(watrview_ctx_t *ctx, int delta)
{
	int status;

	EnterCriticalSection(&(ctx->csec));

	status = watrview_set_scroll_pos(
		ctx, ctx->segbuf.scroll_pos + delta);

	LeaveCriticalSection(&(ctx->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* scroll viewer for specified scrollbar slider position */
int watrview_drag_scrbar(watrview_ctx_t *ctx, int slider_pos)
{
	int status = 0;

	if(!ctx->scrbar.is_inited)
		return 0;

	EnterCriticalSection(&(ctx->csec));

	if(ctx->scrbar.slider_len < ctx->buf.scr_h)
	{
		status = watrview_set_scroll_pos(ctx, slider_pos *
			(ctx->segbuf.chain_max_len - ctx->buf.img_h) /
			(ctx->buf.scr_h - ctx->scrbar.slider_len));
	}

	LeaveCriticalSection(&(ctx->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* accumulate spectrum frame to buffer */
static int watrview_data_accum(watrview_ctx_t *ctx, double fc_input, unsigned int fs_input,
							   float *fft_pwr_buf, size_t fft_pwr_buf_size, size_t input_framelen)
{
	int i;
	double df_in, f_in_0, f_in_1;
	/*float nf;*/

	df_in = (double)fs_input / (double)fft_pwr_buf_size;
	f_in_0 = fc_input - df_in * (double)(fft_pwr_buf_size / 2 - 1);
	f_in_1 = fc_input + df_in * (double)(fft_pwr_buf_size / 2);

	if(ctx->framediv_ctr == 0)
	{
		scale_pwr_spect(
			ctx->buf.spect_buf_db, ctx->buf.img_w, ctx->f_0, ctx->f_1,
			fft_pwr_buf, (int)fft_pwr_buf_size, f_in_0, f_in_1, ctx->cfg.scale_mode);
		ctx->buf.spect_framelen_total = input_framelen;
	}
	else
	{
		scale_pwr_spect(
			ctx->buf.spect_buf_db_temp, ctx->buf.img_w, ctx->f_0, ctx->f_1,
			fft_pwr_buf, (int)fft_pwr_buf_size, f_in_0, f_in_1, ctx->cfg.scale_mode);

		/*switch(ctx->cfg.scale_mode)
		{
		case SPECSCALE_MAP_PEAK:*/
			for(i = 0; i < ctx->buf.img_w; i++) {
				if(ctx->buf.spect_buf_db_temp[i] > ctx->buf.spect_buf_db[i])
					ctx->buf.spect_buf_db[i] = ctx->buf.spect_buf_db_temp[i];
			}
			/*break;
		case SPECSCALE_MAP_AVG:
			for(i = 0; i < ctx->buf.img_w; i++)
				ctx->buf.spect_buf_db[i] += ctx->buf.spect_buf_db_temp[i];
			break;
		}*/

		ctx->buf.spect_framelen_total += input_framelen;
	}

	ctx->framediv_ctr++;

	if(ctx->framediv_ctr >= ctx->cfg.framediv)
	{
		/*if( (ctx->cfg.scale_mode == SPECSCALE_MAP_AVG) && 
			(ctx->framediv_ctr > 1) )
		{
			nf = 1.0f / ctx->framediv_ctr;
			for(i = 0; i < ctx->buf.img_w; i++)
				ctx->buf.spect_buf_db[i] *= nf;
		}*/

		ctx->framediv_ctr = 0;
		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* render new line */
static int watrview_line_render(watrview_ctx_t *ctx, HDC hdc_dst, int dst_y)
{
	int i, level;
	COLORREF cr;

	scale_mag(
		ctx->buf.spect_j_buf, WATRVIEW_CR_MAP_LEN, ctx->cr_map.m_0, ctx->cr_map.m_1,
		ctx->buf.spect_buf_db, ctx->buf.img_w);

	for(i = 0; i < ctx->buf.img_w; i++)
	{
		level = ctx->buf.spect_j_buf[i];
		if(level < 0)
			level = 0;

		if(!ctx->cfg.use_chan_crmap) {
			cr = ctx->cr_map.cr_map_bkgnd[level];
		} else {
			cr = (ctx->buf.chanmap[i] == WATRVIEW_MAP_BKGND) ?
				ctx->cr_map.cr_map_bkgnd[level] : ctx->cr_map.cr_map_chan[level];
		}

		SetPixel(hdc_dst, i, dst_y, cr);
	}

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* append data to waterfall viewer */
int watrview_append(watrview_ctx_t *ctx, HDC hdc, double fc_input, unsigned int fs_input,
					float *fft_pwr_buf, size_t fft_pwr_buf_size, size_t input_framelen)
{
	HGDIOBJ hbm_src_prev, hbm_out_prev;
	watrview_seg_t *seg, *seg_trim;
	double t_frame;
	int insert_pos, status = 0;

	/* check parameters */
	if((fs_input == 0) || (fft_pwr_buf_size == 0))
		return 0;

	/* check resources */
	if((!ctx->buf.is_inited) || (!ctx->cr_map.is_inited))
		return 0;

	EnterCriticalSection(&(ctx->csec));

	/* accumulate data to spectrum buffer */
	if( !watrview_data_accum(ctx, fc_input, fs_input,
		fft_pwr_buf, fft_pwr_buf_size, input_framelen) )
	{
		LeaveCriticalSection(&(ctx->csec));
		return -1;
	}

	/* get current / create new buffer segment */
	seg = watrview_seg_getforappend(&(ctx->segbuf), &(ctx->res), hdc,
		ctx->buf.img_w, ctx->f_0, ctx->f_1);

	if(seg != NULL)
	{
		/* get new data insertion position */
		insert_pos = seg->len - seg->data_len - 1;

		/* render new line */
		hbm_src_prev = SelectObject(ctx->segbuf.hdc_src, seg->hbm_src);
		hbm_out_prev = SelectObject(ctx->segbuf.hdc_out, seg->hbm_out);

		watrview_line_render(ctx, ctx->segbuf.hdc_src, insert_pos);

		BitBlt(
			ctx->segbuf.hdc_out, 0, insert_pos, seg->out_w, 1,
			ctx->segbuf.hdc_src, 0, insert_pos, SRCCOPY);

		SelectObject(ctx->segbuf.hdc_src, hbm_src_prev);
		SelectObject(ctx->segbuf.hdc_out, hbm_out_prev);

		/* update source buffer line info */
		t_frame = (double)(ctx->buf.spect_framelen_total) / (double)(ctx->rx->fs_input);
		seg->src_data[insert_pos].t_frame = t_frame;
		seg->src_data[insert_pos].f_0 = ctx->f_0;
		seg->src_data[insert_pos].f_1 = ctx->f_1;

		seg->data_len++;

		/* autoscroll to fix image in window */
		if( ctx->scrbar.autoscroll_force ||
			((ctx->segbuf.scroll_pos != 0) && (!ctx->scrbar.autoscroll_inh)) )
		{
			watrview_autoscroll(ctx);
		}

		/* trim inactive segments from chain (keep one for reuse) */
		seg_trim = watrview_seg_firstinactive(&(ctx->segbuf));
		if(watrview_seg_check(seg_trim, ctx->buf.img_w, ctx->segbuf.seg_len))
			seg_trim = seg_trim->next;
		watrview_seg_delete(&(ctx->segbuf), seg_trim);

		/* update and redraw time scale data */
		watrview_update_time_scale(ctx);

		/* force copy to main buffer */
		ctx->buf.is_img_copied = 0;

		status = 1;
	}

	LeaveCriticalSection(&(ctx->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* compare colormap points by magnitude for sorting */
static int watrview_crpt_comp(const watrview_cr_map_pt_t *a, const watrview_cr_map_pt_t *b)
{
	/* compare point magnitudes */
	if(a->m < b->m)
		return -1;
	if(a->m > b->m)
		return 1;

	/* if magnitudes equal, compare priority */
	if(a->priority < b->priority)
		return -1;
	if(a->priority > b->priority)
		return 1;

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* build colormap buffer from reference points and get magnitude range */
int watrview_crmap_build(watrview_cr_map_pt_t *cr_pt_buf, int cr_pt_count,
						 COLORREF *buf_bkgnd, COLORREF *buf_chan, int buf_len,
						 double *pm_0, double *pm_1)
{
	double m_0, m_1, m, m_step, h_0, h_1, af;
	int pt_cur, i;

	/* check parameters */
	if((cr_pt_count < 2) || (buf_len < 2))
		return 0;

	/* sort reference points with priority of original order */
	for(i = 0; i < cr_pt_count; i++)
		cr_pt_buf[i].priority = i;
	qsort(cr_pt_buf, cr_pt_count, sizeof(watrview_cr_map_pt_t), watrview_crpt_comp);

	/* get magnitude range from reference points */
	m_0 = cr_pt_buf[0].m;
	m_1 = cr_pt_buf[cr_pt_count - 1].m;

	/* ensure magnitude range is not empty */
	if(m_1 - m_0 < 1e-6)
		return 0;

	/* output buffer magnitude step per point */
	m_step = (m_1 - m_0) / (buf_len - 1);

	pt_cur = 0; /* current reference point */

	/* for all output buffer points */
	for(i = 0; i < buf_len; i++)
	{
		/* magnitude of current output point */
		m = m_0 + i * m_step;

		/* move to ref point for current magnitude */
		while((pt_cur < cr_pt_count - 1) && (m > cr_pt_buf[pt_cur + 1].m))
			pt_cur++;

		/* first and second reference points magnitudes */
		h_0 = cr_pt_buf[pt_cur].m;
		h_1 = cr_pt_buf[pt_cur + 1].m;

		/* calculate color mix factor from magnitudes */
		if(h_1 - h_0 < 1e-6) {
			af = 0;
		} else {
			af = (m - h_0) / (h_1 - h_0);
		}

		/* calculate background output color */
		buf_bkgnd[i] = cr_mix(
			cr_pt_buf[pt_cur].cr_bkgnd,
			cr_pt_buf[pt_cur + 1].cr_bkgnd,
			af);

		/* calculate channel bandwidth output color */
		buf_chan[i] = cr_mix(
			cr_pt_buf[pt_cur].cr_chan,
			cr_pt_buf[pt_cur + 1].cr_chan,
			af);
	}

	/* return colormap magnitude range */
	*pm_0 = m_0;
	*pm_1 = m_1;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* rebuild colormap */
static int watrview_crmap_update(watrview_ctx_t *ctx)
{
	watrview_cr_map_pt_t *cr_pt_buf;

	/* check configuration */
	if((ctx->cfg.cr_pt_count < 2) || (ctx->cfg.cr_pt_count > WATRVIEW_CR_PT_MAX))
		return 0;

	EnterCriticalSection(&(ctx->csec));

	ctx->cr_map.is_inited = 0;

	/* allocate temp buffer for color sorting reference points */
	if( (cr_pt_buf = malloc(sizeof(watrview_cr_map_pt_t) * ctx->cfg.cr_pt_count)) == NULL )
	{
		LeaveCriticalSection(&(ctx->csec));
		return 0;
	}

	/* copy reference points to temp buffer */
	memcpy(cr_pt_buf, ctx->cfg.cr_pt, sizeof(watrview_cr_map_pt_t) * ctx->cfg.cr_pt_count);

	/* rebuild colormap */
	if( watrview_crmap_build(cr_pt_buf, ctx->cfg.cr_pt_count, 
		ctx->cr_map.cr_map_bkgnd, ctx->cr_map.cr_map_chan,
		WATRVIEW_CR_MAP_LEN, &(ctx->cr_map.m_0), &(ctx->cr_map.m_1)) )
	{
		ctx->cr_map.is_inited = 1;
	}

	/* free temp buffer */
	free(cr_pt_buf);

	LeaveCriticalSection(&(ctx->csec));

	return ctx->cr_map.is_inited;
}

/* ---------------------------------------------------------------------------------------------- */

/* rebuild channel map */
int watrview_set_chmap(watrview_ctx_t *ctx)
{
	rxproc_t *proc;
	double fc, bw_0, bw_1, dj;
	int j_0, j_1, j;

	if(!ctx->buf.is_inited)
		return 0;

	EnterCriticalSection(&(ctx->csec));

	/* fill channel map with background */
	memset(ctx->buf.chanmap, WATRVIEW_MAP_BKGND, ctx->buf.img_w);

	/* calculate pixel coordinate delta per hz */
	dj = (double)(ctx->buf.img_w - 1) / (ctx->f_1 - ctx->f_0);

	/* walk through all channels */
	for(proc = ctx->rx->proc_first; proc != NULL; proc = proc->proc_next)
	{
		/* get channel center frequency and bandwidth */
		fc = proc->cfg.fc;
		bw_0 = fc - proc->cfg.filter_fc;
		bw_1 = fc + proc->cfg.filter_fc;

		/* mark channel center frequency */
		if((fc >= ctx->f_0) && (fc <= ctx->f_1))
		{
			j = (int)((fc - ctx->f_0) * dj + 0.5);
			ctx->buf.chanmap[j] = WATRVIEW_MAP_CHAN_FC;
		}

		/* mark channel bandwidth */
		if((bw_0 <= ctx->f_1) && (bw_1 >= ctx->f_0))
		{
			/* limit frequencies to current display range */
			if(bw_0 < ctx->f_0)
				bw_0 = ctx->f_0;
			if(bw_1 > ctx->f_1)
				bw_1 = ctx->f_1;

			/* map frequencies to pixel coordinates */
			j_0 = (int)((bw_0 - ctx->f_0) * dj + 0.5);
			j_1 = (int)((bw_1 - ctx->f_0) * dj + 0.5);

			/* fill channel bandwidth range */
			for(j = j_0; j <= j_1; j++)
			{
				if(ctx->buf.chanmap[j] == WATRVIEW_MAP_BKGND)
					ctx->buf.chanmap[j] = WATRVIEW_MAP_CHAN_BW;
			}
		}
	}

	LeaveCriticalSection(&(ctx->csec));

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* update all graphics */
int watrview_update(watrview_ctx_t *ctx, HDC hdc)
{
	int error = 0, is_updated = 0;

	EnterCriticalSection(&(ctx->csec));

	if(!ctx->buf.is_background_erased)
	{
		if(!watrview_draw_bkgnd(ctx))
			error = 1;
		is_updated = 1;
	}

	if(!ctx->time_scale.is_drawn)
	{
		if(!watrview_draw_time_scale(ctx))
			error = 1;
		is_updated = 1;
	}

	if(!ctx->scrbar.is_drawn)
	{
		if(!watrview_draw_scrbar(ctx))
			error = 1;
		is_updated = 1;
	}

	if(!ctx->buf.is_img_copied)
	{
		if(!watrview_img_copy(ctx, hdc))
			error = 1;
		is_updated = 1;
	}

	LeaveCriticalSection(&(ctx->csec));

	return (error ? 0 : (is_updated ? 1 : -1));
}

/* ---------------------------------------------------------------------------------------------- */

/* set current display frequency range */
int watrview_set_freq_range(watrview_ctx_t *ctx, double f_0, double f_1)
{
	/* check parameters */
	if((f_0 < RXLIM_FC_MIN) || (f_1 > RXLIM_FC_MAX) || (f_1 <= f_0))
		return 0;

	/* check frequency range actually changed */
	if((fabs(f_0 - ctx->f_0) < 1e-3) && (fabs(f_1 - ctx->f_1) < 1e-3))
		return -1;

	EnterCriticalSection(&(ctx->csec));

	/* set new frequency range */
	ctx->f_0 = f_0;
	ctx->f_1 = f_1;

	/* update channel map */
	watrview_set_chmap(ctx);

	/* force image redraw */
	ctx->buf.is_img_copied = 0;

	LeaveCriticalSection(&(ctx->csec));

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* determine element type located at pixel coordinates */
watrview_point_type_t watrview_map_point(watrview_ctx_t *ctx, int x, int y)
{
	/* check configuration and parameters */
	if( (!ctx->buf.is_inited) ||
		(x < 0) || (x >= ctx->buf.win_w) ||
		(y < 0) || (y >= ctx->buf.win_h) )
	{
		return WATRVIEW_POINT_OUTOFRANGE;
	}

	/* check image intersection */
	if( (x >= ctx->buf.img_x) && (x < ctx->buf.img_x + ctx->buf.img_w) &&
		(y >= ctx->buf.img_y) && (y < ctx->buf.img_y + ctx->buf.img_h) )
	{
		return WATRVIEW_POINT_IMAGE;
	}

	/* check scrollbar intersection */
	if( (x >= ctx->buf.scr_x) && (x < ctx->buf.scr_x + ctx->buf.scr_w) &&
		(y >= ctx->buf.scr_y) && (y < ctx->buf.scr_y + ctx->buf.scr_h) )
	{
		return WATRVIEW_POINT_SCRBAR;
	}

	/* frame by default */
	return WATRVIEW_POINT_FRAME;
}

/* ---------------------------------------------------------------------------------------------- */

/* map horizontal cursor movement distance to frequency delta */
int watrview_map_cursormovex(watrview_ctx_t *ctx, int dx, double *out_df)
{
	if(!ctx->buf.is_inited)
		return 0;

	*out_df = dx * (ctx->f_1 - ctx->f_0) / (ctx->buf.img_w - 1);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* free all gdi resources */
static void watrview_res_free(watrview_res_ctx_t *res)
{
	DeleteObject(res->hbr_window);
	DeleteObject(res->hbr_bkgnd);
	DeleteObject(res->hbr_scrbar);

	DeleteObject(res->hpen_ticks);
	DeleteObject(res->hpen_scrbar);
}

/* ---------------------------------------------------------------------------------------------- */

/* allocate all needed gdi resources */
static int watrview_res_init(watrview_res_ctx_t *res, watrview_cfg_t *cfg)
{
	if(res->is_inited)
	{
		watrview_res_free(res);
		res->is_inited = 0;
	}

	res->cr_window = GetSysColor(COLOR_BTNFACE);

	res->hbr_window = CreateSolidBrush(res->cr_window);
	res->hbr_bkgnd = CreateSolidBrush(cfg->cr_bkgnd);
	res->hbr_scrbar = CreateSolidBrush(cfg->cr_scrbar);

	res->hpen_ticks = CreatePen(PS_SOLID, 1, cfg->cr_ticks);
	res->hpen_scrbar = CreatePen(PS_SOLID, 1, cfg->cr_scrbar);

	if( (res->hbr_window == NULL) || (res->hbr_bkgnd == NULL) ||
		(res->hbr_scrbar == NULL) || (res->hpen_scrbar == NULL) ||
		(res->hpen_ticks == NULL) )
	{
		watrview_res_free(res);
		return 0;
	}

	res->is_inited = 1;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* reallocate all gdi resources and invalidate graphics */
int watrview_res_reinit(watrview_ctx_t *ctx)
{
	int status = 0;

	EnterCriticalSection(&(ctx->csec));

	status = watrview_res_init(&(ctx->res), &(ctx->cfg));

	watrview_crmap_update(ctx);

	ctx->buf.is_background_erased = 0;
	ctx->buf.is_img_copied = 0;
	ctx->time_scale.is_drawn = 0;
	ctx->scrbar.is_drawn = 0;

	LeaveCriticalSection(&(ctx->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize segment buffer */
static int watrview_seg_buf_init(watrview_ctx_t *ctx, HDC hdc, int chain_max_len, int seg_len)
{
	watrview_seg_buf_ctx_t *segbuf;

	segbuf = &(ctx->segbuf);

	segbuf->ctx = ctx;

	segbuf->hdc_src = CreateCompatibleDC(hdc);
	segbuf->hdc_out = CreateCompatibleDC(hdc);

	if((segbuf->hdc_src != NULL) && (segbuf->hdc_out != NULL))
	{
		if(chain_max_len > 0) {
			segbuf->chain_max_len = chain_max_len;
		} else {
			segbuf->chain_max_len = GetSystemMetrics(SM_CYSCREEN);
		}

		segbuf->seg_len = seg_len;

		segbuf->scroll_pos = 0;

		return 1;
	}

	DeleteDC(segbuf->hdc_out);
	DeleteDC(segbuf->hdc_src);

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* free segment buffer */
static void watrview_seg_buf_free(watrview_seg_buf_ctx_t *segbuf)
{
	watrview_seg_delete(segbuf, segbuf->head);

	DeleteDC(segbuf->hdc_out);
	DeleteDC(segbuf->hdc_src);
}

/* ---------------------------------------------------------------------------------------------- */

/* set maximum chain length and segment length */
int watrview_set_len(watrview_ctx_t *ctx, int chain_max_len, int seg_len)
{
	int chain_max_len_set;


	/* check parameters */
	if( ((chain_max_len != 0) &&
			((chain_max_len < WATRVIEW_CHAINMAXLEN_MIN) || (chain_max_len > WATRVIEW_CHAINMAXLEN_MAX))) ||
		(seg_len < WATRVIEW_SEGLEN_MIN) || (seg_len > WATRVIEW_SEGLEN_MAX) )
	{
		return 0;
	}

	EnterCriticalSection(&(ctx->csec));

	/* autoselect max chain length if requested */
	if(chain_max_len == 0) {
		chain_max_len_set = GetSystemMetrics(SM_CYSCREEN);
	} else {
		chain_max_len_set = chain_max_len;
	}

	/* check max chain length changed */
	if(chain_max_len_set != ctx->segbuf.chain_max_len)
	{
		ctx->segbuf.chain_max_len = chain_max_len_set;

		/* update scrollbar */
		watrview_scroll(ctx, 0);
		watrview_update_scrbar(ctx);
	}

	/* update segment length */
	ctx->segbuf.seg_len = seg_len;

	/* update settings */
	ctx->cfg.chain_max_len = chain_max_len;
	ctx->cfg.seg_len = seg_len;

	LeaveCriticalSection(&(ctx->csec));

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* free window buffer */
static void watrview_buf_free(watrview_buf_ctx_t *buf)
{
	free(buf->chanmap);

	free(buf->spect_j_buf);
	free(buf->spect_buf_db_temp);
	free(buf->spect_buf_db);

	DeleteDC(buf->hdc_buf);
	DeleteObject(buf->hbm_buf);
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize window buffer */
static int watrview_buf_init(watrview_buf_ctx_t *buf, HDC hdc, int win_w, int win_h)
{
	if(buf->is_inited)
	{
		if( (win_w == buf->win_w) && (win_h == buf->win_h) )
			return -1;

		watrview_buf_free(buf);
		buf->is_inited = 0;
	}

	buf->win_w = win_w;
	buf->win_h = win_h;

	buf->img_x = WATRVIEW_IMG_OFS_LEFT;
	buf->img_y = WATRVIEW_IMG_OFS_TOP;
	buf->img_w = win_w - WATRVIEW_FRAME_W;
	buf->img_h = win_h - WATRVIEW_FRAME_H;

	buf->scr_x = buf->win_w - WATRVIEW_IMG_OFS_RIGHT + 2;
	buf->scr_y = buf->img_y;
	buf->scr_w = WATRVIEW_IMG_OFS_RIGHT - 5;
	buf->scr_h = buf->img_h;

	buf->ts_x = 0;
	buf->ts_y = buf->img_y;
	buf->ts_w = WATRVIEW_IMG_OFS_LEFT;
	buf->ts_h = buf->img_h;

	buf->hbm_buf = CreateCompatibleBitmap(hdc, win_w, win_h);
	buf->hdc_buf = CreateCompatibleDC(hdc);

	buf->spect_buf_db = malloc(buf->img_w * sizeof(float));
	buf->spect_buf_db_temp = malloc(buf->img_w * sizeof(float));
	buf->spect_j_buf = malloc(buf->img_w * sizeof(int));

	buf->chanmap = malloc(buf->img_w);

	if( (buf->hbm_buf != NULL) && (buf->hdc_buf != NULL) &&
		(buf->spect_buf_db != NULL) && (buf->spect_buf_db_temp != NULL) &&
		(buf->spect_j_buf != NULL) && (buf->chanmap != NULL) )
	{
		if( SelectObject(buf->hdc_buf, buf->hbm_buf) != NULL )
		{
			buf->is_inited = 1;

			/* invalidate image */
			buf->is_background_erased = 0;
			buf->is_img_copied = 0;

			return 1;
		}
	}

	free(buf->chanmap);

	free(buf->spect_j_buf);
	free(buf->spect_buf_db_temp);
	free(buf->spect_buf_db);

	DeleteDC(buf->hdc_buf);
	DeleteObject(buf->hbm_buf);

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* set window size */
int watrview_set_size(watrview_ctx_t *ctx, HDC hdc, int win_w, int win_h)
{
	/* check parameters */
	if( (win_w < WATRVIEW_MIN_W) || (win_w > WATRVIEW_MAX_W) ||
		(win_h < WATRVIEW_MIN_H) || (win_h > WATRVIEW_MAX_H) )
	{
		return 0;
	}

	/* check window buffer reinit actually needed */
	if( ctx->buf.is_inited &&
		(win_w == ctx->buf.win_w) &&
		(win_h == ctx->buf.win_h) )
	{
		return -1;
	}

	EnterCriticalSection(&(ctx->csec));

	/* reinit window buffer */
	if(!watrview_buf_init(&(ctx->buf), hdc, win_w, win_h))
	{
		LeaveCriticalSection(&(ctx->csec));
		return 0;
	}

	/* update and redraw time scale and force redraw */
	watrview_update_time_scale(ctx);
	ctx->time_scale.is_drawn = 0;

	/* update scrollbar data and force redraw */
	watrview_scroll(ctx, 0);
	watrview_update_scrbar(ctx);
	ctx->scrbar.is_drawn = 0;

	/* force waterfall image redraw */
	ctx->buf.is_img_copied = 0;

	/* update channel map */
	watrview_set_chmap(ctx);

	LeaveCriticalSection(&(ctx->csec));

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* copy waterfall viewer image */
int watrview_copy(watrview_ctx_t *ctx, HDC hdc_dest, int x, int y)
{
	int status = 1;

	/* check parameters */
	if((x < 0) || (y < 0))
		return 0;

	/* check config */
	if(!ctx->buf.is_inited)
		return 0;

	EnterCriticalSection(&(ctx->csec));

	/* copy image */
	status = (int)BitBlt(
		hdc_dest, x, y, ctx->buf.win_w, ctx->buf.win_h,
		ctx->buf.hdc_buf, 0, 0, SRCCOPY);

	LeaveCriticalSection(&(ctx->csec));

	return status;
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize default waterfall viewer config */
void watrview_cfg_reset(watrview_cfg_t *cfg)
{
	/* initialize colors */
	cfg->cr_bkgnd = RGB(0, 0, 64);
	cfg->cr_label = RGB(0, 0, 0);
	cfg->cr_scrbar = RGB(0, 0, 0);
	cfg->cr_ticks = RGB(0, 0, 0);

	/* initialize gradient color reference points */
	cfg->cr_pt_count = 6;

	cfg->cr_pt[0].m = -20.0;
	cfg->cr_pt[0].cr_bkgnd = RGB(0, 0, 64);
	cfg->cr_pt[0].cr_chan = RGB(0, 32, 64);

	cfg->cr_pt[1].m = -10.0;
	cfg->cr_pt[1].cr_bkgnd = RGB(0, 0, 128);
	cfg->cr_pt[1].cr_chan = RGB(0, 64, 128);

	cfg->cr_pt[2].m = 0.0;
	cfg->cr_pt[2].cr_bkgnd = RGB(192, 0, 0);
	cfg->cr_pt[2].cr_chan = RGB(192, 0, 0);

	cfg->cr_pt[3].m = 10.0;
	cfg->cr_pt[3].cr_bkgnd = RGB(255, 128, 0);
	cfg->cr_pt[3].cr_chan = RGB(255, 128, 0);

	cfg->cr_pt[4].m = 20.0;
	cfg->cr_pt[4].cr_bkgnd = RGB(255, 255, 0);
	cfg->cr_pt[4].cr_chan = RGB(255, 255, 0);

	cfg->cr_pt[5].m = 30.0;
	cfg->cr_pt[5].cr_bkgnd = RGB(255, 255, 255);
	cfg->cr_pt[5].cr_chan = RGB(255, 255, 255);

	/* initialize settings */
	cfg->scale_mode = SPECSCALE_MAP_PEAK;
	cfg->framediv = 1;
	cfg->chain_max_len = 0;
	cfg->seg_len = 1000;
	cfg->use_chan_crmap = 1;
}

/* ---------------------------------------------------------------------------------------------- */

/* load waterfall viewer configuration */
void watrview_cfg_load(watrview_cfg_t *cfg, ini_sect_t *sect)
{
	int bufsize, i;
	TCHAR *buf, *p;

	/* load colors */
	ini_getcr(sect, _T("cr_bkgnd"), &(cfg->cr_bkgnd));
	ini_getcr(sect, _T("cr_label"), &(cfg->cr_label));
	ini_getcr(sect, _T("cr_ticks"), &(cfg->cr_ticks));
	ini_getcr(sect, _T("cr_scrbar"), &(cfg->cr_scrbar));

	/* load colormap reference points */
	bufsize = 16 * WATRVIEW_CR_PT_MAX;
	if( (buf = malloc(bufsize * sizeof(TCHAR))) != NULL )
	{
		/* read magnitudes line */
		if(ini_copys(sect, _T("cm_mag"), buf, bufsize))
		{
			/* parse magnitudes line */
			cfg->cr_pt_count = 0;
			p = _tcstok(buf, _T(","));
			while(p != NULL)
			{
				if(cfg->cr_pt_count == WATRVIEW_CR_PT_MAX)
					break;

				cfg->cr_pt[cfg->cr_pt_count].m = _tstof(p);
				cfg->cr_pt[cfg->cr_pt_count].cr_bkgnd = 0;
				cfg->cr_pt[cfg->cr_pt_count].cr_chan = 0;
				cfg->cr_pt_count++;

				p = _tcstok(NULL, _T(","));
			}

			/* read and parse background colors */
			if(ini_copys(sect, _T("cm_bkgnd"), buf, bufsize))
			{
				i = 0;
				p = _tcstok(buf, _T(","));
				while(p != NULL)
				{
					if(i == cfg->cr_pt_count)
						break;
					parse_rgb(p, &(cfg->cr_pt[i++].cr_bkgnd));
					p = _tcstok(NULL, _T(","));
				}
			}

			/* read and parse channel bandwidth colors */
			if(ini_copys(sect, _T("cm_chan"), buf, bufsize))
			{
				i = 0;
				p = _tcstok(buf, _T(","));
				while(p != NULL)
				{
					if(i == cfg->cr_pt_count)
						break;
					parse_rgb(p, &(cfg->cr_pt[i++].cr_chan));
					p = _tcstok(NULL, _T(","));
				}
			}

			/* verify magnitudes */
			for(i = 0; i < cfg->cr_pt_count; i++)
			{
				if( (cfg->cr_pt[i].m < WATRVIEW_CRMAP_MAGN_MIN) ||
					(cfg->cr_pt[i].m > WATRVIEW_CRMAP_MAGN_MAX) )
				{
					cfg->cr_pt_count = 0;
					break;
				}
			}
		}

		free(buf);
	}

	/* load settings */
	ini_gete(sect, _T("scale_mode"), &(cfg->scale_mode), SPECSCAL_MAP_MODE_COUNT);
	ini_getir(sect, _T("framediv"), &(cfg->framediv),
		WATRVIEW_FRAMEDIV_MIN, WATRVIEW_FRAMEDIV_MAX);
	ini_getir(sect, _T("chain_max_len"), &(cfg->chain_max_len),
		WATRVIEW_CHAINMAXLEN_MIN, WATRVIEW_CHAINMAXLEN_MAX);
	ini_getir(sect, _T("seg_len"), &(cfg->seg_len),
		WATRVIEW_SEGLEN_MIN, WATRVIEW_SEGLEN_MAX);
	ini_getb(sect, _T("bkgnd_map"), &(cfg->use_chan_crmap));
}

/* ---------------------------------------------------------------------------------------------- */

/* save waterfall viewer configuration */
void watrview_cfg_save(ini_sect_t *sect, const watrview_cfg_t *cfg)
{
	TCHAR *buf, *p;
	int i, bufsize;

	/* save colors */
	ini_setcr(sect, _T("cr_bkgnd"), cfg->cr_bkgnd);
	ini_setcr(sect, _T("cr_label"), cfg->cr_label);
	ini_setcr(sect, _T("cr_ticks"), cfg->cr_ticks);
	ini_setcr(sect, _T("cr_scrbar"), cfg->cr_scrbar);

	/* save colormap */
	if(cfg->cr_pt_count <= 0)
	{
		ini_set(sect, _T("cm_mag"), _T(""));
		ini_set(sect, _T("cm_chan"), _T(""));
		ini_set(sect, _T("cm_bkgnd"), _T(""));
	}
	else
	{
		bufsize = 16 * cfg->cr_pt_count;
		if( (buf = malloc(bufsize * sizeof(TCHAR))) != NULL )
		{
			/* save magnitudes */
			p = buf;
			for(i = 0; i < cfg->cr_pt_count; i++)
			{
				if(i != 0) *(p++) = ',';
				fmt_dbl(p, 16, cfg->cr_pt[i].m, 0, 6, 1);
				p += _tcslen(p);
			}
			ini_set(sect, _T("cm_mag"), buf);

			/* save background colors */
			p = buf;
			for(i = 0; i < cfg->cr_pt_count; i++)
			{
				if(i != 0) *(p++) = ',';
				fmt_rgb(p, cfg->cr_pt[i].cr_bkgnd);
				p += _tcslen(p);
			}
			ini_set(sect, _T("cm_bkgnd"), buf);

			/* save channel bandwidth colors */
			p = buf;
			for(i = 0; i < cfg->cr_pt_count; i++)
			{
				if(i != 0) *(p++) = ',';
				fmt_rgb(p, cfg->cr_pt[i].cr_chan);
				p += _tcslen(p);
			}
			ini_set(sect, _T("cm_chan"), buf);

			free(buf);
		}
	}

	/* save settings */
	ini_seti(sect, _T("scale_mode"), cfg->scale_mode);
	ini_seti(sect, _T("framediv"), cfg->framediv);
	ini_seti(sect, _T("chain_max_len"), cfg->chain_max_len);
	ini_seti(sect, _T("seg_len"), cfg->seg_len);
	ini_setb(sect, _T("bkgnd_map"), cfg->use_chan_crmap);
}

/* ---------------------------------------------------------------------------------------------- */

/* set viewer configuration */
int watrview_cfg_set(watrview_ctx_t *ctx, const watrview_cfg_t *cfg_new, int *p_update)
{
	int status = 1, need_update = 0;

	/* Check for color configuration changes */
	if( (cfg_new->cr_bkgnd != ctx->cfg.cr_bkgnd) || 
		(cfg_new->cr_label != ctx->cfg.cr_label) || 
		(cfg_new->cr_ticks != ctx->cfg.cr_ticks) || 
		(cfg_new->cr_scrbar != ctx->cfg.cr_scrbar) || 
		(cfg_new->cr_pt_count != ctx->cfg.cr_pt_count) || 
		(memcmp(cfg_new->cr_pt, ctx->cfg.cr_pt,
			cfg_new->cr_pt_count * sizeof(watrview_cr_map_pt_t)) != 0) )
	{
		/* Set new color configuration */
		ctx->cfg.cr_bkgnd = cfg_new->cr_bkgnd;
		ctx->cfg.cr_label = cfg_new->cr_label;
		ctx->cfg.cr_ticks = cfg_new->cr_ticks;
		ctx->cfg.cr_scrbar = cfg_new->cr_scrbar;
		ctx->cfg.cr_pt_count = cfg_new->cr_pt_count;
		memcpy(ctx->cfg.cr_pt, cfg_new->cr_pt, sizeof(ctx->cfg.cr_pt));

		/* Reinit GDI resources */
		if(!watrview_res_reinit(ctx))
			status = 0;

		need_update = 1;
	}

	/* Check for buffer length changed */
	if( (cfg_new->chain_max_len != ctx->cfg.chain_max_len) ||
		(cfg_new->seg_len != ctx->cfg.seg_len) )
	{
		/* Update buffer segments */
		if(!watrview_set_len(ctx, cfg_new->chain_max_len, cfg_new->seg_len))
			status = 0;

		need_update = 1;
	}

	/* Check for other parameters change (no update needed) */
	ctx->cfg.scale_mode = cfg_new->scale_mode;
	ctx->cfg.framediv = cfg_new->framediv;

	if(p_update)
		*p_update = need_update;

	return status;

}

/* ---------------------------------------------------------------------------------------------- */

/* initialize waterfall viewer */
watrview_ctx_t *watrview_init(HDC hdc, HFONT hfont,
							  rxstate_t *rx, double f_0, double f_1,
							  int win_w, int win_h,
							  const watrview_cfg_t *cfg_init)
{
	watrview_ctx_t *ctx;

	/* check parameters */
	if( (win_w < WATRVIEW_MIN_W) || (win_w > WATRVIEW_MAX_W) ||
		(win_h < WATRVIEW_MIN_H) || (win_h > WATRVIEW_MAX_H) ||
		(f_0 < RXLIM_FC_MIN) || (f_1 > RXLIM_FC_MAX) || (f_1 <= f_0) )
	{
		return 0;
	}

	/* allocate structure */
	if( (ctx = calloc(1, sizeof(watrview_ctx_t))) != NULL )
	{
		/* allocate memory */
		ctx->cr_map.cr_map_chan = malloc(WATRVIEW_CR_MAP_LEN * sizeof(DWORD));
		ctx->cr_map.cr_map_bkgnd = malloc(WATRVIEW_CR_MAP_LEN * sizeof(DWORD));
		ctx->cr_map.cr_pt_temp = malloc(WATRVIEW_CR_PT_MAX * sizeof(watrview_cr_map_pt_t));
		InitializeCriticalSection(&(ctx->csec));

		if( (ctx->cr_map.cr_map_chan != NULL) &&
			(ctx->cr_map.cr_map_bkgnd != NULL) &&
			(ctx->cr_map.cr_pt_temp != NULL) )
		{
			/* initialize data */
			ctx->rx = rx;
			ctx->f_0 = f_0;
			ctx->f_1 = f_1;
			ctx->hfont = hfont;

			/* initialize config */
			memcpy(&(ctx->cfg), cfg_init, sizeof(ctx->cfg));

			/* allocate all gdi resources */
			if(watrview_res_init(&(ctx->res), &(ctx->cfg)))
			{
				/* allocate window buffer */
				if(watrview_buf_init(&(ctx->buf), hdc, win_w, win_h))
				{
					/* initialize segment buffer */
					if(watrview_seg_buf_init(ctx, hdc, ctx->cfg.chain_max_len, ctx->cfg.seg_len))
					{
						/* update graphics data */
						watrview_update_time_scale(ctx);
						watrview_update_scrbar(ctx);
						watrview_set_chmap(ctx);
						watrview_crmap_update(ctx);

						return ctx;
					}

					/* free window buffer */
					watrview_buf_free(&(ctx->buf));
				}

				/* free all gdi resources */
				watrview_res_free(&(ctx->res));
			}
		}

		/* cleanup */
		free(ctx->cr_map.cr_pt_temp);
		free(ctx->cr_map.cr_map_bkgnd);
		free(ctx->cr_map.cr_map_chan);
		DeleteCriticalSection(&(ctx->csec));
		free(ctx);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* free waterfall viewer */
void watrview_cleanup(watrview_ctx_t *ctx)
{
	if(ctx != NULL)
	{
		/* free time scale label buffer */
		free(ctx->time_scale.label);

		/* free waterfall image segments */
		watrview_seg_buf_free(&(ctx->segbuf));

		/* free window buffer */
		if(ctx->buf.is_inited) {
			watrview_buf_free(&(ctx->buf));
		}

		/* free all gdi resources */
		if(ctx->res.is_inited) {
			watrview_res_free(&(ctx->res));
		}

		/* cleanup */
		free(ctx->cr_map.cr_pt_temp);
		free(ctx->cr_map.cr_map_bkgnd);
		free(ctx->cr_map.cr_map_chan);
		DeleteCriticalSection(&(ctx->csec));
		free(ctx);
	}
}

/* ---------------------------------------------------------------------------------------------- */
