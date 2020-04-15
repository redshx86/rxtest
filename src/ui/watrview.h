/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "uicommon.h"
#include "specscal.h"
#include "colormap.h"
#include "../util/list.h"
#include "../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

#define WATRVIEW_FRAMEDIV_MIN				1
#define WATRVIEW_FRAMEDIV_MAX				100

#define WATRVIEW_CHAINMAXLEN_MIN			100
#define WATRVIEW_CHAINMAXLEN_MAX			1000000

#define WATRVIEW_SEGLEN_MIN					32
#define WATRVIEW_SEGLEN_MAX					65535

#define WATRVIEW_CRMAP_MAGN_MIN				-120.0
#define WATRVIEW_CRMAP_MAGN_MAX				60.0

/* ---------------------------------------------------------------------------------------------- */

#define WATRVIEW_CR_PT_MAX					64
#define WATRVIEW_CR_MAP_LEN					1024

/* ---------------------------------------------------------------------------------------------- */

#define WATRVIEW_TIME_GAUGE_MIN_LABEL_H		15

/* ---------------------------------------------------------------------------------------------- */

#define WATRVIEW_IMG_OFS_LEFT				35
#define WATRVIEW_IMG_OFS_RIGHT				10
#define WATRVIEW_IMG_OFS_TOP				0
#define WATRVIEW_IMG_OFS_BOTTOM				0

#define WATRVIEW_FRAME_W					(WATRVIEW_IMG_OFS_LEFT + WATRVIEW_IMG_OFS_RIGHT)
#define WATRVIEW_FRAME_H					(WATRVIEW_IMG_OFS_TOP + WATRVIEW_IMG_OFS_BOTTOM)

#define WATRVIEW_MIN_W						(WATRVIEW_FRAME_W + 2)
#define WATRVIEW_MIN_H						(WATRVIEW_FRAME_H + 2)
#define WATRVIEW_MAX_W						65535
#define WATRVIEW_MAX_H						65535

/* ---------------------------------------------------------------------------------------------- */

/* colormap reference point */
typedef struct watrview_cr_map_pt {
	double m;					/* magnitude, dB */
	COLORREF cr_bkgnd;			/* background color at magnitude */
	COLORREF cr_chan;			/* channel bandwidth color at magnitude */
	int priority;				/* <internal use> */
} watrview_cr_map_pt_t;

/* ---------------------------------------------------------------------------------------------- */

/* waterfall viewer config */
typedef struct watrview_cfg {

	/* image element colors */
	COLORREF cr_bkgnd;
	COLORREF cr_label;
	COLORREF cr_ticks;
	COLORREF cr_scrbar;

	/* colormap ref pointes */
	int cr_pt_count;
	watrview_cr_map_pt_t cr_pt[WATRVIEW_CR_PT_MAX];

	/* settings */
	specscale_map_mode_t scale_mode;
	int framediv;
	int chain_max_len;
	int seg_len;
	int use_chan_crmap;

} watrview_cfg_t;

/* ---------------------------------------------------------------------------------------------- */

/* rasterized colormap context */
typedef struct watrview_cr_map_ctx {

	int is_inited;				/* colormap is valid */

	double m_0;					/* min magnitude */
	double m_1;					/* max magnitude */

	COLORREF *cr_map_chan;		/* channel colormap rasterized */
	COLORREF *cr_map_bkgnd;		/* background colormap rasterized */

	watrview_cr_map_pt_t *cr_pt_temp;	/* temp ref point buffer for sorting */

} watrview_cr_map_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* gdi objects structure */
typedef struct watrview_res_ctx {

	int is_inited;			/* all objects are allocated */

	COLORREF cr_window;

	HBRUSH hbr_window;
	HBRUSH hbr_bkgnd;
	HBRUSH hbr_scrbar;

	HPEN hpen_ticks;
	HPEN hpen_scrbar;

} watrview_res_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* waterfall image per-line data */
typedef struct watrview_line_data {

	double t_frame;				/* input time per line, s */

	double f_0;					/* lower frequency, Hz */
	double f_1;					/* upper frequency, Hz */

} watrview_line_data_t;

/* ---------------------------------------------------------------------------------------------- */

/* waterfall image segment */
typedef struct watrview_seg {

	/* segment buffer pointer */
	struct watrview_seg_buf_ctx *segbuf;

	/* segment chain pointers */
	struct watrview_seg *next;	/* pointer to older segment */
	struct watrview_seg *prev;	/* pointer to newer segment */

	int len;					/* length (i.e. image height) of buffers */

	int data_len;				/* number of data lines */

	int src_w;					/* source buffer width */
	HBITMAP hbm_src;			/* source buffer bitmap */
	watrview_line_data_t *src_data;	/* source buffer per-line data */

	int out_w;					/* output buffer width */
	double out_f_0;				/* output buffer lower display freq */
	double out_f_1;				/* output buffer upper display freq */
	HBITMAP hbm_out;			/* output buffer bitmap */

} watrview_seg_t;

/* ---------------------------------------------------------------------------------------------- */

/* segment buffer */
typedef struct watrview_seg_buf_ctx {

	struct watrview_ctx *ctx;	/* viewer context */

	int chain_max_len;			/* max chain data length */
	int seg_len;				/* data length per segment */

	int scroll_pos;				/* scroll position */

	/* segment chain pointers */
	watrview_seg_t *head;		/* neweset segment */
	watrview_seg_t *tail;		/* oldest segment */

	/* memory dcs for segment manipulation */
	HDC hdc_src;
	HDC hdc_out;

} watrview_seg_buf_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* channel map point type */
enum {
	WATRVIEW_MAP_BKGND,
	WATRVIEW_MAP_CHAN_FC,
	WATRVIEW_MAP_CHAN_BW
};

/* ---------------------------------------------------------------------------------------------- */

/* window buffer */
typedef struct watrview_buf_ctx {

	/* buffer successfully allocated */
	int is_inited;

	/* window size */
	int win_w;
	int win_h;

	/* time_scale position and size */
	int ts_x;
	int ts_y;
	int ts_w;
	int ts_h;

	/* scrollbar position and size */
	int scr_x;
	int scr_y;
	int scr_w;
	int scr_h;

	/* waterfall image position and size */
	int img_x;
	int img_y;
	int img_w;
	int img_h;

	/* window image buffer */
	HDC hdc_buf;
	HBITMAP hbm_buf;

	/* data buffers */
	float *spect_buf_db_temp;
	float *spect_buf_db;
	size_t spect_framelen_total;
	int *spect_j_buf;

	/* channel map */
	unsigned char *chanmap;

	/* update flags */
	int is_background_erased;
	int is_img_copied;

} watrview_buf_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* time_scale label description */
typedef struct watrview_time_scale_label {

	int position;
	int value;

} watrview_time_scale_label_t;

/* ---------------------------------------------------------------------------------------------- */

/* time scale data */
typedef struct watrview_time_scale_ctx {

	int is_inited;
	int is_drawn;

	int label_capacity;
	int label_count;
	watrview_time_scale_label_t *label;

} watrview_time_scale_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* scrollbar data */
typedef struct watrview_scrbar_ctx {

	int is_inited;
	int is_drawn;

	int slider_len;
	int slider_pos;
	int height;

	int autoscroll_force;
	int autoscroll_inh;

} watrview_scrbar_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* waterfall viewer context */
typedef struct watrview_ctx {

	CRITICAL_SECTION csec;

	rxstate_t *rx;

	double f_0;
	double f_1;

	watrview_cfg_t cfg;
	watrview_res_ctx_t res;
	watrview_buf_ctx_t buf;
	watrview_seg_buf_ctx_t segbuf;
	watrview_cr_map_ctx_t cr_map;
	watrview_scrbar_ctx_t scrbar;
	watrview_time_scale_ctx_t time_scale;

	int framediv_ctr;

	HFONT hfont;

} watrview_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* waterfall viewer element type 
    returned by watrview_map_point */
typedef enum watrview_point_type {
	WATRVIEW_POINT_OUTOFRANGE,
	WATRVIEW_POINT_IMAGE,
	WATRVIEW_POINT_SCRBAR,
	WATRVIEW_POINT_FRAME
} watrview_point_type_t;

/* ---------------------------------------------------------------------------------------------- */

/* initialize default waterfall viewer config */
void watrview_cfg_reset(watrview_cfg_t *cfg);

/* load waterfall viewer configuration */
void watrview_cfg_load(watrview_cfg_t *cfg, ini_sect_t *sect);

/* save waterfall viewer configuration */
void watrview_cfg_save(ini_sect_t *sect, const watrview_cfg_t *cfg);

/* ---------------------------------------------------------------------------------------------- */

/* initialize waterfall viewer */
watrview_ctx_t *watrview_init(HDC hdc, HFONT hfont,
							  rxstate_t *rx, double f_0, double f_1,
							  int win_w, int win_h,
							  const watrview_cfg_t *cfg_init);

/* free waterfall viewer */
void watrview_cleanup(watrview_ctx_t *ctx);

/* ---------------------------------------------------------------------------------------------- */

/* set viewer configuration */
int watrview_cfg_set(watrview_ctx_t *ctx, const watrview_cfg_t *cfg_new, int *p_update);

/* set window size */
int watrview_set_size(watrview_ctx_t *ctx, HDC hdc, int win_w, int win_h);

/* set maximum chain length and segment length */
//int watrview_set_len(watrview_ctx_t *ctx, int chain_max_len, int seg_len);

/* scroll viewer by specified pixel count */
int watrview_scroll(watrview_ctx_t *ctx, int delta);

/* scroll viewer for specified scrollbar slider position */
int watrview_drag_scrbar(watrview_ctx_t *ctx, int slider_pos);

/* set current display frequency range */
int watrview_set_freq_range(watrview_ctx_t *ctx, double f_0, double f_1);

/* rebuild channel map */
int watrview_set_chmap(watrview_ctx_t *ctx);

/* append data to waterfall viewer */
int watrview_append(watrview_ctx_t *ctx, HDC hdc, double fc_input, unsigned int fs_input,
					float *fft_pwr_buf, size_t fft_pwr_buf_size, size_t input_framelen);

/* ---------------------------------------------------------------------------------------------- */

/* update all graphics */
int watrview_update(watrview_ctx_t *ctx, HDC hdc);

/* ---------------------------------------------------------------------------------------------- */

/* copy waterfall viewer image */
int watrview_copy(watrview_ctx_t *ctx, HDC hdc_dest, int x, int y);

/* ---------------------------------------------------------------------------------------------- */

/* determine element type located at pixel coordinates */
watrview_point_type_t watrview_map_point(watrview_ctx_t *ctx, int x, int y);

/* map horizontal cursor movement distance to frequency delta */
int watrview_map_cursormovex(watrview_ctx_t *ctx, int dx, double *out_df);

/* ---------------------------------------------------------------------------------------------- */

/* build colormap buffer from reference points and get magnitude range */
int watrview_crmap_build(watrview_cr_map_pt_t *cr_pt_buf, int cr_pt_count,
						 COLORREF *buf_bkgnd, COLORREF *buf_chan, int buf_len,
						 double *pm_0, double *pm_1);

/* ---------------------------------------------------------------------------------------------- */
