/**
 * @file lib/fonts.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Библиотека для работы обработки шрифтов
 * @version 0.3.0
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include <kernel.h>
#include <io/imaging.h>
#include <lib/stdio.h>
#include <io/ports.h>
char* fileFont = "/initrd/fonts.duke";			///< Файл шрифта по умолчанию
char* datFont = "/initrd/fonts.fdat";			///< Файл с параметрами шрифта по умолчанию
struct DukeImageMeta* fontMeta;					///< Параметры meta Duke
uint32_t 	err = -1, 							///< Код ошибки
			xF = 0, 							///< Конфигуратор 1
			pF = 0, 							///< Конфигуратор 2
			hF = 0, 							///< Конфигуратор 3
			colorFont = 0xFFFFFF, 				///< Текущий цвет печати
			colorBg = 0x000000, 				///< Текущий цвет заднего фона
			mW, 								///< Длина шрифта
			mH, 								///< Высота шрифта
			mA;									///< Прозрачность
char* imageFont;								///< Буфер файла
char alphaFont;									///< Статус альфа-канала
char* configFont;								///< Буфер файла с настройками
char*** array;									///< Массив с настройками
char* Alphabet = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯйцукенгшщзхъфывапролджэячсмитьбюё!«№;%:?*()_+-=@#$^&[]{}|\\/QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890.,";

uint32_t Map[250] = {160,159,158,157,156,155,175,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,119,169,172,118,123,115,125,167,166,121,170,165,171,164,126,128,113,175,114,117,124,122,162,160,168,174,116,120,173,163,127,161,158,33,1147,3278,1988,37,58,63,42,40,41,95,43,45,61,64,35,36,94,38,91,93,123,125,124,92,47,81,87,69,82,84,89,85,73,79,80,65,83,68,70,71,72,74,75,76,90,88,67,86,66,78,77,113,119,101,114,116,121,117,105,111,112,97,115,100,102,103,104,106,107,108,122,120,99,118,98,110,109,49,50,51,52,53,54,55,56,57,48,46,44,1144,1151,1149,1142,3238,-68,3236,-81,3242,-96,3242,-95,3242,-86,3242,-85,3242,-84,3242,-78,3242,-96,3242,-95,3242,-78,3242,-70,3242,-68,3240,-124,3240,-118,3240,-117,3240,-116,3240,-113,3240,-90,3238,-70,3238,-69,3270,-98,1149,-61,-105,3278,-123,3278,-109,3278,2166,-124,-94,-50,-87,75,-61,-123,3278,-82,3276,-109,3276,-108,3276,-101,3276,-100,3276,-99,3276,-98,3276,-115,3276,-114,3270,-126,3270,-122,3270,-113,3270,-111,3270,-110,3270,-107,3270,-103,3270,-102,3270,-98,3270,-97,3270,-87,3270,-85,3268,-120,3268,-96,3268,-95,3268,-92,3268,-91,1127,1157,1147,1131,3286,-71,3286,-70,34,3286,-104,3286,-103,3286,-100,3286,-99,3286,-102,3286,-98,1151,1136,3286,-96,3286,-95,3286,-90,3278,-94,1149,1144,3282,-84,3282,-93,1153,1155,3282,-67,1156,1154,3242,-124,3242,-128};

/**
 * @brief Установка цвета
 *
 * @param int color - Цвет
 */
void setColorFont(int color){
    colorFont = color;
}

/**
 * @brief Установка цвета заднего фона
 *
 * @param int color - Цвет
 */
void setColorFontBg(int color){
    colorBg = color;
}

/**
 * @brief Получение текущего цвета шрифта
 */
int getColorFont(){
    return colorFont;
}

/**
 * @brief Получение параметра настроек шрифта
 *
 * @param int k - Параметр
 *
 * @return int - Значение параметра
 */
int getConfigFonts(int k){
    return (k==3?err:(k==2?hF:(k==1?pF:xF)));
}

/**
 * @brief Настройка шрифта
 *
 * @param int x - X
 * @param int p - P
 * @param int h - H
 */
void setConfigurationFont(int x,int p,int h){
    xF = x;
    pF = p;
    hF = h;
}
/**
 * @brief Установка пути к файлам
 *
 * @param char* path - Путь к файлу с шрифтов (duke)
 * @param char* dat - Путь к файлу с информацией (fdat)
 */
void setFontPath(char* path,char* dat){
    fileFont = path;
    datFont = dat;
    qemu_log("[FONTS] Set configurate:\n\tFile: %s\n\tData:%s",fileFont,datFont);
}

/**
 * @brief Загрузка информации о шрифте
 */
void loadFontData(){
    qemu_log("[FONTS] Loading information fonts...");
    FILE* fdata = fopen(datFont,"r");
    if (ferror(fdata) != 0){
        err = 2;
        qemu_log("[FONTS] [ERROR] File `%s` no found!",datFont);
        return;
    }
    char * configFont = fread(fdata);
    fclose(fdata);
    uint32_t cS1 = str_cdsp(configFont,";");
    qemu_log("[A] %s / %d",configFont,cS1);
    err = 0;
}

/**
 * @brief Инициализация системы шрифтов
 */
void fontInit(){
    char meta[sizeof(struct DukeImageMeta)];
    if (!vfs_exists(fileFont)){
        err = 1;
        qemu_log("[FONTS] Sorry, file `%s` no found!",fileFont);
        return;
    }
    uint32_t node = vfs_foundMount(fileFont);
    int elem = vfs_findFile(fileFont);
    vfs_read(node,elem,0,sizeof(struct DukeImageMeta),meta);
    uint32_t fds = (vfs_getLengthFilePath(fileFont));
    qemu_log("[Fonts] Size duke: %d",fds);
    struct DukeImageMeta* fontMeta = (struct DukeImageMeta*)meta;
    imageFont = kmalloc(fds);
    vfs_read(node,elem, sizeof(struct DukeImageMeta), fds, imageFont);
    mW = fontMeta->width;
    mH = fontMeta->height;
    mA = fontMeta->alpha;

    alphaFont = fontMeta->alpha?4:3;
    qemu_log("[FONTS] Configurate:\n\tFile:%s\n\tSize:%d\n\tAlpha:%d",fileFont,fds,alphaFont);
    loadFontData();
    qemu_log("[FONTS] Complete");
}

/**
 * @brief Освобождение места и прекращение работы с шрифтами
 */
void destroyFont(){
    kfree(imageFont);
}

/**
 * @brief Получение позиции по букве
 *
 * @param uint32_t c - Символ
 * @param uint32_t offset - Отступ
 *
 * @return Возращение позиции буквы
 */
uint32_t getPositionChar(uint32_t c,uint32_t offset){
    uint32_t a = 0;
    for(uint32_t z=0;z < 250;z++){
        if (offset == -47 && c == 175){
            return 50;  // р (русская)
        } else if (offset == -47 && c == 158){
            return 65;  // ё
        } else if (offset == -48 && c == 175){
            return 6;   // Ё
        } else if (offset == -47 && c == 160){
            return 56;   // я
        } else if (c == 123 && offset == 0){
            return 87;  // {
        } else if (c == 124 && offset == 0){
            return 89;  // }
        } else if (c == 125 && offset == 0){
            return 88;  // |
        } else if (c == 113 && offset == 0){
            return 118;   // q
        } else if (c == 114 && offset == 0){
            return 121;   // r
        } else if (c == 115 && offset == 0){
            return 129;   // s
        } else if (c == 116 && offset == 0){
            return 122;   // t
        } else if (c == 117 && offset == 0){
            return 124;   // u
        } else if (c == 118 && offset == 0){
            return 140;   // v
        } else if (c == 119 && offset == 0){
            return 119;   // w
        } else if (c == 120 && offset == 0){
            return 138;   // x
        } else if (c == 121 && offset == 0){
            return 123;   // y
        } else if (c == 122 && offset == 0){
            return 137;   // z
        } else if (c == 33 && offset == 0){
            return 66;   // !
        } else if (c == 34 && offset == 0){
            return 67;   // "
        } else if (c == 46 && offset == 0){
            return 154;   // .
        } else if (c == 47 && offset == 0){
            return 91;   // /
        } else if (c == 92 && offset == 0){
            return 90;   // обратный шлеф
        } else if (c == 91 && offset == 0){
            return 85;   // [
        } else if (c == 93 && offset == 0){
            return 86;   // ]
        } else if (c == 37 && offset == 0){
            return 70;   // %
        } else if (c == 58 && offset == 0){
            return 71;   // :
        } else if (c == 63 && offset == 0){
            return 72;   // ?
        } else if (c == 44 && offset == 0){
            return 155;   // ,
        } else if (Map[z] == c){
            //qemu_log("[gPC] %d == %d = POS: %d",a,c,z);
            return z;
        }
        //qemu_log("\t[gPC] %d != %d",a,c);
    }
    return -1;
}
/**
 * @brief - Рисуем букву по параметрам
 *
 * @param int x - Координаты X на экране
 * @param int y - Координаты Y на экране
 * @param int sx - Отступ на картинке по X
 * @param int sy - Отступ на картинке по Y
 * @param int width - Длина отрисовки
 * @param int height - Высота отрисовки
 *
 * @return 0 - если все в порядке
 */
int drawFont(int x, int y, int sx, int sy, int width, int height){
    if (err != 0){
        return 1;
    }
    if(width>mW) { width = mW; }
    if(height>mH) { height = mH; }
    int wx = sx, wy = sy;
    char mod = mA?4:3;
    while(wy<(height-sy)) {
        wx = sx;
        while((wx-sx)<(width)) {
            int px = pixidx(mW*mod, wx*mod, wy);
            int r = imageFont[px];
            int g = imageFont[px+1];
            int b = imageFont[px+2];
            int a = imageFont[px+3];
            int color = ((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff);
            if(mod && color != 0xFFFFFF) {
                set_pixel(x+(wx-sx), y+(wy-sy), colorFont);
            }
            wx++;
        }
        wy++;
    }
    return 0;
}
/**
 * @brief Отрисовка буквы по позициям
 *
 * @param char c - Символ 1
 * @param char c1 - Символ 2
 * @param int x - Позиция по X
 * @param int y - Позиция по Y
 */
void drawCharFont(char c,char c1,int x,int y){
    if (err != 0){
        return;
    }
    int idx,X;
    if (isUTF(c)){
        idx = getPositionChar(UTFConvert(c,c1),c);
    } else if (isSymbol(c)){
        idx = getPositionChar(SymConvert(c,c1,0),(uint32_t) c);
    } else {
        idx = getPositionChar((uint32_t) c,0);
    }
    if (idx != -1){
        X = (int) ((idx)*pF-1);
        //qemu_log("[dF] X:%d PX:%d",X,px);
        drawRect(x,y,xF,hF,colorBg);
        drawFont(x,y,X,0,xF,128);
    }
}

/**
 * @brief - Рисуем строку при помощи шрифта
 *
 * @param char* str - Строка
 * @param int kx - Координаты X на экране
 * @param int ky - Координаты Y на экране
 * @param int py - Разница в уровне с предыдущей буквы по оси Y
 */
void drawStringFont(char str[],int kx,int ky, int py){
    if (err != 0){
        return;
    }
    int x=kx,y=ky,idx,X,P=pF,sd=0,px=xF;
    qemu_log("[df] String: %s",str);
    for(int i=0;i < strlen(str);i++){
        if (isUTF(str[i])){
            //qemu_log("[UTF] %d",(uint32_t) str[i]);
            idx = getPositionChar(UTFConvert(str[i],str[i+1]),str[i]);
            //qemu_log("\tidx %d",UTFConvert(str[i],str[i+1]));
            i++;
        } else if (isSymbol(str[i])){
            //qemu_log("[SYM] %d",(uint32_t) str[i]);
            idx = getPositionChar(SymConvert(str[i],str[i+1],str[i+2]),(uint32_t) str[i]);
            //qemu_log("\tidx %d|%d",SymConvert(str[i],str[i+1],str[i+2]),(uint32_t) str[i]);
            i++;
        } else {
            idx = getPositionChar((uint32_t) str[i],0);
        }
        //qemu_log("[dF] `%s` -> IDX: %d | I:%d | SD:%d",str[i],idx,(uint32_t) str[i],sd);
        x = x+px;
        y = y+py;
        if (idx != -1){
            X = (int) ((idx)*P-1);
            //qemu_log("[dF] X:%d PX:%d",X,px);
            drawFont(x,y,X,0,px,128);
        }
    }
}
