#include <stdio.h>
#include <stdlib.h>

#define VECTOR_TYPE int

#include "number2.c"

int main() {
    
    test_create_and_basic();
    test_push_back_capacity();
    test_delete_at();
    test_is_equal();
    test_copy_operations();
    test_edge_cases();
    
    printf("ALL TESTS PASSED! Vector library works correctly!\n");
    
    return 0;
}
