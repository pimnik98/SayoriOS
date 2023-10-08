/**
 * @file sys/trigger.c
 * @authors Пиминов Никита (github.com/pimnik98 | VK: @piminov_remont)
 * @brief Система триггеров
 * @version 0.3.3
 * @date 2023-06-01
 *
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 *
 */
#include <kernel.h>
#include <io/ports.h>
#include <io/duke_image.h>
#include <io/status_loggers.h>
#include <lib/stdio.h>
#include <sys/trigger.h>
#include <drv/input/keymap.h>

trigger_t Triggers[1024];		///< Сетка смонтированных триггеров
size_t TriggersCount = 0;		///< Колво активных триггеров


thread_t* threadASYNCRUN01;
thread_t* threadASYNCRUN02;
thread_t* threadASYNCRUN03;
thread_t* threadASYNCRUN04;

/**
 * @brief Регистрация триггера
 */
size_t RegTrigger(int type,trigger_cmd_t handler){
	///< Попытка регистрации триггера
	qemu_log("[Trigger] An attempt to register trigger '%x' was detected.",type);
	Triggers[TriggersCount].index = TriggersCount;
	Triggers[TriggersCount].type = type;
	Triggers[TriggersCount].ready = 1;
	Triggers[TriggersCount].cmd = handler;
	qemu_log("[Trigger] Format %x trigger has successfully registered and has index number %d.",type,TriggersCount);
	TriggersCount++;
	return (TriggersCount)-1;
}

/**
 * @brief Включить триггер
 */
void OnTrigger(int index){
	qemu_log("[Trigger] Trigger #%d has been enabled",index);
	Triggers[index].ready = 1;
}

/**
 * @brief Выключить триггер
 */
void OffTrigger(int index){
	qemu_log("[Trigger] Trigger #%d has been disabled",index);
	Triggers[index].ready = 0;
}

/**
 * @brief Функция для вызовов триггеров (Если самостоятельно надо вызвать триггер)
 */
void CallTrigger(int type, void* data1, void* data2, void* data3, void* data4, void* data5){
	if (type == TRIGGER_KEY_PRESSED) {
		qemu_log("[Trigger] [Keyboard] Press: %d", (int) data1);
	} else if (type == TRIGGER_MOUSE_MOVE) {
		qemu_log("[Trigger] [Mouse] X: %d | Y: %d", (int) data1, (int) data2);
	} else if (type == TRIGGER_MOUSE_CLICK) {
		qemu_log("[Trigger] [Mouse] LEFT: %d | RIGHT: %d | CENTER: %d", (int) data1, (int) data2, (int) data3);
	} else if (type == TRIGGER_MAIN_RUNNER1 ||
			   type == TRIGGER_MAIN_RUNNER2 ||
			   type == TRIGGER_MAIN_RUNNER3 ||
			   type == TRIGGER_MAIN_RUNNER4
			) {
		//qemu_log("[Trigger] [ASYNC] [RUNNER] Thread: %x",type);
	} else if (type == TRIGGER_ELF_START) {
		qemu_log("[Trigger] ELF Start\n\tPath:%s", (char *) data1);
	} else if (type == TRIGGER_ELF_END) {
		qemu_log("[Trigger] ELF Complete\n\tPath:%s\n\tCode:%d", (char *) data1, (int) data2);
	} else if (type == TRIGGER_ELF_START) {
		qemu_log("[Trigger] ELF Error\n\tPath:%s\n\tCode:%d", (char *) data1, (int) data2);
	} else {
		qemu_log("[Trigger] An attempt to call trigger '%d' was detected.", type);
	}
	for (size_t inx = 0; inx < TriggersCount; inx++){
		//if (log) qemu_log("[Trigger] Scan: %d | Type: %x | Ready: %d",inx,Triggers[inx]->type,Triggers[inx]->ready);
		if (type == Triggers[inx].type && Triggers[inx].ready == 1){
			///< Проверяем, что оно дейстивельно для данного типа и делаем вызов, если устройство в сети
			Triggers[inx].cmd(data1, data2, data3, data4, data5);
		}
	}
}

/**
 * @brief Специальный поток №1
 */
void ASYNC_RUNNER_1(){
    qemu_log("[ASYNC] [RUN:1] Started....");
    while (1) {
		CallTrigger(TRIGGER_MAIN_RUNNER1,0,0,0,0,0);
        //punch();
		//sleep_ms(100);
    }
    thread_exit(threadASYNCRUN01);
}

/**
 * @brief Специальный поток №2
 * Обновляется один раз в 500 тиков.
 */
void ASYNC_RUNNER_2(){
    qemu_log("[ASYNC] [RUN:2] Started....");
	size_t old = 0;
    while (1) {
		if (old+500 < getTicks()) continue;
		CallTrigger(TRIGGER_MAIN_RUNNER2,0,0,0,0,0);
		old = getTicks();
        //punch();
		//sleep_ms(100);
    }
    qemu_log("[ASYNC] [RUN:2] CRASHED!!!");
    thread_exit(threadASYNCRUN02);
}

/**
 * @brief Специальный поток №3
 */
void ASYNC_RUNNER_3(){
    qemu_log("[ASYNC] [RUN:3] Started....");
    while (1) {
		CallTrigger(TRIGGER_MAIN_RUNNER3,0,0,0,0,0);
        //punch();
		sleep_ms(100);
    }
    qemu_log("[ASYNC] [RUN:3] CRASHED!!!");
    thread_exit(threadASYNCRUN03);
}

/**
 * @brief Специальный поток №1
 */
void ASYNC_RUNNER_4(){
    qemu_log("[ASYNC] [RUN:4] Started....");
    while (1) {
		CallTrigger(TRIGGER_MAIN_RUNNER4,0,0,0,0,0);
        //punch();
		sleep_ms(100);
    }
    qemu_log("[ASYNC] [RUN:4] CRASHED!!!");
    thread_exit(threadASYNCRUN04);
}



/**
 * @brief Инициализация триггеров
 */
void triggersConfig(){
	qemu_log("[Trigger] Configurate...");

	process_t* proc = get_current_proc();
    threadASYNCRUN01 = thread_create(proc,
			   &ASYNC_RUNNER_1,
			   0x4000,
			   true,
			   false);
    threadASYNCRUN02 = thread_create(proc,
			   &ASYNC_RUNNER_2,
			   0x4000,
			   true,
			   false);
	//Triggers = kmalloc(sizeof(trigger_t)*1024);	///< Выделяем память на триггеры
}

