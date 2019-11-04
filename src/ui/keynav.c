/* ---------------------------------------------------------------------------------------------- */

#include "keynav.h"

/* ---------------------------------------------------------------------------------------------- */

static int keynav_findwindow(keynav_ctx_t *kn, HWND hwnd, size_t *indexp)
{
	size_t i;

	if( (kn->we_cur_idx < kn->we_cnt) &&
		((hwnd == kn->we_buf[kn->we_cur_idx].hwnd) || 
			(hwnd == kn->we_buf[kn->we_cur_idx].hwndsub)) )
	{
		*indexp = kn->we_cur_idx;
		return 1;
	}

	for(i = 0; i < kn->we_cnt; i++)
	{
		if( (hwnd == kn->we_buf[i].hwnd) ||
			(hwnd == kn->we_buf[i].hwndsub) )
		{
			*indexp = i;
			return 1;
		}
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static keynav_wndentry_t *keynav_getnextwindow(keynav_ctx_t *kn, HWND hwndcur, int lookback)
{
	size_t index;

	if(kn->we_cnt == 0)
		return NULL;

	if(hwndcur == kn->hwnd)
	{
		if(!lookback) {
			index = 0;
		} else {
			index = kn->we_cnt - 1;
		}
	}
	else
	{
		if(!keynav_findwindow(kn, hwndcur, &index))
			return NULL;

		if(!lookback)
		{
			if(index == kn->we_cnt - 1)
				index = 0;
			else
				index++;
		}
		else
		{
			if(index == 0)
				index = kn->we_cnt - 1;
			else
				index--;
		}
	}

	kn->we_cur_idx = index;

	return &(kn->we_buf[index]);
}

/* ---------------------------------------------------------------------------------------------- */

static void keynav_selectwindow(keynav_wndentry_t *we)
{
	switch(we->type)
	{
	case KEYNAV_WNDTYPE_EDIT:
		Edit_SetSel(we->hwnd, 0, -1);
		break;
	case KEYNAV_WNDTYPE_COMBOBOX:
		ComboBox_SetEditSel(we->hwnd, 0, -1);
		break;
	}

	SetFocus(we->hwnd);
}

/* ---------------------------------------------------------------------------------------------- */

static int keynav_selectnextwindow(keynav_ctx_t *kn, HWND hwndcur, int lookback)
{
	HWND hwnd;
	keynav_wndentry_t *we;

	hwnd = hwndcur;

	for( ; ; )
	{
		we = keynav_getnextwindow(kn, hwnd, lookback);
		if( (we == NULL) || (we->hwnd == hwndcur) )
			return 0;

		hwnd = we->hwnd;

		if( ! (GetWindowStyle(hwnd) & WS_DISABLED) )
		{
			keynav_selectwindow(we);
			break;
		}
	}
	
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static int keynav_confirm(keynav_ctx_t *kn, HWND hwnd, int ok)
{
	size_t index;

	if(!kn->is_confirm_enabled)
		return 0;

	if(hwnd != kn->hwnd)
	{
		if(!keynav_findwindow(kn, hwnd, &index))
			return 0;
	}

	SendMessage(kn->hwnd, WM_COMMAND, ok ? IDOK : IDCANCEL, (LPARAM)hwnd);
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int keynav_msghandle(keynav_ctx_t *kn, MSG *msg)
{
	if( (kn->hwnd != NULL) && (kn->is_enabled) )
	{
		switch(msg->message)
		{
		case WM_KEYDOWN:
			switch(LOBYTE(msg->wParam))
			{
			case VK_TAB:
				keynav_selectnextwindow(kn, msg->hwnd,
					(GetKeyState(VK_SHIFT) & 0x8000) != 0);
				break;
			case VK_RETURN:
				keynav_confirm(kn, msg->hwnd, 1);
				break;
			case VK_ESCAPE:
				keynav_confirm(kn, msg->hwnd, 0);
				break;
			}
			break;
		}
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

static keynav_wndtype_t keynav_getwndtype(HWND hwnd)
{
	TCHAR buf[64];

	if(GetClassName(hwnd, buf, 64))
	{
		if(_tcsicmp(buf, _T("EDIT")) == 0)
			return KEYNAV_WNDTYPE_EDIT;
		if(_tcsicmp(buf, _T("COMBOBOX")) == 0)
			return KEYNAV_WNDTYPE_COMBOBOX;
	}

	return KEYNAV_WNDTYPE_OTHER;
}

/* ---------------------------------------------------------------------------------------------- */

static HWND keynav_gethwndsub(HWND hwnd, keynav_wndtype_t type)
{
	switch(type)
	{
	case KEYNAV_WNDTYPE_COMBOBOX:
		return GetFirstChild(hwnd);
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

static BOOL CALLBACK keynav_enumproc(HWND hwnd, LPARAM lp)
{
	keynav_ctx_t *kn = (void*)lp;
	keynav_wndentry_t *we_buf_new;
	size_t we_cap_new;
	keynav_wndtype_t wndtype;
	HWND hwndsub;

	if(GetWindowStyle(hwnd) & WS_TABSTOP)
	{
		if(kn->we_cnt == kn->we_cap)
		{
			we_cap_new = kn->we_cap + 32;
			we_buf_new = realloc(kn->we_buf, we_cap_new * sizeof(keynav_wndentry_t));
			if(we_buf_new == NULL)
				return FALSE;
			kn->we_cap = we_cap_new;
			kn->we_buf = we_buf_new;
		}

		wndtype = keynav_getwndtype(hwnd);
		hwndsub = keynav_gethwndsub(hwnd, wndtype);

		kn->we_buf[kn->we_cnt].hwnd = hwnd;
		kn->we_buf[kn->we_cnt].hwndsub = hwndsub;
		kn->we_buf[kn->we_cnt].type = wndtype;

		kn->we_cnt++;
	}

	return TRUE;
}

/* ---------------------------------------------------------------------------------------------- */

static int keynav_findwindows(keynav_ctx_t *kn, HWND hwnd)
{
	kn->we_cnt = 0;
	kn->we_cur_idx = 0;

	EnumChildWindows(hwnd, keynav_enumproc, (LPARAM)kn);

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int keynav_update(keynav_ctx_t *kn, HWND hwnd)
{
	if(kn->hwnd != hwnd)
		return 0;
	
	if(!keynav_findwindows(kn, hwnd))
		return 0;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int keynav_setcurwnd(keynav_ctx_t *kn, HWND hwnd, int is_confirm_enabled)
{
	if(hwnd == NULL)
		return 0;

	if(hwnd == kn->hwnd)
	{
		kn->is_enabled = 1;
		kn->is_confirm_enabled = is_confirm_enabled;
		return 1;
	}

	if(!keynav_findwindows(kn, hwnd))
		return 0;

	kn->hwnd = hwnd;
	kn->is_enabled = 1;
	kn->is_confirm_enabled = is_confirm_enabled;

	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int keynav_unsetcurwnd(keynav_ctx_t *kn, HWND hwnd)
{
	if(hwnd == NULL)
		return 0;

	if(hwnd == kn->hwnd)
	{
		kn->is_enabled = 0;
		return 1;
	}

	return 0;
}

/* ---------------------------------------------------------------------------------------------- */

void keynav_init(keynav_ctx_t *kn, HINSTANCE h_inst)
{
	kn->h_inst = h_inst;
}

/* ---------------------------------------------------------------------------------------------- */

void keynav_cleanup(keynav_ctx_t *kn)
{
	free(kn->we_buf);
}

/* ---------------------------------------------------------------------------------------------- */
