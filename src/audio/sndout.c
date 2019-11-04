/* ---------------------------------------------------------------------------------------------- */

#include "sndout.h"

/* ---------------------------------------------------------------------------------------------- */

static void sndout_unprepareheaders(sndout_state_t *s)
{
	WAVEHDR *phdr;

	/* unprepare done headers */
	while(s->hdr_sent_count != 0)
	{
		phdr = s->phdr + s->hdr_offset;
		if( ! (phdr->dwFlags & WHDR_DONE) )
			break;

		waveOutUnprepareHeader(s->hwo, phdr, sizeof(WAVEHDR));
		phdr->dwFlags = 0;

		s->hdr_offset++;
		if(s->hdr_offset == s->hdr_count)
			s->hdr_offset = 0;

		s->hdr_fill_count--;
		s->hdr_sent_count--;
	}
}

/* ---------------------------------------------------------------------------------------------- */

static void sndout_processheaders(sndout_state_t *s)
{
	size_t hdr_index;
	WAVEHDR *phdr;

	/* unprepare done headers */
	sndout_unprepareheaders(s);

	/* pause or restart playback */
	if( (s->hdr_fill_count <= s->rd_cnt_pause) && (!s->f_rd_pause) )
	{
		waveOutPause(s->hwo);
		s->f_rd_pause = 1;
	}
	if( (s->hdr_fill_count >= s->rd_cnt_restart) && (s->f_rd_pause) )
	{
		waveOutRestart(s->hwo);
		s->f_rd_pause = 0;
	}

	/* prepare and send filled headers */
	while(s->hdr_sent_count != s->hdr_fill_count)
	{
		/* select next filled, but not sent header */
		hdr_index = s->hdr_offset + s->hdr_sent_count;
		if(hdr_index >= s->hdr_count)
			hdr_index -= s->hdr_count;
		phdr = s->phdr + hdr_index;

		/* prepare and send header */
		waveOutPrepareHeader(s->hwo, phdr, sizeof(WAVEHDR));
		waveOutWrite(s->hwo, phdr, sizeof(WAVEHDR));

		s->hdr_sent_count++;
	}
}

/* ---------------------------------------------------------------------------------------------- */

enum {
	SNDOUT_EVENT_STOP,
	SNDOUT_EVENT_HDR,

	SNDOUT_EVENT_COUNT
};

unsigned int __stdcall sndout_threadproc(sndout_state_t *s)
{
	HANDLE event[SNDOUT_EVENT_COUNT];
	DWORD res;

	event[SNDOUT_EVENT_STOP] = s->h_evt_stop;
	event[SNDOUT_EVENT_HDR] = s->h_evt_hdr;

	for( ; ; )
	{
		res = WaitForMultipleObjects(SNDOUT_EVENT_COUNT, event, FALSE, INFINITE);

		if(res == SNDOUT_EVENT_STOP)
			break;

		if(res == SNDOUT_EVENT_HDR)
		{
			EnterCriticalSection(&(s->csec));
			sndout_processheaders(s);
			LeaveCriticalSection(&(s->csec));
		}
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* open wave output device */
int sndout_open(sndout_state_t *s, int devid, unsigned int nch,
				unsigned int rate, unsigned int bps,
				size_t hdrcnt, size_t hdrlen)
{
	size_t i;
	unsigned int tid;

	/* check configuration */
	if( (devid < -1) || 
		((nch != 1) && (nch != 2)) ||
		(rate == 0) || (bps == 0) ||
		(hdrcnt < 6) || (hdrlen == 0) )
	{
		return 0;
	}

	/* initialize structure */
	memset(s, 0, sizeof(sndout_state_t));

	s->hdr_count = hdrcnt;
	s->hdr_length = hdrlen;

	s->blocksize = nch * bps / 8;

	s->wr_cnt_reenable = (hdrcnt - 2) / 2;

	s->rd_cnt_restart = (hdrcnt - 2) / 2 + 2;
	s->rd_cnt_pause = 2;

	/* initialize format */
	s->fmt.wFormatTag = WAVE_FORMAT_PCM;
	s->fmt.nChannels = (WORD)nch;
	s->fmt.nSamplesPerSec = rate;
	s->fmt.nAvgBytesPerSec = (DWORD)(s->blocksize * rate);
	s->fmt.nBlockAlign = (WORD)(s->blocksize);
	s->fmt.wBitsPerSample = (WORD)bps;

	/* allocate resources */
	InitializeCriticalSection(&(s->csec));
	s->h_evt_stop = CreateEvent(NULL, FALSE, FALSE, NULL);
	s->h_evt_hdr = CreateEvent(NULL, FALSE, FALSE, NULL);
	s->phdr = calloc(hdrcnt, sizeof(WAVEHDR));
	s->hdrbuf = malloc(hdrcnt * hdrlen * s->blocksize);

	/* check for allocation errors */
	if( (s->h_evt_stop != NULL) && (s->h_evt_hdr != NULL) &&
		(s->phdr != NULL) && (s->hdrbuf != NULL) )
	{
		/* initialize headers */
		for(i = 0; i < hdrcnt; i++)
		{
			s->phdr[i].lpData = (LPSTR)(s->hdrbuf + s->blocksize * hdrlen * i);
			s->phdr[i].dwBufferLength = (DWORD)(s->blocksize * hdrlen);
		}

		/* open waveout device */
		if( waveOutOpen(&(s->hwo), (UINT)devid, &(s->fmt),
			(DWORD_PTR)(s->h_evt_hdr), 0, CALLBACK_EVENT) == MMSYSERR_NOERROR )
		{
			s->h_thread = (HANDLE)_beginthreadex(
				NULL, 0, sndout_threadproc, s, 0, &tid);

			if(s->h_thread != NULL)
			{
				SetThreadPriority(s->h_thread, THREAD_PRIORITY_ABOVE_NORMAL);
				return 1;
			}

			waveOutClose(s->hwo);
		}
	}

	/* cleanup */
	free(s->hdrbuf);
	free(s->phdr);
	CloseHandle(s->h_evt_hdr);
	CloseHandle(s->h_evt_stop);
	DeleteCriticalSection(&(s->csec));

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

/* close wave output device */
void sndout_close(sndout_state_t *s)
{
	/* terminate processing thread */
	SetEvent(s->h_evt_stop);
	WaitForSingleObject(s->h_thread, INFINITE);
	CloseHandle(s->h_thread);

	/* unprepare remaining headers */
	waveOutReset(s->hwo);
	sndout_unprepareheaders(s);

	/* close waveout device */
	waveOutClose(s->hwo);

	/* cleanup */
	free(s->hdrbuf);
	free(s->phdr);
	CloseHandle(s->h_evt_hdr);
	CloseHandle(s->h_evt_stop);
	DeleteCriticalSection(&(s->csec));
}

/* ---------------------------------------------------------------------------------------------- */

/* write data to wave output wrapper */
size_t sndout_write(sndout_state_t *s, void *buf, size_t nsamp)
{
	char *psrc = buf;
	size_t nsampused = 0, hdr_index;
	size_t hdr_num_free, nsampwr;
	WAVEHDR *hdr;
	int f_processhdrs = 0;

	EnterCriticalSection(&(s->csec));

	/* try process all input samples */
	while(nsamp != 0)
	{
		/* check overflow condition */
		if(s->hdr_fill_count == s->hdr_count)
		{
			s->f_wr_inhibit = 1;
			break;
		}

		if(s->f_wr_inhibit)
		{
			hdr_num_free = s->hdr_count - s->hdr_fill_count;
			if(hdr_num_free < s->wr_cnt_reenable)
				break;
			s->f_wr_inhibit = 0;
		}

		/* select header for filling */
		hdr_index = s->hdr_offset + s->hdr_fill_count;
		if(hdr_index >= s->hdr_count)
			hdr_index -= s->hdr_count;
		hdr = s->phdr + hdr_index;

		/* copy data to header */
		nsampwr = min(nsamp, s->hdr_length - s->hdr_fill_len);

		memcpy((char*)(hdr->lpData) + s->hdr_fill_len * s->blocksize,
			psrc, nsampwr * s->blocksize);
		psrc += nsampwr * s->blocksize;
		nsamp -= nsampwr;
		nsampused += nsampwr;

		s->hdr_fill_len += nsampwr;

		/* check for header completion */
		if(s->hdr_fill_len == s->hdr_length)
		{
			s->hdr_fill_count++;
			s->hdr_fill_len = 0;
			f_processhdrs = 1;
		}
	}

	LeaveCriticalSection(&(s->csec));

	/* process headers */
	if(f_processhdrs) {
		SetEvent(s->h_evt_hdr);
	}

	return nsampused;
}

/* ---------------------------------------------------------------------------------------------- */

/* return number of samples to be played */
size_t sndout_nsamp_pending(sndout_state_t *s)
{
	size_t nsamp_pending, nsamp_sent;
	size_t nsamp_rtn_est_pending;

	EnterCriticalSection(&(s->csec));

	nsamp_pending = s->hdr_fill_count * s->hdr_length + s->hdr_fill_len;

	/* assume that ~1.5 sent headers are played but not returned yet */
	nsamp_sent = s->hdr_sent_count * s->hdr_length;
	nsamp_rtn_est_pending = min(nsamp_sent, 3UL * s->hdr_length / 2UL);
	nsamp_pending -= nsamp_rtn_est_pending;

	LeaveCriticalSection(&(s->csec));

	return nsamp_pending;
}

/* ---------------------------------------------------------------------------------------------- */
