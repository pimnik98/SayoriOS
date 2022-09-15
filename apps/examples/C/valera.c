#include "../../include/stdio.h"
#include "../../extlibs/ValeraC/valera.h"

int main() {
	valera_node_t *object = valera_new();
	valera_push_string(object, "hello", "World");
	valera_push_string(object, "valera_c_author", "Andrey Pavlenko");
	valera_push_string(object, "sayori_os_author", "Nikita Piminov");
	valera_push_string(object, "all_in_one_tester", "Denis Litvinov");

	valera_print(object);
	puts(""); // Newline

	return 0;
}
