#include "kernel.h"
#include "eBat.h"
#include "eBatRuntime.h"


BAT_T* bat_create_session(){
    BAT_T* bat = calloc(1, sizeof *bat);
    if(!bat) {
        return 0;
    }
    bat->Group = calloc(1, sizeof(size_t));
    if(!bat->Group) {
        free(bat);
        return 0;
    }
    bat->GoTo = calloc(1, sizeof(size_t));
    if(!bat->GoTo) {
        free(bat);
        return 0;
    }
    bat->Size = 0;
    bat->Capacity = 1;
    bat->Size_GT = 0;
    bat->Capacity_GT = 1;
    bat->CurGoTo = NULL;
    return bat;
}

BAT_GROUP_T* bat_create_group(){
    BAT_GROUP_T* group = calloc(1, sizeof *group);
    if(!group) {
        return 0;
    }
    group->Tokens = calloc(1, sizeof(size_t));
    if(!group->Tokens) {
        free(group);

        return 0;
    }
    group->Size = 0;
    group->Capacity = 1;
    return group;
}

// Создание хранилища меток GoTo
BAT_GoTo_T* goto_create(const char* identifier) {
    BAT_GoTo_T* goto_store = calloc(1, sizeof *goto_store);
    if (!goto_store) {
        return NULL;
    }
    goto_store->Identifier = bat_strdup(identifier); // Копируем строку идентификатора
    if (!goto_store->Identifier) {
        free(goto_store);
        return NULL;
    }
    goto_store->Groups = calloc(1, sizeof(BAT_GROUP_T*));
    if (!goto_store->Groups) {
        free(goto_store->Identifier);
        free(goto_store);
        return NULL;
    }
    goto_store->Size = 0;
    goto_store->Capacity = 1;
    return goto_store;
}

// Добавление группы в хранилище GoTo
void goto_add_group(BAT_GoTo_T* goto_store, BAT_GROUP_T* group) {
    if (goto_store->Size >= goto_store->Capacity) {
        size_t new_cap = goto_store->Capacity + ((goto_store->Capacity + 1) / 2);
        void* new_buf = realloc(goto_store->Groups, sizeof(BAT_GROUP_T*) * new_cap);

        if (!new_buf)
            return;

        goto_store->Groups = (BAT_GROUP_T**)new_buf;
        goto_store->Capacity = new_cap;
    }
    goto_store->Groups[goto_store->Size++] = group;
}

void bat_add_group(BAT_T * bat, size_t element) {
    if(bat->Size >= bat->Capacity) {
        size_t new_cap = bat->Capacity + ((bat->Capacity + 1) / 2);
        void* new_buf = realloc(bat->Group, sizeof(size_t) * new_cap);

        if(!new_buf)
            return;

        bat->Group = new_buf;
        bat->Capacity = new_cap;
    }
    bat->Group[bat->Size++] = element;
}

void bat_add_goto(BAT_T* bat, size_t element){
    if(bat->Size_GT >= bat->Capacity_GT) {
        size_t new_cap = bat->Capacity_GT + ((bat->Capacity_GT + 1) / 2);
        void* new_buf = realloc(bat->GoTo, sizeof(size_t) * new_cap);

        if(!new_buf)
            return;

        bat->GoTo = new_buf;
        bat->Capacity_GT = new_cap;
    }
    bat->GoTo[bat->Size_GT++] = element;
}

void bat_add_token(BAT_GROUP_T* bat, size_t element) {
    if(bat->Size >= bat->Capacity) {
        size_t new_cap = bat->Capacity + ((bat->Capacity + 1) / 2);
        void* new_buf = realloc(bat->Tokens, sizeof(size_t) * new_cap);

        if(!new_buf)
            return;

        bat->Tokens = new_buf;
        bat->Capacity = new_cap;
    }
    bat->Tokens[bat->Size++] = element;
}
BAT_TOKEN_T* bat_create_token(BAT_TOKEN_TYPE type, char* value) {
    BAT_TOKEN_T* token = calloc(1, sizeof(BAT_TOKEN_T));
    if (token == 0){
        return NULL;
    }
    memset(token, 0, sizeof(BAT_TOKEN_T));
    token->type = type;

    int len = strlen(value);

    token->value = malloc(len + 1);
    memset(token->value, 0, len + 1);
    memcpy(token->value, value, len);

    //bat_fatalerror(0, "Get: '%s'\nIns: '%s'\n",token->value,value);
    return token;
}
char* bat_debug_type(BAT_TOKEN_TYPE Type){
    switch (Type) {
        case BAT_TOKEN_TYPE_GOTO: return "GOTO";
        case BAT_TOKEN_TYPE_ECHO: return "ECHO";
        case BAT_TOKEN_TYPE_IF: return "IF";
        case BAT_TOKEN_TYPE_SET: return "SET";
        case BAT_TOKEN_TYPE_TRUE: return "TRUE";
        case BAT_TOKEN_TYPE_FALSE: return "FALSE";
        case BAT_TOKEN_TYPE_NOT: return "NOT";
        case BAT_TOKEN_TYPE_EXIST: return "EXIST";
        case BAT_TOKEN_TYPE_ISSET: return "ISSET";
        case BAT_TOKEN_TYPE_STRING: return "STRING";
        case BAT_TOKEN_TYPE_NUMBER: return "NUMBER";
        case BAT_TOKEN_TYPE_OPERATOR: return "OPERATOR";
        case BAT_TOKEN_TYPE_EQUAL: return "EQUAL";
        case BAT_TOKEN_TYPE_NOT_EQUAL: return "NOT_EQUAL";
        case BAT_TOKEN_TYPE_GREATER: return "GREATER";
        case BAT_TOKEN_TYPE_LESS: return "LESS";
        case BAT_TOKEN_TYPE_LESS_EQUAL: return "LESS_EQUAL";
        case BAT_TOKEN_TYPE_GREATER_EQUAL: return "GREATER_EQUAL";
        case BAT_TOKEN_TYPE_EXIT: return "EXIT";
        case BAT_TOKEN_TYPE_VARIABLE: return "VARIABLE";
        case BAT_TOKEN_TYPE_ALIAS: return "ALIAS";
        case BAT_TOKEN_TYPE_START: return "START";
        case BAT_TOKEN_TYPE_FOR: return "FOR";
        case BAT_TOKEN_TYPE_WHILE: return "WHILE";
        case BAT_TOKEN_TYPE_DO: return "DO";
        case BAT_TOKEN_TYPE_IN: return "IN";
        case BAT_TOKEN_TYPE_STEP: return "STEP";
        case BAT_TOKEN_TYPE_BREAK: return "BREAK";
        case BAT_TOKEN_TYPE_DEBUG: return "DEBUG";
        case BAT_TOKEN_TYPE_COMMENT: return "COMMENT";
        case BAT_TOKEN_TYPE_CONTINUE: return "CONTINUE";
        default: return "UNKNOWN";
    }
}
// Функция для определения типа лексемы
BAT_TOKEN_TYPE bat_parse_token(char* str) {
    bat_trim(str);
    str = bat_toLower(str);
    bat_str_debug(str);

    if (strcmp(str, "echo") == 0) return BAT_TOKEN_TYPE_ECHO;
    if (strcmp(str, "if") == 0) return BAT_TOKEN_TYPE_IF;
    if (strcmp(str, "set") == 0) return BAT_TOKEN_TYPE_SET;
    if (strcmp(str, "=") == 0) return BAT_TOKEN_TYPE_SET;
    if (strcmp(str, "goto") == 0) return BAT_TOKEN_TYPE_GOTO;
    if (strcmp(str, "isset") == 0) return BAT_TOKEN_TYPE_ISSET;
    if (strcmp(str, "debug") == 0) return BAT_TOKEN_TYPE_DEBUG;
    if (strcmp(str, "null") == 0) return BAT_TOKEN_TYPE_NOT;

    if (strcmp(str, "rem") == 0) return BAT_TOKEN_TYPE_COMMENT;
    if (strcmp(str, "::") == 0)  return BAT_TOKEN_TYPE_COMMENT;

    if (strcmp(str, "on") == 0 || strcmp(str, "true") == 0 || strcmp(str, "enabled") == 0) return BAT_TOKEN_TYPE_TRUE;
    if (strcmp(str, "off") == 0 || strcmp(str, "false") == 0 || strcmp(str, "disabled") == 0) return BAT_TOKEN_TYPE_FALSE;


    if (strcmp(str, "not") == 0) return BAT_TOKEN_TYPE_NOT;
    if (strcmp(str, "exist") == 0) return BAT_TOKEN_TYPE_EXIST;
    if (strcmp(str, "exit") == 0) return BAT_TOKEN_TYPE_EXIT;


    if (strcmp(str, "start") == 0) return BAT_TOKEN_TYPE_START;
    if (strcmp(str, "alias") == 0) return BAT_TOKEN_TYPE_ALIAS;

    if (strcmp(str, "==") == 0) return BAT_TOKEN_TYPE_EQUAL;
    if (strcmp(str, "!=") == 0) return BAT_TOKEN_TYPE_NOT_EQUAL;
    if (strcmp(str, ">")  == 0) return BAT_TOKEN_TYPE_GREATER;
    if (strcmp(str, "<")  == 0) return BAT_TOKEN_TYPE_LESS;
    if (strcmp(str, "<=") == 0) return BAT_TOKEN_TYPE_LESS_EQUAL;
    if (strcmp(str, ">=") == 0) return BAT_TOKEN_TYPE_GREATER_EQUAL;

    if (strcmp(str, "equ") == 0) return BAT_TOKEN_TYPE_EQUAL;
    if (strcmp(str, "neq") == 0) return BAT_TOKEN_TYPE_NOT_EQUAL;
    if (strcmp(str, "gtr")  == 0) return BAT_TOKEN_TYPE_GREATER;
    if (strcmp(str, "lss")  == 0) return BAT_TOKEN_TYPE_LESS;
    if (strcmp(str, "leq") == 0) return BAT_TOKEN_TYPE_LESS_EQUAL;
    if (strcmp(str, "geq") == 0) return BAT_TOKEN_TYPE_GREATER_EQUAL;

    if (strcmp(str, "pause") == 0) return BAT_TOKEN_TYPE_PAUSE;

    if (strcmp(str, "for") == 0) return BAT_TOKEN_TYPE_FOR;
    if (strcmp(str, "while") == 0) return BAT_TOKEN_TYPE_WHILE;
    if (strcmp(str, "do") == 0) return BAT_TOKEN_TYPE_DO;
    if (strcmp(str, "in") == 0) return BAT_TOKEN_TYPE_IN;
    if (strcmp(str, "step") == 0) return BAT_TOKEN_TYPE_STEP;
    if (strcmp(str, "break") == 0) return BAT_TOKEN_TYPE_BREAK;
    if (strcmp(str, "continue") == 0) return BAT_TOKEN_TYPE_CONTINUE;
    if (strcmp(str, "run") == 0) return BAT_TOKEN_TYPE_RUN;

    if (isdigit(str[0])) {
        return BAT_TOKEN_TYPE_NUMBER;
    } else {
        return BAT_TOKEN_TYPE_UNKNOWN;
    }
}
BAT_GROUP_T* bat_parse_line(BAT_T* bat, char* Line){
    BAT_GROUP_T* group = bat_create_group();

    int c = str_cdsp2(Line, 0x20);
    char** exp = explode(Line, 0x20);

    int curline = 1;

    int inString = 0;

    char* currentString = NULL;
    for (int u = 0; u <= c; u++){
        curline++;
        bat_debug("    |--- [%u] %s\n", u, exp[u]);
        bat_trim(exp[u]);
        size_t len = strlen(exp[u]);
        if (exp[u][0] == 0x3a && exp[u][1] != 0x3a){
            currentString = malloc(len * sizeof(char) + 1 );
            if (currentString == NULL){
                bat_debug("MALLOC ERROR\n");
                return group;
            }
            memset(currentString, 0, len * sizeof(char) + 1);
            memcpy(currentString, exp[u] + 1 , len - 1);

            currentString[len - 1] = '\0';

            //BAT_TOKEN_T* xtok = bat_create_token(BAT_TOKEN_TYPE_GOTO_INIT, currentString);



            //////////////////////////////
            bat_debug("Create goto: %s\n", currentString);

            BAT_GoTo_T* gt = goto_create(currentString);
            if (gt == NULL){
                bat_debug("ERROR CREATE GOTO\n");
                free(currentString);
                return group;
            }
            bat_debug("Line: %d\n", curline);
            gt->Line = curline;
            bat->CurGoTo = gt;
            bat_add_goto(bat, (size_t) gt);
            //BAT_GOTO_T * gt = bat_create_goto(currentString);
            //bat_add_goto(bat, (size_t) gt);
            //bat->CurGoTo = bat->Size_GoTo;
            //bat_debug("   CurGoTo: %d | SizeGoTo:%d\n", bat->CurGoTo, bat->Size_GoTo);
            free(currentString);
            return group;
        } else if (exp[u][0] == '"' && !inString && exp[u][len - 1] == '"'){
            currentString = malloc(len * sizeof(char) + 1 );
            memset(currentString, 0, len * sizeof(char) + 1);
            memcpy(currentString, exp[u] + 1 , len - 2);

            currentString[len - 1] = '\0';
            //inString = 0;
            BAT_TOKEN_T* xtok = bat_create_token(BAT_TOKEN_TYPE_STRING, currentString);
            bat_debug("create token string method 1: '%s'\n", currentString);
            bat_add_token(group, (size_t) xtok);
            free(currentString);
        } else if (exp[u][0] == '"' && !inString) {
            inString = 1;
            currentString = malloc(len * sizeof(char));
            memset(currentString, 0, len * sizeof(char));
            strcpy(currentString, exp[u] + 1);
        } else if (inString) {
            if (exp[u][len - 1] == '"') {
                currentString = realloc(currentString, (strlen(currentString) + len + 2) * sizeof(char));
                strcat(currentString, " ");
                strcat(currentString, exp[u]);
                currentString[strlen(currentString) - 1] = '\0';
                inString = 0;
                BAT_TOKEN_T* xtok = bat_create_token(BAT_TOKEN_TYPE_STRING, currentString);

                bat_debug("create token string method 2: '%s'\n", currentString);
                bat_add_token(group, (size_t) xtok);
                free(currentString);
            } else {
                currentString = realloc(currentString, (strlen(currentString) + len + 2) * sizeof(char));
                strcat(currentString, " ");
                strcat(currentString, exp[u]);
            }
        } else {
            bat_str_debug(exp[u]);

            if (exp[u][0] == '%' && exp[u][len - 1] == '%'){
                char* temp = malloc(len);
                memset(temp, 0, len);
                memcpy(temp, exp[u] + 1, len - 2);
                bat_debug("create token variable method 1: '%s'\n", temp);
                BAT_TOKEN_T* xtok = bat_create_token(BAT_TOKEN_TYPE_VARIABLE, temp);
                bat_add_token(group, (size_t) xtok);
            } else {
                BAT_TOKEN_TYPE type = bat_parse_token(exp[u]);
                bat_debug("[AUTO_DETECT TYPE] Type: %s | Str: '%s'\n", bat_debug_type(type), exp[u]);
                if (type == BAT_TOKEN_TYPE_COMMENT){
                    return group;
                }
                BAT_TOKEN_T* xtok = bat_create_token(type, exp[u]);
                bat_add_token(group, (size_t) xtok);

            }

        }


    }
    if (inString) {
        currentString = realloc(currentString, (strlen(currentString) + 1) * sizeof(char));
        currentString[strlen(currentString) - 2] = '\0';
        inString = 0;

        bat_debug("create token string method 3: '%s'\n", currentString);
        BAT_TOKEN_T* xtok = bat_create_token(BAT_TOKEN_TYPE_STRING, currentString);
        bat_add_token(group, (size_t) xtok);

        free(currentString);
    }
    for (int u = 0; u <= c; u++){
        free(exp[u]);
    }
    free(exp);

    return group;
}
BAT_T* bat_parse_string(char* String){
    BAT_T* bat = bat_create_session();
    bat_debug("\n========================\n");
    bat_debug("%s", String);
    bat_debug("\n========================\n");

    int cLine = str_cdsp2(String, '\n');
    char** Line = explode(String, '\n');
    for (int uL = 0; uL <= cLine; uL++){
        if (Line[uL] == NULL){
            continue;
        }
        int len = strlen(Line[uL]);
        bat_debug("[LINE %d | %d] [len: %d] %s\n", uL +1, cLine +1, len, Line[uL]);
        bat_debug("[0x%x] '%c'\n", Line[uL][0], Line[uL][0]);
        if (len == 0 || (len == 1 && Line[uL][0] == 0xd)){
            bat_debug("!RESET TO CurGoTo!\n");
            bat->CurGoTo = NULL;
            continue;
        }
        BAT_GROUP_T* group = bat_parse_line(bat, Line[uL]);
        if (group == NULL){
            bat_debug("!group is NULL! SKIP\n");
        } else if (bat->CurGoTo != NULL){
            bat_debug("GOTO group (%s) ins\n", bat->CurGoTo->Identifier);
            goto_add_group(bat->CurGoTo, group);
        } else {
            bat_debug("Classic group ins\n");
            bat_add_group(bat, (size_t) group);
        }
        free(Line[uL]);
    }
    free(Line);
    return bat;
}

void bat_destroy_token(BAT_TOKEN_T** token, int Size){
    qemu_log("[BAT] [Destroy] [Tokens] Size: %d");
    for (int x = 0; x < Size; x++){
        BAT_TOKEN_T* tok = token[x];
        qemu_log("    |--- [%d | %d] TYPE: %s | Value: '%s'",x + 1, Size, bat_debug_type(tok->type), tok->value);
        free(tok->value);
        free(tok);
    }
    free(token);
}
/*
 * @todo FIX ME
 */
void bat_destroy_group(BAT_GROUP_T** group, int Size){
    qemu_log("[BAT] [Destroy] [Group] Size: %d", Size);
    for (int x = 0; x < Size; x++){
        BAT_GROUP_T* gz = group[x];
        qemu_log(" |--- [%d | %d]",x + 1, Size);
        //bat_destroy_token((BAT_TOKEN_T**) gz->Tokens, gz->Size);
        free(gz);
    }
    free(group);
}

void bat_destroy(BAT_T* bat){
    qemu_log("[BAT] [Destroy]");
    qemu_log("  |--- Goto (%d)", bat->Size_GT);
    for (int a = 0; a < bat->Size_GT; a++){
        BAT_GoTo_T* gt = (BAT_GoTo_T*) bat->GoTo[a];
        qemu_log("    |--- ID: %s", gt->Identifier);
        if (gt->Identifier != NULL){
            free(gt->Identifier);
        }
        bat_destroy_group(gt->Groups, gt->Size);
        free(gt);
    }
    free(bat->GoTo);
    bat_destroy_group((BAT_GROUP_T**) bat->Group, bat->Size);
    free(bat);
}
/**
int main(int argc, char *argv[]) {
    char* file = "examples/goto-if.bat";
    char* batFile = readFile((argc > 1?argv[1]:file));
    printf("\nFile: %s\n", (argc > 1?argv[1]:file));

    if (argc <= 1){
        printf("Embedded Batch File Handler\n * Author: Nikita Piminoff\n * Version: %s\n * Example: ./eBat mybat.bat\n", BAT_VERSION);
        return 0;
    }

    BAT_T* token = bat_parse_string(batFile);
    token->Debug = 0;
    token->Echo = 1;
    bat_debug("========================\n");

    int ret = bat_runtime_exec(token);
    bat_debug("BAT LEX INFO\n");
    bat_debug("Echo: %d\n", token->Echo);
    bat_debug("Debug: %d\n", token->Debug);
    bat_debug("Error: %d\n", token->ErrorCode);
    bat_debug("Count: %d\n", token->Size);
    bat_debug("\n========================\n");
    bat_debug("RETURN CODE: %d\n",ret);

    bat_destroy(token);
    return ret;
}
*/
