/**
 * @file drv/input/keyboard.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Драйвер клавиатуры
 * @version 0.3.5
 * @date 2022-11-01
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */
extern void tty_backspace();

#include <lib/string.h>
#include <io/ports.h>
#include <sys/trigger.h>
#include "drv/input/keyboard.h"
#include "sys/sync.h"
#include "sys/timer.h"
#include "io/tty.h"
#include "drv/psf.h"
#include "sys/isr.h"

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
        RU = false;             ///< Печатаем русскими?
int     lastKey = 0;            ///< Последний индекс клавишы
uint8_t kbdstatus = 0;          ///< Статус клавиатуры
bool    echo = true;            ///< Включен ли вывод?
bool    key_ctrl = false;

char kmode = 0;
char* curbuf = 0;
uint32_t chartyped = 0;

/**
 * @brief Выводит правильный символ, в зависимости от языка и шифта
 *
 * @param en_s - Символ маленький англиский
 * @param en_b - Символ большой англиский
 * @param ru_s - Символ маленький русский
 * @param ru_b - Символ большой русский
 *
 * @return char* - Символ в зависимости от раскладки и языка
 */
char* __getCharKeyboard(char* en_s, char* en_b, char* ru_s, char* ru_b){
    return (RU?(SHIFT?ru_b:ru_s):(SHIFT?en_b:en_s));
}

/**
 * @brief Выводит символ, в зависимости от кода полученного с клавиатуры
 *
 * @param key - Код клавиатуры
 * @param mode - Какой-то режим
 *
 * @return char* - Или символ или код
 */
char* getCharKeyboard(int key, bool mode){
	// TODO: Make a layout manager that supports any custom keyboard layout.
    char* b;// = kmalloc(sizeof(char)*3);
    bool found = false;

    switch(key) {
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

    return mode?(char*)key:(found?b:0);
}

uint8_t getPressReleaseKeyboard() {
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

bool is_lctrl_key() {
    return key_ctrl;
}

int getIntKeyboardWait(){
    bool kmutex = false;
    mutex_get(&kmutex, true);

    while(lastKey==0 || (lastKey & 0x80)) {}

    mutex_release(&kmutex);
    return lastKey;
}

void* getCharKeyboardWait(bool ints) {
	kmode = 2;
	while(kmode==2) {
		if (lastKey != 0 && !(lastKey & 0x80)) {
			kmode = 0;
			lastKey = 0;
		}
	}

    if(ints) {
    	return curbuf;
    } else {
    	return getCharKeyboard((int)curbuf, ints);
    }
}

void kbd_add_char(char *buf, char* key) {
	if(kmode == 1 && curbuf != 0) {
		if (!(lastKey == 0x1C || lastKey == 0x0E)) {
			strcat(buf, key);
			chartyped++;
		}
		
		if(lastKey == 0x0E) { // BACKSPACE
            if(chartyped > 0) {
				tty_backspace();
				chartyped--;
				buf[chartyped] = 0;
			}
        }
	} else if(kmode == 2) {
		curbuf = key;
	}
}

void gets(char *buffer) { // TODO: Backspace
    // qemu_log("KMODE is: %d, curbuf at: %x", kmode, (int)((void*)curbuf));

    kmode = 1;
    curbuf = buffer;

    while(kmode == 1) {
        if (lastKey == 0x9C) { // Enter key pressed
            curbuf = 0;
            lastKey = 0;
            kmode = 0;
            chartyped = 0;
        }
    }
}

// Limited version of gets.
// Returns 0 if okay, returns 1 if you typed more than `length` keys.
int gets_max(char *buffer, int length) { // TODO: Backspace
    kmode = 1;
    curbuf = buffer;

    while(kmode == 1) {
        if(chartyped >= length) {
            curbuf = 0;
            lastKey = 0;
            kmode = 0;
            chartyped = 0;
            return 1;
        }

        if (lastKey == 0x9C) { // Enter key pressed
            curbuf = 0;
            lastKey = 0;
            kmode = 0;
            chartyped = 0;
        }
    }

    return 0;
}

/**
 * @brief Обработчик клавиатуры
 */
void keyboardHandler(registers_t regs){
    //qemu_log("%d < %d",timePresed < getTicks());
//    if (!enabled) return;

//    outb(0x20, 0x20);

    kbdstatus = inb(KBD_STATE_REG);
    if (kbdstatus & 0x01) {
        lastKey = inb(KBD_DATA_PORT);
        int cl = 1;

        CallTrigger(
			0x0001,
			(void*)(lastKey % 0x80),
			(void*)!getPressReleaseKeyboard(),
			0,
			0,
			&cl ///< Вешаем событие на 5й аргумент
		);

		//qemu_log("[N-CL] %x | %d",cl,cl);
		if (cl == 0)
            return;
		
        if (lastKey == 42) { // SHIFT press
            SHIFT = true;
            return;
        } else if (lastKey == 0x3B){ // F1
            RU = !RU;
            return;
        } else if (lastKey == 170) { // Shift release
            SHIFT = false;
        } else if (lastKey == 29) { // Left Ctrl press
            key_ctrl = true;
        } else if (lastKey == 157) { // Left Ctrl release
            key_ctrl = false;
        }

        char* key = getCharKeyboard(lastKey, false);
        if (key != 0 && lastKey < 128){
            if(echo) {
                if(!key_ctrl && lastKey != 0x0E)
                    tty_printf("%s", key);
            }
			kbd_add_char(curbuf, key);
        }

        /*        
        if(lastKey & 0x80) {
            qemu_log("[KBD] Release on key: %d", lastKey);
        }else{
            qemu_log("[KBD] Press on key: %d", lastKey);
        }
        */

        return;
    }
}

uint8_t ps2_read_configuration_byte() {
    outb(KBD_STATE_REG, 0x20);

    return inb(KBD_DATA_PORT);
}

void ps2_write_configuration_byte(uint8_t byte) {
    outb(KBD_STATE_REG, 0x60);
    outb(KBD_DATA_PORT, byte);
}

// false - error; true - ok
bool ps2_test() {
    outb(KBD_STATE_REG, 0xAA); // Test

    uint8_t reply = inb(KBD_DATA_PORT);

    return reply == 0x55;
}

/**
 * @brief Выполняет инициализацию клавиатуры
 */
void keyboardInit() {
    // Disable keyboard 
    outb(KBD_STATE_REG, 0xAD); // 1
    outb(KBD_STATE_REG, 0xA7); // 2

    // Flush The Output Buffer
    while(inb(KBD_STATE_REG) & 1) {
        inb(KBD_DATA_PORT);
    }

    uint8_t byte = ps2_read_configuration_byte();

    if((byte >> 5) & 1) {
        qemu_log("Dual channel PS/2 controller!");
    } else {
        qemu_log("Not a dual channel PS/2 controller!");
    }

    byte &= 0b10111100;

    ps2_write_configuration_byte(byte);

    bool test_ok = ps2_test();

    if(test_ok) {
        qemu_log("PS/2 test ok!");
    }

    if((byte >> 5) & 1) { // If it's dual channel...
        outb(KBD_STATE_REG, 0xA8); // Enable second port

        byte = ps2_read_configuration_byte();

        if((byte >> 5) & 1) {
            qemu_log("Bit 5 is set, can't be dual channel PS/2 controller...");
        } else {
            qemu_log("Dual channel! Disabling second port...");

            outb(KBD_STATE_REG, 0xA7); // Disable second port
        }
    }

    qemu_log("Testing first port...");

    outb(KBD_STATE_REG, 0xAB); // Test first port

    uint8_t result = inb(KBD_DATA_PORT);

    if(result == 0x00) {
        qemu_log("Passed test for channel 1!");
        outb(KBD_STATE_REG, 0xAE);
    } else {
        qemu_log("Channel 1: Test failed! Result: %x", result);
    }

    if(!((byte >> 5) & 1)) {
        // Dual channel?

        outb(KBD_STATE_REG, 0xA9); // Test second port

        result = inb(KBD_DATA_PORT);

        if(result == 0x00) {
            qemu_log("Passed test for channel 2!");
            outb(KBD_STATE_REG, 0xA8);
        } else {
            qemu_log("Channel 2: Test failed! Result: %x", result);
        }
    }

    // Enable interrupts

    byte = ps2_read_configuration_byte();
    byte |= 0b01000011;

    ps2_write_configuration_byte(byte);

    // Register interrupts
    register_interrupt_handler(IRQ1, &keyboardHandler);
    qemu_log("Keyboard installed");
}
