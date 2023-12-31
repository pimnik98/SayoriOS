#include <io/ports.h>
#include <drv/beeper.h>
#include <io/status_loggers.h>
#include <io/status_sounds.h>
#include <io/tty.h>

void tty_error(char* format, ...)
{
    uint32_t orig = tty_getcolor();
    
    ERROR_sound();
    tty_setcolor(COLOR_ERROR);

    va_list args;
    va_start(args, format);

    tty_print(format, args);

    va_end(args);

    tty_setcolor(orig);
}

void tty_attention(char* format, ...)
{
    uint32_t orig = tty_getcolor();

    ATTENTION_sound();
    tty_setcolor(COLOR_ATENTION);

    va_list args;
    va_start(args, format);

    tty_print(format, args);

    va_end(args);

    tty_setcolor(orig);
}

void tty_alert(char* format, ...)
{
    uint32_t orig = tty_getcolor();

    ALERT_sound();
    tty_setcolor(COLOR_ALERT);

    va_list args;
    va_start(args, format);

    tty_print(format, args);

    va_end(args);

    tty_setcolor(orig);
}

void tty_global_error(char* format, ...)
{
    uint32_t orig = tty_getcolor();

    GLOBAL_ERROR_sound();
    tty_setcolor(COLOR_ERROR);

    va_list args;
    va_start(args, format);

    tty_print(format, args);

    va_end(args);

    tty_setcolor(orig);
}