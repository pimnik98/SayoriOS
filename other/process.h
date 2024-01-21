#ifndef		PROCESS_H
#define		PROCESS_H

#include	"types.h"
#include	"thread.h"
#include	"memory.h"
#include	"scheduler.h"
#include	"system.h"
#include	"elf_loader.h"
#include	"vconsole.h"

typedef struct elf_sections elf_sections_t;
typedef struct virtual_tty	virtual_tty_t;

/*-----------------------------------------------------------------------------
 * 		Process structure
 *---------------------------------------------------------------------------*/
typedef struct process process_t;

typedef	struct process
{
	list_item_t		list_item;			/* List item */
	physaddr_t		page_dir;			/* Page directory */
	size_t			threads_count;		/* Count of threads */
	bool			suspend;			/* Suspend flag */
	u32int			pid;				/* Process ID (PID) */
	char			name[256];			/* Process name */

	void*			page_dir_vaddr;
	void*			user_stack_vaddr;

	physaddr_t		stack_paddr;
	physaddr_t		user_stack_paddr;
	size_t			stack_page_count;

	physaddr_t		seg_paddr;
	size_t			seg_page_count;

	physaddr_t		heap_paddr;
	size_t			heap_page_count;
	physaddr_t		blocks_paddr;
	size_t			blocks_page_count;

	u32int			thread_id[1024];

	virtual_tty_t*	tty;

	thread_t*		parent_proc_thread;

}__attribute__((packed)) process_t;



u32int exec_proc(char* name, bool kernel, char* cmd_line);

u32int usr_exec(char* name, char* cmd_line);

void destroy_proc(void);

u32int get_pid(void);

void start_elf(elf_sections_t* elf, process_t* proc, char* cmd_line);
