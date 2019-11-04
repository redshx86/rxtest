/* ---------------------------------------------------------------------------------------------- */

#include "colormap.h"

/* ---------------------------------------------------------------------------------------------- */

static COLORREF crtab_norm[] =
{
	RGB(0, 0, 64),
	RGB(0, 0, 128),
	RGB(192, 0, 0),
	RGB(255, 128, 0),
	RGB(255, 255, 0),
	RGB(255, 255, 255)
};

/* ---------------------------------------------------------------------------------------------- */

static COLORREF crtab_chan[] =
{
	RGB(0, 32, 64),
	RGB(0, 64, 128),
	RGB(192, 0, 0),
	RGB(255, 128, 0),
	RGB(255, 255, 0),
	RGB(255, 255, 255)
};

/* ---------------------------------------------------------------------------------------------- */

static COLORREF crtab_grey[] =
{
	RGB(0, 0, 0),
	RGB(255, 255, 255)
};

/* ---------------------------------------------------------------------------------------------- */

static COLORREF crtab_jet[] =
{
	RGB(0, 0, 128),
	RGB(0, 0, 255),
	RGB(0, 128, 255),
	RGB(0, 255, 255),
	RGB(128, 255, 128),
	RGB(255, 255, 0),
	RGB(255, 128, 0),
	RGB(255, 0, 0),
	RGB(128, 0, 0)
};

/* ---------------------------------------------------------------------------------------------- */

colormap_def_t crmap_def[COLORMAP_COUNT] =
{
	{ _T("[6] Standard normal"),	crtab_norm,		ARRAY_SIZE(crtab_norm)	},
	{ _T("[6] Standard channel"),	crtab_chan,		ARRAY_SIZE(crtab_chan)	},
	{ _T("[2] Greyscale"),			crtab_grey,		ARRAY_SIZE(crtab_grey)	},
	{ _T("[9] Jet (MATLAB)"),		crtab_jet,		ARRAY_SIZE(crtab_jet)	}
};

/* ---------------------------------------------------------------------------------------------- */

COLORREF cr_mix(COLORREF cr_a, COLORREF cr_b, double f)
{
	int cr_a_r, cr_a_g, cr_a_b;
	int cr_b_r, cr_b_g, cr_b_b;

	cr_a_r = GetRValue(cr_a);
	cr_a_g = GetGValue(cr_a);
	cr_a_b = GetBValue(cr_a);

	cr_b_r = GetRValue(cr_b);
	cr_b_g = GetGValue(cr_b);
	cr_b_b = GetBValue(cr_b);

	return RGB(
		(BYTE)((double)cr_a_r * (1.0 - f) + (double)cr_b_r * f + 0.5 - 1e-6),
		(BYTE)((double)cr_a_g * (1.0 - f) + (double)cr_b_g * f + 0.5 - 1e-6),
		(BYTE)((double)cr_a_b * (1.0 - f) + (double)cr_b_b * f + 0.5 - 1e-6) );
}

/* ---------------------------------------------------------------------------------------------- */

static COLORREF crmap_make(double f, COLORREF *data, size_t count)
{
	int index;
	double f_step, f_part;

	index = (int)(f * (double)(count - 1));

	if(index >= (int)(count - 1))
		return data[count - 1];

	f_step = 1.0 / (double)(count - 1);
	f_part = (f - (double)index * f_step) / f_step;

	return cr_mix(data[index], data[index + 1], f_part);
}

/* ---------------------------------------------------------------------------------------------- */

void crmap_ref(colormap_type_t type, COLORREF *buf, size_t count)
{
	size_t i;

	for(i = 0; i < count; i++)
	{
		buf[i] = crmap_make((double)i / (double)(count - 1),
			crmap_def[type].ref_pt_arr, crmap_def[type].ref_pt_num);
	}
}

/* ---------------------------------------------------------------------------------------------- */
