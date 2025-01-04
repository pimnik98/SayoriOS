/**
 * @file src/os/main.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief SayoriOS Whisper (Точка входа в ОС)
 * @version 0.4.0
 * @date 2025-01-03
 * @copyright Copyright SayoriOS Team (c) 2025
 */
#include <stdio.h>
#include <string.h>
#include "../devices/loader.h"
#include "libs/psf_v1.h"
#include "modules/registry.h"

//#include "../loader.h"

/**
 * @brief Точка входа в ОС
 */
void whisper(){
    /// Инициализация шрифтов
    int psf = psf1_init("./Whisper/Fonts/EN-RU.psf");

    
  
  for (int x = 0; x <display_width();  x++){
        for (int y = 0; y < display_height(); y++){
            display_set_pixel(x, y, 0xFFFFFFFF);
        }
    }

    /**
     * Черновик по реестрам

    Registry registry;
    registry_init(&registry);

    registry_create_path(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS");
    registry_set_value(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Version", "1.0");
    registry_set_value(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Soft", "false");

    char version[REG_MAX_VALUE_LEN];
    char soft[REG_MAX_VALUE_LEN];
    char lang[REG_MAX_VALUE_LEN];
    char lang2[REG_MAX_VALUE_LEN];
    char lang3[REG_MAX_VALUE_LEN];
    registry_get_value(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Version", version, sizeof(version));
    registry_get_value(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Soft", soft, sizeof(soft));

    registry_create_path(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings");
    registry_set_value(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings\\Lang", "eng");

    registry_get_value(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings\\Lang", lang, sizeof(lang));
    
    psf1_write_str(lang, strlen(lang), 32, 80, 0xFFAAAAAA);
    // Удаление только ключа "Settings"
    if(registry_delete_key(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings")) {
        //debug("Key 'Settings' deleted.\n");
    } else {
        //debug("Failed to delete key 'Settings'.\n");
    }
    
    registry_get_value(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings\\Lang", lang2, sizeof(lang2));
    psf1_write_str(lang2, strlen(lang2), 32, 96, 0xFFAAAAAA);
    //Создание пути заново
    registry_create_path(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings");
     registry_set_value(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings\\Lang", "ru");
    // Удаление пути "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings"
      if(registry_delete_path(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings")) {
        //debug("Path 'HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings' deleted.\n");
    } else {
        //debug("Failed to delete path 'HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings'.\n");
    }
    
    registry_get_value(&registry, "HKEY_LOCAL_MACHINE\\SOFTWARE\\SayoriOS\\Settings\\Lang", lang3, sizeof(lang3));
    psf1_write_str(lang3, strlen(lang3), 32, 114, 0xFFAAAAAA);


    
    psf1_write_str(version, strlen(version), 32, 48, 0xFFDDDDDD);
    psf1_write_str(soft, strlen(soft), 32, 60, 0xFFDDDDDD);

     */
    
    char* PSF1_TEST = "SayoriOS Whisper | PSF1 Passed";
    psf1_write_str(PSF1_TEST, strlen(PSF1_TEST), 32, 32, 0xFFAAAAAA);


    display_update();

    while(1){

    }
}