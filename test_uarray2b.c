#include <stdio.h>
#include <stdlib.h>
#include "uarray2b.h"
#include <uarray2.h>
#include "assert.h"
#include <stdbool.h>


void
running_sum_helper(int col_idx, int row_idx, UArray2b_T a, void *possible_val, 
                  void *closure)
{       (void) possible_val;
        *((int*)closure) += *((int*)UArray2b_at(a, col_idx, row_idx));

        // printf("ar[%d,%d]\n", col_idx, row_idx);
        // printf("running sum: %d\n", *((int*)closure));
}


int main(int argc, char const *argv[]) {
    (void) argc;
    (void) argv;

    // UArray2b_T test_UArray2b = UArray2b_new(12, 12, sizeof(int), 4);
    // UArray2b_T zero_UArray2b = UArray2b_new(0, 0, sizeof(int), 4);
    // 
    // /* *((int*)UArray2b_at(zero_UArray2b, 0, 0)) = 1; */
    // int sum2 = 0;
    // UArray2b_map(zero_UArray2b, running_sum_helper, &sum2);
    // printf("sum2: %d\n", sum2);
    // 
    // 
    // (void) test_UArray2b;
    // 
    // /* testing width and height */
    // assert(12 == UArray2b_width(test_UArray2b)); //testing width
    // assert(12 == UArray2b_height(test_UArray2b)); //testing height
    // 
    // /* testing uarray2 at */
    // *((int*)UArray2b_at(test_UArray2b, 0, 0)) = 1;
    // *((int*)UArray2b_at(test_UArray2b, 0, 1)) = 2;
    // *((int*)UArray2b_at(test_UArray2b, 0, 2)) = 3;
    // *((int*)UArray2b_at(test_UArray2b, 1, 0)) = 4;
    // *((int*)UArray2b_at(test_UArray2b, 1, 1)) = 5;
    /* *((int*)UArray2b_at(test_UArray2b, 11, 11)) = 10; */
    
    /* throws error */
    /* *((int*)UArray2b_at(test_UArray2b, 12, 12)) = 10; */
    /* *((int*)UArray2b_at(test_UArray2b, -1, -1)) = 10; */
    
    /* throw error for new function */
    // UArray2b_T test_1 = UArray2b_new(-1, -1, -1, 0);
    // (void) test_1;
    // UArray2b_T zero_UArray2b = UArray2b_new(0, 0, sizeof(int), 4);
    // 
    // int sum = 0;
    // UArray2b_map(test_UArray2b, running_sum_helper, &sum);
    // printf("sum: %d", sum);
    // //testing free
    // UArray2b_free(&test_UArray2b);
    // UArray2b_free(&zero_UArray2b);
}



