// Minesweeper by Sal1r
// Partial fixes by NDRAEY

#include <kernel.h>
#include <gui/basics.h>
#include <gui/circle.h>
#include <drv/input/keyboard.h>
#include <drv/input/mouse.h>
#include <lib/rand.h>
#include "./minesweeper_sprites.h"

#define BLOCK_SIZE 32
#define FIELD_WIDTH 9
#define FIELD_HEIGHT 9

// Баг с выходом чинить надо

// Убрать поля позиции. Сделать метод для получения координат. 
// Вообще, много говнокода. Можно многое уменьшить.
typedef struct block { // Переписать
  int32_t fx, fy; // Позиция на поле
  int32_t sx, sy; // Позиция на экране
  uint8_t has_mine; // 0 - без мины, 1 - с миной
  uint8_t closed; // 0 - открыт, 1 - закрыт
  uint8_t checked; // 0 - не помечен, 1 - помечен
  uint8_t count; // Кол-во мин вокруг. Для has_mine == 0
} block;

typedef struct array {
  block** arr;
  size_t size;
} array;

void init_array(array* arr) {
  arr->arr = NULL;
  arr->size = 0;
}

void array_append(array* arr, block* item) {
  arr->arr = krealloc(arr->arr, (arr->size + 1) * sizeof(block*));
  ++(arr->size);
  arr->arr[arr->size - 1] = item;
}

block* array_get(array* arr, size_t index) {
  return arr->arr[index];
}

void array_clear(array* arr) {
  kfree(arr->arr);
  arr->arr = NULL;
  arr->size = 0;
}

void move_array(array* from, array* to) {
  kfree(to->arr);
  to->arr = from->arr;
  to->size = from->size;
  from->arr = NULL;
  from->size = 0;
}

block field[FIELD_WIDTH * FIELD_HEIGHT] = {0};
uint8_t running = 1;
uint8_t mb1 = 0;
uint8_t mb2 = 0;

void draw_sprite(const uint32_t* sprite, const uint32_t sprite_size, int32_t sx, int32_t sy) {
  for (int x = 0; x < sprite_size; ++x) {
    for (int y = 0; y < sprite_size; ++y) {
      uint32_t pix = sprite[sprite_size * y + x];
      if (pix >> 24 == 0) continue;
      set_pixel(sx + x, sy + y,
            ((pix & 0xFF0000) >> 16) + (pix & 0x00FF00) + ((pix & 0x0000FF) << 16));
    }
  }
}

void init_field(int32_t sx, int32_t sy) {
  for (int x = 0; x < 9; ++x) {
    for (int y = 0; y < 9; ++y) {
      block* b = &field[FIELD_HEIGHT * x + y]; // Переписать
      b->fx = x;
      b->fy = y;
      b->sx = sx + x * BLOCK_SIZE + x * 2;
      b->sy = sy + y * BLOCK_SIZE + y * 2;
      b->has_mine = 0;
      b->closed = 1;
      b->checked = 0;
      b->count = 0;
    }
  }
  // Переписать
  for (int i = 0; i < 10; ++i) {
    int r = (uint32_t)rand() % 81;
    block* b = &field[r];
    b->has_mine = 1;
  }

  for (int i = 0; i < 81; ++i) {
    block* b = &field[i];

    if (!b->has_mine) { // Переписать
      uint8_t count = 0;

      for (int x = -1; x < 2; ++x) {
        for (int y = -1; y < 2; ++y) {
          int xx = b->fx + x;
          int yy = b->fy + y;
          if (xx < 0 || yy < 0 || xx >= FIELD_WIDTH || 
            yy >= FIELD_HEIGHT || (xx == b->fx && yy == b->fy)) continue; // Переписать
          block* bb = &field[FIELD_HEIGHT * xx + yy]; // Переписать
          if (bb->has_mine) ++count;
        }
      }

      b->count = count;
    }
  }
}

void draw_field() { // Переписать
  for (int i = 0; i < 81; ++i) {
    block* b = &field[i];

    if (b->closed) {
      // Закрытая
      draw_filled_rectangle(b->sx, b->sy, BLOCK_SIZE, BLOCK_SIZE, 0xAAAAAA);
      if (b -> checked)
        // С флажком
        draw_sprite(flag_sprite, FLAG_FRAME_SIZE, b->sx, b->sy);
    } else {
      // Открытая
      draw_filled_rectangle(b->sx, b->sy, BLOCK_SIZE, BLOCK_SIZE, 0x777777);
      // С числом мин
      if (b->count > 0) {
        // char* count_str;
        // itoa(b->count, count_str);
        // draw_vga_str(count_str, 1, b->sx, b->sy, 0xFFFFFF);
        uint32_t* num = nums_sprites[b->count - 1];
        draw_sprite(num, NUMS_FRAME_SIZE, b->sx, b->sy);
      }
      // С миной
      if (b->has_mine) {
        draw_sprite(mine_sprite, MINE_FRAME_SIZE, b->sx, b->sy);
      }
    }
  }
}

void input() {
  if (getCharRaw() == 1) running = 0;

  // Открытие клетки
  if (mouse_get_b1()) {
    if (!mb1) {
      mb1 = 1;
      int32_t mx = mouse_get_x();
      int32_t my = mouse_get_y();
      
      for (int i = 0; i < FIELD_WIDTH * FIELD_HEIGHT; ++i) {
        block* b = &field[i];

        if (mx >= b->sx && mx <= b->sx + BLOCK_SIZE &&
            my >= b->sy && my <= b->sy + BLOCK_SIZE) {
          if (!b->checked && b->closed) {
            b->closed = 0;

            array arr = {0};
            init_array(&arr);
            array_append(&arr, b);

            array second_arr = {0};
            init_array(&second_arr);

            do {
              for (size_t i = 0; i < arr.size; ++i) {
              b = array_get(&arr, i);

              for (int x = -1; x < 2; ++x) {
                int xx = b->fx + x;
                int yy = b->fy;
                if (xx < 0 || yy < 0 || xx >= FIELD_WIDTH || 
                  yy >= FIELD_HEIGHT || (xx == b->fx && yy == b->fy)) continue; // Переписать
                block* bb = &field[FIELD_HEIGHT * xx + yy]; // Переписать
                if (!bb->checked && bb->closed && !bb->has_mine) {
                  bb->closed = 0;
                  if (bb->count == 0) array_append(&second_arr, bb);
                }
              }

              for (int y = -1; y < 2; ++y) {
                int xx = b->fx;
                int yy = b->fy + y;
                if (xx < 0 || yy < 0 || xx >= FIELD_WIDTH || 
                  yy >= FIELD_HEIGHT || (xx == b->fx && yy == b->fy)) continue; // Переписать
                block* bb = &field[FIELD_HEIGHT * xx + yy]; // Переписать
                if (!bb->checked && bb->closed && !bb->has_mine) {
                  bb->closed = 0;
                  if (bb->count == 0) array_append(&second_arr, bb);
                }
              }

              }
              move_array(&second_arr, &arr);

            } while (arr.size != 0);

			array_clear(&arr);
			array_clear(&second_arr);

            break;
          }
          if (b->has_mine) running = 0;
        }
      }
    }
  } else {
    mb1 = 0;
  }

  // Флажок
  if (mouse_get_b2()) {
    if (!mb2) {
      mb2 = 1;
      int32_t mx = mouse_get_x();
      int32_t my = mouse_get_y();
      
      for (int i = 0; i < 81; ++i) {
        block* b = &field[i];
        if (mx >= b->sx && mx <= b->sx + BLOCK_SIZE &&
            my >= b->sy && my <= b->sy + BLOCK_SIZE) {
          b->checked = !b->checked;
        }
      }
    }
  } else {
    mb2 = 0;
  }
}

void minesweeper() {
  clean_screen();
  set_cursor_enabled(false);
  keyboardctl(KEYBOARD_ECHO, false);

  init_field(10, 10);

  int lmx = 0;
  int lmy = 0;

  running = 1;

  while (running) {
    input();

    draw_field();

    draw_circle(lmx, lmy, 3, 0x000000);

    int32_t mx = mouse_get_x();
    int32_t my = mouse_get_y();
    draw_circle(mx, my, 3, 0xFFFFFF);
    lmx = mx;
    lmy = my; // По-красивше сделать курсор
    punch();
  }

  while (1) {
    if (getCharRaw() == 1) break;
  }

  clean_screen();
  set_cursor_enabled(true);

  keyboardctl(KEYBOARD_ECHO, true);
}
