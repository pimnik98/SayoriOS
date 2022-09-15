/*
	VALERA-C by NDRAEY (c) 2022
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "valera.h"

void valera_value_print(valera_value_t *val) {
	if(val->type==VAL_NUM) {
		printf("%i", val->num);
	}else if(val->type==VAL_STR) {
		printf("\"%s\"", val->str);
	}else if(val->type==VAL_OBJ) {
		valera_print(val->obj);
	}else if(val->type==VAL_UNDEF) {
		printf("null");
	}else if(val->type==VAL_ARR) {
		valera_array_print(val->arr);
	}else if(!val) {
		printf("null");
	}
}

int valera_digit_count(int number)
{
    int digits = 0;
    if (number < 0) digits = 1;
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}

/*
int valera_value_string_size(valera_value_t *val) {
	int size = 0;
	if(val->type==VAL_NUM) {
		size=valera_digit_count(val->num);
	}else if(val->type==VAL_STR) {
		size=2+strlen(val->str);
	}else if(val->type==VAL_OBJ) {
		size=valera_json_size(val->obj);
	}else if(val->type==VAL_UNDEF || !val) {
		size=4;
	}else if(val->type==VAL_ARR) {
		size=valera_array_string_size(val->arr);
	}
	return size;
}
*/

/*
void valera_value_string(valera_value_t *val, char *string) {
	if(val->type==VAL_NUM) {
		sprintf(string, "%i", val->num);
	}else if(val->type==VAL_STR) {
		sprintf(string, "\"%s\"", val->str);
	}else if(val->type==VAL_OBJ) {
		char *st = malloc(valera_json_size(val->obj));
		valera_json(val->obj, st);
		sprintf(string, "%s", st);
		free(st);
	}else if(val->type==VAL_UNDEF || !val) {
		sprintf(string, "null");
	}else if(val->type==VAL_ARR) {
		char *st = malloc(valera_array_string_size(val->arr)+1);
		valera_array_string(val->arr, st);
		sprintf(string, "%s", st);
		free(st);
	}
}
*/
valera_value_t *valera_value_new() {
	valera_value_t *tmp = calloc(1, sizeof(valera_value_t));
	VCHECKMEM(tmp);
	return tmp;
}

void valera_value_set_string(valera_value_t *val, char* string) {val->type = VAL_STR; val->str = string;}
void valera_value_set_number(valera_value_t *val, int number) {val->type = VAL_NUM; val->num = number;}
void valera_value_set_object(valera_value_t *val, valera_node_t* object) {val->type = VAL_OBJ; val->obj = object;}
void valera_value_set_array(valera_value_t *val, valera_array_t* array) {val->type = VAL_ARR; val->arr = array;}

void valera_value_destroy(valera_value_t *val) {
	if(val==0) return;
	
	// valera_value_print(val);
	// puts("");
	// if(val->type==VAL_OBJ) {valera_destroy(val->obj);}
	// else if(val->type==VAL_ARR) {valera_array_destroy(val->arr);}
	val->used = 0;
	val->type = 0;
	free(val);
}

void valera_array_destroy(valera_array_t *arr) {
	if(arr==0) return;
	for(int i=0; i<arr->length; i++) {
		valera_value_destroy(arr->values[i]);
	}
	free(arr->values);
	free(arr);
}

valera_array_t *valera_array_new() {
	valera_array_t *arr = calloc(1, sizeof(valera_array_t));
	VCHECKMEM(arr);
	arr->length = 0;
	arr->setmax = 1;
	arr->values = calloc(1, sizeof(valera_value_t**));
	return arr;
}

void _valera_array_extend(valera_array_t *arr) {
	arr->setmax++;
	arr->values = realloc(arr->values, sizeof(valera_value_t**)*arr->setmax);
}

void valera_array_push(valera_array_t *arr, valera_value_t *value) {
	if(arr->length>=arr->setmax) _valera_array_extend(arr);
	arr->values[arr->length] = value;
	arr->length++;
}

void valera_array_push_string(valera_array_t *arr, char *str) {
	valera_value_t *val = valera_value_new();
	valera_value_set_string(val, str);
	valera_array_push(arr, val);
}

void valera_array_push_number(valera_array_t *arr, int num) {
	valera_value_t *val = valera_value_new();
	valera_value_set_number(val, num);
	valera_array_push(arr, val);
}

void valera_array_push_object(valera_array_t *arr, valera_node_t *obj) {
	valera_value_t *val = valera_value_new();
	valera_value_set_object(val, obj);
	valera_array_push(arr, val);
}

void valera_array_push_array(valera_array_t *arr, valera_array_t *arr2) {
	valera_value_t *val = valera_value_new();
	valera_value_set_array(val, arr2);
	valera_array_push(arr, val);
}

void valera_array_pop(valera_array_t *arr) {
	arr->length--;
	arr->setmax--;
	arr->values = realloc(arr->values, sizeof(valera_value_t)*arr->setmax);
}

int valera_array_length(valera_array_t *arr) {
	return arr->length;
}

void valera_array_removeat(valera_array_t *arr, unsigned int idx) {
	if(idx > (unsigned int)arr->length) return;
	valera_value_destroy(arr->values[idx]);
	int w = 1;

	for(int i=idx; i<arr->length; i++) {
		arr->values[i] = arr->values[idx+w];
		w++;
	}
	
	arr->length--;
	arr->setmax--;
	arr->values = realloc(arr->values, sizeof(valera_value_t)*arr->setmax);
}

valera_value_t *valera_array_get(valera_array_t *arr, int index) {
	if(index > arr->length) return 0;
	return arr->values[index];
}

/*
int valera_array_string_size(valera_array_t *arr) {
	int size = 2; // []

	for(int i=0; i<arr->length; i++) {
		size+=valera_value_string_size(valera_array_get(arr, i));
		if(!(i==arr->length-1)) size+=2; // , 
	}
	
	return size;
}
*/

/*
void valera_array_string(valera_array_t *arr, char *string) {
	int point = 0;
	string[point++] = '[';

	for(int i=0, sz=valera_array_length(arr); i<sz; i++) {
		int stsize = valera_value_string_size(valera_array_get(arr, i));
		char *vl = malloc(stsize);
		VCHECKMEM(vl);
		valera_value_string(valera_array_get(arr, i), vl);
		for(int j=0; j<stsize; j++) {
			string[point++] = vl[j];
		}
		if(!(i==sz-1)) {
			string[point++] = ',';
			string[point++] = ' ';
		}
		free(vl);
	}
	string[point++] = ']';
}
*/

void valera_array_print(valera_array_t *arr) {
	printf("[");
	for(int i=0; i<arr->length; i++) {
		valera_value_print(arr->values[i]);
		if(i+1!=arr->length) printf(", ");
	}
	printf("]");
}

valera_node_t *valera_new() {
	valera_node_t *obj = calloc(1, sizeof(valera_node_t));
	VCHECKMEM(obj);
	obj->busy  = 0;
	obj->next  = calloc(1, sizeof(valera_node_t));
	VCHECKMEM(obj->next);
	obj->value = 0; //calloc(1, sizeof(valera_value_t));
	//VCHECKMEM(obj->value);
	return obj;
}

void _valera_new(valera_node_t *obj) {
	obj->busy  = 0;
	obj->next  = calloc(1, sizeof(valera_node_t));
	VCHECKMEM(obj->next);
	obj->value = 0; //calloc(1, sizeof(valera_value_t));
	//VCHECKMEM(obj->value);
}

void valera_destroy(valera_node_t *obj) {
	if(obj==0) return;
	// TODO: Memory cleanup
	while(1) {
		if(obj->next->busy==0) break;
		//obj->busy = 0;
		valera_value_destroy(obj->value);
		obj = obj->next;
	}
	//printf("%d\n", valera_level(obj));
	free(obj);
}

void valera_print(valera_node_t *obj) {
	printf("{");

	while(1) {
		printf("\"%s\": ", obj->name);
		valera_value_print(obj->value);
		char stop = obj->next->busy==0;
		if(stop) break;
		if(!stop) printf(", ");
		obj = obj->next;
	}
	
	printf("}");
}

void valera_set(valera_node_t *obj, char* name, valera_value_t *value) {
	obj->busy  = 1;
	obj->name  = name;
	obj->value = value;
}

void valera_keys(valera_node_t *obj, valera_array_t *arr) {
	while(1) {
		char stop = obj->next->busy==0;

		valera_value_t *tmp = valera_value_new();
		valera_value_set_string(tmp, obj->name);
		valera_array_push(arr, tmp);
		
		if(stop) break;
		obj = obj->next;
	}
}

valera_value_t *valera_get(valera_node_t *obj, char *name) {
	while(1) {
		char stop = obj->next->busy==0;
		//char stop2 = obj->name == name;
		char stop2 = strcmp(obj->name, name)==0;

		if(stop2) return obj->value;
		if(stop) break;
		obj = obj->next;
	}
	return 0;
}

char valera_has(valera_node_t *obj, char *name) {
	while(1) {
		char stop = obj->next->busy==0;
		char stop2 = strcmp(obj->name,name)==0;
		
		if(stop2) return 1;
		if(stop) break;
		obj = obj->next;
	}
	return 0;
}

int valera_level(valera_node_t *obj) {
	int level = 0;
	while(1) {
		if(obj->busy==0) break;
		obj = obj->next;
		level++;
	}
	return level;
}

valera_node_t *valera_get_last(valera_node_t *obj) {
	while(1) {
		char stop = obj->next->busy==0;
		if(stop) return obj;
		obj = obj->next;
	}
	return 0;
}

valera_node_t *_valera_get_last_unused(valera_node_t *obj) {
	return valera_get_last(obj)->next;
}

void valera_push(valera_node_t *obj, char *name, valera_value_t *val) {
	int level = valera_level(obj);
	if(level==0) {
		valera_set(obj, name, val);
	}else{
		valera_node_t *oper = _valera_get_last_unused(obj);
		_valera_new(oper);
		valera_set(oper, name, val);
	}
}

void valera_push_string(valera_node_t *obj, char *name, char *str) {
	valera_value_t *val = valera_value_new();
	valera_value_set_string(val, str);
	valera_push(obj, name, val);
}

void valera_push_number(valera_node_t *obj, char *name, int num) {
	valera_value_t *val = valera_value_new();
	valera_value_set_number(val, num);
	valera_push(obj, name, val);
}

void valera_push_object(valera_node_t *obj, char *name,valera_node_t *obj2) {
	valera_value_t *val = valera_value_new();
	valera_value_set_object(val, obj2);
	valera_push(obj, name, val);
}

void valera_push_array(valera_node_t *obj, char *name, valera_array_t *arr) {
	valera_value_t *val = valera_value_new();
	valera_value_set_array(val, arr);
	valera_push(obj, name, val);
}

void valera_debug(valera_node_t *n) {
	printf("Name: %s\nBusy: %d\n", n->name, n->busy);
	printf("Value: "); valera_value_print(n->value); puts("");
}

/*
int valera_json_size(valera_node_t *obj) {
	int size = 2; // {}

	for(int i=0, sz = valera_level(obj); i<sz; i++) {
		size+=2+strlen(obj->name); // "name"
		size+=2; // : 
		size+=valera_value_string_size(obj->value);
		if(!(i==sz-1)) size++;
		obj = obj->next;
	}
	
	return size;
}
*/
/*
void valera_json(valera_node_t *obj, char *str) {
	int point = 0;
	str[point++] = '{';
	for(int i=0, sz = valera_level(obj); i<sz; i++) { // Work at object
		str[point++] = '"';
		for(int j=0, sz2 = strlen(obj->name); j<sz2; j++) { // Work at name
			str[point++] = obj->name[j];
		}
		str[point++] = '"';
		str[point++] = ':';
		str[point++] = ' ';
		int stsize = valera_value_string_size(obj->value);
		char *st = malloc(stsize);
		valera_value_string(obj->value, st);
		for(int j=0; j<stsize; j++) {
			str[point++] = st[j];
		}
		free(st);
		if(!(i==sz-1)) {
			str[point++] = ',';
			str[point++] = ' ';
		}
		obj = obj->next;
	}
	str[point++] = '}';
}
*/

/* This function indicates how much memory the user should allocate */
int valera_array_join_size(valera_array_t* array, char* sign) {
	int overall_length = 0;
	int length = valera_array_length(array);
	int signlen = strlen(sign);

	for(int i=0; i<length; i++) {
		valera_value_t* value = valera_array_get(array, i);
		if(value->type!=VAL_STR) {
			valera_value_destroy(value);
			return 0; // Only strings allowed to join!
		}
		overall_length += strlen(value->str)+signlen;
	}
	// overall_length -= signlen;
	return overall_length+1;
}

char valera_array_join(valera_array_t* array, char* buf, char* sign) {
	int length = valera_array_length(array);
	int at = 0;
	int signlen = strlen(sign);
	//printf("Signlen is: %d\n", signlen);
	
	for(int i=0; i<length; i++) {
		valera_value_t* value = valera_array_get(array, i);
		if(value->type!=VAL_STR) {
			//valera_value_destroy(value);
			return -1; // Only strings allowed to join!
		}
		
		int strl = strlen(value->str);
		memcpy(buf + at, value->str, strl);
		at += strl;
		if(i+1<length) {
			memcpy(buf + at, sign, signlen);
			at += signlen;
		}
	}
	return 0;
}
