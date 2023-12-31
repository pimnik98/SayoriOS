#ifndef SAYORI_JSE_FUNCTION_H
#define SAYORI_JSE_FUNCTION_H

char* jse_strstr(const char* haystack, const char* needle);
void jse_ncpy(char *destination, const char *source, int length);
void jse_trim(char *str);
char* jse_mergeBuffers(char *buffer1, char *buffer2, int bufferSize1, int bufferSize2);
void jse_func_tolower(char* as);
void jse_func_toupper(char* as);
char jse_func_char_tolower(char ch);
int jse_func_atoi(const char* str);
int jse_p_int(const char* str, char** endptr);
char* jse_strdup(const char* str);
#endif //SAYORI_JSE_FUNCTION_H


#ifdef SAYORI_ELK_H
void jse_canvas_config(struct js* js);
void jse_canvas_destroy(struct js* js);
void jse_event_config(struct js* js);
void jse_array_config(struct js* js);
void jse_array_destroy(struct js* js);
int jse_getInt(struct js *js, jsval_t arg);
#endif //SAYORI_ELK_H