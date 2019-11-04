/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "uicommon.h"
#include "specscal.h"
#include "../rx/rxstate.h"

/* ---------------------------------------------------------------------------------------------- */

#define SPECVIEW_UPS_BUFSIZE		1024
#define SPECVIEW_UPS_MAXDELTA		5.0

#define SPECVIEW_MAGN_MINABS		-120.0
#define SPECVIEW_MAGN_MAXABS		60.0

#define SPECVIEW_MAG_LABEL_MIN_H	15
#define SPECVIEW_FREQ_LABEL_MIN_W	30

#define SPECVIEW_TEXT_MAXLEN		256

#define SPECVIEW_SQL_MARK_WIDTH		6

/* ---------------------------------------------------------------------------------------------- */

/* schematic frame dimensions */
#define SPECVIEW_SHM_OFS_LEFT		35
#define SPECVIEW_SHM_OFS_RIGHT		10
#define SPECVIEW_SHM_OFS_TOP		10
#define SPECVIEW_SHM_OFS_BOTTOM		35

#define SPECVIEW_FRAME_W			(SPECVIEW_SHM_OFS_LEFT + SPECVIEW_SHM_OFS_RIGHT)
#define SPECVIEW_FRAME_H			(SPECVIEW_SHM_OFS_TOP + SPECVIEW_SHM_OFS_BOTTOM)

/* minimum and maximum buffer dimensions */
#define SPECVIEW_MIN_W				(SPECVIEW_FRAME_W + 2)
#define SPECVIEW_MIN_H				(SPECVIEW_FRAME_H + 2)
#define SPECVIEW_MAX_W				65535
#define SPECVIEW_MAX_H				65535

/* ---------------------------------------------------------------------------------------------- */

/* #s of schematic colors */
enum specview_shm_color {

	SPECVIEW_COLOR_BKGND,
	SPECVIEW_COLOR_INPUT_FC,
	SPECVIEW_COLOR_CHAN_FC,
	SPECVIEW_COLOR_CHAN_BW,
	SPECVIEW_COLOR_CHAN_BWOVL,

	SPECVIEW_COLOR_BKGND_GRID,
	SPECVIEW_COLOR_INPUT_FC_GRID,
	SPECVIEW_COLOR_CHAN_FC_GRID,
	SPECVIEW_COLOR_CHAN_BW_GRID,
	SPECVIEW_COLOR_CHAN_BWOVL_GRID,

	SPECVIEW_COLOR_COUNT

} specview_shm_color_t;

/* ---------------------------------------------------------------------------------------------- */

/* spectrum viewer configuration */
typedef struct specview_cfg {

	double m_0;
	double m_1;

	COLORREF cr_label;
	COLORREF cr_ticks;
	COLORREF cr_text;
	COLORREF cr_textbg;

	COLORREF cr_sq_sense_op;
	COLORREF cr_sq_sense_cl;
	COLORREF cr_sq_thres_op;
	COLORREF cr_sq_thres_cl;

	COLORREF cr_shm_hot[SPECVIEW_COLOR_COUNT];
	COLORREF cr_shm_cold[SPECVIEW_COLOR_COUNT];

	specscale_map_mode_t scale_mode;

	int show_ups_counter;

} specview_cfg_t;

/* ---------------------------------------------------------------------------------------------- */

typedef enum specview_map_type {
	SPECVIEW_MAP_NOTHING,
	SPECVIEW_MAP_INPUT_FC,
	SPECVIEW_MAP_CHAN_FC,
	SPECVIEW_MAP_CHAN_BW_EDGE_LOWER,
	SPECVIEW_MAP_CHAN_BW_EDGE_UPPER
} specview_map_type_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct specview_chmap_point {
	specview_map_type_t type;
	unsigned int chid;
} specview_chmap_point_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct specview_buf_ctx {

	/* buffer initialization flag */
	int is_inited;

	/* buffer dimensions */
	int win_w, win_h;

	/* schematic dimensions */
	int shm_x, shm_y;
	int shm_w, shm_h;

	/* buffer bitmaps */
	HBITMAP hbm_buf;
	HBITMAP hbm_shm_src_cold;
	HBITMAP hbm_shm_src_hot;
	HBITMAP hbm_shm_mask;
	HBITMAP hbm_layer_text;
	HBITMAP hbm_layer_sq;
	HBITMAP hbm_mask_sq;

	/* buffer memory dcs */
	HDC hdc_buf;
	HDC hdc_shm_src_cold;
	HDC hdc_shm_src_hot;
	HDC hdc_shm_mask;
	HDC hdc_layer_text;
	HDC hdc_layer_sq;
	HDC hdc_mask_sq;

	/* channel map state */
	int is_chmap_inited;		/* channel map initialized */
	specview_chmap_point_t *chmap_buf;	/* channel map buffer */
	unsigned char *shm_cr_map;	/* schematic color map */
	unsigned char *shm_cr_map_actual;	/* schematic actual color map */

	/* schematics state */
	int is_shm_src_prepared;	/* schematic src images backgrounds erased and grid painted */
	int is_shm_src_updated;		/* schematic src images updated to current channel map */
	int is_shm_copied;			/* schematic copied to main buffer */

	/* spectrum buffers */
	float *spec_db_buf;			/* y-dB buffer (temp) */
	int *spec_j_buf;			/* y-pixel buffer (temp) */

} specview_buf_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* gdi resources */
typedef struct specview_res_ctx {

	int is_inited;

	HPEN hpen_stock_null;
	HPEN hpen_stock_black;
	HPEN hpen_stock_white;
	
	HPEN hpen_ticks;

	HPEN hpen_sq_sense_op;
	HPEN hpen_sq_sense_cl;
	HPEN hpen_sq_thres_op;
	HPEN hpen_sq_thres_cl;

	HPEN hpen_hot[SPECVIEW_COLOR_COUNT];
	HPEN hpen_cold[SPECVIEW_COLOR_COUNT];

	COLORREF cr_window;
	HBRUSH hbr_window;
	HBRUSH hbr_textbg;
	HBRUSH hbr_bkgnd_cold;
	HBRUSH hbr_bkgnd_hot;

} specview_res_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* magnitude scale state */
typedef struct specview_mag_scale_ctx {

	int is_inited;
	int is_drawn;

	double m_0;
	double m_1;
	double m_step;
	int height;

} specview_mag_scale_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* frequency scale display mode */
typedef enum specview_freq_scale_mode {
	SPECVIEW_FREQSCALE_HZ_KHZ,
	SPECVIEW_FREQSCALE_KHZ_MHZ,
	SPECVIEW_FREQSCALE_MHZ_GHZ
} specview_freq_scale_mode_t;

/* frequency scale state */
typedef struct specview_freq_scale_ctx {

	int is_inited;
	int is_drawn;

	double f_0;
	double f_1;
	double f_step;
	double f_coarse_step;
	specview_freq_scale_mode_t f_scale_mode;
	int width;

} specview_freq_scale_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* text layer state */
typedef struct specview_text_layer_ctx {

	TCHAR *buf;
	int init_x, init_y;
	int text_x, text_y;
	int text_w, text_h;
	int is_enabled;
	int is_changed;

	int cur_x, cur_y;
	int cur_w, cur_h;
	int is_drawn;

} specview_text_layer_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct specview_sql_entry {

	struct specview_sql_entry *l_next;
	struct specview_sql_entry *l_prev;

	unsigned int hash;
	struct specview_sql_entry *h_next;
	struct specview_sql_entry *h_prev;

	unsigned int chid;

	int pos, pos_drawn;

	int sense, sense_drawn;
	int thres_op, thres_op_drawn;
	int thres_cl, thres_cl_drawn;
	int is_open, is_open_drawn;
	int draw_sense, sense_is_drawn;

	int is_drawn;
	int is_deleted;

} specview_sql_entry_t;

/* ---------------------------------------------------------------------------------------------- */

#define SPECVIEW_SQL_HASHSIZE		64

typedef struct specview_sql_layer_ctx {

	specview_sql_entry_t *l_first;
	specview_sql_entry_t *l_last;
	specview_sql_entry_t *h_firsts[SPECVIEW_SQL_HASHSIZE];
	specview_sql_entry_t *h_lasts[SPECVIEW_SQL_HASHSIZE];

	int use_full_redraw;

	int is_inited;
	int is_updated;
	int is_copied;

} specview_sql_layer_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* spectrum viewer state */
typedef struct specview_ctx {

	CRITICAL_SECTION csec;

	rxstate_t *rx;

	double f_0;
	double f_1;

	specview_cfg_t cfg;
	specview_res_ctx_t res;
	specview_buf_ctx_t buf;
	specview_mag_scale_ctx_t mag;
	specview_freq_scale_ctx_t freq;
	specview_text_layer_ctx_t text;
	specview_sql_layer_ctx_t sql;

	DWORD *ups_buf;
	int ups_ptr;
	double ups;

	HFONT hfont;

} specview_ctx_t;

/* ---------------------------------------------------------------------------------------------- */

/* returned point type with specview_chmap_point */
typedef enum specview_point_type {
	SPECVIEW_POINT_OUTOFRANGE,
	SPECVIEW_POINT_FRAME,
	SPECVIEW_POINT_SHM,
	SPECVIEW_POINT_INPUT_FC,
	SPECVIEW_POINT_CHAN_FC,
	SPECVIEW_POINT_CHAN_BW_EDGE_LOWER,
	SPECVIEW_POINT_CHAN_BW_EDGE_UPPER,
	SPECVIEW_POINT_CHAN_SQL_OPEN_THRES,
	SPECVIEW_POINT_CHAN_SQL_CLOSE_THRES
} specview_point_type_t;

/* ---------------------------------------------------------------------------------------------- */

/* initialize spectrum viewer */
specview_ctx_t *specview_init(HDC hdc, HFONT hfont,
							  rxstate_t *rx, double f_0, double f_1,
							  int win_w, int win_h, ini_sect_t *sect);

/* clean up spectrum viewer */
void specview_cleanup(specview_ctx_t *ctx, ini_sect_t *sect);

/* ---------------------------------------------------------------------------------------------- */

/* reinit gdi resources after config change */
int specview_res_reinit(specview_ctx_t *ctx);

/* ---------------------------------------------------------------------------------------------- */

/* set new spectrum viewer size */
int specview_set_size(specview_ctx_t *ctx, HDC hdc, int win_w, int win_h);

/* set new frequency range (get end frequencies from config) */
int specview_set_freq_range(specview_ctx_t *ctx);

/* set new magnitude range (get end magnitudes from config) */
int specview_set_mag_range(specview_ctx_t *ctx);

/* set new channel map (reads channels from receiver state) */
int specview_set_chmap(specview_ctx_t *ctx);

/* set new sql marks data (reads from receiver state) */
int specview_set_sql(specview_ctx_t *ctx, int use_full_redraw, int only_sense);

/* set or delete text */
int specview_set_text(specview_ctx_t *ctx, TCHAR *text, int x, int y, int xm, int ym, int w, int h);

/* set new spectrum mask */
int specview_set_mask(specview_ctx_t *ctx, double fc_input, unsigned int fs_input,
					  float *fft_pwr_buf, size_t fft_pwr_buf_length);

/* clear spectrum mask */
int specview_clear_mask(specview_ctx_t *ctx);

/* ---------------------------------------------------------------------------------------------- */

/* update everything */
int specview_update(specview_ctx_t *ctx);

/* update text and copy changes directly to window */
int specview_update_text(specview_ctx_t *ctx, HDC hdc_copy, int copy_x, int copy_y);

/* ---------------------------------------------------------------------------------------------- */

/* copy main buffer to window */
int specview_copy(specview_ctx_t *ctx, HDC hdc, int x, int y);

/* ---------------------------------------------------------------------------------------------- */

/* map pixel coordinates to element type and channel id */
specview_point_type_t specview_chmap_point(specview_ctx_t *ctx, int x, int y, unsigned int *pchid);

/* map pixel distance to frequency offset */
int specview_map_dist_x(specview_ctx_t *ctx, int dx, double *out_df);

/* map pixel distance to magnitude offset */
int specview_map_dist_y(specview_ctx_t *ctx, int dy, double *out_dm);

/* ---------------------------------------------------------------------------------------------- */
