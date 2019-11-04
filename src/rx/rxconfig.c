/* ---------------------------------------------------------------------------------------------- */

#include "rxconfig.h"

/* ---------------------------------------------------------------------------------------------- */

static void rxconfig_set_defaults(rxconfig_t *p)
{
	/* input headers */
	p->input_hdr_count					= 20;
	p->input_hdr_ms						= 5.0;

	/* processing channel buffers */
	p->proc_buf_len_ms					= 50.0;
	p->proc_in_whyst_ms					= 30.0;
	p->proc_out_rhyst_ms				= 30.0;
	p->proc_workbuf_ms					= 5.0;

	/* baseband */
	p->base_fsmin						= 32000;
	p->base_fsmax						= 48000;

	/* processing channel decimator section */
	p->proc_decim_frgran				= 2501;
	p->proc_decim_mcsdf					= 4;
	p->proc_decim_dff					= 0.1;
	p->proc_decim_as					= 60.0;

	/* processing channel baseband filter */
	p->proc_filter_fc					= 10000.0;
	p->proc_filter_df					= 2500.0;
	p->proc_filter_as					= 60.0;

	/* processing channel level meters */
	p->proc_level_num_points			= 100;
	p->proc_level_update_int			= 25.0;

	/* demodulator and squelch defaults */
	rxproc_fm_cfg_reset(&(p->proc_fm_def));
	rxsql_cfg_reset(&(p->proc_sql_def));

	/* processing channel output section */
	p->proc_dcrem_alpha					= 0.025;

	/* audio output */
	p->output_static_gain				= -6;
	p->output_resamp_as					= 60.0;
	p->output_resamp_df					= 2500;
	p->output_lim_thres					= 0.6;
	p->output_lim_range					= 2.0;
	p->output_hdr_cnt					= 10;
	p->output_hdr_ms					= 20.0;
	p->output_device_id					= -1; /* mapper */
	p->output_fs						= 48000;
	p->output_bps						= 16;
	p->output_levelmtr_num_points		= 100;
	p->output_levelmtr_tick_interval	= 25.0;
	p->output_gain						= 0.0;

	/* spectrum analyzer */
	p->spect_ups_req					= 15;
	p->spect_length						= 65536;
	p->spect_bufcount					= 4;
	p->spect_wndtype					= 1; /* hamming */
	p->spect_wndarg						= 0;
	p->spect_magref						= 1.0;
}

/* ---------------------------------------------------------------------------------------------- */

void rxconfig_save(rxconfig_t *p, ini_data_t *ini)
{
	ini_sect_t *sect;

	/* input headers */
	sect = ini_sect_get(ini, _T("input"), 1);
	ini_setus(sect, _T("hdr_count"), p->input_hdr_count);
	ini_setf(sect, _T("hdr_ms"), 6, p->input_hdr_ms);

	/* processing channel */
	sect = ini_sect_get(ini, _T("proc"), 1);

	/* processing channel buffers */
	ini_setf(sect, _T("buf_len_ms"), 6, p->proc_buf_len_ms);
	ini_setf(sect, _T("in_whyst_ms"), 6, p->proc_in_whyst_ms);
	ini_setf(sect, _T("out_rhyst_ms"), 6, p->proc_out_rhyst_ms);
	ini_setf(sect, _T("workbuf_ms"), 6, p->proc_workbuf_ms);

	/* processing channel baseband sampling frequency */
	ini_setu(sect, _T("base_fsmin"), p->base_fsmin);
	ini_setu(sect, _T("base_fsmax"), p->base_fsmax);

	/* processing channel decimator section defaults */
	ini_setu(sect, _T("def_decim_frgran"), p->proc_decim_frgran);
	ini_setu(sect, _T("def_decim_mcsdf"), p->proc_decim_mcsdf);
	ini_setf(sect, _T("def_decim_dff"), 6, p->proc_decim_dff);
	ini_setf(sect, _T("def_decim_as"), 6, p->proc_decim_as);

	/* processing channel filter section defaults */
	ini_setf(sect, _T("def_filter_fc"), 3, p->proc_filter_fc);
	ini_setf(sect, _T("def_filter_df"), 3, p->proc_filter_df);
	ini_setf(sect, _T("def_filter_as"), 6, p->proc_filter_as);

	/* processing channel level meters */
	ini_setus(sect, _T("level_num_points"), p->proc_level_num_points);
	ini_setf(sect, _T("level_update_int"), 6, p->proc_level_update_int);

	/* processing channel output section defaults */
	ini_setf(sect, _T("def_dcrem_alpha"), 6, p->proc_dcrem_alpha);

	/* demodulator and squelch defaults */
	sect = ini_sect_get(ini, _T("def_demod_sql"), 1);
	rxproc_fm_cfg_save(&(p->proc_fm_def), sect);
	rxsql_cfg_save(&(p->proc_sql_def), sect);

	/* audio output */
	sect = ini_sect_get(ini, _T("audio"), 1);
	ini_setf(sect, _T("static_gain"), 6, p->output_static_gain);
	ini_setu(sect, _T("resamp_df"), p->output_resamp_df);
	ini_setf(sect, _T("resamp_as"), 6, p->output_resamp_as);
	ini_setf(sect, _T("lim_thres"), 6, p->output_lim_thres);
	ini_setf(sect, _T("lim_range"), 6, p->output_lim_range);
	ini_setus(sect, _T("hdr_cnt"), p->output_hdr_cnt);
	ini_setf(sect, _T("hdr_ms"), 6, p->output_hdr_ms);
	ini_seti(sect, _T("device_id"), p->output_device_id);
	ini_setu(sect, _T("fs"), p->output_fs);
	ini_setu(sect, _T("bps"), p->output_bps);
	ini_setus(sect, _T("levelmtr_num_points"), p->output_levelmtr_num_points);
	ini_setf(sect, _T("levelmtr_tick_interval"), 6, p->output_levelmtr_tick_interval);
	ini_setf(sect, _T("gain"), 6, p->output_gain);

	/* spectrum analyzer */
	sect = ini_sect_get(ini, _T("spect"), 1);
	ini_setu(sect, _T("ups_req"), p->spect_ups_req);
	ini_setus(sect, _T("length"), p->spect_length);
	ini_setus(sect, _T("buf_count"), p->spect_bufcount);
	ini_seti(sect, _T("wndtype"), p->spect_wndtype);
	ini_setf(sect, _T("wndarg"), 6, p->spect_wndarg);
	ini_setf(sect, _T("magref"), 6, p->spect_magref);
}

/* ---------------------------------------------------------------------------------------------- */

void rxconfig_load(rxconfig_t *p, ini_data_t *ini)
{
	ini_sect_t *sect;

	/* input headers */
	sect = ini_sect_get(ini, _T("input"), 0);
	ini_getusr(sect, _T("hdr_count"), &(p->input_hdr_count),
		RXLIM_INPUT_HDR_COUNT_MIN, RXLIM_INPUT_HDR_COUNT_MAX);
	ini_getfr(sect, _T("hdr_ms"), &(p->input_hdr_ms),
		RXLIM_INPUT_HDR_MS_MIN, RXLIM_INPUT_HDR_MS_MAX);

	/* processing channel */
	sect = ini_sect_get(ini, _T("proc"), 0);

	/* processing channel buffers */
	ini_getfr(sect, _T("buf_len_ms"), &(p->proc_buf_len_ms),
		RXLIM_PROC_BUF_LEN_MS_MIN, RXLIM_PROC_BUF_LEN_MS_MAX);
	ini_getfr(sect, _T("in_whyst_ms"), &(p->proc_in_whyst_ms),
		RXLIM_PROC_BUF_LEN_MS_MIN, RXLIM_PROC_BUF_LEN_MS_MAX);
	ini_getfr(sect, _T("out_rhyst_ms"), &(p->proc_out_rhyst_ms),
		RXLIM_PROC_BUF_LEN_MS_MIN, RXLIM_PROC_BUF_LEN_MS_MAX);
	ini_getfr(sect, _T("workbuf_ms"), &(p->proc_workbuf_ms),
		RXLIM_PROC_WORKBUF_MS_MIN, RXLIM_PROC_WORKBUF_MS_MAX);

	if(p->proc_in_whyst_ms > p->proc_buf_len_ms)
		p->proc_in_whyst_ms = p->proc_buf_len_ms;
	if(p->proc_out_rhyst_ms > p->proc_buf_len_ms)
		p->proc_out_rhyst_ms = p->proc_buf_len_ms;

	/* processing channel baseband sampling frequency */
	ini_getur(sect, _T("base_fsmin"), &(p->base_fsmin),
		RXLIM_BASE_FS_MINABS, RXLIM_BASE_FS_MAXABS);
	ini_getur(sect, _T("base_fsmax"), &(p->base_fsmax),
		RXLIM_BASE_FS_MINABS, RXLIM_BASE_FS_MAXABS);

	if(p->base_fsmax < p->base_fsmin)
		p->base_fsmax = p->base_fsmin;

	/* processing channel decimator section defaults */
	ini_getur(sect, _T("def_decim_frgran"), &(p->proc_decim_frgran),
		RXLIM_PROC_DECIM_FRGRAN_MIN, RXLIM_PROC_DECIM_FRGRAN_MAX);
	ini_getur(sect, _T("def_decim_mcsdf"), &(p->proc_decim_mcsdf),
		RXLIM_PROC_DECIM_MCSDF_MIN, RXLIM_PROC_DECIM_MCSDF_MAX);
	ini_getfr(sect, _T("def_decim_dff"), &(p->proc_decim_dff),
		RXLIM_PROC_DECIM_DFF_MIN, RXLIM_PROC_DECIM_DFF_MAX);
	ini_getfr(sect, _T("def_decim_as"), &(p->proc_decim_as),
		RXLIM_PROC_DECIM_AS_MIN, RXLIM_PROC_DECIM_AS_MAX);

	/* processing channel filter section defaults */
	ini_getfr(sect, _T("def_filter_fc"), &(p->proc_filter_fc),
		RXLIM_PROC_FILTER_FC_MIN, RXLIM_PROC_FILTER_FC_MAX);
	ini_getfr(sect, _T("def_filter_df"), &(p->proc_filter_df),
		RXLIM_PROC_FILTER_DF_MIN, RXLIM_PROC_FILTER_DF_MAX);
	ini_getfr(sect, _T("def_filter_as"), &(p->proc_filter_as),
		RXLIM_PROC_FILTER_AS_MIN, RXLIM_PROC_FILTER_AS_MAX);

	/* processing channel output section defaults */
	ini_getfr(sect, _T("def_dcrem_alpha"), &(p->proc_dcrem_alpha),
		RXLIM_PROC_DCREM_ALPHA_MIN, RXLIM_PROC_DCREM_ALPHA_MAX);

	/* processing channel level meters */
	ini_getusr(sect, _T("level_num_points"), &(p->proc_level_num_points),
		RXLIM_PROC_LEVEL_NUM_POINTS_MIN, RXLIM_PROC_LEVEL_NUM_POINTS_MAX);
	ini_getfr(sect, _T("level_update_int"), &(p->proc_level_update_int),
		RXLIM_PROC_LEVEL_UPDATE_INT_MIN, RXLIM_PROC_LEVEL_UPDATE_INT_MAX);

	/* demodulator and squelch defaults */
	sect = ini_sect_get(ini, _T("def_demod_sql"), 0);
	rxproc_fm_cfg_load(&(p->proc_fm_def), sect);
	rxsql_cfg_load(&(p->proc_sql_def), sect);

	/* audio output */
	sect = ini_sect_get(ini, _T("audio"), 0);
	ini_getfr(sect, _T("static_gain"), &(p->output_static_gain),
		RXLIM_OUTPUT_STATIC_GAIN_MIN, RXLIM_OUTPUT_STATIC_GAIN_MAX);
	ini_getur(sect, _T("resamp_df"), &(p->output_resamp_df),
		RXLIM_OUTPUT_RESAMP_DF_MIN, RXLIM_OUTPUT_RESAMP_DF_MAX);
	ini_getfr(sect, _T("resamp_as"), &(p->output_resamp_as),
		RXLIM_OUTPUT_RESAMP_AS_MIN, RXLIM_OUTPUT_RESAMP_AS_MAX);
	ini_getfr(sect, _T("lim_thres"), &(p->output_lim_thres),
		RXLIM_OUTPUT_LIM_THRES_MIN, RXLIM_OUTPUT_LIM_THRES_MAX);
	ini_getfr(sect, _T("lim_range"), &(p->output_lim_range),
		RXLIM_OUTPUT_LIM_RANGE_MIN, RXLIM_OUTPUT_LIM_RANGE_MAX);
	ini_getusr(sect, _T("hdr_cnt"), &(p->output_hdr_cnt),
		RXLIM_OUTPUT_HDR_CNT_MIN, RXLIM_OUTPUT_HDR_CNT_MAX);
	ini_getfr(sect, _T("hdr_ms"), &(p->output_hdr_ms),
		RXLIM_OUTPUT_HDR_MS_MIN, RXLIM_OUTPUT_HDR_MS_MAX);
	ini_getir(sect, _T("device_id"), &(p->output_device_id),
		-1, RXLIM_OUTPUT_MAX_NUM_DEVS - 1);
	ini_getur(sect, _T("fs"), &(p->output_fs),
		RXLIM_OUTPUT_FS_MIN, RXLIM_OUTPUT_FS_MAX);
	ini_getu(sect, _T("bps"), &(p->output_bps));
	if((p->output_bps != 8) && (p->output_bps != 16))
		p->output_bps = 16;
	ini_getusr(sect, _T("levelmtr_num_points"), &(p->output_levelmtr_num_points),
		RXLIM_OUTPUT_LEVELMTR_NUM_POINTS_MIN, RXLIM_OUTPUT_LEVELMTR_NUM_POINTS_MAX);
	ini_getfr(sect, _T("levelmtr_tick_interval"), &(p->output_levelmtr_tick_interval),
		RXLIM_OUTPUT_LEVELMTR_TICK_INTERVAL_MIN, RXLIM_OUTPUT_LEVELMTR_TICK_INTERVAL_MAX);
	ini_getfr(sect, _T("gain"), &(p->output_gain), 
		RXLIM_AUDIO_GAIN_MIN, RXLIM_AUDIO_GAIN_MAX);

	/* spectrum analyzer */
	sect = ini_sect_get(ini, _T("spect"), 0);

	ini_getur(sect, _T("ups_req"), &(p->spect_ups_req),
		RXLIM_SPECT_UPS_REQ_MIN, RXLIM_SPECT_UPS_REQ_MAX);
	ini_getusr(sect, _T("length"), &(p->spect_length),
		RXLIM_SPECT_LENGTH_MIN, RXLIM_SPECT_LENGTH_MAX);
	if( (p->spect_length & (p->spect_length - 1)) != 0 )
		p->spect_length = 65536;
	ini_getusr(sect, _T("buf_count"), &(p->spect_bufcount),
		RXLIM_SPECT_BUF_COUNT_MIN, RXLIM_SPECT_BUF_COUNT_MAX);
	ini_geti(sect, _T("wndtype"), &(p->spect_wndtype));
	ini_getf(sect, _T("wndarg"), &(p->spect_wndarg));
	ini_getf(sect, _T("magref"), &(p->spect_magref));
}

/* ---------------------------------------------------------------------------------------------- */

int rxconfig_init(rxconfig_t *p)
{
	rxconfig_set_defaults(p);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

void rxconfig_cleanup(rxconfig_t *p)
{
}

/* ---------------------------------------------------------------------------------------------- */
