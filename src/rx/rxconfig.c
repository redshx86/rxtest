/* ---------------------------------------------------------------------------------------------- */

#include "rxconfig.h"

/* ---------------------------------------------------------------------------------------------- */

int rxconfig_init(rxconfig_t *cfg)
{
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

void rxconfig_cleanup(rxconfig_t *cfg)
{
}

/* ---------------------------------------------------------------------------------------------- */

void rxconfig_set_defaults(rxconfig_t *cfg)
{
	/* input headers */
	cfg->input_hdr_count				= 20;
	cfg->input_hdr_ms					= 5.0;

	/* processing channel buffers */
	cfg->proc_buf_len_ms				= 50.0;
	cfg->proc_in_whyst_ms				= 30.0;
	cfg->proc_out_rhyst_ms				= 30.0;
	cfg->proc_workbuf_ms				= 5.0;

	/* baseband */
	cfg->base_fsmin						= 32000;
	cfg->base_fsmax						= 48000;

	/* processing channel level meters */
	cfg->proc_level_num_points			= 100;
	cfg->proc_level_update_int			= 25.0;

	/* audio output */
	cfg->output_static_gain				= -6;
	cfg->output_resamp_as				= 60.0;
	cfg->output_resamp_df				= 2500;
	cfg->output_lim_thres				= 0.6;
	cfg->output_lim_range				= 2.0;
	cfg->output_hdr_cnt					= 10;
	cfg->output_hdr_ms					= 20.0;
	cfg->output_device_id				= -1; /* mapper */
	cfg->output_fs						= 48000;
	cfg->output_bps						= 16;
	cfg->output_levelmtr_num_points		= 100;
	cfg->output_levelmtr_tick_interval	= 25.0;
	cfg->output_gain					= 0.0;

	/* spectrum analyzer */
	cfg->spect_ups_req					= 15;
	cfg->spect_length					= 65536;
	cfg->spect_bufcount					= 4;
	cfg->spect_wndtype					= 1; /* hamming */
	cfg->spect_wndarg					= 0;
	cfg->spect_magref					= 1.0;
}

/* ---------------------------------------------------------------------------------------------- */

void rxconfig_copy(rxconfig_t *dst, const rxconfig_t *src)
{
	/* input headers */
	dst->input_hdr_count				= src->input_hdr_count;
	dst->input_hdr_ms					= src->input_hdr_ms;

	/* processing channel buffers */
	dst->proc_buf_len_ms				= src->proc_buf_len_ms;
	dst->proc_in_whyst_ms				= src->proc_in_whyst_ms;
	dst->proc_out_rhyst_ms				= src->proc_out_rhyst_ms;
	dst->proc_workbuf_ms				= src->proc_workbuf_ms;

	/* baseband */
	dst->base_fsmin						= src->base_fsmin;
	dst->base_fsmax						= src->base_fsmax;

	/* processing channel level meters */
	dst->proc_level_num_points			= src->proc_level_num_points;
	dst->proc_level_update_int			= src->proc_level_update_int;

	/* audio output */
	dst->output_static_gain				= src->output_static_gain;
	dst->output_resamp_as				= src->output_resamp_as;
	dst->output_resamp_df				= src->output_resamp_df;
	dst->output_lim_thres				= src->output_lim_thres;
	dst->output_lim_range				= src->output_lim_range;
	dst->output_hdr_cnt					= src->output_hdr_cnt;
	dst->output_hdr_ms					= src->output_hdr_ms;
	dst->output_device_id				= src->output_device_id;
	dst->output_fs						= src->output_fs;
	dst->output_bps						= src->output_bps;
	dst->output_levelmtr_num_points		= src->output_levelmtr_num_points;
	dst->output_levelmtr_tick_interval	= src->output_levelmtr_tick_interval;
	dst->output_gain					= src->output_gain;

	/* spectrum analyzer */
	dst->spect_ups_req					= src->spect_ups_req;
	dst->spect_length					= src->spect_length;
	dst->spect_bufcount					= src->spect_bufcount;
	dst->spect_wndtype					= src->spect_wndtype;
	dst->spect_wndarg					= src->spect_wndarg;
	dst->spect_magref					= src->spect_magref;
}

/* ---------------------------------------------------------------------------------------------- */

void rxconfig_load(rxconfig_t *cfg, ini_data_t *ini)
{
	ini_sect_t *sect;

	/* input headers */
	sect = ini_sect_get(ini, _T("input"), 0);
	ini_getusr(sect, _T("hdr_count"), &(cfg->input_hdr_count),
		RXLIM_INPUT_HDR_COUNT_MIN, RXLIM_INPUT_HDR_COUNT_MAX);
	ini_getfr(sect, _T("hdr_ms"), &(cfg->input_hdr_ms),
		RXLIM_INPUT_HDR_MS_MIN, RXLIM_INPUT_HDR_MS_MAX);

	/* processing channel */
	sect = ini_sect_get(ini, _T("proc"), 0);

	/* processing channel buffers */
	ini_getfr(sect, _T("buf_len_ms"), &(cfg->proc_buf_len_ms),
		RXLIM_PROC_BUF_LEN_MS_MIN, RXLIM_PROC_BUF_LEN_MS_MAX);
	ini_getfr(sect, _T("in_whyst_ms"), &(cfg->proc_in_whyst_ms),
		RXLIM_PROC_BUF_LEN_MS_MIN, RXLIM_PROC_BUF_LEN_MS_MAX);
	ini_getfr(sect, _T("out_rhyst_ms"), &(cfg->proc_out_rhyst_ms),
		RXLIM_PROC_BUF_LEN_MS_MIN, RXLIM_PROC_BUF_LEN_MS_MAX);
	ini_getfr(sect, _T("workbuf_ms"), &(cfg->proc_workbuf_ms),
		RXLIM_PROC_WORKBUF_MS_MIN, RXLIM_PROC_WORKBUF_MS_MAX);

	if(cfg->proc_in_whyst_ms > cfg->proc_buf_len_ms)
		cfg->proc_in_whyst_ms = cfg->proc_buf_len_ms;
	if(cfg->proc_out_rhyst_ms > cfg->proc_buf_len_ms)
		cfg->proc_out_rhyst_ms = cfg->proc_buf_len_ms;

	/* processing channel baseband sampling frequency */
	ini_getur(sect, _T("base_fsmin"), &(cfg->base_fsmin),
		RXLIM_BASE_FS_MINABS, RXLIM_BASE_FS_MAXABS);
	ini_getur(sect, _T("base_fsmax"), &(cfg->base_fsmax),
		RXLIM_BASE_FS_MINABS, RXLIM_BASE_FS_MAXABS);

	if(cfg->base_fsmax < cfg->base_fsmin)
		cfg->base_fsmax = cfg->base_fsmin;

	/* processing channel level meters */
	ini_getusr(sect, _T("level_num_points"), &(cfg->proc_level_num_points),
		RXLIM_PROC_LEVEL_NUM_POINTS_MIN, RXLIM_PROC_LEVEL_NUM_POINTS_MAX);
	ini_getfr(sect, _T("level_update_int"), &(cfg->proc_level_update_int),
		RXLIM_PROC_LEVEL_UPDATE_INT_MIN, RXLIM_PROC_LEVEL_UPDATE_INT_MAX);

	/* audio output */
	sect = ini_sect_get(ini, _T("audio"), 0);
	ini_getfr(sect, _T("static_gain"), &(cfg->output_static_gain),
		RXLIM_OUTPUT_STATIC_GAIN_MIN, RXLIM_OUTPUT_STATIC_GAIN_MAX);
	ini_getur(sect, _T("resamp_df"), &(cfg->output_resamp_df),
		RXLIM_OUTPUT_RESAMP_DF_MIN, RXLIM_OUTPUT_RESAMP_DF_MAX);
	ini_getfr(sect, _T("resamp_as"), &(cfg->output_resamp_as),
		RXLIM_OUTPUT_RESAMP_AS_MIN, RXLIM_OUTPUT_RESAMP_AS_MAX);
	ini_getfr(sect, _T("lim_thres"), &(cfg->output_lim_thres),
		RXLIM_OUTPUT_LIM_THRES_MIN, RXLIM_OUTPUT_LIM_THRES_MAX);
	ini_getfr(sect, _T("lim_range"), &(cfg->output_lim_range),
		RXLIM_OUTPUT_LIM_RANGE_MIN, RXLIM_OUTPUT_LIM_RANGE_MAX);
	ini_getusr(sect, _T("hdr_cnt"), &(cfg->output_hdr_cnt),
		RXLIM_OUTPUT_HDR_CNT_MIN, RXLIM_OUTPUT_HDR_CNT_MAX);
	ini_getfr(sect, _T("hdr_ms"), &(cfg->output_hdr_ms),
		RXLIM_OUTPUT_HDR_MS_MIN, RXLIM_OUTPUT_HDR_MS_MAX);
	ini_getir(sect, _T("device_id"), &(cfg->output_device_id),
		-1, RXLIM_OUTPUT_MAX_NUM_DEVS - 1);
	ini_getur(sect, _T("fs"), &(cfg->output_fs),
		RXLIM_OUTPUT_FS_MIN, RXLIM_OUTPUT_FS_MAX);
	ini_getu(sect, _T("bps"), &(cfg->output_bps));
	if((cfg->output_bps != 8) && (cfg->output_bps != 16))
		cfg->output_bps = 16;
	ini_getusr(sect, _T("levelmtr_num_points"), &(cfg->output_levelmtr_num_points),
		RXLIM_OUTPUT_LEVELMTR_NUM_POINTS_MIN, RXLIM_OUTPUT_LEVELMTR_NUM_POINTS_MAX);
	ini_getfr(sect, _T("levelmtr_tick_interval"), &(cfg->output_levelmtr_tick_interval),
		RXLIM_OUTPUT_LEVELMTR_TICK_INTERVAL_MIN, RXLIM_OUTPUT_LEVELMTR_TICK_INTERVAL_MAX);
	ini_getfr(sect, _T("gain"), &(cfg->output_gain), 
		RXLIM_AUDIO_GAIN_MIN, RXLIM_AUDIO_GAIN_MAX);

	/* spectrum analyzer */
	sect = ini_sect_get(ini, _T("spect"), 0);
	ini_getur(sect, _T("ups_req"), &(cfg->spect_ups_req),
		RXLIM_SPECT_UPS_REQ_MIN, RXLIM_SPECT_UPS_REQ_MAX);
	ini_getusr(sect, _T("length"), &(cfg->spect_length),
		RXLIM_SPECT_LENGTH_MIN, RXLIM_SPECT_LENGTH_MAX);
	if( (cfg->spect_length & (cfg->spect_length - 1)) != 0 )
		cfg->spect_length = 65536;
	ini_getusr(sect, _T("buf_count"), &(cfg->spect_bufcount),
		RXLIM_SPECT_BUF_COUNT_MIN, RXLIM_SPECT_BUF_COUNT_MAX);
	ini_geti(sect, _T("wndtype"), &(cfg->spect_wndtype));
	ini_getf(sect, _T("wndarg"), &(cfg->spect_wndarg));
	ini_getf(sect, _T("magref"), &(cfg->spect_magref));
}

/* ---------------------------------------------------------------------------------------------- */

void rxconfig_save(ini_data_t *ini, const rxconfig_t *cfg)
{
	ini_sect_t *sect;

	/* input headers */
	sect = ini_sect_get(ini, _T("input"), 1);
	ini_setus(sect, _T("hdr_count"), cfg->input_hdr_count);
	ini_setf(sect, _T("hdr_ms"), 6, cfg->input_hdr_ms);

	/* processing channel */
	sect = ini_sect_get(ini, _T("proc"), 1);

	/* processing channel buffers */
	ini_setf(sect, _T("buf_len_ms"), 6, cfg->proc_buf_len_ms);
	ini_setf(sect, _T("in_whyst_ms"), 6, cfg->proc_in_whyst_ms);
	ini_setf(sect, _T("out_rhyst_ms"), 6, cfg->proc_out_rhyst_ms);
	ini_setf(sect, _T("workbuf_ms"), 6, cfg->proc_workbuf_ms);

	/* processing channel baseband sampling frequency */
	ini_setu(sect, _T("base_fsmin"), cfg->base_fsmin);
	ini_setu(sect, _T("base_fsmax"), cfg->base_fsmax);

	/* processing channel level meters */
	ini_setus(sect, _T("level_num_points"), cfg->proc_level_num_points);
	ini_setf(sect, _T("level_update_int"), 6, cfg->proc_level_update_int);

	/* audio output */
	sect = ini_sect_get(ini, _T("audio"), 1);
	ini_setf(sect, _T("static_gain"), 6, cfg->output_static_gain);
	ini_setu(sect, _T("resamp_df"), cfg->output_resamp_df);
	ini_setf(sect, _T("resamp_as"), 6, cfg->output_resamp_as);
	ini_setf(sect, _T("lim_thres"), 6, cfg->output_lim_thres);
	ini_setf(sect, _T("lim_range"), 6, cfg->output_lim_range);
	ini_setus(sect, _T("hdr_cnt"), cfg->output_hdr_cnt);
	ini_setf(sect, _T("hdr_ms"), 6, cfg->output_hdr_ms);
	ini_seti(sect, _T("device_id"), cfg->output_device_id);
	ini_setu(sect, _T("fs"), cfg->output_fs);
	ini_setu(sect, _T("bps"), cfg->output_bps);
	ini_setus(sect, _T("levelmtr_num_points"), cfg->output_levelmtr_num_points);
	ini_setf(sect, _T("levelmtr_tick_interval"), 6, cfg->output_levelmtr_tick_interval);
	ini_setf(sect, _T("gain"), 6, cfg->output_gain);

	/* spectrum analyzer */
	sect = ini_sect_get(ini, _T("spect"), 1);
	ini_setu(sect, _T("ups_req"), cfg->spect_ups_req);
	ini_setus(sect, _T("length"), cfg->spect_length);
	ini_setus(sect, _T("buf_count"), cfg->spect_bufcount);
	ini_seti(sect, _T("wndtype"), cfg->spect_wndtype);
	ini_setf(sect, _T("wndarg"), 6, cfg->spect_wndarg);
	ini_setf(sect, _T("magref"), 6, cfg->spect_magref);
}

/* ---------------------------------------------------------------------------------------------- */
