#include "valera.h"
#include <stdlib.h>

int main() {
  valera_array_t* array = valera_array_new();
  valera_array_push_string(array, "Why");
  valera_array_push_string(array, ",");
  valera_array_push_string(array, "are");
  valera_array_push_string(array, ",");
  valera_array_push_string(array, "you");
  valera_array_push_string(array, " ");
  valera_array_push_string(array, ",");
  valera_array_push_string(array, ",");
  valera_array_push_string(array, ",");
  valera_array_push_string(array, " ");
  valera_array_push_string(array, "skipping");
  valera_array_push_string(array, " ");
  valera_array_push_string(array, "focken");
  valera_array_push_string(array, " ");
  valera_array_push_string(array, "commas?");
  
  int allocateto = valera_array_join_size(array, "");
  printf("Allocatting %d bytes\n", allocateto);
  char* total_string = malloc(allocateto);
  valera_array_join(array, total_string, "");
  printf("String is: %s\n", total_string);
  free(total_string);
  
  valera_array_destroy(array);
  return 0;
}
