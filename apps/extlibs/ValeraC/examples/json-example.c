#include <stdio.h>
#include <stdlib.h>
#include "valera.h"

int main() {
	valera_node_t *obj = valera_new();
	valera_push_number(obj, "number", 7);
	valera_push_string(obj, "ohwhow", "Valera is cool!");

	char *str = malloc(valera_json_size(obj));
	valera_json(obj, str);

	printf("%s\n", str);
	free(str);
	return 0;
}
