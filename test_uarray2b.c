#include <stdio.h>
#include <stdlib.h>
#include "uarray2b.h"
#include <uarray2.h>
#include "assert.h"
#include <stdbool.h>


void
running_sum_helper(int col_idx, int row_idx, UArray2b_T a, void *possible_val, void *closure)
{       (void) possible_val;
        *((int*)closure) += *((int*)UArray2b_at(a, col_idx, row_idx));

        // printf("ar[%d,%d]\n", col_idx, row_idx);
        // printf("running sum: %d\n", *((int*)closure));
}


int main(int argc, char const *argv[]) {
    (void) argc;
    (void) argv;

    UArray2b_T test_UArray2b = UArray2b_new(12, 12, sizeof(int), 4);
    assert(10 == UArray2b_width(test_UArray2b)); //testing width
    assert(10 == UArray2b_height(test_UArray2b)); //testing height
    // UArray2b_free(&test_UArray2b); //testing free
    
    
    *((int*)UArray2b_at(test_UArray2b, 0, 0)) = 1;
    *((int*)UArray2b_at(test_UArray2b, 1, 0)) = 2;
    *((int*)UArray2b_at(test_UArray2b, 2, 0)) = 2;
    *((int*)UArray2b_at(test_UArray2b, 0, 1)) = 3;


    // printf("uarray at 0, 0 after setting something: %d\n", *((int*)UArray2b_at(test_UArray2b, 0, 0)));
    int sum = 0;
    UArray2b_map(test_UArray2b, running_sum_helper, &sum);
    printf("sum: %d", sum);
}

/*
void test_block_length()
{
    
        UArray2b_T test_UArray2b = UArray2b_new(10, 10, sizeof(int), 4);
        assert(10 == UArray2b_width(test_UArray2b)); //testing width
        assert(10 == UArray2b_height(test_UArray2b)); //testing height
        
        
}*/


