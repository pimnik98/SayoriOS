#include "valera.h"

int main() {
	valera_array_t* array = valera_array_new();

	valera_array_push_string(array, "Hello");
	valera_array_push_string(array, "my");
	valera_array_push_string(array, "dear"); // 2
	valera_array_push_string(array, "friend");

	printf("Source array: ");
	valera_array_print(array);
	puts("");

	valera_array_removeat(array, 2); // dear

	printf("New array: ");
	valera_array_print(array);
	puts("");

	return 0;
}
