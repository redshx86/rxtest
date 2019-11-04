/* ---------------------------------------------------------------------------------------------- */

#include "sndmix.h"

/* ---------------------------------------------------------------------------------------------- */

/* set mixer channel input gain.
 *	gain: input gain, dB. */
void sndmix_input_gain(sndmix_input_t *in, double gain)
{
	in->gainl = DB2LINM(gain);
}

/* ---------------------------------------------------------------------------------------------- */

/* set mixer output gain.
 *	gain: output gain, dB. */
void sndmix_output_gain(sndmix_state_t *s, double gain)
{
	s->gainl = DB2LINM(gain + s->sgain);
}

/* ---------------------------------------------------------------------------------------------- */

/* write data to mixer input.
 *	data: data to write, float samples,
 *	nsamp: number of input samples (per channel),
 *	mode: data channels mode.
 * Note: if mode is CHAN_NONE, data buffer ignored.
 * Note: if mode is INPUT_CHAN_ABI, data buffer contains 2*nsamp samples. */
size_t sndmix_input_data(sndmix_input_t *in, float *data, size_t nsamp, sndmix_input_mode_t mode)
{
	float gain, temp, *dst;
	size_t i, ns, wrnsamp;
	size_t totwrnsamp = 0;

	/* combined input gain */
	gain = (float)(in->gainl * in->mixstate->gainl);

	while(nsamp != 0)
	{
		ns = min(nsamp, in->tempnsamp);

		/* copy input samples to scratchpad */
		dst = in->tempdata;
		switch(mode)
		{
			/* copy data to channel A only */
		case SNDMIX_INPUT_CHAN_A:
			for(i = 0; i < ns; i++) {
				*(dst++) = *(data++) * gain;
				*(dst++) = 0;
			}
			break;

			/* copy data to channel B only */
		case SNDMIX_INPUT_CHAN_B:
			for(i = 0; i < ns; i++) {
				*(dst++) = 0;
				*(dst++) = *(data++) * gain;
			}
			break;

			/* copy same data to channels A and B */
		case SNDMIX_INPUT_CHAN_AB:
			for(i = 0; i < ns; i++) {
				temp = *(data++) * gain;
				*(dst++) = temp;
				*(dst++) = temp;
			}
			break;

			/* copy data to channels A and B interleaved */
		case SNDMIX_INPUT_CHAN_ABI:
			for(i = 0; i < ns; i++) {
				*(dst++) = *(data++) * gain;
				*(dst++) = *(data++) * gain;
			}
			break;

			/* load zeros */
		default:
			for(i = 0; i < ns; i++) {
				*(dst++) = 0;
				*(dst++) = 0;
			}
			break;
		}

		/* write scratchpad to data buffer */
		wrnsamp = buf_write(&(in->buf), in->tempdata, ns);
		totwrnsamp += wrnsamp;

		/* buffer overflow? */
		if(wrnsamp != ns)
			break;

		nsamp -= ns;
	}

	return totwrnsamp;
}

/* ---------------------------------------------------------------------------------------------- */

/* read data from mixer.
 *	buf: buffer to store data, float samples,
 *	nsamp: number of samples to read (per channel). */
void sndmix_output_data(sndmix_state_t *s, float *buf, size_t nsamp)
{
	size_t ns, nsin;
	float *src, *dst;
	sndmix_input_t *in;

	/* clear output buffer */
	memset(buf, 0, nsamp * sizeof(float) * 2);

	/* process input channels */
	EnterCriticalSection(&(s->csec_in));

	while(nsamp != 0)
	{
		ns = min(nsamp, s->tempnsamp);

		/* for each input channel */
		for(in = s->first_input; in != NULL; in = in->next_input)
		{
			/* read data from input buffer */
			nsin = buf_read(&(in->buf), s->tempdata, ns);

			/* add input samples to output buffer */
			dst = buf;
			src = s->tempdata;

			while(nsin--)
			{
				*(dst++) += *(src++);
				*(dst++) += *(src++);
			}
		}

		buf += (ns * 2);
		nsamp -= ns;
	}

	LeaveCriticalSection(&(s->csec_in));
}

/* ---------------------------------------------------------------------------------------------- */

/* create mixer input.
 *	bufns: buffer capacity, samples,
 *	bufrhns: buffer read hysteresis, samples,
 *	bufwhns: buffer write hysteresis, samples. */
sndmix_input_t *sndmix_input_create(sndmix_state_t *s, size_t bufns, size_t bufrhns, size_t bufwhns)
{
	sndmix_input_t *in;

	/* check params */
	if(bufns == 0)
		return 0;

	/* initialize structure */
	if((in = malloc(sizeof(sndmix_input_t))) != NULL)
	{
		in->mixstate = s;
		in->next_input = NULL;
		in->prev_input = NULL;

		/* initialize scratchpad */
		in->tempnsamp = 256;
		if((in->tempdata = malloc(in->tempnsamp * sizeof(float) * 2)) != NULL)
		{
			if(buf_init(&(in->buf), sizeof(float) * 2, bufns, bufrhns, bufwhns) )
			{
				in->gainl = 1.0f;

				/* register input */
				EnterCriticalSection(&(s->csec_in));
				DLIST_INSBACK(in, s->first_input, s->last_input, next_input, prev_input);
				LeaveCriticalSection(&(s->csec_in));

				return in;
			}

			free(in->tempdata);
		}

		free(in);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

/* remove mixer input */
void sndmix_input_destroy(sndmix_input_t *in)
{
	sndmix_state_t *s;

	s = in->mixstate;

	/* unregister input */
	EnterCriticalSection(&(s->csec_in));
	DLIST_REMOVE(in, s->first_input, s->last_input, next_input, prev_input);
	LeaveCriticalSection(&(s->csec_in));

	/* cleanup */
	buf_cleanup(&(in->buf));
	free(in->tempdata);
	free(in);
}

/* ---------------------------------------------------------------------------------------------- */

/* initialize mixer.
 *	sgain: static mixer gain, dB,
 *	ogain: init output gain, dB. */
int sndmix_init(sndmix_state_t *s, double sgain, double ogain)
{
	/* check parameter */
	if( (sgain < -200.0) || (sgain > 200.0) )
		return 0;

	/* initialize input list */
	InitializeCriticalSection(&(s->csec_in));
	s->first_input = NULL;
	s->last_input = NULL;

	s->sgain = sgain;
	s->gainl = DB2LINM(s->sgain + ogain);

	s->tempnsamp = 256;
	if((s->tempdata = malloc(s->tempnsamp * sizeof(float) * 2)) != NULL)
		return 1;

	DeleteCriticalSection(&(s->csec_in));
	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* free mixer */
void sndmix_cleanup(sndmix_state_t *s)
{
	free(s->tempdata);
	DeleteCriticalSection(&(s->csec_in));
}

/* ---------------------------------------------------------------------------------------------- */
