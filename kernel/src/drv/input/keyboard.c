/**
 * @file drv/input/keyboard.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), Андрей Павленко (andrejpavlenko666@gmail.com)
 * @brief Драйвер клавиатуры
 * @version 0.3.0
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022
 */
#include <kernel.h>
#include <io/ports.h>

#define		KEY_BUFFER_SIZE		16
#define		KBD_IS_READDATA			(1 << 0)
#define		KBD_IS_WRITEDATA		(1 << 1)
#define		KBD_IS_RESET			(1 << 2)
#define		KBD_IS_CMD				(1 << 3)
#define		KBD_IS_LOCK				(1 << 4)
#define		KBD_IS_MOUSEDATA		(1 << 5)
#define		KBD_IS_TIMEOUT			(1 << 6)
#define		KBD_IS_ODDERROR			(1 << 7)
#define		KBD_DATA_PORT		0x60
#define		KBD_CTRL_REG		0x61
#define		KBD_STATE_REG		0x64

bool    SHIFT = false,          ///< Включен ли SHIFT
        RU = false,             ///< Печатаем русскими?
        enabled = true;         ///< Включен ли вывод?
int     lastKey = 0,            ///< Последний индекс клавишы
        timePresed = 0;         ///< Время последнего нажатия
char    kbdbuf[256] = {0};      ///< Буфер клавиатуры
uint8_t kbdstatus = 0;          ///< Статус клавиатуры
bool    echo = true;            ///< Включен ли вывод?

char kmode = 0;
char* curbuf = 0;
uint32_t chartyped = 0;

/**
 * @brief Выводит правильный символ, в зависимости от языка и шифта
 *
 * @param char* en_s - Символ маленький англиский
 * @param char* en_b - Символ большой англиский
 * @param char* ru_s - Символ маленький русский
 * @param char* ru_b - Символ большой русский
 *
 * @return char* - Символ в зависимости от раскладки и языка
 */
char* __getCharKeyboard(char* en_s,char* en_b,char* ru_s,char* ru_b){
    return (RU?(SHIFT?ru_b:ru_s):(SHIFT?en_b:en_s));
}

/**
 * @brief Выводит символ, в зависимости от кода полученного с клавиатуры
 *
 * @param int key - Код клавиатуры
 * 
 * @return void* - Или символ или код
 */
void* getCharKeyboard(int key,bool mode){
	// TODO: Make a layout manager that supports any custom keyboard layout.
    char* b;// = kmalloc(sizeof(char)*3);
    bool found = false;
    switch (key){
        // case 0:    b = "?"; found = false; break;
        case 0x29: b = __getCharKeyboard("`","~","ё","Ё"); found = true; break;
        case 0x02: b = __getCharKeyboard("1","!","1","!"); found = true; break;
        case 0x03: b = __getCharKeyboard("2","@","2","\"");found = true; break;
        case 0x04: b = __getCharKeyboard("3","#","3","№"); found = true; break;
        case 0x05: b = __getCharKeyboard("4","$","4",";"); found = true; break;
        case 0x06: b = __getCharKeyboard("5","%","5","%"); found = true; break;
        case 0x07: b = __getCharKeyboard("6","^","6",":"); found = true; break;
        case 0x08: b = __getCharKeyboard("7","&","7","?"); found = true; break;
        case 0x09: b = __getCharKeyboard("8","*","8","*"); found = true; break;
        case 0x0A: b = __getCharKeyboard("9","(","9","("); found = true; break;
        case 0x0B: b = __getCharKeyboard("0",")","0",")"); found = true; break;
        case 0x0C: b = __getCharKeyboard("-","_","-","_"); found = true; break;
        case 0x0D: b = __getCharKeyboard("=","+","=","+"); found = true; break;

        case 0x10: b = __getCharKeyboard("q","Q","й","Й"); found = true; break;
        case 0x11: b = __getCharKeyboard("w","W","ц","Ц"); found = true; break;
        case 0x12: b = __getCharKeyboard("e","E","у","У"); found = true; break;
        case 0x13: b = __getCharKeyboard("r","R","к","К"); found = true; break;
        case 0x14: b = __getCharKeyboard("t","T","е","Е"); found = true; break;
        case 0x15: b = __getCharKeyboard("y","Y","н","Н"); found = true; break;
        case 0x16: b = __getCharKeyboard("u","U","г","Г"); found = true; break;
        case 0x17: b = __getCharKeyboard("i","I","ш","Ш"); found = true; break;
        case 0x18: b = __getCharKeyboard("o","O","щ","Щ"); found = true; break;
        case 0x19: b = __getCharKeyboard("p","P","з","З"); found = true; break;
        case 0x1A: b = __getCharKeyboard("[","{","х","Х"); found = true; break;
        case 0x1B: b = __getCharKeyboard("]","}","ъ","Ъ"); found = true; break;

        case 0x1E: b = __getCharKeyboard("a","A","ф","Ф"); found = true; break;
        case 0x1F: b = __getCharKeyboard("s","S","ы","Ы"); found = true; break;
        case 0x20: b = __getCharKeyboard("d","D","в","В"); found = true; break;
        case 0x21: b = __getCharKeyboard("f","F","а","А"); found = true; break;
        case 0x22: b = __getCharKeyboard("g","G","п","П"); found = true; break;
        case 0x23: b = __getCharKeyboard("h","H","р","Р"); found = true; break;
        case 0x24: b = __getCharKeyboard("j","J","о","О"); found = true; break;
        case 0x25: b = __getCharKeyboard("k","K","л","Л"); found = true; break;
        case 0x26: b = __getCharKeyboard("l","L","д","Д"); found = true; break;
        case 0x27: b = __getCharKeyboard(";",":","ж","Ж"); found = true; break;
        case 0x28: b = __getCharKeyboard("'","\"","э","Э");found = true; break;
        case 0x2B: b = __getCharKeyboard("\\","|","\\","/");found = true; break;

        case 0x2C: b = __getCharKeyboard("z","Z","я","Я"); found = true; break;
        case 0x2D: b = __getCharKeyboard("x","X","ч","Ч"); found = true; break;
        case 0x2E: b = __getCharKeyboard("c","C","с","С"); found = true; break;
        case 0x2F: b = __getCharKeyboard("v","V","м","М"); found = true; break;
        case 0x30: b = __getCharKeyboard("b","B","и","И"); found = true; break;
        case 0x31: b = __getCharKeyboard("n","N","т","Т"); found = true; break;
        case 0x32: b = __getCharKeyboard("m","M","ь","Ь"); found = true; break;
        case 0x33: b = __getCharKeyboard(",","<,","б","Б"); found = true; break;
        case 0x34: b = __getCharKeyboard(".",">","ю","Ю"); found = true; break;
        case 0x35: b = __getCharKeyboard("/","?",".",","); found = true; break;

        // case 0x0E: b = __getCharKeyboard("\b","\b","\b","\b"); found = true; break; // Backspace
        case 0x0E: b = __getCharKeyboard("","","",""); found = true; break; // Backspace
        case 0x0F: b = __getCharKeyboard("\t","\t","\t","\t"); found = true; break; // Tab
        case 0x39: b = __getCharKeyboard(" "," "," "," "); found = true; break; // Space
        case 0x1C: b = __getCharKeyboard("\n","\n","\n","\n"); found = true; break; // Enter

        case 0xA9: b = __getCharKeyboard("`","~","ё","Ё"); found = true; break;
        case 0x82: b = __getCharKeyboard("1","!","1","!"); found = true; break;
        case 0x83: b = __getCharKeyboard("2","@","2","\"");found = true; break;
        case 0x84: b = __getCharKeyboard("3","#","3","№"); found = true; break;
        case 0x85: b = __getCharKeyboard("4","$","4",";"); found = true; break;
        case 0x86: b = __getCharKeyboard("5","%","5","%"); found = true; break;
        case 0x87: b = __getCharKeyboard("6","^","6",":"); found = true; break;
        case 0x88: b = __getCharKeyboard("7","&","7","?"); found = true; break;
        case 0x89: b = __getCharKeyboard("8","*","8","*"); found = true; break;
        case 0x8A: b = __getCharKeyboard("9","(","9","("); found = true; break;
        case 0x8B: b = __getCharKeyboard("0",")","0",")"); found = true; break;
        case 0x8C: b = __getCharKeyboard("-","_","-","_"); found = true; break;
        case 0x8D: b = __getCharKeyboard("=","+","=","+"); found = true; break;

        case 0x90: b = __getCharKeyboard("q","Q","й","Й"); found = true; break;
        case 0x91: b = __getCharKeyboard("w","W","ц","Ц"); found = true; break;
        case 0x92: b = __getCharKeyboard("e","E","у","У"); found = true; break;
        case 0x93: b = __getCharKeyboard("r","R","к","К"); found = true; break;
        case 0x94: b = __getCharKeyboard("t","T","е","Е"); found = true; break;
        case 0x95: b = __getCharKeyboard("y","Y","н","Н"); found = true; break;
        case 0x96: b = __getCharKeyboard("u","U","г","Г"); found = true; break;
        case 0x97: b = __getCharKeyboard("i","I","ш","Ш"); found = true; break;
        case 0x98: b = __getCharKeyboard("o","O","щ","Щ"); found = true; break;
        case 0x99: b = __getCharKeyboard("p","P","з","З"); found = true; break;
        case 0x9A: b = __getCharKeyboard("[","{","х","Х"); found = true; break;
        case 0x9B: b = __getCharKeyboard("]","}","ъ","Ъ"); found = true; break;

        case 0x9E: b = __getCharKeyboard("a","A","ф","Ф"); found = true; break;
        case 0x9F: b = __getCharKeyboard("s","S","ы","Ы"); found = true; break;
        case 0xA0: b = __getCharKeyboard("d","D","в","В"); found = true; break;
        case 0xA1: b = __getCharKeyboard("f","F","а","А"); found = true; break;
        case 0xA2: b = __getCharKeyboard("g","G","п","П"); found = true; break;
        case 0xA3: b = __getCharKeyboard("h","H","р","Р"); found = true; break;
        case 0xA4: b = __getCharKeyboard("j","J","о","О"); found = true; break;
        case 0xA5: b = __getCharKeyboard("k","K","л","Л"); found = true; break;
        case 0xA6: b = __getCharKeyboard("l","L","д","Д"); found = true; break;
        case 0xA7: b = __getCharKeyboard(";",":","ж","Ж"); found = true; break;
        case 0xA8: b = __getCharKeyboard("'","\"","э","Э");found = true; break;
        case 0xAB: b = __getCharKeyboard("\\","|","\\","/");found = true; break;

        case 0xAC: b = __getCharKeyboard("z","Z","я","Я"); found = true; break;
        case 0xAD: b = __getCharKeyboard("x","X","ч","Ч"); found = true; break;
        case 0xAE: b = __getCharKeyboard("c","C","с","С"); found = true; break;
        case 0xAF: b = __getCharKeyboard("v","V","м","М"); found = true; break;
        case 0xB0: b = __getCharKeyboard("b","B","и","И"); found = true; break;
        case 0xB1: b = __getCharKeyboard("n","N","т","Т"); found = true; break;
        case 0xB2: b = __getCharKeyboard("m","M","ь","Ь"); found = true; break;
        case 0xB3: b = __getCharKeyboard(",","<,","б","Б"); found = true; break;
        case 0xB4: b = __getCharKeyboard(".",">","ю","Ю"); found = true; break;
        case 0xB5: b = __getCharKeyboard("/","?",".",","); found = true; break;

        case 0xB9: b = __getCharKeyboard(" "," "," "," "); found = true; break; // Space

        default: b = "?"; found = false; break;
    }
    //qemu_log("[Char] Keyboard: %d => %s", key, b);
    return (mode?key:(found?b:0));
}

unsigned char getPressReleaseKeyboard() {
    return lastKey & 0x80; // if true -> Released / else - Pressed
}

void keyboardctl(uint8_t param, bool value) {
    if(param == KEYBOARD_ECHO) {
        echo = value;
    }
}

int getCharRaw() {
    return lastKey;
}

int getIntKeyboardWait(){
    int kmutex = 0;
    mutex_get(&kmutex, true);

    while(lastKey==0 || (lastKey & 0x80)) {}

    mutex_release(&kmutex);
    return lastKey;
}

void* getCharKeyboardWait(bool ints) {
	/*
    int kmutex = 0;
    mutex_get(&kmutex, true);
    void* ret = 0;

    while(lastKey==0 || ((lastKey & 0x80) && ret==0)) {}

    ret = getCharKeyboard(lastKey,ints);
    lastKey = 0;

    mutex_release(&kmutex);
    return ret;
	*/
	// char* ret = 0;

	kmode = 2;
	while(kmode==2) {
		if (lastKey != 0 && !(lastKey & 0x80)) {
			kmode = 0;
			// ret = lastKey;
			lastKey = 0;
		}
	}

	return getCharKeyboard(curbuf, false);
}

/**
 * @brief Получение виртуального буфера с клавиатуры
 *
 * @return char* - Или символ или код
 */
char* getStringBufferKeyboard(){
    memset(kbdbuf, 0, 256);
    while(1){

        int kblen;
        if ((kblen = strlen(kbdbuf)) > 254){
            qemu_log("Buffer FULL! Max 255!!");
            break;
        }
        if (lastKey == 0) continue;
        int ikey = getIntKeyboardWait();
        lastKey = 0;
        char* key = getCharKeyboard(ikey, false);
        //qemu_log("[LK] %x | %x\n",ikey,getCharKeyboardWait(true));
        //tty_printf("[LK] %x | %d\n",ikey,key);
        if (ikey == 0x9C || ikey == 0xE0 || ikey == 0x1C){
            // Нажат Enter отбрасываем обработку
            break;
        }
        /*if(ikey == 0x0E || ikey == 0x8E) { // BACKSPACE
            int kbdl = strlen(kbdbuf);
        	if(kbdl > 0) {
        		kbdbuf[kbdl - 1] = 0;
        		tty_backspace();
            }
            qemu_log("BKSP!!!");
            qemu_log("%s > %d", kbdbuf, strlen(kbdbuf));
        }*/
        if (key != 0 && lastKey < 128){
            strcat(kbdbuf, key);
            //tty_printf("\n[%d/256] Buffer: %s; KEY: %s; LK: %x | %x\n", kblen, kbdbuf, key,ikey,lastKey);
            continue;
        }
        lastKey = 0;
    }
    return kbdbuf;
}

void kbd_add_char(char *buf, char* key) {
	if(kmode==1 && curbuf!=0) {
		if (!(lastKey == 0x1C || lastKey == 0x0E)) {
			strcat(buf, key);
			chartyped++;
		}
		
		if(lastKey == 0x0E) { // BACKSPACE
			if(chartyped > 0) {
				tty_backspace();
				chartyped--;
				qemu_log("Deleted character: %c", buf[chartyped]);
				buf[chartyped] = 0;
			}
        }
	}else if(kmode==2){
		curbuf = key;
	}
}

void gets(char *buffer) { // TODO: Backspace
	// qemu_log("KMODE is: %d, curbuf at: %x", kmode, (int)((void*)curbuf));
    
	kmode = 1;
	curbuf = buffer;
	    
	while(kmode==1) {
		if (/*lastKey == 0x9C || lastKey == 0xE0 || */ lastKey == 0x1C){
            curbuf = 0;
			lastKey = 0;
            kmode = 0;
			chartyped = 0;
        }
	}
}

/**
 * @brief Обработчик клавиатуры
 */
void keyboardHandler(registers_t regs){
    //qemu_log("%d < %d",timePresed < getTicks());
    if (!enabled) return;

    outb(0x20, 0x20);

    kbdstatus = inb(KBD_STATE_REG);
    if (kbdstatus & 0x01) {
        // if (timePresed > getTicks()){
        //     return;
        // }

        lastKey = inb(KBD_DATA_PORT);
        if (lastKey == 42) {
            SHIFT = true;
            //tty_printf("SHIFTED\n");
            return;
        } else if (lastKey == 0x3B){
            RU = !RU;
            //tty_printf("Ru %s\n",(RU?"вкл":"выкл"));
            return;
        }

        char* key = getCharKeyboard(lastKey, false);
        if (key != 0 && lastKey < 128){
            if(echo) tty_printf("%s", key);
			// qemu_log("Key is: %d => %s", lastKey, key);
			kbd_add_char(curbuf, key);
        }

        /*
        if(lastKey & 0x80) {
            qemu_log("[KBD] Release on key: %s", key);
        }else{
            qemu_log("[KBD] Press on key: %s", key);
        }
        */
        timePresed = getTicks()+100;
        SHIFT = false;
        // lastKey = 0;
        return;
    }
}

/**
 * @brief Выполняет инициализацию клавиатуры
 */
void keyboardInit() {
    register_interrupt_handler(IRQ1, &keyboardHandler);
    qemu_log("Keyboard installed");
}
