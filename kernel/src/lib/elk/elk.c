#include "common.h"
#include "portability.h"
#include "lib/string.h"
#include <io/ports.h>
#include <lib/stdio.h>
#include <lib/sprintf.h>
#include <stdarg.h>

#include "elk_config.h"
#include "elk.h"

#ifndef JS_EXPR_MAX
#define JS_EXPR_MAX 20
#endif

#ifndef JS_GC_THRESHOLD
#define JS_GC_THRESHOLD 0.75
#endif



static const char *typestr(uint8_t t) {
    const char *names[] = { "object", "prop", "string", "undefined", "null",
                            "number", "boolean", "function", "coderef",
                            "cfunc", "err", "nan" };
    return (t < sizeof(names) / sizeof(names[0])) ? names[t] : "??";
}

// Pack JS values into uin64_t, double nan, per IEEE 754
// 64bit "double": 1 bit sign, 11 bits exponent, 52 bits mantissa
//
// seeeeeee|eeeemmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm
// 11111111|11110000|00000000|00000000|00000000|00000000|00000000|00000000 inf
// 11111111|11111000|00000000|00000000|00000000|00000000|00000000|00000000 qnan
//
// 11111111|1111tttt|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv
//  NaN marker |type|  48-bit placeholder for values: pointers, strings
//
// On 64-bit platforms, pointers are really 48 bit only, so they can fit,
// provided they are sign extended

static void js_printPos(struct js *js) {
    size_t line = 1;
    bool lastStop = false;
    size_t endLine = 0;
    char* buf = js->code;
    if (js->toff < js->incSize) {
        qemu_err("[JSE] [Error] Library parsing...");
        return;
    }
    for (int i = js->incSize; i < js->clen; i++) {
        //qemu_log("[%d > %d] Line:%d | `%c`",i, js->toff, line, buf[i]);
        if (i > js->toff) lastStop = true;
        if (buf[i] == '\n' && lastStop) {
            endLine = i;
            break;
        } else if (buf[i] == '\n' && !lastStop) {
            line++;
        }
    }

    char* t_dump = calloc(1, (endLine - js->toff) + 1);

    memcpy(t_dump, buf + js->toff, endLine - js->toff);

    jse_trim(t_dump);

    qemu_err("[JSE] [Error] Line %d: %s", line + js->paramSize, t_dump);
    kfree(t_dump);
}

static void elk_dump(struct js *js) {
    qemu_note("[JSE] Engine dump:");
    qemu_note("  |--- Maximum view RAM | %d",js->css);
    qemu_note("  |--- Low WaterMark | %d",js->lwm);
    qemu_note("  |--- Is fatal | %d",js->isFatal);
    qemu_note("  |--- Last token | %d",js->tok);
    qemu_note("  |--- Consumed | %d",js->consumed);
    qemu_note("  |--- Flags | %x",js->flags);
    qemu_note("  |--- Length code | %d",js->clen);
    qemu_note("  |--- Position code | %d",js->pos);
    qemu_note("  |--- Last position token (offset) | %d",js->toff);
    qemu_note("  |--- Last position token (len) | %d",js->tlen);
    qemu_note("  |--- Scope | %d",js->scope);
    qemu_note("  |--- Size | %d",js->size);
    qemu_note("  |--- brk | %d",js->brk);
    qemu_note("  |--- gct | %d",js->gct);
    qemu_note("  |--- Max RAM JS | %d",js->maxcss);
}

static void setlwm(struct js *js) {
    jsoff_t n = 0, css = 0;
    if (js->brk < js->size) n = js->size - js->brk;
    if (js->lwm > n) js->lwm = n;
    if ((char *) js->cstk > (char *) &n)
        css = (jsoff_t) ((char *) js->cstk - (char *) &n);
    if (css > js->css) js->css = css;
}

// Copy src to dst, make no overflows, 0-terminate. Return bytes copied
static size_t cpy(char *dst, size_t dstlen, const char *src, size_t srclen) {
    size_t i = 0;
    for (i = 0; i < dstlen && i < srclen && src[i] != 0; i++) dst[i] = src[i];
    if (dstlen > 0) dst[i < dstlen ? i : dstlen - 1] = '\0';
    return i;
}

// Stringify JS object
static size_t js_getObjLite(struct js *js, jsval_t obj, char *buf, size_t len) {
    qemu_log("[JSE] js_getObjLite Len:%d", len);
    size_t n = cpy(buf, len, "{", 1);
    jsoff_t next = loadoff(js, (jsoff_t) vdata(obj)) & ~3U;  // First prop offset
    while (next < js->brk && next != 0) {                    // Iterate over props

        //qemu_log("[JSE] STROBJ %d < %d && %d != 0", next , js->brk , next);
        jsoff_t koff = loadoff(js, next + (jsoff_t) sizeof(next));
        jsval_t val = loadval(js, next + (jsoff_t) (sizeof(next) + sizeof(koff)));
        // printf("PROP %u, koff %u\n", next & ~3, koff);
        n += cpy(buf + n, len - n, ",", n == 1 ? 0 : 1);
        n += tostr(js, mkval(T_STR, koff), buf + n, len - n);
        n += cpy(buf + n, len - n, ":", 1);
        if (js_type(val) == JS_NUM) {
            //qemu_note("is number");
            n += tostr(js, val, buf + n, len - n);
        } else {
            //qemu_note("is NO numer");
            n += tostr(js, val, buf + n, len - n);
        }
        next = loadoff(js, next) & ~3U;  // Load next prop offset
        //qemu_log("  |-- STROBJ NEXT (n:%d)(%d) %s", n, strlen(buf), buf);
    }
    return n + cpy(buf + n, len - n, "}", 1);
}

// Stringify JS object
static size_t strobj(struct js *js, jsval_t obj, char *buf, size_t len) {
    qemu_log("[JSE] STROBJ Len:%d", len);
    size_t n = cpy(buf, len, "{", 1);
    jsoff_t next = loadoff(js, (jsoff_t) vdata(obj)) & ~3U;  // First prop offset
    while (next < js->brk && next != 0) {                    // Iterate over props

        //qemu_log("[JSE] STROBJ %d < %d && %d != 0", next , js->brk , next);
        jsoff_t koff = loadoff(js, next + (jsoff_t) sizeof(next));
        jsval_t val = loadval(js, next + (jsoff_t) (sizeof(next) + sizeof(koff)));
        // printf("PROP %u, koff %u\n", next & ~3, koff);
        n += cpy(buf + n, len - n, ",", n == 1 ? 0 : 1);
        n += cpy(buf + n, len - n, "\"", 1);
        n += tostr(js, mkval(T_STR, koff), buf + n, len - n);
        n += cpy(buf + n, len - n, "\"", 1);
        n += cpy(buf + n, len - n, ":", 1);
        n += cpy(buf + n, len - n, "\"", 1);
        if (js_type(val) == JS_NUM) {
            //qemu_note("is number");
            n += tostr(js, val, buf + n, len - n);
            n += cpy(buf + n - 1, len - n, "\"", 1);
        } else {
            //qemu_note("is NO numer");
            n += tostr(js, val, buf + n, len - n);
            n += cpy(buf + n, len - n, "\"", 1);
        }
        next = loadoff(js, next) & ~3U;  // Load next prop offset
        //qemu_log("  |-- STROBJ NEXT (n:%d)(%d) %s", n, strlen(buf), buf);
    }
    for (int i = 0; i < n; i++) {
        if (buf[i] == 0x0) buf[i] = ' ';
        //qemu_log("[%d] %x | %c",i, buf[i], buf[i]);
    }
    qemu_log("[JSE] STROBJ RETURn %d", len);
    return n + cpy(buf + n, len - n, "}", 1);
}

// Stringify numeric JS value
// Во время тестов вызывался только когда требовался result
static size_t strnum(jsval_t value, char *buf, size_t len) {
    double_t dv = tod2(value), iv;
    return (size_t) snprintf(buf, len, "%d", dv);
}

// Return mem offset and length of the JS string
static jsoff_t vstr(struct js *js, jsval_t value, jsoff_t *len) {
    jsoff_t off = (jsoff_t) vdata(value);
    if (len) *len = offtolen(loadoff(js, off));
    return (jsoff_t) (off + sizeof(off));
}

// Stringify string JS value
static size_t strstring(struct js *js, jsval_t value, char *buf, size_t len) {
    jsoff_t slen, off = vstr(js, value, &slen);
    size_t n = 0;
    //n += cpy(buf + n, len - n, "\"", 1);
    n += cpy(buf + n, len - n, (char *) &js->mem[off], slen);
    //n += cpy(buf + n, len - n, "\"", 1);
    return n;
}

// Stringify JS function
static size_t strfunc(struct js *js, jsval_t value, char *buf, size_t len) {
    jsoff_t sn, off = vstr(js, value, &sn);
    size_t n = cpy(buf, len, "function", 8);
    return n + cpy(buf + n, len - n, (char *) &js->mem[off], sn);
}

jsval_t js_mkerr(struct js *js, const char *xx, ...) {
    js_printPos(js);
    va_list ap;
    size_t n = cpy(js->errmsg, sizeof(js->errmsg), "ERROR: ", 7);
    va_start(ap, xx);
    vsnprintf(js->errmsg + n, sizeof(js->errmsg) - n, xx, ap);
    va_end(ap);
    js->errmsg[sizeof(js->errmsg) - 1] = '\0';
    js->isFatal = 1;
    js->pos = js->clen, js->tok = TOK_EOF, js->consumed = 0;  // Jump to the end
    return mkval(T_ERR, 0);
}

// Stringify JS value into the given buffer
static size_t tostr(struct js *js, jsval_t value, char *buf, size_t len) {
    switch (vtype(value)) {
        case T_UNDEF: return cpy(buf, len, "undefined", 9);
        case T_NULL:  return cpy(buf, len, "null", 4);
        case T_BOOL:  return cpy(buf, len, vdata(value) & 1 ? "true" : "false", vdata(value) & 1 ? 4 : 5);
        case T_OBJ:   return strobj(js, value, buf, len);
        case T_STR:   return strstring(js, value, buf, len);
        case T_NUM:   return strnum(value, buf, len);
        case T_FUNC:  return strfunc(js, value, buf, len);
        case T_CFUNC: return (size_t) snprintf(buf, len, "\"c_func_0x%lx\"", (unsigned long) vdata(value));
        case T_PROP:  return (size_t) snprintf(buf, len, "PROP@%lu", (unsigned long) vdata(value));
        default:      return (size_t) snprintf(buf, len, "VTYPE%d", vtype(value));
    }
}

static int js_cmp(struct js *js, jsval_t a, jsval_t b) {
    if (a == b) {
        return 0;  // Значения a и b равны
    } else {
        const char *a_str = js_str(js, a);
        const char *b_str = js_str(js, b);

        if (strcmp(a_str, b_str) < 0) {
            return -1;  // Значение a меньше b
        } else if (strcmp(a_str, b_str) > 0) {
            return 1;  // Значение a больше b
        } else {
            return 0;  // Значения a и b равны
        }
    }
    return 1;
}
#ifdef SAYORI_JSE_ARRAY_H
// Конвектировать объект в массив
JSE_ARRAY* js_getObjectToArray(struct js *js, jsval_t value) {
    char *buf = (char *) &js->mem[js->brk + sizeof(jsoff_t)];
    size_t len, available = js->size - js->brk - sizeof(jsoff_t);
    if (is_err(value)) return js->errmsg;
    JSE_ARRAY* array = jse_array_link_create();
    if (js->brk + sizeof(jsoff_t) >= js->size) return array;

    jsoff_t next = loadoff(js, (jsoff_t) vdata(value)) & ~3U;  // First prop offset
    while (next < js->brk && next != 0) {                    // Iterate over props
        jsoff_t koff = loadoff(js, next + (jsoff_t) sizeof(next));
        jsval_t val = loadval(js, next + (jsoff_t) (sizeof(next) + sizeof(koff)));
        char* KEY = calloc(1, available + 1);
        tostr(js, mkval(T_STR, koff), KEY, available);
        char* VALUE = calloc(1, available + 1);
        if (js_type(val) == JS_NUM) {
            //qemu_note("is number");
            //tostr(js, val, VALUE, available);
            jse_array_push(array, KEY, (JSE_ARRAY_VALUE){.int_value = js_getnum(val)}, JSE_ARRAY_TYPE_INT);
        } else {
            //qemu_note("is NO numer");
            tostr(js, val, VALUE, available);
            jse_array_push(array, KEY, (JSE_ARRAY_VALUE){.str_value = jse_strdup(VALUE)}, JSE_ARRAY_TYPE_STRING);
        }
        free(KEY);
        free(VALUE);
        next = loadoff(js, next) & ~3U;  // Load next prop offset
    }
    return array;
}
#endif //SAYORI_JSE_ARRAY_H

// Stringify JS value into a free JS memory block
const char *js_str(struct js *js, jsval_t value) {
    // Leave jsoff_t placeholder between js->brk and a stringify buffer,
    // in case if next step is convert it into a JS variable
    char *buf = (char *) &js->mem[js->brk + sizeof(jsoff_t)];
    size_t len, available = js->size - js->brk - sizeof(jsoff_t);
    if (is_err(value)) return js->errmsg;
    if (js->brk + sizeof(jsoff_t) >= js->size) return "";
    len = tostr(js, value, buf, available);
    js_mkstr(js, NULL, len);
    return buf;
}

bool js_truthy(struct js *js, jsval_t v) {
    uint8_t t = vtype(v);

    // qemu_warn("[js_truthy] v: %d | %f", tod2(v), tod2(v));
    return (t == T_BOOL && vdata(v) != 0) || (t == T_NUM && tod2(v) != 0.0) ||
           (t == T_OBJ || t == T_FUNC) || (t == T_STR && vstrlen(js, v) > 0);
}

static jsoff_t js_alloc(struct js *js, size_t size) {
    jsoff_t ofs = js->brk;
    size = align32((jsoff_t) size);  // 4-byte align, (n + k - 1) / k * k
    if (js->brk + size > js->size) return ~(jsoff_t) 0;
    js->brk += (jsoff_t) size;
    return ofs;
}

static jsval_t mkentity(struct js *js, jsoff_t b, const void *buf, size_t len) {
    jsoff_t ofs = js_alloc(js, len + sizeof(b));
    if (ofs == (jsoff_t) ~0) return js_mkerr(js, "oom");
    memcpy(&js->mem[ofs], &b, sizeof(b));
    // Using memmove - in case we're stringifying data from the free JS mem
    if (buf != NULL) memmove(&js->mem[ofs + sizeof(b)], buf, len);
    if ((b & 3) == T_STR) js->mem[ofs + sizeof(b) + len - 1] = 0;  // 0-terminate
    // printf("MKE: %u @ %u type %d\n", js->brk - ofs, ofs, b & 3);
    return mkval(b & 3, ofs);
}

jsval_t js_mkstr(struct js *js, const void *ptr, size_t len) {
    jsoff_t n = (jsoff_t) (len + 1);
    // printf("MKSTR %u %u\n", n, js->brk);
    return mkentity(js, (jsoff_t) ((n << 2) | T_STR), ptr, n);
}

static jsval_t mkobj(struct js *js, jsoff_t parent) {
    return mkentity(js, 0 | T_OBJ, &parent, sizeof(parent));
}

static jsval_t setprop(struct js *js, jsval_t obj, jsval_t k, jsval_t v) {
    jsoff_t koff = (jsoff_t) vdata(k);          // Key offset
    jsoff_t b, head = (jsoff_t) vdata(obj);     // Property list head
    char buf[sizeof(koff) + sizeof(v)];         // Property memory layout
    memcpy(&b, &js->mem[head], sizeof(b));      // Load current 1st prop offset
    memcpy(buf, &koff, sizeof(koff));           // Initialize prop data: copy key
    memcpy(buf + sizeof(koff), &v, sizeof(v));  // Copy value
    jsoff_t brk = js->brk | T_OBJ;              // New prop offset
    memcpy(&js->mem[head], &brk, sizeof(brk));  // Repoint head to the new prop
    return mkentity(js, (b & ~3U) | T_PROP, buf, sizeof(buf));  // Create new prop
}

// Return T_OBJ/T_PROP/T_STR entity size based on the first word in memory
static inline jsoff_t esize(jsoff_t w) {
    switch (w & 3U) {
    case T_OBJ:   return (jsoff_t) (sizeof(jsoff_t) + sizeof(jsoff_t));
    case T_PROP:  return (jsoff_t) (sizeof(jsoff_t) + sizeof(jsoff_t) + sizeof(jsval_t));
    case T_STR:   return (jsoff_t) (sizeof(jsoff_t) + align32(w >> 2U));
    default:      return (jsoff_t) ~0U;
    }
}

static bool is_mem_entity(uint8_t t) {
    return t == T_OBJ || t == T_PROP || t == T_STR || t == T_FUNC;
}

#define GCMASK ~(((jsoff_t) ~0) >> 1)  // Entity deletion marker
static void js_fixup_offsets(struct js *js, jsoff_t start, jsoff_t size) {
    for (jsoff_t n, v, off = 0; off < js->brk; off += n) {  // start from 0!
        v = loadoff(js, off);
        n = esize(v & ~GCMASK);
        if (v & GCMASK) continue;  // To be deleted, don't bother
        if ((v & 3) != T_OBJ && (v & 3) != T_PROP) continue;
        if (v > start) saveoff(js, off, v - size);
        if ((v & 3) == T_OBJ) {
            jsoff_t u = loadoff(js, (jsoff_t) (off + sizeof(jsoff_t)));
            if (u > start) saveoff(js, (jsoff_t) (off + sizeof(jsoff_t)), u - size);
        }
        if ((v & 3) == T_PROP) {
            jsoff_t koff = loadoff(js, (jsoff_t) (off + sizeof(off)));
            if (koff > start) saveoff(js, (jsoff_t) (off + sizeof(off)), koff - size);
            jsval_t val = loadval(js, (jsoff_t) (off + sizeof(off) + sizeof(off)));
            if (is_mem_entity(vtype(val)) && vdata(val) > start) {
                saveval(js, (jsoff_t) (off + sizeof(off) + sizeof(off)),
                        mkval(vtype(val), (unsigned long) (vdata(val) - size)));
            }
        }
    }
    // Fixup js->scope
    jsoff_t off = (jsoff_t) vdata(js->scope);
    if (off > start) js->scope = mkval(T_OBJ, off - size);
    if (js->nogc >= start) js->nogc -= size;
    // Fixup code that we're executing now, if required
    if (js->code > (char *) js->mem && js->code - (char *) js->mem < js->size &&
        js->code - (char *) js->mem > start) {
        js->code -= size;
        // printf("GC-ing code under us!! %ld\n", js->code - (char *) js->mem);
    }
    // printf("FIXEDOFF %u %u\n", start, size);
}

static void js_delete_marked_entities(struct js *js) {
    for (jsoff_t n, v, off = 0; off < js->brk; off += n) {
        v = loadoff(js, off);
        n = esize(v & ~GCMASK);
        if (v & GCMASK) {  // This entity is marked for deletion, remove it
            // printf("DEL: %4u %d %x\n", off, v & 3, n);
            // assert(off + n <= js->brk);
            js_fixup_offsets(js, off, n);
            memmove(&js->mem[off], &js->mem[off + n], js->brk - off - n);
            js->brk -= n;  // Shrink brk boundary by the size of deleted entity
            n = 0;         // We shifted data, next iteration stay on this offset
        }
    }
}

static void js_mark_all_entities_for_deletion(struct js *js) {
    for (jsoff_t v, off = 0; off < js->brk; off += esize(v)) {
        v = loadoff(js, off);
        saveoff(js, off, v | GCMASK);
    }
}

static jsoff_t js_unmark_entity(struct js *js, jsoff_t off) {
    jsoff_t v = loadoff(js, off);
    if (v & GCMASK) {
        saveoff(js, off, v & ~GCMASK);
        // printf("UNMARK %5u %d\n", off, v & 3);
        if ((v & 3) == T_OBJ) js_unmark_entity(js, v & ~(GCMASK | 3));
        if ((v & 3) == T_PROP) {
            js_unmark_entity(js, v & ~(GCMASK | 3));  // Unmark next prop
            js_unmark_entity(js, loadoff(js, (jsoff_t) (off + sizeof(off))));  // key
            jsval_t val = loadval(js, (jsoff_t) (off + sizeof(off) + sizeof(off)));
            if (is_mem_entity(vtype(val))) js_unmark_entity(js, (jsoff_t) vdata(val));
        }
    }
    return v & ~(GCMASK | 3U);
}

static void js_unmark_used_entities(struct js *js) {
    jsval_t scope = js->scope;
    do {
        js_unmark_entity(js, (jsoff_t) vdata(scope));
        scope = upper(js, scope);
    } while (vdata(scope) != 0);  // When global scope is GC-ed, stop
    if (js->nogc) js_unmark_entity(js, js->nogc);
    // printf("UNMARK: nogc %u\n", js->nogc);
    // js_dump(js);
}

void js_gc(struct js *js) {
    // printf("================== GC %u\n", js->nogc);
    setlwm(js);
    if (js->nogc == (jsoff_t) ~0) return;  // ~0 is a special case: GC Is disabled
    js_mark_all_entities_for_deletion(js);
    js_unmark_used_entities(js);
    js_delete_marked_entities(js);
}

// Skip whitespaces and comments
static jsoff_t skiptonext(const char *code, jsoff_t len, jsoff_t n) {
    // printf("SKIP: [%.*s]\n", len - n, &code[n]);
    while (n < len) {
        if (is_space(code[n])) {
            n++;
        } else if (n + 1 < len && code[n] == '/' && code[n + 1] == '/') {
            for (n += 2; n < len && code[n] != '\n';) n++;
        } else if (n + 3 < len && code[n] == '/' && code[n + 1] == '*') {
            for (n += 4; n < len && (code[n - 2] != '*' || code[n - 1] != '/');) n++;
        } else {
            break;
        }
    }
    return n;
}

static bool streq(const char *buf, size_t len, const char *p, size_t n) {
    return n == len && memcmp(buf, p, len) == 0;
}

static uint8_t parsekeyword(const char *buf, size_t len) {
    switch (buf[0]) {
        case 'b': if (streq("break", 5, buf, len)) return TOK_BREAK; break;
        case 'c': if (streq("class", 5, buf, len)) return TOK_CLASS; if (streq("case", 4, buf, len)) return TOK_CASE; if (streq("catch", 5, buf, len)) return TOK_CATCH; if (streq("const", 5, buf, len)) return TOK_CONST; if (streq("continue", 8, buf, len)) return TOK_CONTINUE; break;
        case 'd': if (streq("do", 2, buf, len)) return TOK_DO;  if (streq("default", 7, buf, len)) return TOK_DEFAULT; break; // if (streq("delete", 6, buf, len)) return TOK_DELETE; break;
        case 'e': if (streq("else", 4, buf, len)) return TOK_ELSE; break;
        case 'f': if (streq("for", 3, buf, len)) return TOK_FOR; if (streq("function", 8, buf, len)) return TOK_FUNC; if (streq("finally", 7, buf, len)) return TOK_FINALLY; if (streq("false", 5, buf, len)) return TOK_FALSE; break;
        case 'i': if (streq("if", 2, buf, len)) return TOK_IF; if (streq("in", 2, buf, len)) return TOK_IN; if (streq("instanceof", 10, buf, len)) return TOK_INSTANCEOF; break;
        case 'l': if (streq("let", 3, buf, len)) return TOK_LET; break;
        case 'n': if (streq("new", 3, buf, len)) return TOK_NEW; if (streq("null", 4, buf, len)) return TOK_NULL; break;
        case 'r': if (streq("return", 6, buf, len)) return TOK_RETURN; break;
        case 's': if (streq("switch", 6, buf, len)) return TOK_SWITCH; break;
        case 't': if (streq("try", 3, buf, len)) return TOK_TRY; if (streq("this", 4, buf, len)) return TOK_THIS; if (streq("throw", 5, buf, len)) return TOK_THROW; if (streq("true", 4, buf, len)) return TOK_TRUE; if (streq("typeof", 6, buf, len)) return TOK_TYPEOF; break;
        case 'u': if (streq("undefined", 9, buf, len)) return TOK_UNDEF; break;
        case 'v': if (streq("var", 3, buf, len)) return TOK_VAR; if (streq("void", 4, buf, len)) return TOK_VOID; break;
        case 'w': if (streq("while", 5, buf, len)) return TOK_WHILE; if (streq("with", 4, buf, len)) return TOK_WITH; break;
        case 'y': if (streq("yield", 5, buf, len)) return TOK_YIELD; break;
    }
    return TOK_IDENTIFIER;
}

static uint8_t parseident(const char *buf, jsoff_t len, jsoff_t *tlen) {
    if (is_ident_begin(buf[0])) {
        while (*tlen < len && is_ident_continue(buf[*tlen])) (*tlen)++;
        return parsekeyword(buf, *tlen);
    }
    return TOK_ERR;
}

static uint8_t next(struct js *js) {
    if (js->consumed == 0) return js->tok;
    js->consumed = 0;
    js->tok = TOK_ERR;
    js->toff = js->pos = skiptonext(js->code, js->clen, js->pos);
    js->tlen = 0;
    const char *buf = js->code + js->toff;
    if (js->toff >= js->clen) { js->tok = TOK_EOF; return js->tok; }
#define TOK(T, LEN) { js->tok = T; js->tlen = (LEN); break; }
#define LOOK(OFS, CH) js->toff + OFS < js->clen && buf[OFS] == CH
    switch (buf[0]) {
        case '?': TOK(TOK_Q, 1);
        case ':': TOK(TOK_COLON, 1);
        case '(': TOK(TOK_LPAREN, 1);
        case ')': TOK(TOK_RPAREN, 1);
        case '{': TOK(TOK_LBRACE, 1);
        case '}': TOK(TOK_RBRACE, 1);
        case ';': TOK(TOK_SEMICOLON, 1);
        case ',': TOK(TOK_COMMA, 1);
        case '!': if (LOOK(1, '=') && LOOK(2, '=')) TOK(TOK_NE, 3); TOK(TOK_NOT, 1);
        case '.': TOK(TOK_DOT, 1);
        case '~': TOK(TOK_TILDA, 1);
        case '-': if (LOOK(1, '-')) TOK(TOK_POSTDEC, 2); if (LOOK(1, '=')) TOK(TOK_MINUS_ASSIGN, 2); TOK(TOK_MINUS, 1);
        case '+': if (LOOK(1, '+')) TOK(TOK_POSTINC, 2); if (LOOK(1, '=')) TOK(TOK_PLUS_ASSIGN, 2); TOK(TOK_PLUS, 1);
        case '*': if (LOOK(1, '*')) TOK(TOK_EXP, 2); if (LOOK(1, '=')) TOK(TOK_MUL_ASSIGN, 2); TOK(TOK_MUL, 1);
        case '/': if (LOOK(1, '=')) TOK(TOK_DIV_ASSIGN, 2); TOK(TOK_DIV, 1);
        case '%': if (LOOK(1, '=')) TOK(TOK_REM_ASSIGN, 2); TOK(TOK_REM, 1);
        case '&': if (LOOK(1, '&')) TOK(TOK_LAND, 2); if (LOOK(1, '=')) TOK(TOK_AND_ASSIGN, 2); TOK(TOK_AND, 1);
        case '|': if (LOOK(1, '|')) TOK(TOK_LOR, 2); if (LOOK(1, '=')) TOK(TOK_OR_ASSIGN, 2); TOK(TOK_OR, 1);
        case '=': if (LOOK(1, '=') && LOOK(2, '=')) TOK(TOK_EQ, 3); TOK(TOK_ASSIGN, 1);
        case '<': if (LOOK(1, '<') && LOOK(2, '=')) TOK(TOK_SHL_ASSIGN, 3); if (LOOK(1, '<')) TOK(TOK_SHL, 2); if (LOOK(1, '=')) TOK(TOK_LE, 2); TOK(TOK_LT, 1);
        case '>': if (LOOK(1, '>') && LOOK(2, '=')) TOK(TOK_SHR_ASSIGN, 3); if (LOOK(1, '>')) TOK(TOK_SHR, 2); if (LOOK(1, '=')) TOK(TOK_GE, 2); TOK(TOK_GT, 1);
        case '^': if (LOOK(1, '=')) TOK(TOK_XOR_ASSIGN, 2); TOK(TOK_XOR, 1);
        case '"': case '\'':
            js->tlen++;
            while (js->toff + js->tlen < js->clen && buf[js->tlen] != buf[0]) {
                uint8_t increment = 1;
                if (buf[js->tlen] == '\\') {
                    if (js->toff + js->tlen + 2 > js->clen) break;
                    increment = 2;
                    if (buf[js->tlen + 1] == 'x') {
                        if (js->toff + js->tlen + 4 > js->clen) break;
                        increment = 4;
                    }
                }
                js->tlen += increment;
            }
            if (buf[0] == buf[js->tlen]) js->tok = TOK_STRING, js->tlen++;
            break;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
            char *end;
            js->tval = tov2(jse_p_int(buf, &end)); // TODO(lsm): protect against OOB access
            TOK(TOK_NUMBER, (jsoff_t) (end - buf));
        }
        default: js->tok = parseident(buf, js->clen - js->toff, &js->tlen); break;
    }
    js->pos = js->toff + js->tlen;
    //printf("NEXT: %d %d [%s]\n", js->tok, js->pos, (int) js->tlen, buf);
    return js->tok;
}

static inline uint8_t lookahead(struct js *js) {
    uint8_t old = js->tok, tok = 0;
    jsoff_t pos = js->pos;
    js->consumed = 1;
    tok = next(js);
    js->pos = pos, js->tok = old;
    return tok;
}

static void mkscope(struct js *js) {
    passert((js->flags & F_NOEXEC) == 0);
    jsoff_t prev = (jsoff_t) vdata(js->scope);
    js->scope = mkobj(js, prev);
    // printf("ENTER SCOPE %u, prev %u\n", (jsoff_t) vdata(js->scope), prev);
}

static void delscope(struct js *js) {
    js->scope = upper(js, js->scope);
    // printf("EXIT  SCOPE %u\n", (jsoff_t) vdata(js->scope));
}

static jsval_t js_block(struct js *js, bool create_scope) {
    jsval_t res = js_mkundef();
    if (create_scope) mkscope(js);  // Enter new scope
    js->consumed = 1;
    // jsoff_t pos = js->pos;
    while (next(js) != TOK_EOF && next(js) != TOK_RBRACE && !is_err(res)) {
        uint8_t t = js->tok;
        res = js_stmt(js);
        if (!is_err(res) && t != TOK_LBRACE && t != TOK_IF && t != TOK_WHILE &&
            js->tok != TOK_SEMICOLON) {
            res = js_mkerr(js, "; expected");
            break;
        }
    }
    // printf("BLOCKEND %s\n", js_str(js, res));
    if (create_scope) delscope(js);  // Exit scope
    return res;
}

// Seach for property in a single object
static jsoff_t lkp(struct js *js, jsval_t obj, const char *buf, size_t len) {
    jsoff_t off = loadoff(js, (jsoff_t) vdata(obj)) & ~3U;  // Load first prop off
    // printf("LKP: %lu %u [%.*s]\n", vdata(obj), off, (int) len, buf);
    while (off < js->brk && off != 0) {  // Iterate over props
        jsoff_t koff = loadoff(js, (jsoff_t) (off + sizeof(off)));
        jsoff_t klen = (loadoff(js, koff) >> 2) - 1;
        const char *p = (char *) &js->mem[koff + sizeof(koff)];
        // printf("  %u %u[%.*s]\n", off, (int) klen, (int) klen, p);
        if (streq(buf, len, p, klen)) return off;  // Found !
        off = loadoff(js, off) & ~3U;              // Load next prop offset
    }
    return 0;  // Not found
}

// Lookup variable in the scope chain
static jsval_t lookup(struct js *js, const char *buf, size_t len) {
    if (js->flags & F_NOEXEC) return 0;
    for (jsval_t scope = js->scope;;) {
        jsoff_t off = lkp(js, scope, buf, len);
        if (off != 0) return mkval(T_PROP, off);
        if (vdata(scope) == 0) break;
        scope =
                mkval(T_OBJ, loadoff(js, (jsoff_t) (vdata(scope) + sizeof(jsoff_t))));
    }
    //js_printPos(js);
    return js_mkerr(js, "'%.*s' not found", (int) len, buf);
}

static jsval_t resolveprop(struct js *js, jsval_t v) {
    if (vtype(v) != T_PROP) return v;
    return resolveprop(js,
                       loadval(js, (jsoff_t) (vdata(v) + sizeof(jsoff_t) * 2)));
}

static jsval_t assign(struct js *js, jsval_t lhs, jsval_t val) {
    saveval(js, (jsoff_t) ((vdata(lhs) & ~3U) + sizeof(jsoff_t) * 2), val);
    return lhs;
}

static jsval_t do_assign_op(struct js *js, uint8_t op, jsval_t l, jsval_t r) {
    uint8_t m[] = {TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV, TOK_REM, TOK_SHL,
                   TOK_SHR,  TOK_ZSHR,  TOK_AND, TOK_XOR, TOK_OR};
    jsval_t res = do_op(js, m[op - TOK_PLUS_ASSIGN], resolveprop(js, l), r);
    return assign(js, l, res);
}

static jsval_t do_string_op(struct js *js, uint8_t op, jsval_t l, jsval_t r) {
    jsoff_t n1, off1 = vstr(js, l, &n1);
    jsoff_t n2, off2 = vstr(js, r, &n2);
    if (op == TOK_PLUS) {
        jsval_t res = js_mkstr(js, NULL, n1 + n2);
        // printf("STRPLUS %u %u %u %u [%.*s] [%.*s]\n", n1, off1, n2, off2, (int)
        // n1,
        //       &js->mem[off1], (int) n2, &js->mem[off2]);
        if (vtype(res) == T_STR) {
            jsoff_t n, off = vstr(js, res, &n);
            memmove(&js->mem[off], &js->mem[off1], n1);
            memmove(&js->mem[off + n1], &js->mem[off2], n2);
        }
        return res;
    } else if (op == TOK_EQ) {
        bool eq = n1 == n2 && memcmp(&js->mem[off1], &js->mem[off2], n1) == 0;
        return mkval(T_BOOL, eq ? 1 : 0);
    } else if (op == TOK_NE) {
        bool eq = n1 == n2 && memcmp(&js->mem[off1], &js->mem[off2], n1) == 0;
        return mkval(T_BOOL, eq ? 0 : 1);
    } else {
        return js_mkerr(js, "bad str op");
    }
}

static jsval_t do_dot_op(struct js *js, jsval_t l, jsval_t r) {
    const char *ptr = (char *) &js->code[coderefoff(r)];
    if (vtype(r) != T_CODEREF) return js_mkerr(js, "ident expected");
    // Handle stringvalue.length
    if (vtype(l) == T_STR && streq(ptr, codereflen(r), "length", 6)) {
        // qemu_warn("[do_dot_op] [TOV] %d | %f", offtolen(loadoff(js, (jsoff_t) vdata(l))), offtolen(loadoff(js, (jsoff_t) vdata(l))));
        return tov2(offtolen(loadoff(js, (jsoff_t) vdata(l))));
    }
    if (vtype(l) != T_OBJ) return js_mkerr(js, "lookup in non-obj");
    jsoff_t off = lkp(js, l, ptr, codereflen(r));
    return off == 0 ? js_mkundef() : mkval(T_PROP, off);
}

static jsval_t js_call_params(struct js *js) {
    jsoff_t pos = js->pos;
    uint8_t flags = js->flags;
    js->flags |= F_NOEXEC;
    js->consumed = 1;
    for (bool comma = false; next(js) != TOK_EOF; comma = true) {
        if (!comma && next(js) == TOK_RPAREN) break;
        js_expr(js);
        if (next(js) == TOK_RPAREN) break;
        EXPECT(TOK_COMMA, js->flags = flags);
    }
    EXPECT(TOK_RPAREN, js->flags = flags);
    js->flags = flags;
    return mkcoderef(pos, js->pos - pos - js->tlen);
}

static void reverse(jsval_t *args, int nargs) {
    for (int i = 0; i < nargs / 2; i++) {
        jsval_t tmp = args[i];
        args[i] = args[nargs - i - 1], args[nargs - i - 1] = tmp;
    }
}

// Call native C function
static jsval_t call_c(struct js *js,
                      jsval_t (*fn)(struct js *, jsval_t *, int)) {
    int argc = 0;
    while (js->pos < js->clen) {
        if (next(js) == TOK_RPAREN) break;
        jsval_t arg = resolveprop(js, js_expr(js));
        if (js->brk + sizeof(arg) > js->size) return js_mkerr(js, "call oom");
        js->size -= (jsoff_t) sizeof(arg);
        memcpy(&js->mem[js->size], &arg, sizeof(arg));
        argc++;
        // printf("  arg %d -> %s\n", argc, js_str(js, arg));
        if (next(js) == TOK_COMMA) js->consumed = 1;
    }
    reverse((jsval_t *) &js->mem[js->size], argc);
    jsval_t res = fn(js, (jsval_t *) &js->mem[js->size], argc);
    setlwm(js);
    js->size += (jsoff_t) sizeof(jsval_t) * (jsoff_t) argc;  // Restore stack
    return res;
}


// Call JS function. 'fn' looks like this: "(a,b) { return a + b; }"
static jsval_t call_js(struct js *js, const char *fn, jsoff_t fnlen) {
    jsoff_t fnpos = 1;
    // printf("JSCALL [%.*s] -> %.*s\n", (int) js->clen, js->code, (int) fnlen,
    // fn);
    // printf("JSCALL, nogc %u [%.*s]\n", js->nogc, (int) fnlen, fn);
    mkscope(js);  // Create function call scope
    // Loop over arguments list "(a, b)" and set scope variables
    while (fnpos < fnlen) {
        fnpos = skiptonext(fn, fnlen, fnpos);          // Skip to the identifier
        if (fnpos < fnlen && fn[fnpos] == ')') break;  // Closing paren? break!
        jsoff_t identlen = 0;                          // Identifier length
        uint8_t tok = parseident(&fn[fnpos], fnlen - fnpos, &identlen);
        if (tok != TOK_IDENTIFIER) break;
        // Here we have argument name. Calculate arg value
        // printf("  [%.*s] -> %u [%.*s] -> ", (int) identlen, &fn[fnpos], js->pos,
        //       (int) js->clen, js->code);
        js->pos = skiptonext(js->code, js->clen, js->pos);
        js->consumed = 1;
        jsval_t v = js->code[js->pos] == ')' ? js_mkundef() : js_expr(js);
        // Set argument in the function scope
        setprop(js, js->scope, js_mkstr(js, &fn[fnpos], identlen), v);
        js->pos = skiptonext(js->code, js->clen, js->pos);
        if (js->pos < js->clen && js->code[js->pos] == ',') js->pos++;
        fnpos = skiptonext(fn, fnlen, fnpos + identlen);  // Skip past identifier
        if (fnpos < fnlen && fn[fnpos] == ',') fnpos++;   // And skip comma
    }
    if (fnpos < fnlen && fn[fnpos] == ')') fnpos++;  // Skip to the function body
    fnpos = skiptonext(fn, fnlen, fnpos);            // Up to the opening brace
    if (fnpos < fnlen && fn[fnpos] == '{') fnpos++;  // And skip the brace
    size_t n = fnlen - fnpos - 1U;  // Function code with stripped braces
    // printf("flags: %d, body: %zu [%.*s]\n", js->flags, n, (int) n, &fn[fnpos]);
    js->flags = F_CALL;                        // Mark we're in the function call
    jsval_t res = js_eval(js, &fn[fnpos], n);  // Call function, no GC
    if (!is_err(res) && !(js->flags & F_RETURN)) res = js_mkundef();  // No return
    delscope(js);  // Delete call scope
    // printf("  -> %d [%s], tok %d\n", js->flags, js_str(js, res), js->tok);
    return res;
}


jsval_t elk_call_js_fnc(struct js *js, const char *fn, jsoff_t fnlen) {
    return call_js(js,fn,fnlen);
}

static jsval_t do_call_op(struct js *js, jsval_t func, jsval_t args) {
    if (vtype(args) != T_CODEREF) return js_mkerr(js, "bad call");
    if (vtype(func) != T_FUNC && vtype(func) != T_CFUNC)
        return js_mkerr(js, "calling non-function");
    const char *code = js->code;             // Save current parser state
    jsoff_t clen = js->clen, pos = js->pos;  // code, position and code length
    js->code = &js->code[coderefoff(args)];  // Point parser to args
    js->clen = codereflen(args);             // Set args length
    js->pos = skiptonext(js->code, js->clen, 0);  // Skip to 1st arg
    uint8_t tok = js->tok, flags = js->flags;     // Save flags
    jsoff_t nogc = js->nogc;
    jsval_t res = js_mkundef();
    if (vtype(func) == T_FUNC) {
        jsoff_t fnlen, fnoff = vstr(js, func, &fnlen);
        js->nogc = (jsoff_t) (fnoff - sizeof(jsoff_t));
        res = call_js(js, (const char *) (&js->mem[fnoff]), fnlen);
    } else {
        res = call_c(js, (jsval_t(*)(struct js *, jsval_t *, int)) vdata(func));
    }
    js->code = code, js->clen = clen, js->pos = pos;  // Restore parser
    js->flags = flags, js->tok = tok, js->nogc = nogc;
    js->consumed = 1;
    return res;
}

static jsval_t do_op(struct js *js, uint8_t op, jsval_t lhs, jsval_t rhs) {
    if (js->flags & F_NOEXEC) return 0;
    jsval_t l = resolveprop(js, lhs), r = resolveprop(js, rhs);
    // printf("OP %d %d %d\n", op, vtype(lhs), vtype(r));
    setlwm(js);
    if (is_err(l)) return l;
    if (is_err(r)) return r;
    if (is_assign(op) && vtype(lhs) != T_PROP) return js_mkerr(js, "bad lhs");
    switch (op) {
        case TOK_TYPEOF:  return js_mkstr(js, typestr(vtype(r)), strlen(typestr(vtype(r))));
        case TOK_CALL:    return do_call_op(js, l, r);
        case TOK_ASSIGN:  return assign(js, lhs, r);
        case TOK_POSTINC: {
            //if (vtype(lhs) != T_OBJ && vtype(lhs) != T_CODEREF) return js_mkerr(js, "bad lhs pc");
            do_assign_op(js, TOK_PLUS_ASSIGN, lhs, tov2(1));
            return l;
        }
        case TOK_POSTDEC: {
            //if (vtype(lhs) != T_OBJ && vtype(lhs) != T_CODEREF) return js_mkerr(js, "bad lhs dc");
            do_assign_op(js, TOK_MINUS_ASSIGN, lhs, tov2(1));
            return l;
        }
        case TOK_NOT:     if (vtype(r) == T_BOOL) return mkval(T_BOOL, !vdata(r)); break;
    }
    if (is_assign(op))    return do_assign_op(js, op, lhs, r);
    if (vtype(l) == T_STR && vtype(r) == T_STR) return do_string_op(js, op, l, r);
    if (is_unary(op) && vtype(r) != T_NUM) return js_mkerr(js, "type mismatch");
    if (!is_unary(op) && op != TOK_DOT && (vtype(l) != T_NUM || vtype(r) != T_NUM)) return js_mkerr(js, "type mismatch");
    double_t a = tod2(l), b = tod2(r);
    // qemu_warn("[do_op] a: %d | %f", a, a);
    // qemu_warn("[do_op] b: %d | %f", b, b);
    switch (op) {
        //case TOK_EXP:     return tov(pow(a, b));
        case TOK_DIV:     return tod2(r) == 0 ? js_mkerr(js, "div by zero") : tov2(a / b);
        case TOK_REM:     return tov2(a - b * ((double_t) (long) (a / b)));
        case TOK_MUL:     return tov2(a * b);
        case TOK_PLUS:    return tov2(a + b);
        case TOK_MINUS:   return tov2(a - b);
        case TOK_XOR:     return tov2((double_t)((long) a ^ (long) b));
        case TOK_AND:     return tov2((double_t)((long) a & (long) b));
        case TOK_OR:      return tov2((double_t)((long) a | (long) b));
        case TOK_UMINUS:  return tov2(-b);
        case TOK_UPLUS:   return r;
        case TOK_TILDA:   return tov2((double_t)(~(long) b));
        case TOK_NOT:     return mkval(T_BOOL, b == 0);
        case TOK_SHL:     return tov2((double_t)((long) a << (long) b));
        case TOK_SHR:     return tov2((double_t)((long) a >> (long) b));
        case TOK_DOT:     return do_dot_op(js, l, r);
        case TOK_EQ:      return mkval(T_BOOL, (long) a == (long) b);
        case TOK_NE:      return mkval(T_BOOL, (long) a != (long) b);
        case TOK_LT:      return mkval(T_BOOL, a < b);
        case TOK_LE:      return mkval(T_BOOL, a <= b);
        case TOK_GT:      return mkval(T_BOOL, a > b);
        case TOK_GE:      return mkval(T_BOOL, a >= b);
        default:          return js_mkerr(js, "unknown op %d", (int) op);  // LCOV_EXCL_LINE
    }
}

static jsval_t js_str_literal(struct js *js) {
    uint8_t *in = (uint8_t *) &js->code[js->toff];
    uint8_t *out = &js->mem[js->brk + sizeof(jsoff_t)];
    size_t n1 = 0, n2 = 0;
    // printf("STR %u %lu %lu\n", js->brk, js->tlen, js->clen);
    if (js->brk + sizeof(jsoff_t) + js->tlen > js->size)
        return js_mkerr(js, "oom");
    while (n2++ + 2 < js->tlen) {
        if (in[n2] == '\\') {
            if (in[n2 + 1] == in[0]) {
                out[n1++] = in[0];
            } else if (in[n2 + 1] == 'n') {
                out[n1++] = '\n';
            } else if (in[n2 + 1] == 't') {
                out[n1++] = '\t';
            } else if (in[n2 + 1] == 'r') {
                out[n1++] = '\r';
            } else if (in[n2 + 1] == 'x' && is_xdigit(in[n2 + 2]) &&
                       is_xdigit(in[n2 + 3])) {
                out[n1++] = (uint8_t) ((unhex(in[n2 + 2]) << 4U) | unhex(in[n2 + 3]));
                n2 += 2;
            } else {
                return js_mkerr(js, "bad str literal");
            }
            n2++;
        } else {
            out[n1++] = ((uint8_t *) js->code)[js->toff + n2];
        }
    }
    return js_mkstr(js, NULL, n1);
}

static jsval_t js_obj_literal(struct js *js) {
    uint8_t exe = !(js->flags & F_NOEXEC);
    // printf("OLIT1\n");
    jsval_t obj = exe ? mkobj(js, 0) : js_mkundef();
    if (is_err(obj)) return obj;
    js->consumed = 1;
    while (next(js) != TOK_RBRACE) {
        jsval_t key = 0;
        if (js->tok == TOK_IDENTIFIER) {
            if (exe) key = js_mkstr(js, js->code + js->toff, js->tlen);
        } else if (js->tok == TOK_STRING) {
            if (exe) key = js_str_literal(js);
        } else {
            return js_mkerr(js, "parse error");
        }
        js->consumed = 1;
        EXPECT(TOK_COLON, );
        jsval_t val = js_expr(js);
        if (exe) {
            // printf("XXXX [%s] scope: %lu\n", js_str(js, val), vdata(js->scope));
            if (is_err(val)) return val;
            if (is_err(key)) return key;
            jsval_t res = setprop(js, obj, key, resolveprop(js, val));
            if (is_err(res)) return res;
        }
        if (next(js) == TOK_RBRACE) break;
        EXPECT(TOK_COMMA, );
    }
    EXPECT(TOK_RBRACE, );
    return obj;
}

static jsval_t js_func_literal(struct js *js) {
    uint8_t flags = js->flags;  // Save current flags
    js->consumed = 1;
    EXPECT(TOK_LPAREN, js->flags = flags);
    jsoff_t pos = js->pos - 1;
    for (bool comma = false; next(js) != TOK_EOF; comma = true) {
        if (!comma && next(js) == TOK_RPAREN) break;
        EXPECT(TOK_IDENTIFIER, js->flags = flags);
        if (next(js) == TOK_RPAREN) break;
        EXPECT(TOK_COMMA, js->flags = flags);
    }
    EXPECT(TOK_RPAREN, js->flags = flags);
    EXPECT(TOK_LBRACE, js->flags = flags);
    js->consumed = 0;
    js->flags |= F_NOEXEC;              // Set no-execution flag to parse the
    jsval_t res = js_block(js, false);  // Skip function body - no exec
    if (is_err(res)) {                  // But fail short on parse error
        js->flags = flags;
        return res;
    }
    js->flags = flags;  // Restore flags
    jsval_t str = js_mkstr(js, &js->code[pos], js->pos - pos);
    js->consumed = 1;
    // printf("FUNC: %u [%.*s]\n", pos, js->pos - pos, &js->code[pos]);
    return mkval(T_FUNC, (unsigned long) vdata(str));
}

#define RTL_BINOP(_f1, _f2, _cond)  \
  jsval_t res = _f1(js);            \
  while (!is_err(res) && (_cond)) { \
    uint8_t op = js->tok;           \
    js->consumed = 1;               \
    jsval_t rhs = _f2(js);          \
    if (is_err(rhs)) return rhs;    \
    res = do_op(js, op, res, rhs);  \
  }                                 \
  return res;

#define LTR_BINOP(_f, _cond)        \
  jsval_t res = _f(js);             \
  while (!is_err(res) && (_cond)) { \
    uint8_t op = js->tok;           \
    js->consumed = 1;               \
    jsval_t rhs = _f(js);           \
    if (is_err(rhs)) return rhs;    \
    res = do_op(js, op, res, rhs);  \
  }                                 \
  return res;

static jsval_t js_literal(struct js *js) {
    next(js);
    setlwm(js);
    // printf("css : %u\n", js->css);
    if (js->maxcss > 0 && js->css > js->maxcss) return js_mkerr(js, "C stack");
    js->consumed = 1;
    switch (js->tok) {
        case TOK_ERR:         return js_mkerr(js, "parse error");
        case TOK_NUMBER:      return js->tval;
        case TOK_STRING:      return js_str_literal(js);
        case TOK_LBRACE:      return js_obj_literal(js);
        case TOK_FUNC:        return js_func_literal(js);
        case TOK_NULL:        return js_mknull();
        case TOK_UNDEF:       return js_mkundef();
        case TOK_TRUE:        return js_mktrue();
        case TOK_FALSE:       return js_mkfalse();
        case TOK_IDENTIFIER:  return mkcoderef((jsoff_t) js->toff, (jsoff_t) js->tlen);
        default:              return js_mkerr(js, "bad expr");
    }
}

static jsval_t js_group(struct js *js) {
    if (next(js) == TOK_LPAREN) {
        js->consumed = 1;
        jsval_t v = js_expr(js);
        if (is_err(v)) return v;
        if (next(js) != TOK_RPAREN) return js_mkerr(js, ") expected");
        js->consumed = 1;
        return v;
    } else {
        return js_literal(js);
    }
}

static jsval_t js_call_dot(struct js *js) {
    jsval_t res = js_group(js);
    if (is_err(res)) return res;
    if (vtype(res) == T_CODEREF) {
        res = lookup(js, &js->code[coderefoff(res)], codereflen(res));
    }
    while (next(js) == TOK_LPAREN || next(js) == TOK_DOT) {
        if (js->tok == TOK_DOT) {
            js->consumed = 1;
            res = do_op(js, TOK_DOT, res, js_group(js));
        } else {
            jsval_t params = js_call_params(js);
            if (is_err(params)) return params;
            res = do_op(js, TOK_CALL, res, params);
        }
    }
    return res;
}

static jsval_t js_postfix(struct js *js) {
    jsval_t res = js_call_dot(js);
    if (is_err(res)) return res;
    next(js);
    if (js->tok == TOK_POSTINC || js->tok == TOK_POSTDEC) {
        js->consumed = 1;
        res = do_op(js, js->tok, res, 0);
    }
    return res;
}

static jsval_t js_unary(struct js *js) {
    if (next(js) == TOK_NOT || js->tok == TOK_TILDA || js->tok == TOK_TYPEOF ||
        js->tok == TOK_MINUS || js->tok == TOK_PLUS) {
        uint8_t t = js->tok;
        if (t == TOK_MINUS) t = TOK_UMINUS;
        if (t == TOK_PLUS) t = TOK_UPLUS;
        js->consumed = 1;
        return do_op(js, t, js_mkundef(), js_unary(js));
    } else {
        return js_postfix(js);
    }
}

static jsval_t js_mul_div_rem(struct js *js) {
    LTR_BINOP(js_unary,
              (next(js) == TOK_MUL || js->tok == TOK_DIV || js->tok == TOK_REM));
}

static jsval_t js_plus_minus(struct js *js) {
    LTR_BINOP(js_mul_div_rem, (next(js) == TOK_PLUS || js->tok == TOK_MINUS));
}

static jsval_t js_shifts(struct js *js) {
    LTR_BINOP(js_plus_minus, (next(js) == TOK_SHR || next(js) == TOK_SHL ||
                              next(js) == TOK_ZSHR));
}

static jsval_t js_comparison(struct js *js) {
    LTR_BINOP(js_shifts, (next(js) == TOK_LT || next(js) == TOK_LE ||
                          next(js) == TOK_GT || next(js) == TOK_GE));
}

static jsval_t js_equality(struct js *js) {
    LTR_BINOP(js_comparison, (next(js) == TOK_EQ || next(js) == TOK_NE));
}

static jsval_t js_bitwise_and(struct js *js) {
    LTR_BINOP(js_equality, (next(js) == TOK_AND));
}

static jsval_t js_bitwise_xor(struct js *js) {
    LTR_BINOP(js_bitwise_and, (next(js) == TOK_XOR));
}

static jsval_t js_bitwise_or(struct js *js) {
    LTR_BINOP(js_bitwise_xor, (next(js) == TOK_OR));
}

static jsval_t js_logical_and(struct js *js) {
    jsval_t res = js_bitwise_or(js);
    if (is_err(res)) return res;
    uint8_t flags = js->flags;
    while (next(js) == TOK_LAND) {
        js->consumed = 1;
        res = resolveprop(js, res);
        if (!js_truthy(js, res)) js->flags |= F_NOEXEC;  // false && ... shortcut
        if (js->flags & F_NOEXEC) {
            js_logical_and(js);
        } else {
            res = js_logical_and(js);
        }
    }
    js->flags = flags;
    return res;
}

static jsval_t js_logical_or(struct js *js) {
    jsval_t res = js_logical_and(js);
    if (is_err(res)) return res;
    uint8_t flags = js->flags;
    while (next(js) == TOK_LOR) {
        js->consumed = 1;
        res = resolveprop(js, res);
        if (js_truthy(js, res)) js->flags |= F_NOEXEC;  // true || ... shortcut
        if (js->flags & F_NOEXEC) {
            js_logical_or(js);
        } else {
            res = js_logical_or(js);
        }
    }
    js->flags = flags;
    return res;
}

static jsval_t js_ternary(struct js *js) {
    jsval_t res = js_logical_or(js);
    if (next(js) == TOK_Q) {
        uint8_t flags = js->flags;
        js->consumed = 1;
        if (js_truthy(js, resolveprop(js, res))) {
            res = js_ternary(js);
            js->flags |= F_NOEXEC;
            EXPECT(TOK_COLON, js->flags = flags);
            js_ternary(js);
            js->flags = flags;
        } else {
            js->flags |= F_NOEXEC;
            js_ternary(js);
            EXPECT(TOK_COLON, js->flags = flags);
            js->flags = flags;
            res = js_ternary(js);
        }
    }
    return res;
}

static jsval_t js_assignment(struct js *js) {
    RTL_BINOP(js_ternary, js_assignment,
              (next(js) == TOK_ASSIGN || js->tok == TOK_PLUS_ASSIGN ||
               js->tok == TOK_MINUS_ASSIGN || js->tok == TOK_MUL_ASSIGN ||
               js->tok == TOK_DIV_ASSIGN || js->tok == TOK_REM_ASSIGN ||
               js->tok == TOK_SHL_ASSIGN || js->tok == TOK_SHR_ASSIGN ||
               js->tok == TOK_ZSHR_ASSIGN || js->tok == TOK_AND_ASSIGN ||
               js->tok == TOK_XOR_ASSIGN || js->tok == TOK_OR_ASSIGN));
}

static jsval_t js_expr(struct js *js) {
    return js_assignment(js);
}

static jsval_t js_let(struct js *js) {
    uint8_t exe = !(js->flags & F_NOEXEC);
    js->consumed = 1;
    for (;;) {
        EXPECT(TOK_IDENTIFIER, );
        js->consumed = 0;
        jsoff_t noff = js->toff, nlen = js->tlen;
        char *name = (char *) &js->code[noff];
        jsval_t v = js_mkundef();
        js->consumed = 1;
        if (next(js) == TOK_ASSIGN) {
            js->consumed = 1;
            v = js_expr(js);
            if (is_err(v)) return v;  // Propagate error if any
        }
        if (exe) {
            if (lkp(js, js->scope, name, nlen) > 0)
                return js_mkerr(js, "'%.*s' already declared", (int) nlen, name);
            jsval_t x =
                    setprop(js, js->scope, js_mkstr(js, name, nlen), resolveprop(js, v));
            if (is_err(x)) return x;
        }
        if (next(js) == TOK_SEMICOLON || next(js) == TOK_EOF) break;  // Stop
        EXPECT(TOK_COMMA, );
    }
    return js_mkundef();
}

static jsval_t js_block_or_stmt(struct js *js) {
    if (next(js) == TOK_LBRACE) return js_block(js, !(js->flags & F_NOEXEC));
    jsval_t res = resolveprop(js, js_stmt(js));
    js->consumed = 0;  //
    return res;
}

static jsval_t js_if(struct js *js) {
    js->consumed = 1;
    EXPECT(TOK_LPAREN, );
    jsval_t res = js_mkundef(), cond = resolveprop(js, js_expr(js));
    EXPECT(TOK_RPAREN, );
    bool cond_true = js_truthy(js, cond), exe = !(js->flags & F_NOEXEC);
    // printf("IF COND: %s, true? %d\n", js_str(js, cond), cond_true);
    if (!cond_true) js->flags |= F_NOEXEC;
    jsval_t blk = js_block_or_stmt(js);
    if (cond_true) res = blk;
    if (exe && !cond_true) js->flags &= (uint8_t) ~F_NOEXEC;
    if (lookahead(js) == TOK_ELSE) {
        js->consumed = 1;
        next(js);
        js->consumed = 1;
        if (cond_true) js->flags |= F_NOEXEC;
        blk = js_block_or_stmt(js);
        if (!cond_true) res = blk;
        if (cond_true && exe) js->flags &= (uint8_t) ~F_NOEXEC;
    }
    return res;
}

static inline bool expect(struct js *js, uint8_t tok, jsval_t *res) {
    if (next(js) != tok) {
        *res = js_mkerr(js, "parse error");
        return false;
    } else {
        js->consumed = 1;
        return true;
    }
}

static inline bool is_err2(jsval_t *v, jsval_t *res) {
bool r = is_err(*v);
if (r) *res = *v;
return r;
}

static jsval_t js_for(struct js *js) {
    uint8_t flags = js->flags, exe = !(flags & F_NOEXEC);
    jsval_t v, res = js_mkundef();
    jsoff_t pos1 = 0, pos2 = 0, pos3 = 0, pos4 = 0;
    if (exe) mkscope(js);  // Enter new scope
    if (!expect(js, TOK_FOR, &res)) goto done;
    if (!expect(js, TOK_LPAREN, &res)) goto done;

    if (next(js) == TOK_SEMICOLON) {  // initialisation
    } else if (next(js) == TOK_LET) {
        v = js_let(js);
        if (is_err2(&v, &res)) goto done;
    } else {
        v = js_expr(js);
        if (is_err2(&v, &res)) goto done;
    }
    if (!expect(js, TOK_SEMICOLON, &res)) goto done;
    js->flags |= F_NOEXEC;
    pos1 = js->pos;  // condition
    if (next(js) != TOK_SEMICOLON) {
        v = js_expr(js);
        if (is_err2(&v, &res)) goto done;
    }
    if (!expect(js, TOK_SEMICOLON, &res)) goto done;
    pos2 = js->pos;  // final expr
    if (next(js) != TOK_RPAREN) {
        v = js_expr(js);
        if (is_err2(&v, &res)) goto done;
    }
    if (!expect(js, TOK_RPAREN, &res)) goto done;
    pos3 = js->pos;  // body
    v = js_block_or_stmt(js);
    if (is_err2(&v, &res)) goto done;
    pos4 = js->pos;  // end of body
    while (!(flags & F_NOEXEC)) {
        js->flags = flags, js->pos = pos1, js->consumed = 1;
        if (next(js) != TOK_SEMICOLON) {     // Is condition specified?
            v = resolveprop(js, js_expr(js));  // Yes. check condition
            if (is_err2(&v, &res)) goto done;  // Fail short on error
            if (!js_truthy(js, v)) break;      // Exit the loop if condition is false
        }
        js->pos = pos3, js->consumed = 1, js->flags |= F_LOOP;  // Execute the
        v = js_block_or_stmt(js);                               // loop body
        if (is_err2(&v, &res)) goto done;                       // Fail on error
        if (js->flags & F_BREAK) break;  // break was executed - exit the loop!
        js->flags = flags, js->pos = pos2, js->consumed = 1;  // Jump to final expr
        if (next(js) != TOK_RPAREN) {                         // Is it specified?
            v = js_expr(js);                                    // Yes. Execute it
            if (is_err2(&v, &res)) goto done;  // On error, fail short
        }
    }
    js->pos = pos4, js->tok = TOK_SEMICOLON, js->consumed = 0;
    done:
    if (exe) delscope(js);  // Exit scope
    js->flags = flags;      // Restore flags
    return res;
}

// Пример обработчика для ключевого слова var
static jsval_t js_var(struct js *js) {
    uint8_t exe = !(js->flags & F_NOEXEC);
    js->consumed = 1;
    for (;;) {
        EXPECT(TOK_IDENTIFIER, );
        js->consumed = 0;
        jsoff_t noff = js->toff, nlen = js->tlen;
        char *name = (char *) &js->code[noff];
        jsval_t v = js_mkundef();
        js->consumed = 1;
        if (next(js) == TOK_ASSIGN) {
            js->consumed = 1;
            v = js_expr(js);
            if (is_err(v)) return v;  // Propagate error if any
        }
        if (exe) {
            // Логика обработки ключевого слова var
            // Например, регистрация переменной в текущей области видимости или в глобальной области видимости
            if (lkp(js, js->scope, name, nlen) > 0) {
                return js_mkerr(js, "'%.*s' already declared", (int) nlen, name);
            }
            jsval_t x = setprop(js, js->scope, js_mkstr(js, name, nlen), resolveprop(js, v));
            if (is_err(x)) {
                return x;
            }
        }
        if (next(js) == TOK_SEMICOLON || next(js) == TOK_EOF) break;  // Stop
        EXPECT(TOK_COMMA, );
    }
    return js_mkundef();
}

// Пример обработчика для ключевого слова do
/// DISABLED: Вызывает пока что ошибку парсинга
static jsval_t js_do_while(struct js *js) {
    uint8_t exe = !(js->flags & F_NOEXEC);
    jsval_t res = js_mkundef();
    jsoff_t pos1, pos2;

    if (!expect(js, TOK_DO, &res)) {
        return res;
    }
    pos1 = js->pos;  // Начало блока
    js->consumed = 0; // Сброс флага consumed перед блоком
    res = js_block_or_stmt(js);

    if (is_err(res)) {
        return res;
    }

    if (!expect(js, TOK_WHILE, &res)) {
        return res;
    }
    if (!expect(js, TOK_LPAREN, &res)) {
        return res;
    }

    pos2 = js->pos; // Запоминаем позицию для дальнейшего возврата
    js->pos = pos1; // Возвращаемся к началу блока
    if (exe) {
        do {
            res = js_block_or_stmt(js);
            if (is_err(res)) {
                return res;
            }
            js->pos = pos2;
            res = js_expr(js);
            if (is_err(res)) {
                return res;
            }
        } while (js_truthy(js, res));
    } else {
        js->pos = pos2;  // Возвращение к позиции после do
    }
    return res;
}


static jsval_t js_while(struct js *js) {
    uint8_t flags = js->flags, exe = !(flags & F_NOEXEC);
    jsval_t v, res = js_mkundef();
    jsoff_t pos1 = 0, pos2 = 0;
    if (exe) mkscope(js);  // Входим в новую область видимости
    if (!expect(js, TOK_WHILE, &res)) goto done;
    if (!expect(js, TOK_LPAREN, &res)) goto done;

    pos1 = js->pos;  // Начало условия
    v = js_expr(js); // Вычисляем условие выражения
    if (is_err2(&v, &res)) goto done;

    if (!expect(js, TOK_RPAREN, &res)) goto done;
    pos2 = js->pos;  // Конец условия
    v = js_block_or_stmt(js); // Выполняем тело цикла
    if (is_err2(&v, &res)) goto done;

    while (!(flags & F_NOEXEC)) {
        js->flags = flags, js->pos = pos1, js->consumed = 1;
        // Проверяем условие цикла
        v = resolveprop(js, js_expr(js));
        if (is_err2(&v, &res)) goto done;
        if (!js_truthy(js, v)) break; // Если условие ложно, выходим из цикла
        js->pos = pos2, js->consumed = 1; // Переходим к концу условия
        v = js_block_or_stmt(js); // Выполняем тело цикла
        if (is_err2(&v, &res)) goto done;
    }

    done:
    if (exe) delscope(js);  // Выходим из области видимости
    js->flags = flags;  // Восстанавливаем флаги
    js->tok = TOK_SEMICOLON;
    return res;  // Возвращаем результат
}

static jsval_t js_break(struct js *js) {
    if (js->flags & F_NOEXEC) {
    } else {
        if (!(js->flags & F_LOOP)) return js_mkerr(js, "not in loop");
        js->flags |= F_BREAK | F_NOEXEC;
    }
    js->consumed = 1;
    return js_mkundef();
}

static jsval_t js_continue(struct js *js) {
    if (js->flags & F_NOEXEC) {
    } else {
        if (!(js->flags & F_LOOP)) return js_mkerr(js, "not in loop");
        js->flags |= F_NOEXEC;
    }
    js->consumed = 1;
    return js_mkundef();
}

static jsval_t js_return(struct js *js) {
    uint8_t exe = !(js->flags & F_NOEXEC);
    js->consumed = 1;
    if (exe && !(js->flags & F_CALL)) return js_mkerr(js, "not in func");
    if (next(js) == TOK_SEMICOLON) return js_mkundef();
    jsval_t res = resolveprop(js, js_expr(js));
    if (exe) {
        js->pos = js->clen;     // Shift to the end - exit the code snippet
        js->flags |= F_RETURN;  // Tell caller we've executed
    }
    return resolveprop(js, res);
}


static jsval_t js_switch(struct js *js) {
    jsval_t res, expr, match;
    uint8_t exe = !(js->flags & F_NOEXEC);

    js->consumed = 1;
    expr = resolveprop(js, js_expr(js));  // Вычисление выражения оператора switch

    // Цикл поиска совпадения case
    while (next(js) == TOK_CASE) {
        js->consumed = 1;
        match = resolveprop(js, js_expr(js));  // Вычисление выражения case
        if (js_cmp(js, expr, match) == 0) {  // Поиск совпадения
            res = js_block(js, exe);  // Выполнение блока кода для совпавшего case
            break;
        }
    }

    // Поиск и выполнение блока кода default
    if (next(js) == TOK_DEFAULT) {
        js->consumed = 1;
        res = js_block(js, exe);  // Выполнение блока кода для default, если совпадений нет
    }

    //js->tok = TOK_SEMICOLON;
    return res;
}

static jsval_t js_stmt(struct js *js) {
    jsval_t res;
    // jsoff_t pos = js->pos - js->tlen;
    if (js->brk > js->gct) js_gc(js);
    switch (next(js)) {
        case TOK_CASE: case TOK_CATCH: case TOK_CLASS: case TOK_CONST:
        case TOK_DEFAULT: case TOK_DELETE: case TOK_DO: case TOK_FINALLY:
        case TOK_IN: case TOK_INSTANCEOF: case TOK_NEW:
        case TOK_THIS: case TOK_THROW: case TOK_TRY: case TOK_VOID:
        case TOK_WITH: case TOK_YIELD:
            res = js_mkerr(js, "'%.*s' not implemented", (int) js->tlen, js->code + js->toff);
            break;
        case TOK_SWITCH:    res = js_switch(js); break;
        case TOK_WHILE:     res = js_while(js); break;
        case TOK_CONTINUE:  res = js_continue(js); break;
        case TOK_BREAK:     res = js_break(js); break;
        case TOK_LET:       res = js_let(js); break;
        case TOK_VAR:       res = js_var(js); break;
        case TOK_IF:        res = js_if(js); break;
        case TOK_LBRACE:    res = js_block(js, !(js->flags & F_NOEXEC)); break;
        case TOK_FOR:       res = js_for(js); break; // 25222 -> 27660
        case TOK_RETURN:    res = js_return(js); break;
        default:            res = resolveprop(js, js_expr(js)); break;
    }
    //printf("STMT [%.*s] -> %s, tok %d, flags %d\n", (int) (js->pos - pos), &js->code[pos], js_str(js, res), next(js), js->flags);
    if (next(js) != TOK_SEMICOLON && next(js) != TOK_EOF && next(js) != TOK_RBRACE){
        //qemu_err("[JSE] [Fatal] the character ';' is expected, but %d is received", next(js));

        return js_mkerr(js, "; expected");
    }
    js->consumed = 1;
    return res;
}

struct js *js_create(void *buf, size_t len) {
    struct js *js = NULL;
    if (len < sizeof(*js) + esize(T_OBJ)) return js;
    memset(buf, 0, len);                       // Important!
    js = (struct js *) buf;                    // struct js lives at the beginning
    js->mem = (uint8_t *) (js + 1);            // Then goes memory for JS data
    js->size = (jsoff_t) (len - sizeof(*js));  // JS memory size
    js->scope = mkobj(js, 0);                  // Create global scope
    js->size = js->size / 8U * 8U;             // Align js->size by 8 byte
    js->lwm = js->size;                        // Initial LWM: 100% free
    js->isFatal = 0;                            // Сброс состояния фатальности
    js->gct = js->size / 2;
    return js;
}

void js_setgct(struct js *js, size_t gct) { js->gct = (jsoff_t) gct; }
void js_setmaxcss(struct js *js, size_t max) { js->maxcss = (jsoff_t) max; }
jsval_t js_mktrue(void) { return mkval(T_BOOL, 1); }
jsval_t js_mkfalse(void) { return mkval(T_BOOL, 0); }
jsval_t js_mkundef(void) { return mkval(T_UNDEF, 0); }
jsval_t js_mknull(void) { return mkval(T_NULL, 0); }
jsval_t js_mknum(double_t value) {
    // warn("[js_mknum] [TOV] %d | %f", value, value);
    return tov2(value);
}
jsval_t js_mkobj(struct js *js) { return mkobj(js, 0); }
jsval_t js_mkfun(jsval_t (*fn)(struct js *, jsval_t *, int)) { return mkval(T_CFUNC, (size_t) (void *) fn); }
double_t js_getnum(jsval_t value) {
    // qemu_note("[js_getnum] [TOD] %d | %f", tod2(value),tod2(value));
    return tod2(value);
}
int js_getbool(jsval_t value) { return vdata(value) & 1 ? 1 : 0; }

jsval_t js_glob(struct js *js) { (void) js; return mkval(T_OBJ, 0); }

void js_set(struct js *js, jsval_t obj, const char *key, jsval_t val) {
    if (vtype(obj) == T_OBJ) setprop(js, obj, js_mkstr(js, key, strlen(key)), val);
}

char *js_getstr(struct js *js, jsval_t value, size_t *len) {
    if (vtype(value) != T_STR) return NULL;
    jsoff_t n, off = vstr(js, value, &n);
    if (len != NULL) *len = n;
    return (char *) &js->mem[off];
}

int js_type(jsval_t val) {
    switch (vtype(val)) {
        case T_UNDEF:   return JS_UNDEF;
        case T_NULL:    return JS_NULL;
        case T_BOOL:    return vdata(val) == 0 ? JS_FALSE: JS_TRUE;
        case T_STR:     return JS_STR;
        case T_NUM:     return JS_NUM;
        case T_ERR:     return JS_ERR;
        default:        return JS_PRIV;
    }
}
void js_stats(struct js *js, size_t *total, size_t *lwm, size_t *css) {
    if (total) *total = js->size;
    if (lwm) *lwm = js->lwm;
    if (css) *css = js->css;
}

void js_statsInfo(struct js *js) {
    qemu_ok("\n[JSE] Information: \n\      Total RAM: %d b. \n      Low free ram: %d b. \n      Maximum stack: %d b.",js->size,js->lwm, js->css);
}

bool js_chkargs(jsval_t *args, int nargs, const char *spec) {
    int i = 0, ok = 1;
    for (; ok && i < nargs && spec[i]; i++) {
        uint8_t t = vtype(args[i]), c = (uint8_t) spec[i];
        ok = (c == 'b' && t == T_BOOL) || (c == 'd' && t == T_NUM) ||
             (c == 's' && t == T_STR) || (c == 'j');
    }
    if (spec[i] != '\0' || i != nargs) ok = 0;
    return ok;
}

jsval_t js_eval(struct js *js, const char *buf, size_t len) {
    // printf("EVAL: [%.*s]\n", (int) len, buf);
    jsval_t res = js_mkundef();
    if (len == (size_t) ~0U) len = strlen(buf);
    js->consumed = 1;
    js->tok = TOK_ERR;
    js->code = buf;
    js->clen = (jsoff_t) len;
    js->pos = 0;
    js->cstk = &res;
    while (next(js) != TOK_EOF && !is_err(res)) {
        res = js_stmt(js);
    }
    return res;
}

void js_dump(struct js *js) {
  jsoff_t off = 0, v;
  printf("JS size %u, brk %u, lwm %u, css %u, nogc %u\n", js->size, js->brk,
         js->lwm, (unsigned) js->css, js->nogc);
  while (off < js->brk) {
    memcpy(&v, &js->mem[off], sizeof(v));
    printf(" %5u: ", off);
    if ((v & 3U) == T_OBJ) {
      printf("OBJ %u %u\n", v & ~3U,
             loadoff(js, (jsoff_t) (off + sizeof(off))));
    } else if ((v & 3U) == T_PROP) {
      jsoff_t koff = loadoff(js, (jsoff_t) (off + sizeof(v)));
      jsval_t val = loadval(js, (jsoff_t) (off + sizeof(v) + sizeof(v)));
      printf("PROP next %u, koff %u vtype %d vdata %lu\n", v & ~3U, koff,
             vtype(val), (unsigned long) vdata(val));
    } else if ((v & 3) == T_STR) {
      jsoff_t len = offtolen(v);
      printf("STR %u [%.*s]\n", len, (int) len, js->mem + off + sizeof(v));
    } else {
      printf("???\n");
      break;
    }
    off += esize(v);
  }
}
