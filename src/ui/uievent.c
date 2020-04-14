/* ---------------------------------------------------------------------------------------------- */

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "uievent.h"

/* ---------------------------------------------------------------------------------------------- */

void uievent_init(uievent_list_t *event_list)
{
	event_list->event_cap = 0;
	event_list->event_cnt = 0;
	event_list->event = NULL;
}

/* ---------------------------------------------------------------------------------------------- */

void uievent_free(uievent_list_t *event_list)
{
	size_t i;

	for(i = 0; i < event_list->event_cnt; i++)
	{
		free(event_list->event[i].name);
		free(event_list->event[i].handler);
	}
	free(event_list->event);
	event_list->event_cap = 0;
	event_list->event_cnt = 0;
	event_list->event = NULL;
}

/* ---------------------------------------------------------------------------------------------- */

static uievent_t *uievent_find(uievent_list_t *event_list, const char *name)
{
	size_t i;

	for(i = 0; i < event_list->event_cnt; i++)
	{
		if(strcmp(name, event_list->event[i].name) == 0)
			return event_list->event + i;
	}

	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

uievent_t *uievent_register(uievent_list_t *event_list, const char *name)
{
	size_t cap_new;
	uievent_t *events_new, *event;

	if( (event = uievent_find(event_list, name)) != NULL )
		return event;

	if(event_list->event_cnt == event_list->event_cap)
	{
		cap_new = event_list->event_cap + 32;
		events_new = realloc(event_list->event, cap_new * sizeof(uievent_t));
		if(events_new == NULL)
			return NULL;
		event_list->event_cap = cap_new;
		event_list->event = events_new;
	}

	event = event_list->event + event_list->event_cnt;
	event->name = strdup(name);
	event->handler_cap = 0;
	event->handler_cnt = 0;
	event->handler = NULL;
	if(event->name == NULL)
		return NULL;
	event_list->event_cnt++;
	return event;
}

/* ---------------------------------------------------------------------------------------------- */

int uievent_send(uievent_t *event, unsigned int msg, void *param)
{
	size_t i;

	if(event == NULL)
		return 0;
	for(i = 0; i < event->handler_cnt; i++)
		event->handler[i].proc(event->handler[i].ctx, msg, param);
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

static uievent_handler_t *uievent_handler_find(
	uievent_t *event, void *ctx, uievent_handler_proc_t proc)
{
	size_t i;

	for(i = 0; i < event->handler_cnt; i++)
	{
		if((event->handler[i].ctx == ctx) && (event->handler[i].proc == proc))
			return event->handler + i;
	}
	return NULL;
}

/* ---------------------------------------------------------------------------------------------- */

int uievent_handler_add(uievent_list_t *event_list, const char *name,
						void *ctx, uievent_handler_proc_t proc)
{
	uievent_t *event;
	uievent_handler_t *handlers_new, *handler;
	size_t cap_new;

	if( (event = uievent_register(event_list, name)) == NULL )
		return 0;
	if(uievent_handler_find(event, ctx, proc))
		return 0;

	if(event->handler_cnt == event->handler_cap)
	{
		cap_new = event->handler_cap + 8;
		handlers_new = realloc(event->handler, cap_new * sizeof(uievent_handler_t));
		if(handlers_new == NULL)
			return 0;
		event->handler_cap = cap_new;
		event->handler = handlers_new;
	}

	handler = event->handler + event->handler_cnt;
	handler->ctx = ctx;
	handler->proc = proc;
	event->handler_cnt++;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */

int uievent_handler_remove(uievent_list_t *event_list, const char *name,
							void *ctx, uievent_handler_proc_t proc)
{
	uievent_t *event;
	uievent_handler_t *handler;
	size_t index;

	if( (event = uievent_find(event_list, name)) == NULL )
		return 0;
	if( (handler = uievent_handler_find(event, ctx, proc)) == NULL )
		return 0;

	index = handler - event->handler;
	memmove(event->handler + index, event->handler + index + 1,
		(event->handler_cnt - index - 1) * sizeof(uievent_handler_t));
	event->handler_cnt--;
	return 1;
}

/* ---------------------------------------------------------------------------------------------- */
