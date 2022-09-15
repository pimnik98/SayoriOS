#ifndef VALERA_H
#define VALERA_H

#define VCHECKMEM(obj) if(obj==0) {\
	printf("L%d: malloc() failed!", __LINE__);\
	exit(1);\
}

#define VCHECKMEMRET(obj) if(obj==0) {\
	printf("L%d: malloc() failed!", __LINE__);\
	return 0;\
}

typedef struct valera_node {
	char                 busy;
	char*                name;
	struct valera_node*  next;
	struct valera_value* value;
} valera_node_t;

enum {
	VAL_UNDEF, VAL_NUM, VAL_STR, VAL_OBJ, VAL_ARR
};

typedef struct valera_value {
	char           used; // Only used in Valera.Array
	char           type;
	int            num;
	char*          str;
	valera_node_t* obj;
	struct valera_array* arr;
} valera_value_t;

typedef struct valera_array {
	int length;
	int setmax;
	valera_value_t **values;
} valera_array_t;

void valera_value_print(valera_value_t *val);
int valera_value_string_size(valera_value_t *val);
void valera_value_string(valera_value_t *val, char *string);
valera_value_t *valera_value_new();
void valera_value_set_string(valera_value_t *val, char* string);
void valera_value_set_number(valera_value_t *val, int number);
void valera_value_set_object(valera_value_t *val, valera_node_t* object);
void valera_value_set_array(valera_value_t *val, valera_array_t* array);
void valera_value_destroy(valera_value_t *val);
void valera_array_destroy(valera_array_t *arr);
valera_array_t *valera_array_new();
void _valera_array_extend(valera_array_t *arr);
void valera_array_push(valera_array_t *arr, valera_value_t *value);
void valera_array_push_string(valera_array_t *arr, char *str);
void valera_array_push_number(valera_array_t *arr, int num);
void valera_array_push_object(valera_array_t *arr, valera_node_t *obj);
void valera_array_push_array(valera_array_t *arr, valera_array_t *arr2);
void valera_array_pop(valera_array_t *arr);
int valera_array_length(valera_array_t *arr);
valera_value_t *valera_array_get(valera_array_t *arr, int index);
int valera_array_string_size(valera_array_t *arr);
void valera_array_string(valera_array_t *arr, char *string);
void valera_array_print(valera_array_t *arr);
valera_node_t *valera_new();
void _valera_new(valera_node_t *obj);
void valera_destroy(valera_node_t *obj);
void valera_print(valera_node_t *obj);
void valera_set(valera_node_t *obj, char* name, valera_value_t *value);
void valera_keys(valera_node_t *obj, valera_array_t *arr);
valera_value_t *valera_get(valera_node_t *obj, char *name);
int valera_level(valera_node_t *obj);
valera_node_t *valera_get_last(valera_node_t *obj);
valera_node_t *_valera_get_last_unused(valera_node_t *obj);
void valera_push(valera_node_t *obj, char *name, valera_value_t *val);
void valera_push_string(valera_node_t *obj, char *name, char *str);
void valera_push_number(valera_node_t *obj, char *name, int num);
void valera_push_object(valera_node_t *obj, char *name,valera_node_t *obj2);
void valera_push_array(valera_node_t *obj, char *name, valera_array_t *arr);
void valera_debug(valera_node_t *n);
int valera_json_size(valera_node_t *obj);
void valera_json(valera_node_t *obj, char *str);
char valera_array_join(valera_array_t* array, char *buf, char* sign);
int  valera_array_join_size(valera_array_t *arr, char* sign);
char valera_has(valera_node_t* obj, char *name);
void valera_array_removeat(valera_array_t* arr, unsigned int idx);

#endif
