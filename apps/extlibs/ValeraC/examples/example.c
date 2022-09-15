#include <stdio.h>
#include <stdlib.h>
#include "valera.h"

int main() {
	valera_node_t *obj = valera_new();
	VCHECKMEM(obj);

	valera_push_number(obj, "number", 123456);
	valera_push_string(obj, "string", "Hello Valera!");

	valera_node_t *ano = valera_new();
	VCHECKMEM(ano);

	valera_push_string(ano, "name", "Pulsemon");
	valera_push_string(ano, "level", "Rookie");
	valera_push_string(ano, "attrs", "Vaccine");
	valera_push_string(ano, "type", "Beast Humanoid");
	valera_push_string(ano, "prev_form", "Bibimon");

	valera_array_t *next_forms = valera_array_new();
	VCHECKMEM(next_forms);
	valera_array_push_string(next_forms, "Bulkmon");
	valera_array_push_string(next_forms, "Exermon");
	valera_array_push_string(next_forms, "Runnermon");
	valera_array_push_string(next_forms, "Namakemon");

	valera_push_array(ano, "next_forms", next_forms);
	valera_push_string(ano, "description", "It is a Digimon that digivolved under the influence of heartbeat waveform data stored in hospitals and gyms.");
	valera_push_object(obj, "DigiMon", ano);

	valera_array_t *array = valera_array_new();
	VCHECKMEM(array);
	valera_array_push_number(array, 1);
	valera_array_push_number(array, 2);
	valera_array_push_number(array, 3);
	valera_array_push_number(array, 4);
	valera_array_push_number(array, 5);

	valera_push_array(obj, "array", array);

	valera_print(obj);
	puts("");

	valera_array_destroy(array);
	valera_array_destroy(next_forms);
	valera_destroy(ano);
	valera_destroy(obj);
	
	return 0;
}
