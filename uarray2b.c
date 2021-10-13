#include "uarray2b.h"
#include <uarray2.h>
#include <assert.h>
#include <uarray.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

/* this is the actual struct
to refer to struct = struct UArray2_T
to refer to pointer to struct = UArray2_T*/

// defines a pointer to a uarray2b_T
#define T UArray2b_T

struct T
{
    int width;
    int height;
    int blocksize;
    int size;
    UArray2_T elements;
};

T populate_uneven(int width, int height, int cols, int rows, T uarray2b, int size, int blocksize);
T populate_even(int cols, int rows, T uarray2b, int size, int blocksize);
//it is supposed to store pointers
T UArray2b_new (int width, int height, int size, int blocksize)
{
    assert(width > -1 && height > -1 && size > 0 && blocksize > 0);
    T uarray2b = malloc(sizeof(*uarray2b));
    assert(uarray2b != NULL);
    bool is_uneven = false;
    /*determine number of blocks in 2D Array*/
    int cols;
    int rows;
    /*If width % blocksize == 0, #cols of 2d array is (width/blocksize) */
    if (width % blocksize == 0) {
        cols = width / blocksize;
    }
    else {
        cols = (width / blocksize) + 1;
        is_uneven = true;
    }
    
    if (height % blocksize == 0) {
        rows = height / blocksize;
    }
    else {
        rows = (height / blocksize) + 1;
        is_uneven = true;
    }
    /*If height % blocksize == 0, height of 2d array is (height/blocksize) */
    
    /*Else, add 1 to each case where remainder is not 0*/
    uarray2b->width = width;
    uarray2b->height = height;
    uarray2b->blocksize = blocksize;
    uarray2b->size = size;
    uarray2b->elements = UArray2_new(cols, rows, sizeof(UArray_T));
    
    
    if(is_uneven) {
        return populate_uneven(width, height, cols, rows, uarray2b, size, blocksize);
    }
    else {
        return populate_even(cols, rows, uarray2b, size, blocksize);
    }
    /*turn into helper function later*/

}

T populate_uneven(int width, int height, int cols, int rows, T uarray2b, int size, int blocksize) {
    
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
    
            /*we don't have an edge or corner*/
            if ( (c != (cols - 1)) && (r != (rows - 1)) ) {
                *((UArray_T*)UArray2_at(uarray2b->elements, c, r)) = UArray_new((blocksize * blocksize), size);
                printf("This is the length of block c: %d, r: %d = %d\n", c, r, UArray_length(*((UArray_T*)UArray2_at(uarray2b->elements, c, r))));
            }
            /*we got a bottom right corner*/
            else if (c == (cols - 1) && r == (rows - 1)) {
                *((UArray_T*)UArray2_at(uarray2b->elements, c, r)) = UArray_new(((width % blocksize) * (height % blocksize)), size);
                printf("This is the length of block c: %d, r: %d = %d\n", c, r, UArray_length(*((UArray_T*)UArray2_at(uarray2b->elements, c, r))));
            }
            /*we got an right edge*/
            else if (c == (cols - 1) && r != (rows - 1)) {
                *((UArray_T*)UArray2_at(uarray2b->elements, c, r)) = UArray_new((width % blocksize) * (blocksize), size);
                printf("This is the length of block c: %d, r: %d = %d\n", c, r, UArray_length(*((UArray_T*)UArray2_at(uarray2b->elements, c, r))));
            }
            /*we got a bottom edge*/
            else if (r == (rows - 1) && c != (cols - 1)) {
                *((UArray_T*)UArray2_at(uarray2b->elements, c, r)) = UArray_new((height % blocksize) * (blocksize), size);
                printf("This is the length of block c: %d, r: %d = %d\n", c, r, UArray_length(*((UArray_T*)UArray2_at(uarray2b->elements, c, r))));
            }
            else{
                //printf("this broke at r: %d, c: %d\n", r, c);
            }
        }
    }
    return uarray2b;
}

T populate_even(int cols, int rows, T uarray2b, int size, int blocksize) {
    
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            *((UArray_T*)UArray2_at(uarray2b->elements, c, r)) = UArray_new((blocksize * blocksize), size);
        }
    }
    return uarray2b;
}        
    
        

T UArray2b_new_64K_block(int width, int height, int size)
{
    int blocksize = 1024;
    if (blocksize < size) {
        blocksize = 1;
    }
    return UArray2b_new(width, height, size, blocksize);
}

void UArray2b_free(T *array2b) 
{
    assert(array2b != NULL);
    assert(*array2b != NULL);
    
    for (int row_idx = 0; row_idx < UArray2_height((*array2b)->elements); row_idx++) {
        for (int col_idx = 0; col_idx < UArray2_width((*array2b)->elements); col_idx++) {
            UArray_free((UArray_T*) UArray2_at((*array2b)->elements,
                                               col_idx, row_idx));
        }
    }
    
    UArray2_free(&(*(array2b))->elements);
    free(*array2b);
}

int UArray2b_width(T uarray2b)
{
    assert(uarray2b != NULL);
    return uarray2b->width;
}

int UArray2b_height(T uarray2b)
{
    assert(uarray2b != NULL);
    return uarray2b->height;
}

//returns the number of blocks (or number of pointers to blocks)
//bc it is a uarray2 of blocks
int UArray2b_size(T uarray2b)
{
    assert(uarray2b != NULL);
    return uarray2b->size;
}

int UArray2b_blocksize(T uarray2b)
{
    assert(uarray2b != NULL);
    return uarray2b->blocksize;
}

void *UArray2b_at(T array2b, int column, int row) 
{
    assert(array2b != NULL);
    assert(column > -1);
    assert(column < array2b->width);
    assert(row > -1); 
    assert(row < array2b->height);
    
    int block_column = column / array2b->blocksize; //i
    int block_row = row / array2b->blocksize; //j
    
    UArray_T *block = UArray2_at(array2b->elements, block_column, block_row);
    int cell = ((array2b->blocksize) * (row % array2b->blocksize)) + 
              (column % array2b->blocksize);
              
    return UArray_at(*block, cell);
}

void UArray2b_map(UArray2b_T array2b, 
                 void apply(int col, int row, UArray2b_T array2b, void *elem, void *cl),
                 void *cl)
{
    assert(array2b != NULL);
    int blocksize = UArray2b_blocksize(array2b);
    for (int row = 0; row < UArray2_height(array2b->elements); row++) {
        for (int col = 0; col < UArray2_width(array2b->elements); col++) {
            UArray_T* curr_block = UArray2_at(array2b->elements, col, row);
            for (int b_idx = 0; b_idx < UArray_length(*curr_block); b_idx++) {
                int row_idx = (row * blocksize) + (b_idx / blocksize);
                int col_idx = (col * blocksize) + (b_idx % blocksize);
                if (row_idx < array2b->height && col_idx < array2b->width) {
                    apply(col_idx, row_idx, array2b, 
                          UArray_at(*curr_block, b_idx), cl);
                }
            }
        }
    }
}


#undef T