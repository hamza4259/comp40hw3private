#include "uarray2b.h"
#include "uarray.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <uarray2.h>

#define T UArray2b_T

struct T {
    int width;
    int height;
    int size;
    int blocksize;
    UArray2_T blocks;
};


T UArray2b_new (int width, int height, int size, int blocksize) {
    T uarray2b = malloc(sizeof(*uarray2b));
    assert(uarray2b != NULL);
    // int block_width = ceil((double)width / (double)blocksize);
    int block_width = width/blocksize + 1;
    // printf("block width: %d\n", block_width);
    // int block_height = ceil((double)height / (double)blocksize);
    int block_height = height / blocksize + 1;
    // printf("block height: %d\n", block_height);
    uarray2b->blocks = UArray2_new(block_width, block_height, 
                                   sizeof(UArray_T));
    
    uarray2b->width = width;
    uarray2b->height = height;
    uarray2b->size = size;
    uarray2b->blocksize = blocksize;

    for (int col = 0; col < block_width; col++) {
        for (int row = 0; row < block_height; row++) {
            *((UArray_T*) UArray2_at(uarray2b->blocks, col, row)) =
                UArray_new(blocksize * blocksize, size);
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

void UArray2b_free (T* array2b)
{
    assert(array2b != NULL);
    assert(*array2b != NULL);

    int blocks_height = UArray2_height((*array2b)->blocks);
    int blocks_width = UArray2_width((*array2b)->blocks);

    for (int row = 0; row < blocks_height; row++) {
        for (int col = 0; col < blocks_width; col++) {
            UArray_free((UArray_T*) UArray2_at((*array2b)->blocks,
                                               col, row));
        }
    }
    
    UArray2_free(&((*array2b)->blocks));
    free(*array2b);
}

int UArray2b_width (T array2b)
{
    assert(array2b != NULL);
    return array2b->width;
}

int UArray2b_height (T array2b)
{
    assert(array2b != NULL);
    return array2b->height;
}

int UArray2b_size (T array2b)
{
    assert(array2b != NULL);
    return array2b->size;
}

int UArray2b_blocksize(T array2b)
{
    assert(array2b != NULL);
    return array2b->blocksize;
}

void* UArray2b_at(T array2b, int column, int row)
{
    assert(array2b != NULL);
    assert(column < array2b->width && column > -1 
           && row < array2b->height && row > -1);

    int blocksize = array2b->blocksize;
    int block_y = column / blocksize;
    int block_x = row / blocksize;

    UArray_T* curr_block = UArray2_at(array2b->blocks, block_y, block_x);
    int curr_cell = blocksize * (row % blocksize) + (column % blocksize);
    return UArray_at(*curr_block, curr_cell);
}

void UArray2b_map(T array2b, void apply(int col, int row, T array2b,
                            void *elem, void *cl), void *cl)
{
    assert(array2b != NULL);
    int blocksize = UArray2b_blocksize(array2b);
    int block_width = UArray2_width(array2b->blocks);
    int block_height = UArray2_height(array2b->blocks);
    
    
    for (int block_y = 0; block_y < block_width; block_y++) {
        for (int block_x = 0; block_x < block_height; block_x++) {
            UArray_T* curr_block = UArray2_at(array2b->blocks, block_y, 
                                              block_x);
            int total_cells = UArray_length(*curr_block);
            
             for (int cell = 0; cell < total_cells; cell++) {
                 //check that we only visit the cell if it is within bounds
                 int row = (block_x * blocksize) + (cell / blocksize);
                 int col = (block_y * blocksize) + (cell % blocksize);
                 // printf("curr block: (%d, %d, %d)\n", block_y, block_x, cell);
                 // printf("col: %d, row: %d\n", row, col);
                 if (row < array2b->height && col < array2b->width) {
                     apply(col, row, array2b, 
                          UArray_at(*curr_block, cell), cl);
                 }
             }
        }
    }
}


#undef T


