/**
 * @file extra/texplorer.c
 * @authors Пиминов Никита (github.com/pimnik98 | VK: @piminov_remont)
 * @brief TShell
 * @version 0.0.3
 * @date 2023-06-02
 *
 * @copyright Copyright Пиминов Никита (с) 2023
 *
 */
#include <kernel.h>
#include <io/ports.h>
#include <io/duke_image.h>
#include <io/status_loggers.h>
#include <lib/stdio.h>
#include <lib/tui.h>
#include <drv/input/keymap.h>

#define TE_COLOR_BODY 			0x00
#define TE_COLOR_TEXT 			0x01
#define TE_COLOR_START_BODY 	0x02
#define TE_COLOR_START_TEXT 	0x03
#define TE_COLOR_BORDER 		0x04
#define TE_COLOR_BACKGROUND 	0x05
#define TE_COLOR_TITLE_BODY		0x06
#define TE_COLOR_TITLE_TEXT		0x07
#define TE_COLOR_BTN_BODY		0x08
#define TE_COLOR_BTN_TEXT		0x09
#define TE_COLOR_BTN_BORD		0x10
#define TE_COLOR_PGB_BORD		0x11
#define TE_COLOR_PGB_BODY		0x12
#define TE_COLOR_PGB_TEXT		0x13


DukeHeader_t* TE_Icons[32];

int TE_getColor(int item){
	if (item == TE_COLOR_BODY) 			return 0xFFFFFF;
	if (item == TE_COLOR_TEXT) 			return 0x000000;
	if (item == TE_COLOR_START_BODY) 	return 0xFFFFFF;
	if (item == TE_COLOR_START_TEXT) 	return 0x000000;
	if (item == TE_COLOR_BORDER) 		return 0x000000;
	if (item == TE_COLOR_BACKGROUND) 	return 0x000000;
	if (item == TE_COLOR_TITLE_BODY) 	return 0xFFFFFF;
	if (item == TE_COLOR_TITLE_TEXT) 	return 0x000000;
	if (item == TE_COLOR_BTN_BODY) 		return 0xFFFFFF;
	if (item == TE_COLOR_BTN_TEXT) 		return 0x000000;
	if (item == TE_COLOR_BTN_BORD) 		return 0x000000;
	if (item == TE_COLOR_PGB_BORD) 		return 0x000000;
	if (item == TE_COLOR_PGB_BODY) 		return 0xFFFFFF;
	if (item == TE_COLOR_PGB_TEXT) 		return 0x43ACE8;
	return 0x000000;
}

void TE_DrawTime(){
    setPosY(getScreenHeight() - 20);
	
	sayori_time_t time = get_time();
	setPosX(getScreenWidth() - 68);

	tty_puts_color("22:22:22",TE_getColor(TE_COLOR_START_TEXT),TE_getColor(TE_COLOR_START_BODY));

	setPosX(getScreenWidth() - 88);

	tty_puts_color("EN",TE_getColor(TE_COLOR_START_TEXT),TE_getColor(TE_COLOR_START_BODY));

    //tty_printf("%s%d:%s%d:%s%d", (time.hours>10?"":"0"), time.hours, (time.minutes>10?"":"0"),time.minutes, (time.seconds>10?"":"0"),time.seconds);
}

void TE_DrawMessageBox(char* title, char* msg, int mode, int x, int y, int close, int data, int data2){
	//qemu_log("[TE] Title: %s\n Message: %s\n Mode:%x\n Close:%x\n Data1:%d\n Data2:%d",title,msg,mode,close,(int) data,(int) data2);

	int w = 320;
	int h = 64;

	///< Рисуем основу
	drawRect(x,y,w,h,TE_getColor(TE_COLOR_BODY));
	///< Рисуем заголовок
	drawRect(x+1,y+1,w-3,h-3,TE_getColor(TE_COLOR_TITLE_BODY));
	drawRectBorder(x+1,y+1,w-3,h-3,TE_getColor(TE_COLOR_BORDER));
	setPosX(x+6);
	setPosY(y+4);
	_tty_puts_color(title,TE_getColor(TE_COLOR_TITLE_TEXT),TE_getColor(TE_COLOR_TITLE_BODY));
	drawRectBorder(x+1,y+1,w-3,18,TE_getColor(TE_COLOR_BORDER));
	///< Рисуем текст
	setPosX(x+6);
	setPosY(y+22);
	_tty_puts_color(msg,TE_getColor(TE_COLOR_TEXT),TE_getColor(TE_COLOR_BODY));
	///< Рисуем кнопку закрытия
	if (close){
		setPosX(x+w-12);
		setPosY(y+4);
		_tty_puts_color("X",TE_getColor(TE_COLOR_TITLE_TEXT),TE_getColor(TE_COLOR_TITLE_BODY));
		drawRectBorder(x+w-16,y+1,14,18,TE_getColor(TE_COLOR_BORDER));
	}
	if (mode == 0) return;
	if (mode == 1){
		drawRect(x+16,y+h-22,w-32,16,TE_getColor(TE_COLOR_BTN_BODY));
		drawRectBorder(x+17,y+h-21,w-35,15,TE_getColor(TE_COLOR_BTN_BORD));
		setPosX(x+((w-32)/2));
		setPosY(y+h-20);
		_tty_puts_color("OK",TE_getColor(TE_COLOR_BTN_TEXT),TE_getColor(TE_COLOR_BTN_BODY));
	}
	if (mode == 2){
		// ProgreesBar
		
		drawRect(x+16,y+h-22,w-32,16,TE_getColor(TE_COLOR_PGB_BODY));
		drawRectBorder(x+17,y+h-21,w-35,15,TE_getColor(TE_COLOR_PGB_BORD));
		int wb = w-32-8;
		int hb = 10;
		int pc = (((int) data * wb)/(int) data2);
		drawRect(x+20,y+h-18,(pc == 0?((int) data) + (wb) :pc),hb,TE_getColor(TE_COLOR_PGB_TEXT));
		//tty_puts_color("OK",TE_getColor(TE_COLOR_BTN_TEXT),TE_getColor(TE_COLOR_BTN_BODY));
	}
	
	setPosX(0);
	setPosY(0);
}

void TE_IconsLoader(){
	char* app		= "/Sayori/Icons/app.duke";
	char* config	= "/Sayori/Icons/config.duke";
	char* file		= "/Sayori/Icons/file.duke";
	char* folder	= "/Sayori/Icons/folder.duke";
	char* hdd		= "/Sayori/Icons/hdd.duke";
	char* img		= "/Sayori/Icons/img.duke";
	char* off		= "/Sayori/Icons/off.duke";
	char* start		= "/Sayori/Icons/start.duke";
	char* videocard	= "/Sayori/Icons/videocard.duke";

	FILE* fp_app = fopen(app, "r");

	if(!fp_app) {
        qemu_log("[TE] [ICONS] FATAL ERROR LOADER!!! File: %s | Code: %x\n", app,0);
		kfree(TE_Icons[0]);
		return;
	}

	fread(fp_app, 1, sizeof(DukeHeader_t), TE_Icons[0]);
    fclose(fp_app);

	bool error = duke_draw_from_file(app, 0, 0);

    if(error) {
        qemu_log("[TE] [DBG] Во время рендера картинки произошла ошибка.\n");
    }

	
	
	//TE_Icons[1];
}

void TE_DesktopBG(){
	DukeHeader_t* imdata = kmalloc(sizeof(DukeHeader_t));
    char* rpath = "/Sayori/Wallpaper/wallpaper.png.duke";

	///< Вот СХУЯЛИ баня загорелась? ПРИЧЕМ ТУТ ЖД, если считывание с вирт диска???????????
	// Да всё Никита, успокойся, я пофиксил.
	///< Спасибо
	FILE* fp = fopen(rpath, "r");

	if(!fp) {
        qemu_log("[TE] [DBG] Произошла ошибка при открытии файла %s\n", rpath);
		kfree(imdata);
		return;
	}

	fread(fp, 1, sizeof(DukeHeader_t), imdata);
    fclose(fp);

	uint32_t w = getScreenWidth() - imdata->width;
	uint32_t h = getScreenHeight() - imdata->height;

	bool error = duke_draw_from_file(rpath, w / 2, h / 2);

    if(error) {
        qemu_log("[TE] [DBG] Во время рендера картинки произошла ошибка.\n");
    }

    kfree(rpath);
    kfree(imdata);
	
	TE_IconsLoader();
	punch();
}

void TE_Desktop(){
	qemu_log("[TE] Desktop Draw..");

	///< Рисуем квадрат малевича
	drawRect(0, 0, getScreenWidth(), getScreenHeight(), TE_getColor(TE_COLOR_BACKGROUND));

	TE_DesktopBG();

	///< Оформляем меню пуск
	drawRect(0, getScreenHeight() - 24, getScreenWidth(), 24, TE_getColor(TE_COLOR_START_BODY));
	drawRectBorder(2, getScreenHeight() - 21, 38, 18, TE_getColor(TE_COLOR_BORDER));

	setPosX(6);
    setPosY(getScreenHeight() - 20);
    tty_puts_color("Пуск",TE_getColor(TE_COLOR_START_TEXT),TE_getColor(TE_COLOR_START_BODY));

	TE_DrawTime();

	
	setPosX(0);
	setPosY(0);
}

void TExplorer(){
	qemu_log("[TE] Loader..");

	TE_Desktop();

	TE_DrawMessageBox("Заголовок", "А я точно попаду в MessageBox?", 1, getScreenWidth() / 4, getScreenHeight() / 3, 1, 0, 0);
	TE_DrawMessageBox("Инициализация...", "Ожидание ответа NatSuki...", 2, getScreenWidth() / 4, 100, 0, 3, 10);

	setPosX(0);
	setPosY(0);

	int zx = 0;
	while(1){
		if (zx == 0) TE_DrawMessageBox("[1/10] Инициализация...", "Проверка...", 2, getScreenWidth() / 4, 100, 0, 1, 10);
		if (zx == 2) TE_DrawMessageBox("[2/10] Инициализация...", "Инициализация системы...", 2, getScreenWidth() / 4, 100, 0, 2, 10);
		if (zx == 3) TE_DrawMessageBox("[3/10] Инициализация...", "Сбор информации...", 2, getScreenWidth() / 4, 100, 0, 3, 10);
		if (zx == 4) TE_DrawMessageBox("[4/10] Инициализация...", "Подключение к NatSuki...", 2, getScreenWidth() / 4, 100, 0, 4, 10);
		if (zx == 5) TE_DrawMessageBox("[5/10] Инициализация...", "Синхронизация...", 2, getScreenWidth() / 4, 100, 0, 5, 10);
		if (zx == 6) TE_DrawMessageBox("[6/10] Инициализация...", "Загрузка темы оформления...", 2, getScreenWidth() / 4, 100, 0, 6, 10);
		if (zx == 7) TE_DrawMessageBox("[7/10] Инициализация...", "Загрузка меню пуск...", 2, getScreenWidth() / 4, 100, 0, 7, 10);
		if (zx == 8) TE_DrawMessageBox("[8/10] Инициализация...", "Поиск доступных программ...", 2, getScreenWidth() / 4, 100, 0, 8, 10);
		if (zx == 9) TE_DrawMessageBox("[9/10] Инициализация...", "Активация курсора...", 2, getScreenWidth() / 4, 100, 0, 9, 10);
		if (zx == 9) mouse_set_state(CURSOR_NORMAL);
		if (zx == 10)TE_DrawMessageBox("[10/10] Инициализация...", "С возращением, oem!", 2, getScreenWidth() / 4, 100, 0, 10, 10);
		if (zx == 11) break;
		zx++;
		punch();
		sleep_ms(1000);
	}
	shellForceClose();
}
