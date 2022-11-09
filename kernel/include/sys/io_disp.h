/*-----------------------------------------------------------------------------
 *
 * 		I/O ports dispatcher
 * 		(c) maisvendoo, 22.08.2013
 *
 *---------------------------------------------------------------------------*/
#ifndef		IO_DISP_H
#define		IO_DISP_H

#include	"common.h"
#include	"sys/sync.h"

#define		PORTS_NUM	0x10000

/* */
void init_io_dispatcher(void);

/* */
uint8_t in_byte(uint16_t port);

/* */
void out_byte(uint16_t port, uint8_t value);

#endif
