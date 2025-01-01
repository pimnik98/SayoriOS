#include "kernel.h"
#include "eBat.h"
#include "eBatRuntime.h"

BAT_GoTo_T* bat_runtime_find_goto(BAT_T* bat, char* key){
    bat_debug("[Find GoTo] Size: %d\n", bat->Size_GT);
    for (int i = 0; i < bat->Size_GT; i++){
        BAT_GoTo_T* gt = (BAT_GoTo_T*) bat->GoTo[i];
        bat_debug(" |--- '%s' == '%s'\n", gt->Identifier, key);
        if (strcmp(gt->Identifier, key) == 0){
            bat_debug("   |--- TRUE\n");
            return gt;
        }
    }
    bat_debug(" |--- NULL RETURN\n");
    return NULL;
}

void bat_runtime_echo(BAT_T* bat,BAT_GROUP_T* group){
     if (bat->Echo == 1 && group->Size > 0){
        printf("%s ", EBAT_CONFIG_HELLO_LINE);
        for (int i = 0; i < group->Size; i++){
            BAT_TOKEN_T *Line = (BAT_TOKEN_T *) group->Tokens[i];
            if (Line->type == BAT_TOKEN_TYPE_VARIABLE) {
                printf("%c%s%c ", '%', Line->value,'%');
            } else if (Line->type == BAT_TOKEN_TYPE_STRING) {
                printf("\"%s\" ", Line->value);
            } else {
                printf("%s ", Line->value);
            }

        }
        printf("\n");
    }
    #ifdef DEBUG
    if (bat->Debug == 1){
        for (int y = 0; y < group->Size; y++){
            BAT_TOKEN_T* xtok = (BAT_TOKEN_T*) group->Tokens[y];
            bat_debug(" |--- [%d | %d] Tok: [%d] %s | Val: %s \n", y+1, group->Size,  xtok->type, bat_debug_type(xtok->type), xtok->value);
        }
    }
    #endif
}

int bat_runtime_equal_var_and_string(int Line, BAT_TOKEN_T* Data1, BAT_TOKEN_T* Eq, BAT_TOKEN_T* Data2) {
    if (Data1->type != BAT_TOKEN_TYPE_VARIABLE){
        bat_fatalerror(Line, "The first value must be a variable.");
        return 0;
    }
    if (Data2->type != BAT_TOKEN_TYPE_STRING){
        bat_fatalerror(Line, "The second value must be a string.");
        return 0;
    }
    eBatCheckModule(Line, EBAT_CONFIG_SYSTEM_SET, "System.Set");
    eBatCheckVariable(Line, Data1->value, D1);

    qemu_ok("D1: '%s' && D3 '%s'", D1, Data2->value);

    int ret = 0;
    if (Eq->type == BAT_TOKEN_TYPE_EQUAL){
        ret = (strcmp(D1, Data2->value) == 0?1:0);
    } else {
        ret = (strcmp(D1, Data2->value) != 0?1:0);
    }


    free(D1);
    return ret;
}

int bat_runtime_equal(int Line, BAT_TOKEN_T* Data1, BAT_TOKEN_T* Eq, BAT_TOKEN_T* Data2) {
    if (Data1->type != Data2->type || (Data1->type != BAT_TOKEN_TYPE_VARIABLE && Data1->type != BAT_TOKEN_TYPE_NUMBER)) {
        bat_fatalerror(Line, "You cannot compare different types of data. Only NUMBERS and VARIABLES are allowed to be compared.");
        return 0;
    }
    if (Data1->type == BAT_TOKEN_TYPE_VARIABLE) {
        eBatCheckModule(Line, EBAT_CONFIG_SYSTEM_SET, "System.Set");
        eBatCheckVariable(Line, Data1->value, D1);
        eBatCheckVariable(Line, Data2->value, D2);
        if (D1 == NULL || D2 == NULL){
            if (D1 != NULL) free((void*) D1);
            if (D2 != NULL) free((void*) D2);
            return 0;
        }

        int ret = (strcmp(D1, D2) == 0?1:0);

        free(D1);
        free(D2);
        return ret;
    } else if (Data1->type == BAT_TOKEN_TYPE_NUMBER){
        int a = bat_strtol(Data1->value);
        int b = bat_strtol(Data2->value);
        switch (Eq->type) {
            case BAT_TOKEN_TYPE_EQUAL:{
                return (a == b?1:0);
            }
            case BAT_TOKEN_TYPE_NOT_EQUAL:{
                return (a != b?1:0);
            }
            case BAT_TOKEN_TYPE_GREATER:{
                return (a > b?1:0);
            }
            case BAT_TOKEN_TYPE_LESS:{
                return (a < b?1:0);
            }
            case BAT_TOKEN_TYPE_LESS_EQUAL:{
                return (a <= b?1:0);
            }
            case BAT_TOKEN_TYPE_GREATER_EQUAL:{
                return (a >= b?1:0);
            }
            default:
                return 0;
        }
    }
    return 0;
}

int bat_runtime_eval(BAT_T* bat,BAT_GROUP_T* group, int line, int offset) {
    if (group->Size <= 0) {
        bat_debug("Group Size is 0\n");
        return 0;
    }

    BAT_TOKEN_T *MainTok = (BAT_TOKEN_T *) group->Tokens[offset + 0];


    bat_debug("[%d] MainTok == %s\n", offset, bat_debug_type(MainTok->type));

    if (MainTok->type == BAT_TOKEN_TYPE_PAUSE){
        bat_runtime_system_pause();
        return 0;
    } else if (MainTok->type == BAT_TOKEN_TYPE_GOTO){
        EBAT_INVALIDARGC(line, group->Size - offset, 2);
        BAT_TOKEN_T *Data1 = (BAT_TOKEN_T *) group->Tokens[offset + 1];
        qemu_warn("Start goto '%s'\n", Data1->value);
        BAT_GoTo_T* gt = NULL;
        if (Data1->type == BAT_TOKEN_TYPE_VARIABLE){
            eBatCheckVariable(line, Data1->value, D1);
            gt = bat_runtime_find_goto(bat, D1);
            free(D1);
        } else {
            gt = bat_runtime_find_goto(bat, Data1->value);
        }
        eBatCheckGoTo(line, gt);

        int ret = 0;
        for (int x = 0; x < gt->Size; x++){
            BAT_GROUP_T* group = (BAT_GROUP_T*) gt->Groups[x];

            bat_runtime_echo(bat, group);

            ret = bat_runtime_eval(bat, group, line, 0);
            bat_debug("sysline: %d | line: %d | ret: %d\n",gt->Line, line, ret);
            if (EBAT_CONFIG_CRITICAL_STOP == 1 && ret > 0) {
                bat_debug("CRITICAL OUT\n");
                bat->ErrorCode = ret;
                return ret;
                break;
            }
        }
        bat_runtime_eval(bat, (BAT_GROUP_T*) gt->Groups, line, 0);
        return ret;
    }

    if (MainTok->type == BAT_TOKEN_TYPE_EXIT) {
        EBAT_INVALIDARGC(line, group->Size - offset, 2);
        BAT_TOKEN_T *Data1 = (BAT_TOKEN_T *) group->Tokens[offset + 1];
        return (-1 * bat_strtol(Data1->value));
    }

    if (MainTok->type == BAT_TOKEN_TYPE_SET) {
        eBatCheckModule(line, EBAT_CONFIG_SYSTEM_SET, "System.Set");
        EBAT_INVALIDARGC(line, group->Size - offset, 4);
        BAT_TOKEN_T *VAR = (BAT_TOKEN_T *) group->Tokens[offset + 1];
        BAT_TOKEN_T *EQ = (BAT_TOKEN_T *) group->Tokens[offset + 2];
        BAT_TOKEN_T *SET = (BAT_TOKEN_T *) group->Tokens[offset + 3];
        eBatCheckMixingData(line, EQ->type, BAT_TOKEN_TYPE_SET);
        if (SET->type == BAT_TOKEN_TYPE_NOT){
            bat_runtime_system_set(VAR->value, NULL);
        } else {
            eBatCheckMixingData(line, SET->type, BAT_TOKEN_TYPE_STRING);
            bat_runtime_system_set(VAR->value, SET->value);
        }
    }
    if (MainTok->type == BAT_TOKEN_TYPE_ECHO) {
        EBAT_INVALIDMINARGC(line, group->Size - offset, 2);
        BAT_TOKEN_T *TextTok = (BAT_TOKEN_T *) group->Tokens[offset + 1];
        //printf("TextTok is %s => '%s'\n", bat_debug_type(TextTok->type), TextTok->value);
        if (TextTok->type == BAT_TOKEN_TYPE_DEBUG){
            bat->Debug = !(bat->Debug);
            return 0;
        } else if (TextTok->type == BAT_TOKEN_TYPE_TRUE){
            bat->Echo = 1;
            return 0;
        } else if (TextTok->type == BAT_TOKEN_TYPE_FALSE){
            bat->Echo = 0;
            return 0;
        }

        for (int ec = 1; ec < group->Size - offset; ec++){
            int isStart = (ec == 1?1:0);
            int isEnd = (group->Size - offset - 1 == ec?1:0);
            BAT_TOKEN_T *Echo = (BAT_TOKEN_T *) group->Tokens[offset + ec];
            //printf("\n |----- [%d | %d] '%s'\n",ec, group->Size - offset - 1, Echo->value);
            if (Echo->type == BAT_TOKEN_TYPE_VARIABLE){
                eBatCheckModule(line, EBAT_CONFIG_SYSTEM_SET, "System.Set");
                eBatCheckVariable(line, Echo->value, Text);
                bat_runtime_system_echo(Text, isStart, isEnd);
                free(Text);
            } else {
                bat_runtime_system_echo(Echo->value, isStart, isEnd);
            }
        }
        return 0;
    }

    if (MainTok->type == BAT_TOKEN_TYPE_IF) {

        EBAT_INVALIDMINARGC(line, group->Size - offset, 3);

        int isNOT = 0;

        BAT_TOKEN_T *DataNOT = (BAT_TOKEN_T *) group->Tokens[offset + 1];
        if (DataNOT->type == BAT_TOKEN_TYPE_NOT){
            isNOT = 1;
        }


        BAT_TOKEN_T *Data1 = (BAT_TOKEN_T *) group->Tokens[offset + isNOT + 1];
        BAT_TOKEN_T *Data2 = (BAT_TOKEN_T *) group->Tokens[offset + isNOT + 2];
        BAT_TOKEN_T *Data3 = (BAT_TOKEN_T *) group->Tokens[offset + isNOT + 3];

        if (Data1->type == BAT_TOKEN_TYPE_EXIST) {
            qemu_note("D1 == BAT_TOKEN_TYPE_EXIST");
            eBatCheckModule(line, EBAT_CONFIG_FILEIO_EXIST, "FileIO.Exits");
            if (Data2->type == BAT_TOKEN_TYPE_STRING){
                int breq = bat_runtime_fileio_exist(Data2->value);
                bat_debug("IF EXIST STRING: %d | %d\n", breq, (breq == 1 && isNOT == 0) ||
                                                      (breq != 1 && isNOT == 1));
                if ((breq == 1 && isNOT == 0) ||
                    (breq != 1 && isNOT == 1)){
                    bat_debug("Start eval %d\n", offset + isNOT + 3);
                    bat_runtime_eval(bat, group, line, offset + isNOT + 3);
                }
            } else {
                eBatCheckMixingData(line, Data2->type, BAT_TOKEN_TYPE_VARIABLE);
                eBatCheckModule(line, EBAT_CONFIG_SYSTEM_SET, "System.Set");
                eBatCheckVariable(line, Data2->value, Path);

                int breq = bat_runtime_fileio_exist(Path);
                bat_debug("IF EXIST VARIABLE: %d\n", breq);
                if ((breq == 1 && isNOT == 0) ||
                    (breq != 1 && isNOT == 1)){
                    bat_runtime_eval(bat, group, line, offset + isNOT + 3);
                }

                free(Path);
            }

        } else if (Data1->type == BAT_TOKEN_TYPE_NUMBER) {
            qemu_note("D1 == BAT_TOKEN_TYPE_NUMBER");
            EBAT_INVALIDMINARGC(line, group->Size - offset, offset + 4);

            int breq = bat_runtime_equal(line, Data1, Data2, Data3);
            if ((breq == 1 && isNOT == 0) ||
                (breq != 1 && isNOT == 1)){
                bat_runtime_eval(bat, group, line, offset + isNOT + 3);
            }
            bat_debug("breq: %d | xret\n", breq);

        } else if (Data1->type == BAT_TOKEN_TYPE_VARIABLE && Data3->type == BAT_TOKEN_TYPE_VARIABLE) {
            EBAT_INVALIDMINARGC(line, group->Size - offset, offset + 4);

            int breq = bat_runtime_equal(line, Data1, Data2, Data3);
            if ((breq == 1 && isNOT == 0) ||
                (breq != 1 && isNOT == 1)){
                bat_runtime_eval(bat, group, line, offset + isNOT + 3);
            }
        } else if (Data1->type == BAT_TOKEN_TYPE_VARIABLE && Data3->type == BAT_TOKEN_TYPE_STRING) {
            qemu_ok("D1 == VAR && D3 == STR");
            EBAT_INVALIDMINARGC(line, group->Size - offset, offset + 4);

            int breq = bat_runtime_equal_var_and_string(line, Data1, Data2, Data3);
            if ((breq == 1 && isNOT == 0) ||
                (breq != 1 && isNOT == 1)) {
                qemu_ok("[PASS] Line: %d | Off: %d", line,  offset + isNOT + 4);
                bat_runtime_eval(bat, group, line, offset + isNOT + 4);
            }
        }
    }
    if (MainTok->type == BAT_TOKEN_TYPE_RUN || MainTok->type == BAT_TOKEN_TYPE_STRING) {
        int argc = group->Size - offset;
        char **argv = malloc((argc + 1) * sizeof(char*));

        if (argv == NULL) {
            return 1;
        }
        for (int i = 0; i < argc; i++){
            BAT_TOKEN_T *DataX = (BAT_TOKEN_T *) group->Tokens[offset + i];
            if (DataX->type == BAT_TOKEN_TYPE_VARIABLE){
                eBatCheckModule(line, EBAT_CONFIG_SYSTEM_SET, "System.Set");
                eBatCheckVariable(line, DataX->value, Text);
                size_t slen = strlen(Text);
                argv[i] = malloc((slen + 1) * sizeof(char));
                if (argv[i] == NULL) {
                    return 1;
                }
                memset(argv[i], 0, slen + 1);
                memcpy(argv[i], Text, slen);
                free(Text);
            } else {
                size_t slen = strlen(DataX->value);
                argv[i] = malloc((slen + 1) * sizeof(char));
                if (argv[i] == NULL) {
                    return 1;
                }
                memset(argv[i], 0, slen + 1);
                memcpy(argv[i], DataX->value, slen);
            }
            printf("[%d] val: '%s'\n", i, DataX->value);
        }
        int ret = bar_runtime_system_exec(argc, argv);
        return ret;
    }

    return 0;
}

int bat_runtime_exec(BAT_T* bat){
    bat_debug("========================\n");
    bat_debug("Runtime EXEC\n");
    bat_debug("========================\n");
    int ret = 0;
    for (int x = 0; x < bat->Size; x++){
        BAT_GROUP_T* group = (BAT_GROUP_T*) bat->Group[x];

        bat_runtime_echo(bat, group);

        ret = bat_runtime_eval(bat, group, x + 1, 0);
        bat_debug("line: %d | ret: %d\n", x +1, ret);
        if (EBAT_CONFIG_CRITICAL_STOP == 1 && ret > 0) {
            bat_debug("CRITICAL OUT\n");
            //bat->ErrorCode = ret;
            break;
        }

    }
    bat->ErrorCode = ret;
    bat_debug("========================\n");
    return ret;
}
