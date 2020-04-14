/* ---------------------------------------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------------------------------------- */

typedef void (*uievent_handler_proc_t)(void *ctx, unsigned int msg, void *param);

typedef struct uievent_handler {
	uievent_handler_proc_t proc;
	void *ctx;
} uievent_handler_t;

typedef struct uievent {
	char *name;
	size_t handler_cap;
	size_t handler_cnt;
	uievent_handler_t *handler;
} uievent_t;

typedef struct uievent_list {
	size_t event_cap;
	size_t event_cnt;
	uievent_t *event;
} uievent_list_t;

/* ---------------------------------------------------------------------------------------------- */

void uievent_init(uievent_list_t *event_list);
void uievent_free(uievent_list_t *event_list);

uievent_t *uievent_register(uievent_list_t *event_list, const char *name);
int uievent_send(uievent_t *event, unsigned int msg, void *param);

int uievent_handler_add(uievent_list_t *event_list, const char *name,
						void *ctx, uievent_handler_proc_t proc);

int uievent_handler_remove(uievent_list_t *event_list, const char *name,
						   void *ctx, uievent_handler_proc_t proc);

/* ---------------------------------------------------------------------------------------------- */
