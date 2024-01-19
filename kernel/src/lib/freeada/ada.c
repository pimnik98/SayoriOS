/**
 * @file lib/freeada/ada.c
 * @author Даниил Лебедев (github.com/holu31)
 * @brief Мини-порт Ada в Sayori
 * @version 0.0.1
 * @date 2023-12-16
 * @copyright Copyright SayoriOS Team (c) 2022-2024 
*/

// TODO: rewrite to codegen to own VM

#include <lib/string.h>
#include <lib/sprintf.h>
#include <lib/stdio.h>
#include <io/ports.h>

#include "portability.h"
#include "ada.h"

//#define DEBUG

#define TOKEN(ttype, tvalue) ada->token.type = ttype; \
                            ada->token.value = tvalue; \
                            ada->token.start_pos = ada->old_pos; \
                            ada->token.end_pos = ada->pos; \
                            ada->token.line = ada->line;

#define CHECK(ttype) if(ada->token.type != ttype) { ada_err_curtoken(ada, "syntax error, you put"); return 2; }
#define CHECK_NEXT(ttype) ada_next(ada); CHECK(ttype);

ada_string_t *ada_string_new() {
    ada_string_t *str = kmalloc(sizeof(ada_string_t));
    str->size = 0;
    str->data = (char*) kmalloc(sizeof(char) * (str->size+1));

    return str;
}

ada_string_t *ada_string_from(char *s) {
    ada_string_t *str = kmalloc(sizeof(ada_string_t));
    str->size = strlen(s);
    str->data = (char*) kmalloc(sizeof(char) * (str->size+1));
    
    strcpy(str->data, s);
    str->data[str->size] = '\0';
    
    return str;
}

void ada_string_push(ada_string_t *str, char c) {
    str->size++;
    str->data = (char*) krealloc(str->data, sizeof(char) * (str->size+1));
    str->data[str->size-1] = c;
    str->data[str->size] = '\0';
}

void ada_string_free(ada_string_t *str) {
    free(str->data);
    free(str);
}

void ada_lnumber(ada_t *ada) {
    ada_string_t *buffer = ada_string_new();
    char c = ada->data[ada->pos];
    ada_string_push(buffer, c);
    ada->pos++;

    while(ada->pos < ada->size) {
        c = ada->data[ada->pos];

        if(c >= '0' && c <= '9') {
            ada_string_push(buffer, c);
            ada->pos++;
        } else break;
    }
    #ifdef DEBUG
        qemu_note("%s number token = %s", ADA_LPREF, buffer->data);
    #endif
    TOKEN(T_NUMBER, buffer->data);
    ada_string_free(buffer);

}

void ada_lident(ada_t *ada) {
    ada_string_t *buffer = ada_string_new();

    while(ada->pos < ada->size) {
        char c = ada->data[ada->pos];

        if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || 
            (c == '_') || (c >= '0' && c <= '9')) {
                ada_string_push(buffer, c);
                ada->pos++;
        } else break;
    }
    #ifdef DEBUG
        qemu_note("%s ident token = '%s'", ADA_LPREF, buffer->data);
    #endif
    TOKEN(T_IDENT, buffer->data);
    ada_string_free(buffer);
}

void ada_lstring(ada_t *ada) {
    ada_string_t *buffer = ada_string_new();
    ada->pos++;

    while(ada->pos < ada->size) {
        char c = ada->data[ada->pos];

        if (c == '"') {
            ada->pos++;
            break;
        }

        ada_string_push(buffer, c);
        ada->pos++;
    }
    #ifdef DEBUG
        qemu_note("%s string token = '%s'", ADA_LPREF, buffer->data);
    #endif
    TOKEN(T_STRING, buffer->data);
    ada_string_free(buffer);
}

void ada_lchar(ada_t *ada, tokenType type) {
    TOKEN(type, &ada->data[ada->pos]);
    #ifdef DEBUG
        qemu_note("%s char token = '%c'", ADA_LPREF, ada->data[ada->pos]);
    #endif
    ada->pos++;
}

void ada_next(ada_t *ada) {
    while(ada->pos < ada->size) {
        ada->old_pos = ada->pos;
        char c = ada->data[ada->pos];
        
        if((c >= '0' && c <= '9') || (c == '-')){
            return ada_lnumber(ada);
        }
        else if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
            return ada_lident(ada);
        }
        else if(c == '"') {
            return ada_lstring(ada);
        }
        else if(c == '(') return ada_lchar(ada, T_LPAREN);
        else if(c == ')') return ada_lchar(ada, T_RPAREN);
        else if(c == '+') return ada_lchar(ada, T_PLUS);
        else if(c == '-') return ada_lchar(ada, T_MINUS);
        else if(c == '.') return ada_lchar(ada, T_DOT);
        else if(c == ';') return ada_lchar(ada, T_SEMICOLON);
        else ada->pos++;
    }

    TOKEN(T_NONE, "\0");
}

void ada_err_curtoken(ada_t *ada, char *err) {
    qemu_err("%s [%d:%d] %s '%s'", ADA_LPREF,
            ada->token.line, ada->token.start_pos, err, ada->token.value);
}

void ada_p_eval(ada_t *ada, char *left){
    ada_next(ada);
    ada_string_t *right;
    int result;
    switch(ada->token.type) {
        case T_PLUS:
            ada_next(ada);
            right = ada_string_from(ada->token.value);

            result = atoi(left) + atoi(right->data);
            ada_string_free(right);
            itoa(result, left);
            ada_p_eval(ada, left);

            break;
        case T_NUMBER:
            right = ada_string_from(ada->token.value);
            result = atoi(left) + atoi(right->data);
            ada_string_free(right);
            itoa(result, left);
            ada_p_eval(ada, left);
        default: ada->pos--;
    }
}

void *ada_p_expr(ada_t *ada) {
    ada_string_t *t = ada_string_from(ada->token.value);
    if(ada->token.type == T_STRING) {
        return (void*) t->data;
    } else if(ada->token.type == T_IDENT) {
        if(strcmp(t->data,"PI") == 0) {
            ada_p_eval(ada, "3.14");
            return (void*) "3.14";
        }
        else {
            return (void*) t->data;
        }
    } else if(ada->token.type == T_NUMBER) {
        ada_p_eval(ada, t->data);
        return (void*) t->data;
    }
    return (void*) 0;
}

int ada_p_function(ada_t *ada) {
    char *function_name;      // <------------------------------------------------\
    strcpy(function_name, ada->token.value);  // Тоесть блять это пиздец? --------/
    if(ada->token.type == T_IDENT) {
        CHECK_NEXT(T_LPAREN);
        ada_next(ada);
        
        void *expr = ada_p_expr(ada);
        if(expr != (void*) 0){
            for (int i = 0; i < ada->functions_cnt; i++){
                // Без этого почему-то не получается
                // ладно, потом разберусь
                qemu_log("Called function '%s'", ada->functions[i].name->data);
                if(strcmp(function_name, ada->functions[i].name->data) == 0) {
                    ada->functions[i].c_ptr(expr);
                }
            }
        }
        
        CHECK_NEXT(T_RPAREN);
        return 1;
    }
    return 0;
}

int ada_p_line(ada_t *ada) {
    int result = ada_p_function(ada);
    if(result == 2) {
        return 2;
    }
    if(result == 0) {
        qemu_err("%s [%d:%d] Syntax error '%s'", ADA_LPREF,
            ada->token.line, ada->token.start_pos, ada->token.value);
        return 2;
    }
    CHECK_NEXT(T_SEMICOLON);
    return 1;
}

void ada_parse(ada_t *ada) {
    ada_next(ada);
    while (ada->token.type != T_NONE) {
        if(ada_p_line(ada) == 2)
            break;
        ada_next(ada);
    }
}

void ada_handler_fn(ada_t *ada, char *fn_name, ada_fn_t handler) {
    ada->functions_cnt++;
    ada->functions = krealloc(ada->functions, sizeof(ada_function_t) * (ada->functions_cnt));
    ada->functions[ada->functions_cnt-1].name = ada_string_from(fn_name);
    ada->functions[ada->functions_cnt-1].c_ptr = handler;
}

void ada_free_functions(ada_t *ada) {
    for(int i = 0; i < ada->functions_cnt; i++) {
        ada_string_free(ada->functions[i].name);
    }
    kfree(ada->functions);
    ada->functions_cnt = 0;
}

void ada_file(char *filename) {
    FILE *f = fopen(filename, "r");
    if(!f) {
        qemu_err("%s Failed to open file '%s'", ADA_LPREF, filename);
        return;
    }
    int size = fsize(f);

    char *fdata = (char*) kcalloc(sizeof(char) * (size+1), 1);
    fread(f, 1, size, fdata);
    fclose(f);

    qemu_note("%s data: %s, size: %d", ADA_LPREF, fdata, size);

    ada_new(fdata);
}

ada_fn_t *ada_putline(void* expr) {
    qemu_log("%s result of print: %s", ADA_LPREF, (char*) expr);

    return 0;
}

void ada_new(char *data) {
    ada_t *ada = (void*) 0;
    ada->data = data;
    ada->size = strlen(data);
    ada->pos = 0;
    ada->old_pos = 0;
    ada->line = 1;
    ada->functions_cnt = 0;
    ada->functions = kmalloc(sizeof(ada_function_t) * (ada->functions_cnt));

    ada_handler_fn(ada, "Put_Line", (ada_fn_t*) ada_putline);

    ada_parse(ada);

    ada_free_functions(ada);
    kfree(ada->data);
}