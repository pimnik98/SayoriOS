#ifndef		SCHEDULER_H
#define		SCHEDULER_H

#include	"common.h"
#include	"lib/list.h"
#include	"mem/pmm.h"

#define DEFAULT_STACK_SIZE 0x4000

/*-----------------------------------------------------------------------------
 * 		Process structure
 *---------------------------------------------------------------------------*/
typedef	struct
{
	list_item_t		list_item;		/* List item */
	physical_addr_t	page_dir;		/* Page directory */
	size_t			threads_count;	/* Count of threads */
	bool			suspend;		/* Suspend flag */
	uint32_t			pid;		/* Process ID (PID) */
	char			name[256];		/* Process name */
    // Every process should have a path that process operates
}__attribute__((packed)) process_t;

/*-----------------------------------------------------------------------------
 * 		Thread structure
 *---------------------------------------------------------------------------*/
typedef	struct
{
	list_item_t		list_item;			/* List item */
	process_t*		process;			/* This thread's process */
	bool			suspend;			/* Suspend flag */
	size_t			stack_size;			/* Size of thread's stack */
	void*			stack;
	uint32_t			esp;				/* Thread state */
	uint32_t			entry_point;
	uint32_t			id;				/* Thread ID */
	uint32_t			stack_top;
//    uint32_t time_high;
//    uint32_t time_low;  // Time accounting
}__attribute__((packed)) thread_t;

/* Initialization */
void init_task_manager(void);

extern void task_switch(void);

/* Create new thread */
thread_t* thread_create(process_t* proc,
	               	    void* entry_point,
	               	    size_t stack_size,
	               	    bool kernel,
	               	    bool suspend);

/* Get current process */
process_t* get_current_proc(void);

/* Suspend thread */
void thread_suspend(thread_t* thread, bool suspend);

/* Exit from thread */
void thread_exit(thread_t* thread);

void create_process(void* entry_point, char* name, bool suspend, bool is_kernel);

/* Check multitask flag */
bool is_multitask(void);

/* Switch to user mode */
extern void user_mode_switch(void* entry_point, uint32_t user_stack_top);

/* Init user mode */
void init_user_mode(void* entry_point, size_t stack_size);

void scheduler_mode(bool on);

#endif
