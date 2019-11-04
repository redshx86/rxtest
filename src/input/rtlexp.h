/* ---------------------------------------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------------------------------------- */

typedef unsigned int (*rtlsdr_get_device_count_t)();

typedef char* (*rtlsdr_get_device_name_t)(unsigned int index);

typedef int (*rtlsdr_open_t)(void **devp, unsigned int index);
typedef int (*rtlsdr_close_t)(void *dev);

/* configuration functions */

typedef int (*rtlsdr_set_center_freq_t)(void *dev, unsigned int freq);

typedef unsigned int (*rtlsdr_get_center_freq_t)(void *dev);

typedef int (*rtlsdr_set_freq_correction_t)(void *dev, int ppm);

typedef int (*rtlsdr_get_freq_correction_t)(void *dev);

typedef enum rtlsdr_tuner_type {

	RTLSDR_TUNER_UNKNOWN = 0,
	RTLSDR_TUNER_E4000,
	RTLSDR_TUNER_FC0012,
	RTLSDR_TUNER_FC0013,
	RTLSDR_TUNER_FC2580,
	RTLSDR_TUNER_R820T,
	RTLSDR_TUNER_R828D,

	RTLSDR_TUNER_TYPE_COUNT

} rtlsdr_tuner_type_t;

typedef rtlsdr_tuner_type_t (*rtlsdr_get_tuner_type_t)(void *dev);

typedef int (*rtlsdr_get_tuner_gains_t)(void *dev, int *gains);

typedef int (*rtlsdr_set_tuner_gain_t)(void *dev, int gain);

typedef int (*rtlsdr_get_tuner_gain_t)(void *dev);

typedef int (*rtlsdr_set_tuner_gain_mode_t)(void *dev, int manual);

typedef int (*rtlsdr_set_sample_rate_t)(void *dev, unsigned int rate);

typedef unsigned int (*rtlsdr_get_sample_rate_t)(void *dev);

typedef int (*rtlsdr_set_agc_mode_t)(void *dev, int on);

typedef int (*rtlsdr_set_direct_sampling_t)(void *dev, int on);

typedef int (*rtlsdr_get_direct_sampling_t)(void *dev);

typedef int (*rtlsdr_set_offset_tuning_t)(void *dev, int on);

typedef int (*rtlsdr_get_offset_tuning_t)(void *dev);

/* streaming functions */

typedef int (*rtlsdr_reset_buffer_t)(void *dev);

typedef void(*rtlsdr_read_async_cb_t)(unsigned char *buf, unsigned int len, void *ctx);

typedef int (*rtlsdr_read_async_t)(
	void *dev, rtlsdr_read_async_cb_t cb, void *ctx,
	unsigned int buf_num, unsigned int buf_len);

typedef int (*rtlsdr_cancel_async_t)(void *dev);

/* ---------------------------------------------------------------------------------------------- */

typedef struct rtlsdr_proc {

	rtlsdr_get_device_count_t get_device_count;
	rtlsdr_get_device_name_t get_device_name;

	rtlsdr_open_t open;
	rtlsdr_close_t close;

	rtlsdr_set_center_freq_t set_center_freq;
	rtlsdr_get_center_freq_t get_center_freq;

	rtlsdr_set_freq_correction_t set_freq_correction;
	rtlsdr_get_freq_correction_t get_freq_correction;

	rtlsdr_get_tuner_type_t get_tuner_type;

	rtlsdr_get_tuner_gains_t get_tuner_gains;
	rtlsdr_set_tuner_gain_t set_tuner_gain;
	rtlsdr_get_tuner_gain_t get_tuner_gain;

	rtlsdr_set_tuner_gain_mode_t set_tuner_gain_mode;

	rtlsdr_set_sample_rate_t set_sample_rate;
	rtlsdr_get_sample_rate_t get_sample_rate;

	rtlsdr_set_agc_mode_t set_agc_mode;

	rtlsdr_set_direct_sampling_t set_direct_sampling;
	rtlsdr_get_direct_sampling_t get_direct_sampling;

	rtlsdr_set_offset_tuning_t set_offset_tuning;
	rtlsdr_get_offset_tuning_t get_offset_tuning;

	rtlsdr_reset_buffer_t reset_buffer;
	rtlsdr_read_async_t read_async;
	rtlsdr_cancel_async_t cancel_async;

} rtlsdr_proc_t;

/* ---------------------------------------------------------------------------------------------- */
