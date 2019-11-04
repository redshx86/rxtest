/* ---------------------------------------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------------------------------------- */

#define DLIST_INSBACK(p, head, tail, fwdref, bkref) \
	if(head == NULL) { \
		head = tail = p; \
		(p)->fwdref = (p)->bkref = NULL; \
	} else { \
		(p)->fwdref = NULL; \
		(p)->bkref = tail; \
		(tail)->fwdref = p; \
		tail = p; \
	} \

/* ---------------------------------------------------------------------------------------------- */

#define DLIST_INSFRONT(p, head, tail, fwdref, bkref) \
	if(head == NULL) { \
		head = tail = p; \
		(p)->fwdref = (p)->bkref = NULL; \
	} else { \
		(p)->fwdref = head; \
		(p)->bkref = NULL; \
		(head)->bkref = p; \
		head = p; \
	} \

/* ---------------------------------------------------------------------------------------------- */

#define DLIST_INSAFTER(cur, p, tail, fwdref, bkref) \
	{ \
		(p)->fwdref = (cur)->fwdref; \
		(p)->bkref = cur; \
		if((cur)->fwdref != NULL) \
			(cur)->fwdref->bkref = p; \
		else \
			tail = p; \
		(cur)->fwdref = p; \
	} \

/* ---------------------------------------------------------------------------------------------- */

#define DLIST_INSBEFORE(cur, p, head, fwdref, bkref) \
	{ \
		(p)->bkref = (cur)->bkref; \
		(p)->fwdref = cur; \
		if((cur)->bkref != NULL) \
			(cur)->bkref->fwdref = p; \
		else \
			head = p; \
		(cur)->bkref = p; \
	} \

/* ---------------------------------------------------------------------------------------------- */

#define DLIST_REMOVE(p, head, tail, fwdref, bkref) \
	{ \
		if((p)->fwdref != NULL) \
			(p)->fwdref->bkref = (p)->bkref; \
		else \
			tail = (p)->bkref; \
		if((p)->bkref != NULL) \
			(p)->bkref->fwdref = (p)->fwdref; \
		else \
			head = (p)->fwdref; \
		(p)->fwdref = (p)->bkref = NULL; \
	} \

/* ---------------------------------------------------------------------------------------------- */
