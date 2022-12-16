#ifndef		LIST_H
#define		LIST_H

#include	"common.h"
#include	"sys/sync.h"

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
typedef	struct	_list_item_t	list_item_t;

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
typedef struct
{
	list_item_t*	first;
	size_t		count;
	mutex_t		mutex;
	
} list_t;

/*-----------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
struct _list_item_t
{
	list_item_t*	prev;
	list_item_t*	next;
	list_t*			list;
};

void list_init(list_t* list);

void list_add(list_t* list, list_item_t* item);

void list_remove(list_item_t* item);

#endif
