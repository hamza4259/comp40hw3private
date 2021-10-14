/*
 * 
 * Comp40 Homework 3
 * Hamza Ali (hali03) and Pamela Melgar (pmelga01)
 * uarray2b.c
 * 
 * Edited: October 13 2021
 * Purpose: The implementation of the UArray2b_T interface. This file 
 *          defines a 2D array type where the array is stored in "blocks"
 *          rather than simple rows and columns. A blocked UArray2 
 *          representation allows a different type of efficiency when 
 *          accessing elements near each other 
 *
 */

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


/* UArray2b_new
*  Purpose: Creates a new blocked UArray2b_T with an indicated 
*           width, height, element size and blocksize, which indicates 
*           how many elements will be stored per block (each block will 
*           contain [blocksize*blocksize] number of elements)
*  Parameters:
*           int width: Desired width of the overall 2darray (# columns)
*           int height: Desired height of the overall 2darray (# rows)
*           int size: the size of the element that will be stored in each 
*                     cell/index of the 2d array
*           int blocksize: indicates how the 2d array will be partitoned, 
*                          i.e the 2darray's width and height
*  Effects: 
*           If either width or height is 0, calls a checked runtime error
*           If the memory allocation for the blocked 2darray fails, the 
*           program will also terminate in a checked runtime error
*  Returns:
*            A UArray2b_T with desired dimensions and storage capabilities
*/
T UArray2b_new (int width, int height, int size, int blocksize) {
    T uarray2b = malloc(sizeof(*uarray2b));
    assert(uarray2b != NULL && width > -1 && height > -1 && size > 0 && 
           blocksize > 0);
    int block_width = width / blocksize + 1;
    int block_height = height / blocksize + 1;
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



/* UArray2b_new_64K_block
*  Purpose: Tries to create a new blocked UArray2b_T with an indicated 
*           width, height, element size. This function will try to "default" 
*           the blocksize for the client. It is similar to UArray2b_new, but 
*           chooses a blocksize that is as large as possible while still 
*           allowing ONE block to fit 64KB (or 1024 bytes).
*  Parameters:
*           int width: Desired width of the overall 2darray (# columns)
*           int height: Desired height of the overall 2darray (# rows)
*           int size: the size of the element that will be stored in each 
*                     cell/index of the 2d array
*  Effects: 
*           none
*  Returns:
*            A UArray2b_T with desired dimensions and storage capabilities
*/
T UArray2b_new_64K_block(int width, int height, int size)
{
    assert(width > -1 && height > -1 && size > 0);
    int max_size = 1024 * 64;
    double blocksize;
    double block_area = max_size / size;
    
    if (block_area >= max_size) {
        blocksize = floor(sqrt(block_area));
    }
    else {
        blocksize = 1.0;
    }
    
    int blocksize_converted = (int)blocksize;
    return UArray2b_new(width, height, size, blocksize_converted);
}



/* UArray2b_free
*  Purpose: This function frees the memory used by a UArray2b_T blocked 
*           2d array. The free order ensures that the blocks (represented 
*           as UArray_Ts) are freed before the overall 2d array 
*  Parameters:
*           UArray2_T array2b - that indicates the UArray2b blocked 2d array 
*               that needs to be freed
*  Effects: 
*           Program will terminate in a checked runtime error if the supplied 
*           UArray2b is uninitialized/ NULL. 
*  Returns:
*           nothing - void
*/

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



/* UArray2b_width
* Purpose: Return the width of a blocked 2d Array, UArray2b_T (# of columns)
* Parameters: A valid UArray2b_T
* Returns: Integer representing the width of the overall 2darray
*/
int UArray2b_width (T array2b)
{
    assert(array2b != NULL);
    return array2b->width;
}



/* UArray2b_height
* Purpose: Return the height of a blocked 2d Array, UArray2b_T (# of rows)
* Parameters: A valid UArray2b_T
* Returns: Integer representing the height of the overall 2darray
*/
int UArray2b_height (T array2b)
{
    assert(array2b != NULL);
    return array2b->height;
}



/* UArray2b_size
* Purpose: Return the size of the elements that can be stored in each 
*          cell/index of the 2darray
* Parameters: A valid UArray2b_T
* Returns: Integer representing the size of each element
*/
int UArray2b_size (T array2b)
{
    assert(array2b != NULL);
    return array2b->size;
}



/* UArray2b_blocksize
* Purpose: Returns the blocksize that has been set for the overalL 2d array 
*          indicates how the 2d array is partitioned in blocks for traversal
*          and efficient memory access of elements
* Parameters: A valid UArray2b_T
* Returns: Integer representing the block size set for the UArry2b_T
*/
int UArray2b_blocksize(T array2b)
{
    assert(array2b != NULL);
    return array2b->blocksize;
}


/* UArray2b_at
*  Purpose: 
*           To retrieve or set an element value at a given column and row 
*           index from a UArray2b 2D array. This function uses the supplied 
*           coordinates to locate and return one desired element in the 
*           blocked 2D array.
*  Parameters: 
*            UArray2b_T array2b - a UArray2b_T array that is valid
*              (i.e it is non-empty and initialized)
*            int column -  the number coordinate that corresponds to the 
*              x-coordinate of the entire 2D Array, 
*            Int row -  the number coordinate that corresponds to the 
*              y-coordinate of the entire 2D Array
*  Effects: 
*         Calls a Checked Runtime Error if the supplied index is out of 
*         bounds of the given 2d array’s width and height
*         Column and Row index supplied must be between 0 and width 
*         and between 0 and height
*  Returns: 
*  	      A void pointer to the element at the given index of the supplied 
*           blocked 2darray 
*  Note: 
*            By returning a pointer to the location of the desired element, 
*            an element can be acquired/inserted/changed
*/
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



/* UArray2b_map
* Purpose: A mapping function that is specific for a blocked 2d array. 
*          This function traverses the 2d array in block-major order, 
*          that is, it will visit every single cell/index within the 2d 
*          array starting with the top left block and ending with the 
*          bottom right block. In other words, row-major order of the blocks 
*          of the 2d array, and within the sub-map/block itself, each 
*          cell/index will be visited in row-major order itself. Each 
*          cell/index of the overall 2d array will be visited once and 
*          this funciton allows for an apply function to be called for 
*          something to be done to these cells, and a closure can be 
*          passed as a "folding" operation to gather some overall 
*          data after visiting the entire 2d array
* Parameters: UArray2b_T array2b - A valid UArray2b_T
*             void apply() - a function that will be executed on 
*               an individual element/index of the 2d array
*             void *cl - a closure that is passed in to be given to 
*               apply function and gather some overall conclusion/data 
*               after the block-major mapping has completed
* Effects: The program will terminate in a CRE if the supplied UArray2b_T 
*          is NULL and uninitialized. 
* Returns: void - nothing
*/
void UArray2b_map(T array2b, void apply(int col, int row, T array2b,
                            void *elem, void *cl), void *cl)
{
    assert(array2b != NULL);
    assert(apply != NULL);
    int blocksize = UArray2b_blocksize(array2b);
    int block_width = UArray2_width(array2b->blocks);
    int block_height = UArray2_height(array2b->blocks);
    int height = array2b->height;
    int width = array2b->width;
    
    
    for (int block_y = 0; block_y < block_width; block_y++) {
        for (int block_x = 0; block_x < block_height; block_x++) {
            UArray_T* curr_block = UArray2_at(array2b->blocks, block_y, 
                                              block_x);
            int total_cells = UArray_length(*curr_block);
             for (int cell = 0; cell < total_cells; cell++) {
                 /* check that we only visit the cell if it is within bounds */
                 int row = (block_x * blocksize) + (cell / blocksize);
                 int col = (block_y * blocksize) + (cell % blocksize);
                 if (row < height && col < width) {
                     apply(col, row, array2b, 
                          UArray_at(*curr_block, cell), cl);
                 }
             }
        }
    }
}


#undef T


