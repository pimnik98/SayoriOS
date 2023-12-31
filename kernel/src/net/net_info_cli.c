//
// Created by ndraey on 01.11.23.
//

#include <common.h>

#include "net/cards.h"
#include "io/tty.h"
#include "io/ports.h"

uint32_t CLI_CMD_NET(uint32_t c, char **v) {
	uint8_t mac_buffer[6] = {0};

	for(int i = 0; i < netcards_get_count(); i++) {
		netcard_entry_t* entry = netcard_get(i);

		_tty_printf("%s\n", entry->name);
		entry->get_mac_addr(mac_buffer);

		_tty_printf("\t|- MAC адрес: %v:%v:%v:%v:%v:%v\n",
					mac_buffer[0],
					mac_buffer[1],
					mac_buffer[2],
					mac_buffer[3],
					mac_buffer[4],
					mac_buffer[5]
		);

		_tty_printf("\t|- IPv4 адрес: %d.%d.%d.%d\n",
					 entry->ipv4_addr[0],
					 entry->ipv4_addr[1],
					 entry->ipv4_addr[2],
					 entry->ipv4_addr[3]
					 );
	}

	return 0;
}