#define VECTOR_TYPE int

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef VECTOR_TYPE
#error "The vector type is not defined"
#endif


typedef struct {
    VECTOR_TYPE *data;
    size_t size;
    size_t capacity;
    VECTOR_TYPE (*CopyVoidPtr)(VECTOR_TYPE); 
    void (*DeleteVoidPtr)(VECTOR_TYPE);

} Vector;


Vector* create_vector(size_t initial_capacity, VECTOR_TYPE
                    (*CopyFunc)(VECTOR_TYPE), void (*DeleteFunc)(VECTOR_TYPE)) {     
    
    Vector* vector = malloc(sizeof(Vector));
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
    vector->CopyVoidPtr = CopyFunc;
    vector->DeleteVoidPtr = DeleteFunc;

    if (initial_capacity > 0) {
        vector->data = malloc(initial_capacity * sizeof(VECTOR_TYPE));
        if (vector->data) {
            vector->capacity = initial_capacity;
        } else {
            return NULL;
        }
    }

    return vector;

}

void erase_vector(Vector *v) {

    if (!v || !v->data) {
        return;
    }

    if (v->DeleteVoidPtr) {
        for (size_t i = 0; i < v->size; i++) {
            v->DeleteVoidPtr(v->data[i]);
        }
    }
    free(v->data);
    v->data = NULL;
    v->size = 0;
    v->capacity = 0;

}

int is_equal_vector(const Vector *v1, const Vector *v2) {

    if (!v1 || !v2) {
        if (v1 == v2) return 1;
        return 0;
    }

    if (!v1->data || !v2->data) {
        if (v1->data == v2->data) return 1;
        return 0;
    }

    if (v1->size != v2->size) return 0;

    for (size_t i = 0; i < v1->size; i++) {
        if (memcmp(&v1->data[i], &v2->data[i], sizeof(VECTOR_TYPE)) != 0) {
            return 0;
        }
    }

    return 1;
}

void copy_vector(Vector *dest, const Vector *src) {

    if (!dest || !src) return;

    erase_vector(dest);

    dest->data = malloc(src->capacity * sizeof(VECTOR_TYPE));

    if (!dest->data) return;

    dest->size = src->size;
    dest->capacity = src->capacity;

    if (src->CopyVoidPtr) {
        for (size_t i = 0; i < src->size; i++) {
            dest->data[i] = dest->CopyVoidPtr(src->data[i]);
        }
    } else {
        memcpy(dest->data, src->data, src->size * sizeof(VECTOR_TYPE));
    }
}

Vector *copy_vector_new(const Vector *src) {
    if (!src) return NULL;

    Vector* new_vec = create_vector(src->capacity, src->CopyVoidPtr, src->DeleteVoidPtr);

    if(!new_vec) return NULL;

    new_vec->size = src->size;

    if (src->CopyVoidPtr) {
        for (size_t i = 0; i < src->size; i++) {
            new_vec->data[i] = src->CopyVoidPtr(src->data[i]);
        }
    } else {
        memcpy(new_vec->data, src->data, src->size * sizeof(VECTOR_TYPE));
    }

    return new_vec;
}

void push_back_vector(Vector *v, VECTOR_TYPE value) {

    if (!v) return;

    if (v->size == v->capacity) {
        if (v->capacity == 0) v->capacity = 2;

        v->capacity *= 2;
        VECTOR_TYPE* new_data = realloc(v->data, v->capacity * sizeof(VECTOR_TYPE));
        if (!new_data) return;
        v->data = new_data;
    }

    if (v->CopyVoidPtr) {
        v->data[v->size] = v->CopyVoidPtr(value);
    } else {
        v->data[v->size] = value;
    }

    v->size++;
}

void delete_vector(Vector *v) {
    if (!v) return;

    erase_vector(v);
    free(v);
}

void delete_at_vector(Vector *v, size_t index) {

    if (!v) return;

    if (index >= v->size) {
        return;
    }

    Vector* new_vec = create_vector(v->capacity, v->CopyVoidPtr, v->DeleteVoidPtr);

    if (!new_vec) return;
    if (!new_vec->data) return;

    int new_i = 0;
    for(size_t i = 0; i < v->size; i++) {
        if (i != index) {
            if (v->CopyVoidPtr) {
                new_vec->data[new_i] = v->CopyVoidPtr(v->data[i]);
            } else {
                new_vec->data[new_i] = v->data[i];
            }
            new_i++;
        } else {
            // if (v->DeleteVoidPtr) {
            //     v->DeleteVoidPtr(v->data[i]);
            // }
            continue;
        }
    }
    new_vec->size = new_i;

    copy_vector(v, new_vec);
    delete_vector(new_vec);
}

VECTOR_TYPE get_at_vector(const Vector *v, size_t index) {

    if(!v || index >= v->size) {
        VECTOR_TYPE error = {0};
        return error;
    }

    return v->data[index];
}
