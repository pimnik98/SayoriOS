/**
 * @file sys/bootscreen.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief BootScreen - Анимация загрузки ядра
 * @version 0.3.5
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */
#include <version.h>
#include <io/ports.h>
#include "io/tty.h"

uint32_t theme = 0;					///< Текущая тема (0 или 1)
uint32_t bgColorDark = 0x000000;	///< Цвет заднего фона для темной темы
uint32_t txColorDark = 0x92D7D4;	///< Цвет текста для темной темы
uint32_t bgColorLight = 0xD6D2D0;	///< Цвет заднего фона для светлой темы
uint32_t txColorLight = 0x2D3C5D;	///< Цвет текста для светлой темы
uint32_t maxStrLine = 0;			///< Максимальное количество символом на строку
uint32_t maxHeightLine = 0;			///< Максимальное количество строк на экране
bool lazy = false;					///< Ленивая прорисовка
uint32_t curElem = 0;				///< Текущая позиция элемента
uint32_t maxElem = 10;				///< Максимальное позиция элемента
uint32_t mode = 0;  				///< Режим работы (0 - Обычный | 1 - Режим логирования)
bool bs_logs = true;                ///< Включено ли логгирование этапов BootScreen

/**
 * @brief Включить ленивую загрузку для BootScreen
 *
 * @param l - true/false - Вкл/Выкл.
 */
void bootScreenLazy(bool l){
    lazy = l;
}

/**
 * @brief Сменить тему BootScreen
 *
 * @param th - 0 - Dark | 1 - Light
 */
void bootScreenChangeTheme(uint32_t th){
    theme = th;
}

/**
 * @brief Возращает цвет оформления
 *
 * @return uint32_t - код цвета
 */
uint32_t bootScreenTheme(uint32_t type){
    uint32_t map[2][3] = {
			{txColorDark, bgColorDark, 0x262626},
			{txColorLight, bgColorLight, 0x262626}
	};

	if(theme >= 2 || type >= 3) {
		return txColorDark;
	}

	return map[theme][type];

//	if (theme == 0 && type == 0) {
//        return txColorDark;
//    } else if (theme == 0 && type == 1){
//        return bgColorDark;
//    } else if (theme == 0 && type == 2){
//        return 0x262626;
//    } else if (theme == 1 && type == 0){
//        return txColorLight;
//    } else if (theme == 1 && type == 1){
//        return bgColorLight;
//    } else if (theme == 1 && type == 2){
//        return 0x262626;
//    } else {
//        return txColorDark;
//    }
}

/**
 * @brief Завершает работу BootScreen
 *
 * @param bg - Отчистить указаным цветом экран
 * @param tx - Установить цвет для вывода текста
 */
void bootScreenClose(uint32_t bg, uint32_t tx){
    tty_setcolor(tx);
    drawRect(0, 0, getScreenWidth(), getScreenHeight(), bg);
    setPosX(0);
    setPosY(0);
    tty_changeState(true);
}

/**
 * @brief Смена режима отображения BootScreen
 *
 * @param m - Режим (0 - Обычный | 1 - Лог)
 */
void bootScreenChangeMode(int m){
    mode = m;
}

/**
 * @brief Выводит во время загрузки служебную информацию BootScreen
 */
void bootScreenInfo(){
    setPosX(0);
    setPosY(0);
    if (!lazy){
        tty_printf("SayoriOS v%d.%d.%d\nBuilt: %s\n",
        VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,    // Версия ядра
        __TIMESTAMP__                                   // Время окончания компиляции ядра
        );

        char* about = "(c) SayoriOS Team";
        uint32_t centerAbout = (maxStrLine/2)-(strlen(about)/2);

        tty_setcolor(bootScreenTheme(2));
        tty_set_bgcolor(bootScreenTheme(1));
        setPosX(((1+centerAbout)*8));
        setPosY(getScreenHeight() - 32);
        tty_printf(about);
    }
    setPosX(0);
    setPosY(16*5);


}

/**
 * @brief Рисует прогресс-бар для BootScreen
 */
void bootScreenProcentPaint(){
    curElem++;
    if (curElem >= maxElem){
        curElem = maxElem;
    }
    uint32_t padding_h = maxHeightLine/4;
    uint32_t proc = (curElem*100)/maxElem;
    //qemu_log("[BS] Proc: %d | C: %d | E: %d",proc,curElem,maxElem);
    //setPosX(8*8);

    drawRect(8*8,(16*((maxHeightLine-padding_h+2))),(proc)*7,16,bootScreenTheme(0));
}

/**
 * @brief Обновить информацию для BootScreen
 *
 * @param title - Вывести данное сообщение
 */
void bootScreenPaint(char* title){
    if (bs_logs)
        qemu_log("[BOOT] %s",title);
    
    if (mode == 1){
        tty_changeState(true);
        tty_set_bgcolor(bootScreenTheme(1));
        tty_setcolor(bootScreenTheme(0));
        tty_printf("%s\n",title);
        tty_changeState(false);
        punch();
        return;
    }
    
    maxStrLine = (getScreenWidth() / 8) - 2;
    maxHeightLine = getScreenHeight() / 16;
    
    tty_set_bgcolor(bootScreenTheme(1));
    tty_setcolor(bootScreenTheme(0));
    tty_changeState(true);

    uint32_t centerTitle = (maxStrLine/2) - (mb_strlen(title)/2);
    uint32_t padding_h = maxHeightLine/4;
    
    // punch();
    if (lazy){
        drawRect(0,16*((maxHeightLine-padding_h)), getScreenWidth(), 16, bootScreenTheme(1));
    } else {
        drawRect(0, 0, getScreenWidth(), getScreenHeight(), bootScreenTheme(1));
    }
    // punch();
    setPosX(((1+centerTitle)*8));
    setPosY(16*((maxHeightLine-padding_h)));
    tty_printf(title);
    bootScreenInfo();
    bootScreenProcentPaint();
    tty_changeState(false);
    
    punch();
}

/**
 * @brief Инициализирует BootScreen
 *
 * @param count - Кол-во этапов
 */
void bootScreenInit(uint32_t count){
    // Предварительная настройка BootScreen
    maxElem = count;
    if (bs_logs) qemu_log("Init...");
    tty_changeState(false);  // Disabling print functions
    maxStrLine = (getScreenWidth() / 8) - 2;
    maxHeightLine = getScreenHeight() / 16;
    bootScreenPaint("Загрузка...");

}
