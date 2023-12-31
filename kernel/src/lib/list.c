/**
 * @file lib/list.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Массивы
 * @version 0.3.4
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include	"lib/list.h"

void list_init(list_t* list){
    list->first = nullptr;
    list->count = 0;
    list->mutex = false;
}

void list_add(list_t* list, list_item_t* item){
    if (item->list == nullptr){
        mutex_get(&(list->mutex), true);
        if (list->first){
            item->list = list;
            item->next = list->first;
            item->prev = list->first->prev;
            item->prev->next = item;
            item->next->prev = item;
        } else {
            item->list = list;
            item->next = item;
            item->prev = item;
            list->first = item;
        }

        list->count++;
        mutex_release(&(list->mutex));
    }
}

void list_remove(list_item_t* item){
    mutex_get(&(item->list->mutex), true);

    if (item->list->first == item) {
        item->list->first = item->next;
        if (item->list->first == item){
            item->list->first = nullptr;
        }
    }
    item->next->prev = item->prev;
    item->prev->next = item->next;
    item->list->count--;
    mutex_release(&(item->list->mutex));
}
