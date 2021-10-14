/*
 * 
 * Comp40 Homework 3
 * Hamza Ali (hali03) and Pamela Melgar (pmelga01)
 * uarray2b.h
 * 
 * Edited: October 13 2021
 * Purpose: The interface of for a blocked UArray2 (UArray2b). This
 *          file provides the public functions to implement and interact 
 *          with a UArray2b type/object. 
 *
 * Notes: it is a checked run-time error to pass a NULL T
 *        to any function in this interface
 */

#ifndef Uarray2b_dot_h
#define Uarray2b_dot_h

#define T UArray2b_T
typedef struct T *T;

/*
* new blocked 2d array
* blocksize = square root of # of cells in block.
* blocksize < 1 is a checked runtime error
*/
extern T UArray2b_new (int width, int height, int size, int blocksize);

/* new blocked 2d array: blocksize as large as possible provided
* block occupies at most 64KB (if possible)
*/
extern T UArray2b_new_64K_block(int width, int height, int size);

/*Frees the memory that has been allocated by the UArray2b*/
extern void UArray2b_free (T *array2b);

/* A getter function to return the width of the overall 2d array, the width 
*  being representative of the number of columns in the 2d array*/
extern int UArray2b_width (T array2b);

/* A getter function to return the height of the overall 2d array, the height
*  being representative of the number of rows in the 2d array*/
extern int UArray2b_height (T array2b);

/* A getter function to return the size of an individual element that is being
*  stored in each index/cell of the 2d array */
extern int UArray2b_size (T array2b);

/* A getter function to return the blocksize that has been set for the 
*  blocked 2d array to be partitioned in terms of its width and height*/
extern int UArray2b_blocksize(T array2b);

/* return a pointer to the cell in the given column and row.
* index out of range is a checked run-time error
*/
extern void *UArray2b_at(T array2b, int column, int row);

/* visits every cell in one block (in row-major order within the blocks) 
*  before moving to the next block (in row-major order)*/
extern void UArray2b_map(T array2b, void apply(int col, int row, T array2b,
                            void *elem, void *cl), void *cl);


#undef T
#endif
