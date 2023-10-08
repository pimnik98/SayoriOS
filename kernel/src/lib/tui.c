/**
 * @file lib/tui.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Драйвер библиотеки TUI (Text User Interface)
 * @version 0.3.3
 * @date 2022-10-20
 * @copyright Copyright SayoriOS Team (c) 2022-2023
 */
#include <kernel.h>
#include <io/ports.h>
#include <io/duke_image.h>
#include <io/status_loggers.h>
#include <lib/stdio.h>
#include <lib/tui.h>

int32_t bgColor = VESA_BLUE;        // Фон на экране
int32_t txColor = VESA_WHITE;       // Основной текст для печати на экране
int32_t TUIMode = TUI_DEFAULT;      // Режим TUI
int32_t typeDisplay = 0;            // Тип дисплея (?)
int32_t lastUpdate = 0;             // Последнее обновление экрана
int32_t w = 0;                      // Длина рабочего места
int32_t h = 0;                      // Длина рабочего места
int32_t ww = 0;                     // Длина рабочего места
int32_t wh = 0;                     // Высота рабочего места
int32_t maxStrLineTUI = 0;             // Максимальное количество символов на линии
int32_t maxHeightLineTUI = 0;          // Максимальное количество линий
char* Display = 0;                      // Название расширения монитора
int32_t currentMenu = 0;            // Текущие меню
int32_t currentList = 0;            // Текущая позиция на экране
char *listMenu[128];                // Сам список меню
int32_t maxListMenu = 0;            // Максимальное количество элементов
int maxItemScreen = 0;         // Максимальное количество объектов на экране
int pageMenuCurrent = 0;       // Текущая страница
int pageMenuMax = 0;           // Максимальная страница
int ItemsMax   = 256;
int Items[256];
/**
 * @brief Получить цвет TUI
 *
 * @params bool bg - Если true, то фон, в противном случае основной цвет
 */
int32_t getColorsTUI(bool bg){
    return (bg?bgColor:txColor);
}

/**
 * @brief Получить текущий режим меню
 *
 * @return int32_t - Выводит ID статуса
 */
int32_t getModeTUI(){
    return TUIMode;
}

/**
 * @brief Получить длину рабочего пространства
 *
 * @return int32_t - Длина пространства
 */
int32_t getWorkSpaceWidthTUI(){
    return ww;
}

/**
 * @brief Получить высоту рабочего пространств
 *
 * @return int32_t - Высота пространства
 */
int32_t getWorkSpaceHeightTUI(){
    return wh;
}

/**
 * @brief Установить режим работы TUI
 *
 * @param int32_t mode - Режим TUI
 */
int32_t setModeTUI(int32_t mode){
    TUIMode = mode;
}

/**
 * @brief Получить последнее время обновления TUI
 *
 * @return int32_t - Выводит последнее время обновления
 */
int32_t getLastUpdateTUI(){
    return lastUpdate;
}

/**
 * @brief Установить время обновления TUI
 *
 * @param int32_t time - Время (в тиках)
 */
int32_t setLastUpdateTUI(int32_t time){
    lastUpdate = time;
}

/**
 * @brief Получить текущию позицию в меню
 *
 * @return int32_t - Выводит позицию в меню
 */
int32_t getCurrentItemTUI(){
    return currentList;
}

/**
 * @brief Установить позицию в меню
 *
 * @param int32_t item - Позиция в меню
 */
int32_t setCurrentItemTUI(int32_t item){
    currentList = item;
}

/**
 * @brief Установить максимальное количество элементов в меню
 *
 * @param int32_t item - Максимальное кол-во элементов
 */
int32_t setMaxItemTUI(int32_t item){
    maxListMenu = item;
}

/**
 * @brief Получить максимальное количество элементов в меню
 *
 * @return int32_t - Выводит кол-во элементов
 */
int32_t getMaxListMenuTUI(){
    return maxListMenu;
}

/**
 * @brief Получить максимальное символов в одной строке
 *
 * @return int32_t - Выводит кол-во символов на одну строку экрана
 */
int32_t getMaxStrLineTUI(){
    return maxStrLineTUI;
}


/**
 * @brief Отчищает список элементов
 */
void cleanItems(){
	memset(listMenu, 0, 128);
    maxListMenu = 0;
}

int getMaxStrLineBoxTUI(){
    return (((ww-(((maxStrLineTUI/4)*8)*2))/8)-4);
}

/**
 * @brief Добавляет позицию в список элементов
 *
 * @param char* name - Название позиции
 */
/*
ItemTUI* addItem(char* name,bool disabled, char* key, char* value){
    if (maxListMenu > ItemsMax){
        //return false;
    }
    int i = maxListMenu;

    //Items[i]->id = i;
    //Items[i]->name = (char*) malloc(sizeof(char)*strlen(name));
    //Items[i]->disabled = disabled;
    //Items[i]->key = (char*) malloc(strlen(key));
    //Items[i]->value = (char*) malloc(strlen(value));

    if (strlen(name) > getMaxStrLineBoxTUI()){
        qemu_printf("[Name: %s | size: %d | msl: %d]",name,strlen(name),getMaxStrLineBoxTUI());
        substr(name, name, 0, getMaxStrLineBoxTUI());
    }
    listMenu[i] = name;
    maxListMenu++;
    return true;
}
*/

/**
 * @brief Отчистить пользовательское пространство
 *
 * @param int - Цвет для фона
 */
void cleanWorkSpace(int color){
    drawRect(8,16*(3),ww,wh,color);
}
/**
 * @brief Выводит список меню
 *
 * @param title - Заголовок
 */
void createMenuBox(char* title){
    TUIMode = TUI_MENU_BOX;
    // Установка отступа экрана в длину (left/right)
    int padding_w = maxStrLineTUI/4; // 320 - 10 символа; 1024 - 32 символа
    // Установка отступа экрана в высоту (up)
    int padding_h = maxHeightLineTUI/4;
    // Получаем размеры коробки
    int boxWidth = ww-((padding_w*8)*2);
    int boxHeight = wh-((padding_h*16));
    int maxListBox = boxHeight;
    // Получаем максимальное количество символов на строку в коробке
    int maxStrLineBox = (boxWidth/8)-4; // 60 - символов при 1024

    // Высота бокса для меню
    maxItemScreen = (maxListMenu >10?10:maxListMenu);

    // Установка максимальных страниц навигаций
    if (maxListMenu > 0){
        pageMenuMax = (maxListMenu/maxItemScreen)+1;
    } else {
        pageMenuMax = 1;
    }

    // Обрезаем заголовок
    if (strlen(title) > maxStrLineBox){
        substr(title, title, 0, maxStrLineBox);
    }
    // Рисуем бокс и узорчики
    drawRect(8+(padding_w*8),16*(8+padding_h),boxWidth,16*(6+maxItemScreen),TUI_BASE_COLOR_MAIN);
    drawRectLine(8+(padding_w*8),16*(8+padding_h),boxWidth,16*(6+maxItemScreen),TUI_TEXT_COLOR_BODY,TUI_BASE_COLOR_MAIN,15);

    // Отображаем титл
    setPosX(((3+padding_w)*8));
    setPosY(16*((10)+padding_h));
    tty_puts_color(title,TUI_TEXT_COLOR_BODY,TUI_BASE_COLOR_MAIN);

    // Рисуем пункты меню
    for (int x = pageMenuCurrent*maxItemScreen; x < maxItemScreen; x++){
        setPosX(((3+padding_w)*8));
        setPosY(16*((12+x)+padding_h));
        if (currentList == x){
            drawRect(((3+padding_w)*8),16*((12+x)+padding_h),maxStrLineBox*8, 16,TUI_BASE_COLOR_ITEM);
            tty_puts_color(listMenu[x],TUI_TEXT_COLOR_ITEM,TUI_BASE_COLOR_ITEM);
            // Помечаем что, он является текущим
        } else {
            tty_puts_color(listMenu[x],TUI_TEXT_COLOR_BODY,TUI_BASE_COLOR_MAIN);
        }
    }
}

/**
 * @brief Выводит фатальный красный блок
 *
 * @param title - Заголовок
 * @param text  - Текст ошибки.
 */
void createErrorBox(char* title,char* text){
    // Переводим TUI в режим ERRORBOX
    //TUIMode = TUI_ERROR_BOX;
    // Установка отступа экрана в длину (left/right)
    int padding_w = maxStrLineTUI/4; // 320 - 10 символа; 1024 - 32 символа
    // Установка отступа экрана в высоту (up)
    int padding_h = maxHeightLineTUI/4;
    // Получаем размеры коробки
    int boxWidth = ww-((padding_w*8)*2);
    int boxHeight = wh-((padding_h));      // ? Реализовал а зачем, забыл :)
    // Получаем максимальное количество символов на строку в коробке
    int maxStrLineBox = (boxWidth/8)-4; // 60 - символов при 1024
    // maxHeightLine
    // Высота бокса для текса
    int lineHeight = 1;
    // Обрезаем строку до максимального лимита на строку
    substr(title, title, 0, maxStrLineBox);
    // Если текст длинее допустимой строки
    if (strlen(text) > maxStrLineBox){
        lineHeight = maxStrLineBox/strlen(text);
    }
    // Если на экран все не помещается
    if (maxHeightLineTUI < lineHeight){
        lineHeight = maxHeightLineTUI;
    }
    if (strlen(text) > maxStrLineBox){
        // Объявим переменную для деления текста
        char strings[lineHeight][maxStrLineBox+1];
        for (int i = 0;i < lineHeight;i++){
            substr(title, title, 0+(i*maxStrLineBox), ((i+1)*maxStrLineBox));
        }
    }
    // Рисуем коробку
    drawRect(8+(padding_w*8),16*(8+padding_h),boxWidth,16*(6+lineHeight),TUI_BASE_COLOR_ERROR);
    drawRectLine(8+(padding_w*8),16*(8+padding_h),boxWidth,16*(6+lineHeight),TUI_TEXT_COLOR_ERROR,TUI_BASE_COLOR_ERROR,19);
    // Показываем титл и считаем равнение по центру
    int centerTitle = (maxStrLineBox/2)-(strlen(title)/2);

    setPosX(((3+centerTitle+padding_w)*8));
    setPosY(16*((10)+padding_h));
    tty_puts_color(title,TUI_TEXT_COLOR_ERROR,TUI_BASE_COLOR_ERROR);

    // Рисуем буковки
    if (strlen(text) > maxStrLineBox){
        for (int i = 0;i < lineHeight;i++){
            //substr(title, title, 0+(i*maxStrLineBox), ((i+1)*maxStrLineBox));

        }
    } else {
        setPosX(((3+padding_w)*8));
        setPosY(16*((12)+padding_h));
        tty_puts_color(text,TUI_TEXT_COLOR_ERROR,TUI_BASE_COLOR_ERROR);
    }
}



/**
 * @brief Инициализация и сброс TUI на стандартное значение
 */
void tui_configurate(){
    qemu_printf("%s","TUI Configure");
    w = getScreenWidth();
    h = getScreenHeight();
    ww = w-16;
    wh = h-(80);
    maxHeightLineTUI = wh/16;
    maxStrLineTUI = (w/8)-2;
    //changeStageKeyboard(0); // Блокируем нажатие и отображение кнопок
    bgColor = TUI_BASE_COLOR_BODY;
    txColor = TUI_TEXT_COLOR_BODY;
    TUIMode = TUI_DEFAULT;
    lastUpdate = getTicks()+3;

    /*
    th.BASE_COLOR_HEAD  = TUI_BASE_COLOR_HEAD;
    th.BASE_COLOR_BODY  = TUI_BASE_COLOR_BODY;
    th.BASE_COLOR_MAIN  = TUI_BASE_COLOR_MAIN;
    th.BASE_COLOR_ITEM  = TUI_BASE_COLOR_ITEM;
    th.BASE_COLOR_FOOT  = TUI_BASE_COLOR_FOOT;
    th.BASE_COLOR_ERROR = TUI_BASE_COLOR_ERROR;
    th.TEXT_COLOR_HEAD  = TUI_TEXT_COLOR_HEAD;
    th.TEXT_COLOR_BODY  = TUI_TEXT_COLOR_BODY;
    th.TEXT_COLOR_ITEM  = TUI_TEXT_COLOR_ITEM;
    th.TEXT_COLOR_FOOT  = TUI_TEXT_COLOR_FOOT;
    th.TEXT_COLOR_ERROR = TUI_BASE_COLOR_ERROR;
    */
}

void tui_destroy(){
    changeStageKeyboard(1);
    setPosX(0);
    setPosY(0);
}
