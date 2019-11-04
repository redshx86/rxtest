/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "../util/buf.h"
#include "../util/list.h"
#include "../util/db.h"

/* ---------------------------------------------------------------------------------------------- */

struct sndmix_state;

typedef struct sndmix_input {
	struct sndmix_state *mixstate;
	struct sndmix_input *next_input;
	struct sndmix_input *prev_input;
	buf_state_t buf;
	float *tempdata;
	size_t tempnsamp;
	double gainl;
} sndmix_input_t;

/* ---------------------------------------------------------------------------------------------- */

typedef struct sndmix_state {
	CRITICAL_SECTION csec_in;
	sndmix_input_t *first_input;
	sndmix_input_t *last_input;
	double gainl;
	double sgain;
	float *tempdata;
	size_t tempnsamp;
} sndmix_state_t;

/* ---------------------------------------------------------------------------------------------- */

typedef enum sndmix_input_mode {
	SNDMIX_INPUT_CHAN_NONE,	/* input zeros */
	SNDMIX_INPUT_CHAN_A,	/* copy data to channel A */
	SNDMIX_INPUT_CHAN_B,	/* copy data to channel B */
	SNDMIX_INPUT_CHAN_AB,	/* copy same data to channels A and B */
	SNDMIX_INPUT_CHAN_ABI	/* copy data to channels A and B interleaved */
} sndmix_input_mode_t;

/* ---------------------------------------------------------------------------------------------- */

/* set mixer channel input gain.
 *	gain: input gain, dB. */
void sndmix_input_gain(sndmix_input_t *in, double gain);

/* set mixer output gain.
 *	gain: output gain, dB. */
void sndmix_output_gain(sndmix_state_t *s, double gain);

/* write data to mixer input.
 *	data: data to write, float samples,
 *	nsamp: number of input samples (per channel),
 *	mode: data channels mode.
 * Note: if mode is CHAN_NONE, data buffer ignored.
 * Note: if mode is INPUT_CHAN_ABI, data buffer contains 2*nsamp samples. */
size_t sndmix_input_data(sndmix_input_t *in, float *data, size_t nsamp, sndmix_input_mode_t mode);

/* read data from mixer.
 *	buf: buffer to store data, float samples,
 *	nsamp: number of samples to read (per channel). */
void sndmix_output_data(sndmix_state_t *s, float *buf, size_t nsamp);

/* create mixer input.
 *	bufns: buffer capacity, samples,
 *	bufrhns: buffer read hysteresis, samples,
 *	bufwhns: buffer write hysteresis, samples. */
sndmix_input_t *sndmix_input_create(sndmix_state_t *s, size_t bufns, size_t bufrhns, size_t bufwhns);

/* remove mixer input */
void sndmix_input_destroy(sndmix_input_t *in);

/* initialize mixer.
 *	sgain: static mixer gain, dB,
 *	ogain: init output gain, dB. */
int sndmix_init(sndmix_state_t *s, double sgain, double ogain);

/* free mixer */
void sndmix_cleanup(sndmix_state_t *s);

/* ---------------------------------------------------------------------------------------------- */
