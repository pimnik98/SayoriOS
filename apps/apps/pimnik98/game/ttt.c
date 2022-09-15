/**
 * @file apps/apps/pimnik98/game/ttt.c
 * @authors Пиминов Никита (github.com/pimnik98 | VK: @piminov_remont)
 * @brief Tic-Tac-Toe | Игра "Крестики-Нолики"
 * @version 0.0.1
 * @date 2022-09-11
 *
 * @copyright Copyright Пиминов Никита (с) 2022
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vesa.h>
#include <tui.h>
#include <time.h>

int32_t upd = 0;             // Последнее обновление экрана
int32_t mSl = 0;             // Максимальное количество символов на линии
int32_t mHl = 0;          // Максимальное количество линий
int32_t oldPosX     = 0;                // Последние местоположение символа по X (место печати)
int32_t oldPosY     = 0;                // Последние местоположение символа по Y (место печати)
uint32_t x_point = 0;
uint32_t y_point = 0;

void gLog(char* msg){
    qemu_printf("[GAME] [TTT] %s\n",msg);
}

void drawP1(uint32_t point){
    uint32_t tx=0,ty=0;
    if (point == 1){
        tx = 8*6;
    } else if (point == 2){
        tx = 8*12;
    } else if (point == 3){
        ty = 16*6;
    } else if (point == 4){
        tx = 8*6;
        ty = 16*6;
    } else if (point == 5){
        tx = 8*12;
        ty = 16*6;
    } else if (point == 6){
        ty = 16*12;
    } else if (point == 7){
        tx = 8*6;
        ty = 16*12;
    } else if (point == 8){
        tx = 8*12;
        ty = 16*12;
    }

    draw_vga_character(201,tx+x_point+(8*2),ty+y_point+(16*2),0x43ACE8,0x333333);
    draw_vga_character(203,tx+x_point+(8*3),ty+y_point+(16*2),0x43ACE8,0x333333);
    draw_vga_character(187,tx+x_point+(8*4),ty+y_point+(16*2),0x43ACE8,0x333333);

    draw_vga_character(206,tx+x_point+(8*2),ty+y_point+(16*3),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*3),ty+y_point+(16*3),0x43ACE8,0x333333);
    draw_vga_character(206,tx+x_point+(8*4),ty+y_point+(16*3),0x43ACE8,0x333333);


    draw_vga_character(200,tx+x_point+(8*2),ty+y_point+(16*4),0x43ACE8,0x333333);
    draw_vga_character(208,tx+x_point+(8*3),ty+y_point+(16*4),0x43ACE8,0x333333);
    draw_vga_character(188,tx+x_point+(8*4),ty+y_point+(16*4),0x43ACE8,0x333333);
}

void drawP2(uint32_t point){
    uint32_t tx=0,ty=0;
    if (point == 1){
        tx = 8*6;
    } else if (point == 2){
        tx = 8*12;
    } else if (point == 3){
        ty = 16*6;
    } else if (point == 4){
        tx = 8*6;
        ty = 16*6;
    } else if (point == 5){
        tx = 8*12;
        ty = 16*6;
    } else if (point == 6){
        ty = 16*12;
    } else if (point == 7){
        tx = 8*6;
        ty = 16*12;
    } else if (point == 8){
        tx = 8*12;
        ty = 16*12;
    }

    draw_vga_character(177,tx+x_point+(8*2),ty+y_point+(16*2),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*3),ty+y_point+(16*2),0x43ACE8,0x333333);
    draw_vga_character(177,tx+x_point+(8*4),ty+y_point+(16*2),0x43ACE8,0x333333);

    draw_vga_character(0,tx+x_point+(8*2),ty+y_point+(16*3),0x43ACE8,0x333333);
    draw_vga_character(177,tx+x_point+(8*3),ty+y_point+(16*3),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*4),ty+y_point+(16*3),0x43ACE8,0x333333);

    draw_vga_character(177,tx+x_point+(8*2),ty+y_point+(16*4),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*3),ty+y_point+(16*4),0x43ACE8,0x333333);
    draw_vga_character(177,tx+x_point+(8*4),ty+y_point+(16*4),0x43ACE8,0x333333);
}

void drawP0(uint32_t point){
    uint32_t tx=0,ty=0;
    if (point == 1){
        tx = 8*6;
    } else if (point == 2){
        tx = 8*12;
    } else if (point == 3){
        ty = 16*6;
    } else if (point == 4){
        tx = 8*6;
        ty = 16*6;
    } else if (point == 5){
        tx = 8*12;
        ty = 16*6;
    } else if (point == 6){
        ty = 16*12;
    } else if (point == 7){
        tx = 8*6;
        ty = 16*12;
    } else if (point == 8){
        tx = 8*12;
        ty = 16*12;
    }

    draw_vga_character(0,tx+x_point+(8*2),ty+y_point+(16*2),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*3),ty+y_point+(16*2),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*4),ty+y_point+(16*2),0x43ACE8,0x333333);

    draw_vga_character(0,tx+x_point+(8*2),ty+y_point+(16*3),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*3),ty+y_point+(16*3),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*4),ty+y_point+(16*3),0x43ACE8,0x333333);


    draw_vga_character(0,tx+x_point+(8*2),ty+y_point+(16*4),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*3),ty+y_point+(16*4),0x43ACE8,0x333333);
    draw_vga_character(0,tx+x_point+(8*4),ty+y_point+(16*4),0x43ACE8,0x333333);
}

void drawMain(){
    gLog("Drawing main...");
    x_point = ((mSl-38)/2)*8;
    y_point = ((mHl-19)/2)*16;
    // Рисуем игровое поле
    drawRect(x_point,y_point,38*8,19*16,0x333333);
    // Рисуем линии
    drawRect(x_point+(8*1),y_point+(6*16),17*8,16,0x000000);
    drawRect(x_point+(8*1),y_point+(12*16),17*8,16,0x000000);
    drawRect(x_point+(8*6),y_point+(1*16),8,16*17,0x000000);
    drawRect(x_point+(8*12),y_point+(1*16),8,16*17,0x000000);
    char* OSNAME = "Tic-Tac-Toy";
    setPosX(x_point+(8*21));
    setPosY(y_point+(16*3));
    puts_color(OSNAME,0xCAFE12, 0x333333);
    drawP1(2);
    drawP1(3);
    drawP1(4);
    drawP1(6);
    drawP1(8);
    drawP2(0);
    drawP2(1);
    drawP2(5);
    drawP2(7);
}

int main(){
    gLog("Started...");
    mHl = getHeightScreen()/16;
    mSl = (getWidthScreen()/8);
    drawMain();
    return 1;
}
