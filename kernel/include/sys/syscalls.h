#ifndef		SYSCALLS_H
#define		SYSCALLS_H

#include	"common.h"
#include	"sys/isr.h"
#include	"sys/io_disp.h"

#define		NUM_CALLS	8

/*-----------------------------------------------------------------------------
 *		Syscall interrupt number
 *---------------------------------------------------------------------------*/
#define		SYSCALL					0x50

/*-----------------------------------------------------------------------------
 *		System functions
 *---------------------------------------------------------------------------*/
#define		PORT_INPUT_BYTE			0x00
#define		PORT_OUTPUT_BYTE		0x01
#define		TTYCTL					0x02
#define		SH_ENV					0x03
#define		SH_ENV_DEBUG			0x04
#define		SCREENCTL				0x05
#define		MEMCTL					0x06
#define		KBDCTL					0x07

/*-----------------------------------------------------------------------------
 *		Macro for system call declaration
 *---------------------------------------------------------------------------*/

/* No params */
#define		DECL_SYSCALL0(func) int syscall_##func()

#define		DEFN_SYSCALL0(func, num)\
\
int syscall_##func()\
{	int ret = 0;\
	asm volatile ("int $0x50":"=a"(ret):"a"(num));\
	return ret;\
}

/*---- One param ------------------------------------------------------------*/
#define		DECL_SYSCALL1(func, p1) int syscall_##func(p1)

#define		DEFN_SYSCALL1(func, num, P1)\
\
int syscall_##func(P1 p1)\
{	int ret = 0;\
	asm volatile ("int $0x50":"=a"(ret):"a"(num),"b"(p1));\
	return ret;\
}

/*---- Two params -----------------------------------------------------------*/
#define		DECL_SYSCALL2(func, p1, p2) int syscall_##func(p1, p2)

#define		DEFN_SYSCALL2(func, num, P1, P2)\
\
int syscall_##func(P1 p1, P2 p2)\
{	int ret = 0;\
	asm volatile ("int $0x50":"=a"(ret):"a"(num),"b"(p1),"c"(p2));\
	return ret;\
}

/*---- Three params -----------------------------------------------------------*/
#define		DECL_SYSCALL3(func, p1, p2, p3) int syscall_##func(p1, p2, p3)

#define		DEFN_SYSCALL3(func, num, P1, P2, P3)\
\
int syscall_##func(P1 p1, P2 p2, P3 p3)\
{	int ret = 0;\
	asm volatile ("int $0x50":"=a"(ret):"a"(num),"b"(p1),"c"(p2), "d"(p3));\
	return ret;\
}



/*-----------------------------------------------------------------------------
 *		System calls control
 *---------------------------------------------------------------------------*/
void init_syscalls(void);
extern size_t syscall_entry_call(void* entry_point, void* param1, void* param2, void* param3);
void syscall_handler(registers_t regs);
int get_digit(int dig);

/*-----------------------------------------------------------------------------
 *		System calls declaration
 *---------------------------------------------------------------------------*/
DECL_SYSCALL1(in_byte, uint16_t);
DECL_SYSCALL2(out_byte, uint16_t, uint8_t);
DECL_SYSCALL2(tty_ctl, void*, void*);
// ...
DECL_SYSCALL2(screen_ctl, size_t, size_t);

#endif
