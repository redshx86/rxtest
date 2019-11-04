/* ---------------------------------------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------------------------------------- */

/* working (input, channel, visualization) frequency limit */
#define RXLIM_FC_MIN							-1e12
#define RXLIM_FC_MAX							1e12

/* ---------------------------------------------------------------------------------------------- */

/* number of input headers */
#define RXLIM_INPUT_HDR_COUNT_MIN				2
#define RXLIM_INPUT_HDR_COUNT_MAX				64

/* length of input header, ms */
#define RXLIM_INPUT_HDR_MS_MIN					1.0
#define RXLIM_INPUT_HDR_MS_MAX					100.0

/* ---------------------------------------------------------------------------------------------- */

/* processing channel input/ourput buffer length, ms */
#define RXLIM_PROC_BUF_LEN_MS_MIN				15.0
#define RXLIM_PROC_BUF_LEN_MS_MAX				1500.0

/* processing channel working buffer length, ms */
#define RXLIM_PROC_WORKBUF_MS_MIN				1.0
#define RXLIM_PROC_WORKBUF_MS_MAX				100.0

/* ---------------------------------------------------------------------------------------------- */

/* baseband sampling frequency absolute limits, Hz */
#define RXLIM_BASE_FS_MINABS					50
#define RXLIM_BASE_FS_MAXABS					240000

/* ---------------------------------------------------------------------------------------------- */

/* output static gain, dB */
#define RXLIM_OUTPUT_STATIC_GAIN_MIN			-60.0
#define RXLIM_OUTPUT_STATIC_GAIN_MAX			20.0

/* output resampler transition bandwidth, Hz */
#define RXLIM_OUTPUT_RESAMP_DF_MIN				100
#define RXLIM_OUTPUT_RESAMP_DF_MAX				20000

/* output resampler alias attenuation, dB */
#define RXLIM_OUTPUT_RESAMP_AS_MIN				20.0
#define RXLIM_OUTPUT_RESAMP_AS_MAX				140.0

/* output limiter input threshold */
#define RXLIM_OUTPUT_LIM_THRES_MIN				0.0
#define RXLIM_OUTPUT_LIM_THRES_MAX				0.95

/* output limiter input range */
#define RXLIM_OUTPUT_LIM_RANGE_MIN				1.2
#define RXLIM_OUTPUT_LIM_RANGE_MAX				20.0

/* number of audio output WAVEHDRs */
#define RXLIM_OUTPUT_HDR_CNT_MIN				6
#define RXLIM_OUTPUT_HDR_CNT_MAX				32

/* length of output WAVEHDR buffer, ms */
#define RXLIM_OUTPUT_HDR_MS_MIN					10.0
#define RXLIM_OUTPUT_HDR_MS_MAX					1000.0

/* maximum number of output device ids */
#define RXLIM_OUTPUT_MAX_NUM_DEVS				64

/* output sampling frequency, Hz */
#define RXLIM_OUTPUT_FS_MIN						1000
#define RXLIM_OUTPUT_FS_MAX						200000

/* number of output level meter frame points */
#define RXLIM_OUTPUT_LEVELMTR_NUM_POINTS_MIN	10
#define RXLIM_OUTPUT_LEVELMTR_NUM_POINTS_MAX	1000

/* output level meter frame tick interval */
#define RXLIM_OUTPUT_LEVELMTR_TICK_INTERVAL_MIN	1.0
#define RXLIM_OUTPUT_LEVELMTR_TICK_INTERVAL_MAX	100.0

/* spectrum analyzer required refresh rate, Hz */
#define RXLIM_SPECT_UPS_REQ_MIN					1
#define RXLIM_SPECT_UPS_REQ_MAX					250

/* spectrum analyzer output/fft length, samples */
#define RXLIM_SPECT_LENGTH_MIN					32
#define RXLIM_SPECT_LENGTH_MAX					1048576

/* spectrum analyzer input buffer count */
#define RXLIM_SPECT_BUF_COUNT_MIN				1
#define RXLIM_SPECT_BUF_COUNT_MAX				6

/* ---------------------------------------------------------------------------------------------- */

#define RXLIM_PROC_MAXNAME						64

/* ---------------------------------------------------------------------------------------------- */

/* processing channel decimator section */
#define RXLIM_PROC_DECIM_FRGRAN_MIN				50
#define RXLIM_PROC_DECIM_FRGRAN_MAX				100000

#define RXLIM_PROC_DECIM_MCSDF_MIN				2
#define RXLIM_PROC_DECIM_MCSDF_MAX				1000

#define RXLIM_PROC_DECIM_DFF_MIN				0.05
#define RXLIM_PROC_DECIM_DFF_MAX				0.45

#define RXLIM_PROC_DECIM_AS_MIN					20.0
#define RXLIM_PROC_DECIM_AS_MAX					140.0

/* ---------------------------------------------------------------------------------------------- */

/* processing channel filter section defaults */
#define RXLIM_PROC_FILTER_FC_MIN				50.0
#define RXLIM_PROC_FILTER_FC_MAX				240000.0

#define RXLIM_PROC_FILTER_DF_MIN				10.0
#define RXLIM_PROC_FILTER_DF_MAX				20000.0

#define RXLIM_PROC_FILTER_AS_MIN				20.0
#define RXLIM_PROC_FILTER_AS_MAX				140.0

/* ---------------------------------------------------------------------------------------------- */

/* processing channel level meters */
#define RXLIM_PROC_LEVEL_NUM_POINTS_MIN			10
#define RXLIM_PROC_LEVEL_NUM_POINTS_MAX			1000

#define RXLIM_PROC_LEVEL_UPDATE_INT_MIN			1.0
#define RXLIM_PROC_LEVEL_UPDATE_INT_MAX			100.0

/* ---------------------------------------------------------------------------------------------- */

/* processing channel output gain */
#define RXLIM_AUDIO_GAIN_MIN					-60.0
#define RXLIM_AUDIO_GAIN_MAX					20.0

/* ---------------------------------------------------------------------------------------------- */

/* processing channel output section */
#define RXLIM_PROC_DCREM_ALPHA_MIN				0.001
#define RXLIM_PROC_DCREM_ALPHA_MAX				1.0

/* ---------------------------------------------------------------------------------------------- */
