#include <kernel.h>

char* fileFont = "/initrd/var/fonts/MicrosoftLuciaConsole18.duke";
char* datFont = "/initrd/var/fonts/MicrosoftLuciaConsole.fdat";
struct DukeImageMeta* fontMeta;
uint32_t err = -1, xF = 0, pF = 0, hF = 0, colorFont = 0xFFFFFF, mW, mH, mA;
char* imageFont;
char alphaFont;
char* configFont;
char*** array;
char* Alphabet = "АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯйцукенгшщзхъфывапролджэячсмитьбюё!«№;%:?*()_+-=@#$^&[]{}|\\/QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890.,";

uint32_t Map[250] = {160,159,158,157,156,155,175,154,153,152,151,150,149,148,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,119,169,172,118,123,115,125,167,166,121,170,165,171,164,126,128,113,175,114,117,124,122,162,160,168,174,116,120,173,163,127,161,158,33,1147,3278,1988,37,58,63,42,40,41,95,43,45,61,64,35,36,94,38,91,93,123,125,124,92,47,81,87,69,82,84,89,85,73,79,80,65,83,68,70,71,72,74,75,76,90,88,67,86,66,78,77,113,119,101,114,116,121,117,105,111,112,97,115,100,102,103,104,106,107,108,122,120,99,118,98,110,109,49,50,51,52,53,54,55,56,57,48,46,44,1144,1151,1149,1142,3238,-68,3236,-81,3242,-96,3242,-95,3242,-86,3242,-85,3242,-84,3242,-78,3242,-96,3242,-95,3242,-78,3242,-70,3242,-68,3240,-124,3240,-118,3240,-117,3240,-116,3240,-113,3240,-90,3238,-70,3238,-69,3270,-98,1149,-61,-105,3278,-123,3278,-109,3278,2166,-124,-94,-50,-87,75,-61,-123,3278,-82,3276,-109,3276,-108,3276,-101,3276,-100,3276,-99,3276,-98,3276,-115,3276,-114,3270,-126,3270,-122,3270,-113,3270,-111,3270,-110,3270,-107,3270,-103,3270,-102,3270,-98,3270,-97,3270,-87,3270,-85,3268,-120,3268,-96,3268,-95,3268,-92,3268,-91,1127,1157,1147,1131,3286,-71,3286,-70,34,3286,-104,3286,-103,3286,-100,3286,-99,3286,-102,3286,-98,1151,1136,3286,-96,3286,-95,3286,-90,3278,-94,1149,1144,3282,-84,3282,-93,1153,1155,3282,-67,1156,1154,3242,-124,3242,-128};

bool isUTF(char c){
    if (c == -47 || c == -48){
        return true;
    }
    return false;
}

bool isSymbol(char c){
    if (c == -62 || c == -106 || c == -30){
        return true;
    }
    return false;
}

void setColorFont(uint32_t color){
    colorFont = color;
}

uint32_t getColorFont(){
    return colorFont;
}

uint32_t getConfigFonts(int k){
    return (k==3?err:(k==2?hF:(k==1?pF:xF)));
}

void setConfigurationFont(uint32_t x,uint32_t p,uint32_t h){
    xF = x;
    pF = p;
    hF = h;
}

uint32_t SymConvert(char c,char c1,char c2){
    uint32_t s = 0;
    if (c == -62){
        s = 1000;
    } else if (c == -106){
        s = 2000+(((int) c1)* -1);
    } else if (c == -30){
        s = 3000+(((int) c1)* -1);
    }
    return (isSymbol(c)?((((int) c)* -1)+(((int) c1)* -1)+s):((int) c));
}

uint32_t UTFConvert(char c,char c1){
    return (isUTF(c)?((((int) c)* -1)+(((int) c1)* -1)):((int) c));
}

void setFontPath(char* path,char* dat){
    fileFont = path;
    datFont = dat;
    qemu_log("[FONTS] Set configurate:\n\tFile: %s\n\tData:%s",fileFont,datFont);
}


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
    if (cS1 < 3){
        err = 3;
        qemu_log("[FONTS] [ERROR] File `%s` is corrupted or has the wrong format.");
        return;
    }

    char* out[1024] = {0};
    char* par[128] = {0};
    str_split(configFont,out,";");
    for(uint32_t i;i < cS1;i++){
        if (i < 4){
            qemu_log("[A] %d - %d",i,atoi(out[i]));
            array[-1][i] = out[i];
            continue;
        }
        char* par[32] = {0};
        uint32_t cS2 = str_cdsp(out[i],",");
        if (cS2 != 4){
            qemu_log("[FONTS] [%d] Invalid params `%s`",cS2,out[i]);
            continue;
        }
        str_split(out[i],par,",");
        array[(i-3)][0] = atoi(par[0]);   // C
        array[(i-3)][1] = atoi(par[1]);   // X
        array[(i-3)][2] = atoi(par[2]);   // Y
        array[(i-3)][3] = atoi(par[3]);   // W
        array[(i-3)][4] = atoi(par[4]);   // H
        qemu_log("[FONTS] Patch fonts:\n\tCode:%d\n\tX:%d\n\tY:%d\n\tW:%d\n\tH:%d\n\t",array[(i-3)][0],array[(i-3)][1],array[(i-3)][2],array[(i-3)][3],array[(i-3)][4]);
    }
    tty_printf("[FONTS] Detected:\n\tName:%s\n\tVersion:%s\n\tSize:%d\n\tOffset:%d\n",array[-1][0],array[-1][1],atoi(array[-1][2]),atoi(array[-1][3]));
    err = 0;
}


void fontInit(){
    char meta[9];
    if (!vfs_exists(fileFont)){
        err = 1;
        qemu_log("[FONTS] Sorry, file `%s` no found!",fileFont);
        return;
    }
    vfs_read(fileFont,0,9,meta);
    struct DukeImageMeta* fontMeta = (struct DukeImageMeta*)meta;
    imageFont = kheap_malloc(fontMeta->data_length);
    vfs_read(fileFont, 9, fontMeta->data_length, imageFont);
    mW = fontMeta->width;
    mH = fontMeta->height;
    mA = fontMeta->alpha;

    alphaFont = fontMeta->alpha?4:3;
    qemu_log("[FONTS] Configurate:\n\tFile:%s\n\tSize:%d\n\tAlpha:%d",fileFont,fontMeta->data_length,alphaFont);
    loadFontData();
}

void destroyFont(){
    kheap_free(imageFont);
}

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

char drawFont(uint32_t x, uint32_t y, uint32_t sx, uint32_t sy, uint32_t width, uint32_t height){
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

char drawCharFontOLD(uint32_t x, uint32_t y, uint32_t sx, uint32_t sy, uint32_t w, uint32_t h) {
    //width = (sx+width>fontMeta->width?fontMeta->width:width);
    //height = (sy+height>fontMeta->height?fontMeta->height:height);
    qemu_log("[FONTS] [DRAW] [PARAMS] X:% Y:%d SX:%d SY:%d W:%d H:%d",x,y,sx,sy,w,h);
    uint32_t th = h;
    uint32_t tw = w;
        if(sx+w>fontMeta->width) { tw = fontMeta->width; }
        if(sy+h>fontMeta->height) { th = fontMeta->height; }
    qemu_log("[FONTS] [DRAW] [PARAMS] X:% Y:%d SX:%d SY:%d W:%d H:%d",x,y,sx,sy,w,h);
    int wx = sx, wy = sy;
    qemu_log("[Y] %d < %d-%d",wy,th,sy);
    while(wy<(th-sy)) {
        qemu_log("\t[Y] %d < %d-%d",wy,th,sy);
        wx = sx;
        qemu_log("\t[Pre-X] (%d)%d-%d < %d",(wx-sx),wx,sx,tw);
        while((wx-sx)<(tw)) {
            qemu_log("\t\t[X] (%d)%d-%d < %d",(wx-sx),wx,sx,tw);

            qemu_log("\t\t[PX] (%d*%d)[%d] / (%d*%d)[%d] / %d",fontMeta->width,alphaFont,(fontMeta->width*alphaFont),wx,alphaFont,(wx*alphaFont),wy);
            int px = pixidx(fontMeta->width*alphaFont, wx*alphaFont, wy);
            qemu_log("\t\t\t[PX] %d < %d",px,strlen(imageFont));
            int r = imageFont[px];
            qemu_log("[r]");
            int g = imageFont[px+1];qemu_log("[g]");
            int b = imageFont[px+2];qemu_log("[b]");
            qemu_log("\t\t[PX] PX: %d R:%d G:%d B:%d",px,r,g,b);
            //int a = imageFont[px+3];
            int color = ((r&0xff)<<16)|((g&0xff)<<8)|(b&0xff);
            qemu_log("\t\t[FONTS] [COLOR] %x",color);
            if(color!=0xFFFFFF) {
                qemu_log("\t\t\t[MOD] [DRAW] X: %d Y:%d",x+(wx-sx),y+(wy-sy));
                set_pixel(x+(wx-sx), y+(wy-sy), 0xFFFFFF);
            }
            wx++;
        }
        wy++;
    }
}
