#include "valera.h"

int main() {
    valera_array_t* array = valera_array_new();

    valera_node_t* obj1 = valera_new();
    valera_push_string(obj1, "token", "Hello");
    valera_push_number(obj1, "start", 0);
    valera_push_number(obj1, "end", 5);
    valera_array_push_object(array, obj1);
    
    valera_node_t* obj2 = valera_new();
    valera_push_string(obj2, "token", "Valery");
    valera_push_number(obj2, "start", 5);
    valera_push_number(obj2, "end", 11);
    valera_array_push_object(array, obj2);
    
    valera_node_t* obj3 = valera_new();
    valera_push_string(obj3, "token", "!");
    valera_push_number(obj3, "start", 11);
    valera_push_number(obj3, "end", 12);
    valera_array_push_object(array, obj3);
    
    valera_array_print(array);

    valera_array_destroy(array);
    return 0;
}