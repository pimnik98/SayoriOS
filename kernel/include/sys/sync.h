#ifndef		SYNC_H
#define		SYNC_H

#include	"common.h"

typedef		bool	mutex_t;

/* Get mutex */
bool mutex_get(mutex_t* mutex, bool wait);
/* Release mutex */
void mutex_release(mutex_t* mutex);

#endif
