/* ---------------------------------------------------------------------------------------------- */
/* waveout interface wrapper, external sync */

#pragma once

#include <windows.h>
#include <mmsystem.h>
#include <process.h>
#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>

/* ---------------------------------------------------------------------------------------------- */

/* sound output wrapper state */
typedef struct sndout_state {

	CRITICAL_SECTION csec;		/* structure access lock */

	HANDLE h_evt_stop;			/* signalled to stop header handling thread */
	HANDLE h_evt_hdr;			/* signalled to perform header processing */
	HANDLE h_thread;			/* header handling thread */

	WAVEFORMATEX fmt;			/* output format */
	HWAVEOUT hwo;				/* waveout device handle */

	size_t hdr_count;			/* number of headers */
	size_t hdr_length;			/* header length, samples */

	size_t blocksize;			/* sample size, bytes */

	size_t hdr_offset;			/* header buffer offset */
	size_t hdr_sent_count;		/* number of headers sent to device */
	size_t hdr_fill_count;		/* number of headers filled with data */
	size_t hdr_fill_len;		/* number of bytes written to currently filled header */

	int f_wr_inhibit;			/* buffer filling inhibit flag */
	size_t wr_cnt_reenable;		/* free header count to re-enable buffer filling */

	int f_rd_pause;				/* playback inhibited */
	size_t rd_cnt_pause;		/* number of filled headers to pause playback */
	size_t rd_cnt_restart;		/* number of filled headers to restart playback */

	unsigned char *hdrbuf;		/* data buffer */
	WAVEHDR *phdr;				/* header buffer */

} sndout_state_t;

/* ---------------------------------------------------------------------------------------------- */

/* open wave output device */
int sndout_open(sndout_state_t *s, int devid, unsigned int nch,
				unsigned int rate, unsigned int bps,
				size_t hdrcnt, size_t hdrlen);

/* close wave output device */
void sndout_close(sndout_state_t *s);

/* write data to wave output wrapper */
size_t sndout_write(sndout_state_t *s, void *buf, size_t nsamp);

/* return number of samples to be played */
size_t sndout_nsamp_pending(sndout_state_t *s);

/* ---------------------------------------------------------------------------------------------- */
