# Valera

libvalera is a implementation of dynamic-sized arrays and Python-like dicts.

# Dicts

Example:
```c

#include <valera.h>
#include <stdio.h>

int main() {
	valera_node_t *object = valera_new();
	valera_push_string(object, "name", "David");

	valera_print(object);
	puts("");

	return 0;
}
```

Output: ```{"name": "David"}```

# Building

Just run command: ```make install```

It will install libvalera.so to ```/usr/lib/``` folder.

You also can run examples: ```make example``` or ```make json``` or ```make simple```

