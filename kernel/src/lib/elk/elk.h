#ifndef SAYORI_ELK_H
#define SAYORI_ELK_H
#pragma once

#include "common.h"
#include "ext/jse_canvas.h"
#include "ext/jse_event.h"
#include "ext/jse_array.h"

typedef	int		double_t;   // Тип данных для чисел

struct js;                  // Движок JS (непрозрачный)
typedef uint64_t jsval_t;   // Типы данных хранящие значения JS

struct js *js_create(void *buf, size_t len);                        // Создать экземпляр JS
jsval_t js_eval(struct js *, const char *, size_t);                 // Выполнить JS-код
jsval_t js_glob(struct js *);                                       // Возвращает глобальный объект
const char *js_str(struct js *, jsval_t val);                       // Stringify JS value
bool js_chkargs(jsval_t *, int, const char *);                      // Проверяет правильность аргументов
bool js_truthy(struct js *, jsval_t);                               // Проверьте, является ли значение истинным
void js_setmaxcss(struct js *, size_t);                             // Установите максимальный размер стека
void js_setgct(struct js *, size_t);                                // Установите порог срабатывания GC
void js_stats(struct js *, size_t *total, size_t *min, size_t *cstacksize);
void js_dump(struct js *);                                          // Выводит отладочную информацию.
void js_statsInfo(struct js *js);                                   // Печает в консоль результаты выполнения JS экземпляра

// Создайте значения JS из значений C
jsval_t js_mkundef(void);                                           // Создать undefined
jsval_t js_mknull(void);                                            // Создать null, null, true, false
jsval_t js_mktrue(void);                                            // Создать true
jsval_t js_mkfalse(void);                                           // Создать false
jsval_t js_mkstr(struct js *, const void *, size_t);                // Создать строку
jsval_t js_mknum(double_t);                                         // Создать число
jsval_t js_mkerr(struct js *js, const char *fmt, ...);              // Создать error
jsval_t js_mkfun(jsval_t (*fn)(struct js *, jsval_t *, int));       // Создать функцию
jsval_t js_mkobj(struct js *);                                      // Создать объект
void js_set(struct js *, jsval_t, const char *, jsval_t);           // Установить object attr

enum { JS_UNDEF, JS_NULL, JS_TRUE, JS_FALSE, JS_STR, JS_NUM, JS_ERR, JS_PRIV };
int js_type(jsval_t val);                                           // Возвращает тип JS-значения
double_t js_getnum(jsval_t val);                                    // Получить число
int js_getbool(jsval_t val);                                        // Получить boolean, 0 or 1
char *js_getstr(struct js *js, jsval_t val, size_t *len);           // Получить строку


typedef uint32_t jsoff_t;

struct js {
    jsoff_t css;                                                    // Максимальный наблюдаемый размер стека C
    jsoff_t lwm;                                                    // JS RAM low watermark: наблюдается минимальное количество свободной оперативной памяти
    const char *code;                                               // Анализируемый в данный момент фрагмент кода (весь код, который выполняется)
    char errmsg[33];                                                // Сообщение об ошибке
    uint8_t isFatal;                                                // Выполнение закончилось ошибкой?
    uint8_t tok;                                                    // Последнее проанализированное значение токена
    uint8_t consumed;                                               // Индикатор того, что был использован последний проанализированный токен
    uint8_t flags;                                                  // Флаги выполнения, смотрите константы F_* ниже
#define F_NOEXEC 1U                                                 // Разбирать код, но не выполнять
#define F_LOOP 2U                                                   // Код выполняется внутри петли
#define F_CALL 4U                                                   // Код выполняется внутри вызова функции
#define F_BREAK 8U                                                  // Выход из цикла
#define F_RETURN 16U                                                // Был выполнен возврат из функции
    jsoff_t clen;                                                   // Длина фрагмента кода
    jsoff_t pos;                                                    // Текущая позиция синтаксического анализа
    jsoff_t toff;                                                   // Смещение последнего проанализированного токена
    jsoff_t tlen;                                                   // Длина последнего проанализированного токена
    jsoff_t nogc;                                                   // Смещение объекта для исключения из GC
    jsval_t tval;                                                   // Содержит последнее проанализированное числовое или строковое литеральное значение
    jsval_t scope;                                                  // Текущая область применения
    uint8_t *mem;                                                   // Доступная память JS
    jsoff_t size;                                                   // Объем памяти JS
    jsoff_t brk;                                                    // Текущая граница использования mem
    jsoff_t gct;                                                    // Порог GC. Если brk > gct, запустите GC
    jsoff_t maxcss;                                                 // Максимально допустимый размер использования стека C
    jsoff_t incSize;                                                // Размер загруженных библиотек
    jsoff_t paramSize;                                              // Кол-во просканированных параметров JSE CF
    JSE_CANVAS Canvas;                                              // [JSE] [EXT] [Canvas]
    void *cstk;                                                     // Указатель стека C в начале js_eval()
};

// В памяти JS хранятся различные сущности: объекты, свойства, строки
// Все объекты упаковываются в начало буфера.
// `brk` обозначает конец используемой памяти:
//
//    | entity1 | entity2| .... |entityN|     неиспользуемая память    |
//    |---------|--------|------|-------|------------------------------|
//  js.mem                           js.brk                        js.size
//
//  Каждый объект выровнен по 4 байтам, поэтому 2 бита LSB хранят тип объекта.
//  Объект: 8 байт: смещение первого свойства, смещение верхнего объекта
//  Свойство: 8 байт + значение val: 4 байта следующего пропа, 4 байта отстува, N байт значения
//  Строка: 4xN байт: 4 байта len << 2, 4 байта выровненных по 0-завершенных данных
//
// Если импортируются функции C, они используют верхнюю часть памяти в качестве стека для передачи параметров.
// Каждый аргумент помещается в верхнюю часть памяти как jsval_t, а js.size уменьшается на sizeof(jsval_t), т.е. на 8 байт.
// Когда функция возвращается, js.size восстанавливается обратно.
// Таким образом, js.size используется в качестве указателя стека.

enum {
    TOK_ERR, TOK_EOF, TOK_IDENTIFIER, TOK_NUMBER, TOK_STRING, TOK_SEMICOLON,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACE, TOK_RBRACE,
    // Keyword tokens
    TOK_BREAK = 50, TOK_CASE, TOK_CATCH, TOK_CLASS, TOK_CONST, TOK_CONTINUE,
    TOK_DEFAULT, TOK_DELETE, TOK_DO, TOK_ELSE, TOK_FINALLY, TOK_FOR, TOK_FUNC,
    TOK_IF, TOK_IN, TOK_INSTANCEOF, TOK_LET, TOK_NEW, TOK_RETURN, TOK_SWITCH,
    TOK_THIS, TOK_THROW, TOK_TRY, TOK_VAR, TOK_VOID, TOK_WHILE, TOK_WITH,
    TOK_YIELD, TOK_UNDEF, TOK_NULL, TOK_TRUE, TOK_FALSE,
    // JS Operator tokens
    TOK_DOT = 100, TOK_CALL, TOK_POSTINC, TOK_POSTDEC, TOK_NOT, TOK_TILDA,    // 100
    TOK_TYPEOF, TOK_UPLUS, TOK_UMINUS, TOK_EXP, TOK_MUL, TOK_DIV, TOK_REM,    // 106
    TOK_PLUS, TOK_MINUS, TOK_SHL, TOK_SHR, TOK_ZSHR, TOK_LT, TOK_LE, TOK_GT,  // 113
    TOK_GE, TOK_EQ, TOK_NE, TOK_AND, TOK_XOR, TOK_OR, TOK_LAND, TOK_LOR,      // 121
    TOK_COLON, TOK_Q,  TOK_ASSIGN, TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN,
    TOK_MUL_ASSIGN, TOK_DIV_ASSIGN, TOK_REM_ASSIGN, TOK_SHL_ASSIGN,
    TOK_SHR_ASSIGN, TOK_ZSHR_ASSIGN, TOK_AND_ASSIGN, TOK_XOR_ASSIGN,
    TOK_OR_ASSIGN, TOK_COMMA,
};

enum {
    // ВАЖНО: T_OBJ, T_PROP, T_STR должны быть запущены первыми.
    // Этого требуют функции компоновки памяти: типы объектов памяти кодируются в 2 битах,
    // таким образом, значения типов должны быть 0,1,2,3
    T_OBJ, T_PROP, T_STR, T_UNDEF, T_NULL, T_NUM, T_BOOL, T_FUNC, T_CODEREF,
    T_CFUNC, T_ERR
};

static jsval_t tov(double_t d) {
    union { double_t d; jsval_t v; } u = {d};
    return u.v;
}
static jsval_t tov2(int d) { union { int d; jsval_t v; } u = {d}; return u.v; }
static double_t tod(jsval_t v) {
    union { jsval_t v; double_t d; } u = {v};
    return u.d;
}
static double_t tod2(jsval_t v) {
    union { jsval_t v; int d; } u = {v};
    return u.d;
}
static jsval_t mkval(uint8_t type, uint64_t data) { return ((jsval_t) 0x7ff0U << 48U) | ((jsval_t) (type) << 48) | (data & 0xffffffffffffUL); }
static bool is_nan(jsval_t v) { return (v >> 52U) == 0x7ffU; }
static uint8_t vtype(jsval_t v) { return is_nan(v) ? ((v >> 48U) & 15U) : (uint8_t) T_NUM; }
static size_t vdata(jsval_t v) { return (size_t) (v & ~((jsval_t) 0x7fffUL << 48U)); }
static jsval_t mkcoderef(jsval_t off, jsoff_t len) { return mkval(T_CODEREF, (off & 0xffffffU) | ((jsval_t)(len & 0xffffffU) << 24U)); }
static jsoff_t coderefoff(jsval_t v) { return v & 0xffffffU; }
static jsoff_t codereflen(jsval_t v) { return (v >> 24U) & 0xffffffU; }

static uint8_t unhex(uint8_t c) { return (c >= '0' && c <= '9') ? (uint8_t) (c - '0') : (c >= 'a' && c <= 'f') ? (uint8_t) (c - 'W') : (c >= 'A' && c <= 'F') ? (uint8_t) (c - '7') : 0; }
static bool is_space(int c) { return c == ' ' || c == '\r' || c == '\n' || c == '\t' || c == '\f' || c == '\v'; }
static bool is_digit(int c) { return c >= '0' && c <= '9'; }
static bool is_xdigit(int c) { return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
static bool is_alpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
static bool is_ident_begin(int c) { return c == '_' || c == '$' || is_alpha(c); }
static bool is_ident_continue(int c) { return c == '_' || c == '$' || is_alpha(c) || is_digit(c); }
static bool is_err(jsval_t v) { return vtype(v) == T_ERR; }
static bool is_unary(uint8_t tok) { return tok >= TOK_POSTINC && tok <= TOK_UMINUS; }
static bool is_assign(uint8_t tok) { return (tok >= TOK_ASSIGN && tok <= TOK_OR_ASSIGN); }
static void saveoff(struct js *js, jsoff_t off, jsoff_t val) { memcpy(&js->mem[off], &val, sizeof(val)); }
static void saveval(struct js *js, jsoff_t off, jsval_t val) { memcpy(&js->mem[off], &val, sizeof(val)); }
static jsoff_t loadoff(struct js *js, jsoff_t off) { jsoff_t v = 0; passert(js->brk <= js->size); memcpy(&v, &js->mem[off], sizeof(v)); return v; }
static jsoff_t offtolen(jsoff_t off) { return (off >> 2) - 1; }
static jsoff_t vstrlen(struct js *js, jsval_t v) { return offtolen(loadoff(js, (jsoff_t) vdata(v))); }
static jsval_t loadval(struct js *js, jsoff_t off) { jsval_t v = 0; memcpy(&v, &js->mem[off], sizeof(v)); return v; }
static jsval_t upper(struct js *js, jsval_t scope) { return mkval(T_OBJ, loadoff(js, (jsoff_t) (vdata(scope) + sizeof(jsoff_t)))); }
static jsoff_t align32(jsoff_t v) { return ((v + 3) >> 2) << 2; }

#define CHECKV(_v) do { if (is_err(_v)) { res = (_v); goto done; } } while (0)
#define EXPECT(_tok, _e) do { if (next(js) != _tok) { _e; return js_mkerr(js, "parse error"); }; js->consumed = 1; } while (0)

static size_t tostr(struct js *js, jsval_t value, char *buf, size_t len);
static jsval_t js_expr(struct js *js);
static jsval_t js_stmt(struct js *js);
static jsval_t do_op(struct js *, uint8_t op, jsval_t l, jsval_t r);

void jse_file_getBuff(char* buf);
JSE_ARRAY* js_getObjectToArray(struct js *js, jsval_t value);


jsval_t elk_call_js_fnc(struct js *js, const char *fn, jsoff_t fnlen);

#include "ext/jse_function.h"

#endif //SAYORI_ELK_H
