#include "common.h"
#include "net/cards.h"
#include "mem/vmm.h"

netcard_entry_t** netcards_list = 0;
size_t netcards_list_capacity = 0;
size_t netcards_count = 0;

void netcards_list_init() {
    netcards_list_capacity = 1;

    netcards_list = kcalloc(netcards_list_capacity, sizeof(netcard_entry_t*));
}

void netcard_add(netcard_entry_t *card) {
    netcards_list_capacity++;
    netcards_list = krealloc(netcards_list, sizeof(netcard_entry_t*) * netcards_list_capacity);

    netcards_list[netcards_count++] = card;
}

size_t netcards_get_count() {
    return netcards_count;
}

netcard_entry_t* netcard_get(size_t index) {
    if(index >= netcards_count)
        return 0;

    return netcards_list[index];
}