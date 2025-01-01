/**
 * @file sys/trigger.c
 * @authors Пиминов Никита (github.com/pimnik98 | VK: @piminov_remont)
 * @brief Система триггеров
 * @version 0.3.5
 * @date 2023-06-01
 *
 * @warning А зачем они нам нужны если более выгодно вручную проверять клавиатуру и мышь, при этом производительность падает при триггерах
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 *
 */
#include <io/ports.h>
#include <io/status_loggers.h>
#include <lib/stdio.h>
#include <sys/trigger.h>
#include <drv/input/keymap.h>
#include "sys/scheduler.h"
#include "sys/timer.h"

trigger_t Triggers[1024] = {0};		///< Сетка смонтированных триггеров
size_t TriggersCount = 0;		    ///< Колво активных триггеров

int _FoundFreeIndexTrigger(){
    for(int i = 0; i < 1024; i++){
        if (Triggers[i].is_not_delete == 0) return i;
    }
    return -1;
}

/**
 * @brief Регистрация триггера
 *
 * @return int - номер триггера, в противном случае -1 если ошибка
 */
int RegTrigger(int type,trigger_cmd_t handler){
	///< Попытка регистрации триггера
	qemu_log("[Trigger] An attempt to register trigger '%x' was detected.",type);
    int idx = _FoundFreeIndexTrigger();
    if (idx == -1) {
        qemu_err("[Trigger] ERROR NO FREE TRIGGERS SPACE!!");
        return -1;
    }
	Triggers[idx].index = TriggersCount;
	Triggers[idx].type = type;
	Triggers[idx].ready = 1;
    Triggers[idx].is_not_delete = 1;
	Triggers[idx].cmd = handler;
	qemu_ok("[Trigger] Format %x trigger has successfully registered and has index number %d.",type,idx);
	TriggersCount++;
	return idx;
}

/**
 * @brief Удаление триггера
 */
void DeleteTrigger(int index){
    if (index > 1024 || Triggers[index].is_not_delete == 0){
        qemu_err("[Trigger] Unknown #%d trigger",index);
        return;
    }
    qemu_log("[Trigger] Trigger #%d has been deleted",index);
    Triggers[index].is_not_delete = 0;
    Triggers[index].ready = 0;
}

/**
 * @brief Включить триггер
 */
void OnTrigger(int index){
    if (index > 1024 || Triggers[index].is_not_delete == 0){
        qemu_err("[Trigger] Unknown #%d trigger",index);
        return;
    }

	Triggers[index].ready = 1;
}

/**
 * @brief Выключить триггер
 */
void OffTrigger(int index){
    if (index > 1024 || Triggers[index].is_not_delete == 0){
        qemu_err("[Trigger] Unknown #%d trigger",index);
        return;
    }
	qemu_log("[Trigger] Trigger #%d has been disabled",index);
	Triggers[index].ready = 0;
}

/**
 * @brief Функция для вызовов триггеров (Если самостоятельно надо вызвать триггер)
 */
void CallTrigger(int type, void* data1, void* data2, void* data3, void* data4, void* data5){
	for (size_t inx = 0; inx < TriggersCount; inx++){
		//if (log) qemu_log("[Trigger] Scan: %d | Type: %x | Ready: %d",inx,Triggers[inx]->type,Triggers[inx]->ready);
		if (type == Triggers[inx].type && Triggers[inx].ready){
			///< Проверяем, что оно дейстивельно для данного типа и делаем вызов, если устройство в сети
			Triggers[inx].cmd(data1, data2, data3, data4, data5);
		}
	}
}

/**
 * @brief Инициализация триггеров
 */
void triggersConfig(){
	qemu_log("[Trigger] Configurate...");

	//Triggers = kmalloc(sizeof(trigger_t)*1024);	///< Выделяем память на триггеры
}

