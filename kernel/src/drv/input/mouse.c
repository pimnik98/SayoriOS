/**
 * @file drv/input/mouse.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), Рустем Гимадутдинов (https://github.com/rgimad/EOS)
 * @brief Драйвер мыши
 * @version 0.3.5
 * @date 2022-12-11
 * @copyright Copyright SayoriOS Team (c) 2022-2024
 */

#include <io/ports.h>
#include <sys/trigger.h>
#include <drv/input/mouse.h>
#include "io/screen.h"
#include "sys/isr.h"
#include "drv/ps2.h"

uint8_t mouse_ready = 0;        ///< Готова ли мышь к работе

uint32_t mouse_x  = 0;           ///< Позиция мыши по X
uint32_t mouse_y  = 0;           ///< Позиция мыши по Y


int32_t mouse_ox  = 0;           ///< Позиция мыши по X (старое значение)
int32_t mouse_oy  = 0;           ///< Позиция мыши по Y (старое значение)

uint32_t mouse_b1 = 0;           ///< Левая кнопка мыши
uint32_t mouse_b2 = 0;           ///< Правая кнопка мыши
uint32_t mouse_b3 = 0;           ///< Средняя кнопка мыши
uint32_t mouse_b4 = 0;           ///< ???
uint32_t mouse_b5 = 0;           ///< ???

int mouse_wheel = 0;            ///< После каждого чтения меняем на 0

/**
 * @brief Структура данных пакета от мыши
 */
typedef struct mouse_flags_byte {
    unsigned int left_button   : 1;
    unsigned int right_button  : 1;
    unsigned int middle_button : 1;

    unsigned int always1 : 1;

    unsigned int x_sign : 1;
    unsigned int y_sign : 1;

    unsigned int x_overflow : 1;
    unsigned int y_overflow : 1;
} __attribute__((packed)) mouse_flags_byte;


/**
 * @brief Структура данных пакета от мыши
 */
struct dev_ps2m_mouse_packet {
    int16_t movement_x;
    int16_t movement_y;
    uint8_t button_l;
    uint8_t button_m;
    uint8_t button_r;
} ps2m_buffer;

/**
 * @brief Инициализирована ли мышь?
 *
 * @return bool - Да/Нет
 */
bool isMouseInit(){
    return mouse_ready==1?true:false;
}

/**
 * @brief Парсинг пакета мыши
 *
 * @warning Не нужно вызывать самостоятельно, только для обработчика мыши!
 */
void mouse_parse_packet(const char *buf, uint8_t has_wheel, uint8_t has_5_buttons) {
    mouse_flags_byte *mfb = (mouse_flags_byte*) (buf);
    if (mfb->x_overflow || mfb->y_overflow || !mfb->always1) {
        return;
    }

    int offx = (int16_t) (0xff00 * mfb->x_sign) | buf[1];
    int offy = (int16_t) (0xff00 * mfb->y_sign) | buf[2];
    mouse_x += offx;
    mouse_y -= offy;
    mouse_b1 = mfb->left_button;
    mouse_b2 = mfb->right_button;
    mouse_b3 = mfb->middle_button;
    ps2m_buffer.movement_x = offx;
    ps2m_buffer.movement_y = offy;
    ps2m_buffer.button_l = mouse_b1;
    ps2m_buffer.button_r = mouse_b2;
    ps2m_buffer.button_m = mouse_b3;


	if (mouse_b1 || mouse_b2 || mouse_b3 || (mouse_x != mouse_ox) || (mouse_y != mouse_oy)){
		CallTrigger(0x0003,(void*)mouse_b1,(void*)mouse_b2,(void*)mouse_b3,(void*)mouse_x,(void*)mouse_y);
        mouse_ox = mouse_x;
        mouse_oy = mouse_y;
	}
	//qemu_log("MPP: B1: %d | B2: %d | B3: %d | X:%d | Y:%d",mouse_b1,mouse_b2,mouse_b3,mouse_x,mouse_y);

    if (has_wheel) {
        mouse_wheel += (char) ((!!(buf[3] & 0x8)) * 0xf8 | (buf[3] & 0x7));
        if (has_5_buttons) {
            mouse_b4 = !!(buf[3] & 0x20);
            // parse buttons 4-5 (byte 3, bits 4-5)
        }
    }
}

/**
 * @brief Обработчик мыши
 *
 * @warning Не нужно вызывать самостоятельно, только для обработчика ядра!
 */
void mouse_handler(__attribute__((unused)) struct registers r) {
    uint8_t status = inb(0x64);
    if ((status & 1) == 0 || (status >> 5 & 1) == 0) {
        return;
    }

    static int recbyte = 0;
    static char mousebuf[5];

    mousebuf[recbyte++] = inb(0x60);
    if (recbyte == 3 /* + has_wheel */) {
        recbyte = 0;

        mouse_parse_packet(mousebuf, 0, 0);

        // Bounds
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > (int) (getScreenWidth())) mouse_x = getScreenWidth();
        if (mouse_y > (int) (getScreenHeight())) mouse_y = getScreenHeight(); //-10;
		CallTrigger(0x0002,(void*)mouse_x,(void*)mouse_y,0,0,0);
    }
}

/**
 * @brief Ожидание ответа мыши
 *
 * @param a_type - Тип отправляемых данных
 *
 * @warning Не нужно вызывать самостоятельно, только для обработчика мыши!
 */
void mouse_wait(uint8_t a_type) {
    uint32_t _time_out = 100;
    if (a_type == 0) {
        while (_time_out--) { //Data
            if ((inb(0x64) & 1) == 1) {
                return;
            }
        }
        return;
    } else {
        while (_time_out--) { //Signal
            if ((inb(0x64) & 2) == 0) {
                return;
            }
        }
        return;
    }
}

/**
 * @brief Отправка данных для мыши
 *
 * @warning Не нужно вызывать самостоятельно, только для обработчика мыши!
 */
void mouse_write(uint8_t a_write) { //unsigned char
    // Ожидаем возможности, пока можно будет отправить команду
//    mouse_wait(1);
    ps2_in_wait_until_empty();
    // Говорим мышке, что мы отправляем команду
    outb(PS2_STATE_REG, 0xD4);
    // Ожидаем ответа
//    mouse_wait(1);
    ps2_in_wait_until_empty();
    // Отправляем данные
    outb(PS2_DATA_PORT, a_write);
}

/**
 * @brief Считывание данных с мыши
 *
 * @warning Не нужно вызывать самостоятельно, только для обработчика мыши!
 */
uint8_t mouse_read() {
    // Получаем ответ от мыши
//    mouse_wait(0);
    ps2_out_wait_until_full();
    return inb(PS2_DATA_PORT);
}

/**
 * @brief Установщик драйвера мыши
 *
 * @warning Не нужно вызывать самостоятельно, только для обработчика мыши!
 */
void mouse_install() {
    uint8_t status = ps2_read_configuration_byte();

    ps2_write_configuration_byte(status | 2);

    // Скажите мыши использовать настройки по умолчанию
    mouse_write(0xF6);
    mouse_read(); // Acknowledge

    // // Включить мышь
    mouse_write(0xF4);
    mouse_read(); // Acknowledge

    // Установить координаты курсора в середине экрана
    mouse_x = getScreenWidth() / 2;
    mouse_y = getScreenHeight() / 2;
}

void ps2_mouse_install_irq() {
    register_interrupt_handler(IRQ12, &mouse_handler);
    mouse_ready = 1;
}

uint32_t mouse_get_x() {return mouse_x;}
uint32_t mouse_get_y() {return mouse_y;}
uint8_t  mouse_get_b1() {return mouse_b1;}
uint8_t  mouse_get_b2() {return mouse_b2;}
uint8_t  mouse_get_b3() {return mouse_b3;}
uint8_t  mouse_get_b4() {return mouse_b4;}
uint8_t  mouse_get_b5() {return mouse_b5;}

