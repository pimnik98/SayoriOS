// libvector - simple dynamic array (vector) library for C by NDRAEY.

#include "portability.h"
#include "lib/stdlib.h"
#include "lib/string.h"
#include "../include/vector.h"

vector_t* vector_new() {
	vector_t* vec = calloc(1, sizeof *vec);

	if(!vec) {
		return 0;
	}

	vec->data = calloc(1, sizeof(size_t));

	if(!vec->data) {
		free(vec);

		return 0;
	}

	vec->size = 0;
	vec->capacity = 1;

	return vec;
}

void vector_resize(vector_t* vec, size_t size) {
	if(!vec)
		return;

	void* new_buf = realloc(vec->data, sizeof(size_t) * size);

	if(!new_buf)
		return;

	vec->data = new_buf;
	vec->capacity = size;
    vec->size = size;
}

void vector_push_back(vector_t* vec, size_t element) {
	if(vec->size >= vec->capacity) {

		size_t new_cap = vec->capacity + ((vec->capacity + 1) / 2);

		void* new_buf = realloc(vec->data, sizeof(size_t) * new_cap);

		if(!new_buf)
			return;

		vec->data = new_buf;
		vec->capacity = new_cap;
	}

	vec->data[vec->size++] = element;
}

vector_result_t vector_pop_back(vector_t* vec) {
	if(!vec)
		return VEC_ERR;

	if(vec->size == 0)
		return VEC_ERR;

	return (vector_result_t){false, vec->data[(vec->size--) - 1]};
}

vector_result_t vector_get(vector_t* vec, size_t index) {
	if(!vec || index >= vec->size)
		return VEC_ERR;

	return VEC_OK(vec->data[index]);
}

void vector_erase_all(vector_t* vec) {
	if(!vec)
		return;

	free(vec->data);

	vec->data = calloc(1, sizeof(size_t));
	vec->size = 0;
	vec->capacity = 1;
}

vector_result_t vector_erase_nth(vector_t* vec, size_t n) {
	if(!vec)
		return VEC_ERR;

	if(n >= vec->size)
		return VEC_ERR;

	size_t val = vec->data[n];

	for(size_t i = n; i < vec->size - 1; i++) {
		vec->data[i] = vec->data[i + 1];
	}

	vec->size--;

	return VEC_OK(val);
}

vector_t* vector_clone(vector_t* original) {
	if(!original)
		return 0;

	vector_t* new_vec = calloc(1, sizeof *new_vec);

	new_vec->data = calloc(original->capacity, sizeof(size_t));

	memcpy(new_vec->data, original->data, sizeof(size_t) * original->capacity);

	new_vec->size = original->size;
	new_vec->capacity = original->capacity;

	return new_vec;
}

void vector_shrink_fit(vector_t* original) {
	if(!original)
		return;

	void* data = realloc(original->data, original->size);

	if(!data)
		return;

	original->data = data;
	original->capacity = original->size;
}

void vector_swap(vector_t* vec, size_t first, size_t second) {
	if(!vec)
		return;

	if(first >= vec->size || second >= vec->size)
		return;


	size_t b = vec->data[second];
	vec->data[second] = vec->data[first];
	vec->data[first] = b;
}



void vector_insert(vector_t* vec, size_t index, size_t value) {
	if(!vec)
		return;

    if(index >= vec->size)
        return;

    vector_resize(vec, vec->size + 1);

    for(size_t i = vec->size - 1; i > index; i--) {
        vec->data[i] = vec->data[i - 1];
    }

    vec->data[index] = value;
}

void vector_destroy(vector_t* vec) {
	if(!vec)
		return;

	free(vec->data);

	free(vec);
}
