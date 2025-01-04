/**
 * @file src/devices/psp/loader.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Загрузщик ОС для PSP
 * @version 0.4.0
 * @date 2025-01-03
 * @copyright Copyright SayoriOS Team (c) 2025
 */
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspthreadman.h>
#include <psprtc.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "../loader.h"
#include "../../os/libs/psf_v1.h"

PSP_MODULE_INFO("Whisper",0,4,0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

void whisper();

int psp_setup_callback_exit(int arg1, int arg2, void* common){
    sceKernelExitGame();
    return 0;
}

int psp_setup_callback_thread(SceSize args, void* argp){
    int cbid = sceKernelCreateCallback("Exit callback", psp_setup_callback_exit, NULL);
    sceKernelRegisterExitCallback(cbid);

    sceKernelSleepThreadCB();
    return 0;

}

int psp_setup_callbacks() {
    int thid = sceKernelCreateThread("update_thread", psp_setup_callback_thread, 0x11, 0xFA0, 0,NULL);
    if (thid >= 0){
        sceKernelStartThread(thid,0,NULL);
    }
}


int main(){
    /// Инициализация
    psp_setup_callbacks();
    pspDebugScreenInit();
    psp_display_init();
    
    whisper();

    /// Выход
    sceKernelExitGame();
    return 0;
}