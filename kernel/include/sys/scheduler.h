#ifndef		SCHEDULER_H
#define		SCHEDULER_H

#include	"common.h"
#include	"lib/list.h"
#include	"mem/pmm.h"

#define DEFAULT_STACK_SIZE 0x8000

typedef enum {
    CREATED = 0,
    RUNNING,
    PAUSED,
    DEAD
} thread_state_t;

SAYORI_INLINE const char* thread_state_string(thread_state_t state) {
    switch (state) {
        case CREATED:
            return "CREATED";
        case RUNNING:
            return "RUNNING";
        case PAUSED:
            return "PAUSED";
        case DEAD:
            return "DEAD";
        default:
            return "UNKNOWN";
    }
}

typedef	volatile struct {
    // 0
	list_item_t		list_item;		/* List item */
	// 12
    physical_addr_t	page_dir;		/* Page directory */
	// 16
    size_t			threads_count;	/* Count of threads */
    // 20
	bool			suspend;		/* Suspend flag */
    // 24
	uint32_t			pid;		/* Process ID (PID) */
    // 28
    virtual_addr_t page_dir_virt;	/* Virtual address of page directory */
    // 32
	char			name[256];		/* Process name */
	// 32 + 256
	size_t          page_tables_virts[1024];    /* Page table addresses */
    // Every process should have a path that process operates
} __attribute__((packed)) process_t;

/*-----------------------------------------------------------------------------
 * 		Thread structure
 *---------------------------------------------------------------------------*/
typedef volatile struct
{
    // 0
	list_item_t		list_item;			/* List item */
    // 12
	process_t*		process;			/* This thread's process */
    // 16
	bool			suspend;			/* Suspend flag */
    // 20
	size_t			stack_size;			/* Size of thread's stack */
    // 24
	void*			stack;
    // 28
	uint32_t			esp;				/* Thread state */
    // 32
	uint32_t			entry_point;
    // 36
	uint32_t			id;				/* Thread ID */
    // 40
	uint32_t			stack_top;
    // registers here [44]
    uint32_t	eax, ebx, ecx, edx, esi, edi, ebp;
    // 72
    thread_state_t state;
} __attribute__((packed)) thread_t;

/* Initialization */
void init_task_manager(void);

extern void task_switch(registers_t regs);
void task_switch_v2_wrapper(__attribute__((unused)) registers_t regs);
extern void task_switch_v2(thread_t*, thread_t*);

thread_t* _thread_create_unwrapped(process_t* proc, void* entry_point, size_t stack_size,
                                   bool kernel, bool suspend);

void kill_process(size_t id);

/* Create new thread */
thread_t* thread_create(process_t* proc,
	               	    void* entry_point,
	               	    size_t stack_size,
	               	    bool kernel,
	               	    bool suspend);

/* Get current process */
volatile process_t * get_current_proc(void);

/* Suspend thread */
void thread_suspend(thread_t* thread, bool suspend);

/* Exit from thread */
void thread_exit(thread_t* thread);

size_t create_process(void* entry_point, char name[256], bool suspend, bool is_kernel);

/* Check multitask flag */
bool is_multitask(void);

/* Switch to user mode */
extern void user_mode_switch(void* entry_point, uint32_t user_stack_top);

/* Init user mode */
void init_user_mode(void* entry_point, size_t stack_size);

int32_t spawn(const char *name, int argc, char* eargv[]);

void scheduler_mode(bool on);

#endif
