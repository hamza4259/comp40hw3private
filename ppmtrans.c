/*
 * 
 * Comp40 Homework 3
 * Hamza Ali (hali03) and Pamela Melgar (pmelga01)
 * ppmtrans.c
 * 
 * Edited: October 13 2021
 * Purpose: To perform an indicated rotation on in image in either row-major, 
 *          column-major, or blocked-major transformations. This program 
 *          aims to use polymorphism to implement the proper methods for 
 *          some-major transformation
 *
 */
 
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"

/*
* Struct used only for mapping (closure variable)
* Holds the array that will be written to in the mapping functions (i.e.
* stores the rotated image) and the methods set in main
*/
typedef struct cl_methods_arr{
    A2Methods_UArray2 rotated; /* Array being rotated */
    A2Methods_T methods; /* Methods to be used on rotated array */
} cl_methods_arr;

/* Function declarations */
A2Methods_UArray2 decide_transformation(char *time_file_name, int degrees,
                                  A2Methods_T methods, A2Methods_mapfun *map,
                                  Pnm_ppm *pixmap, bool* zero_degrees, 
                                  bool* time_called);
                         
A2Methods_UArray2 transformation(int width,
                            int height,
                            A2Methods_mapfun *map,
                            cl_methods_arr *methods_and_arr,
                            Pnm_ppm *pixmap,
                            char *time_file_name,
                            void (*rotate_func) (int col, int row,
                                                A2Methods_UArray2 array2,
                                                void *elem,
                                                void *methods_and_arr),
                            bool* time_called
                             );
                             
void rotate90(int col, int row, A2Methods_UArray2 array2, void *elem,
              void *methods_and_arr);

void rotate180(int col, int row, A2Methods_UArray2 array2, void *elem,
               void *methods_and_arr);

void rotate270(int col, int row, A2Methods_UArray2 array2, void *elem,
               void *methodsAndArray);
               
void output_time(char *time_file_name, double duration, int num_pixels, 
                 int width, int height);
                 
FILE *get_file(const char *file_name);




#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (0)

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}


int main(int argc, char *argv[]) 
{
        char *time_file_name = NULL;
        int   rotation       = 0; 
        int   i;
        bool  time_called = false;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default;
        assert(map);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-time") == 0) {
                        time_file_name = argv[++i];
                        time_called = true;
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        break;
                }
        }
    /*Read in a file using the given methods and return a 2D array raster 
      of type indicated by 'methods->new'*/
    FILE* fp = get_file(argv[i]);
    assert(fp);
    
    Pnm_ppm pixmap = Pnm_ppmread(fp, methods); /*read input image*/
    assert(pixmap != NULL);
    bool zero_degrees = false;
    pixmap->pixels = decide_transformation(time_file_name, rotation, methods, 
                                           map, &pixmap, &zero_degrees, 
                                           &time_called);

    /*special case for 0 degree rotation: raster should stay the same */
    if (zero_degrees == true && time_called == true) {
        CPUTime_T timer = CPUTime_New();
        double time_used;
        CPUTime_Start(timer);
        pixmap->pixels = decide_transformation(time_file_name, rotation, 
                                        methods, map, &pixmap, &zero_degrees,
                                        &time_called);
        time_used = CPUTime_Stop(timer);
        output_time(time_file_name, time_used, pixmap->width * 
                 pixmap->height, pixmap->width, pixmap->height);
        CPUTime_Free(&timer);        
    }
    
    /*Print transformed pixmap (image raster) to standard out*/
    Pnm_ppmwrite(stdout, pixmap);
    fclose(fp);
    Pnm_ppmfree(&pixmap);
    exit(0);
}



/* get_file_to_read
* Purpose: This function takes in a "string" that corresponds to the 
*          potential name of a file (a ppm image) indicated on the command
*          line to be opened and read with 'file'. If the name of a file was
*          not indicated on the command line during excution of the program,
*          the FILE pointer 'file' will be set to standard input. 
* Parameters: const char *file_name - a character pointer pointing to the 
*               name of a file indicated on the commmand line. If there was 
*               no file indicated, then the pointer will point to 
*               NULL, otherwise it will point to the first character of the 
*               name of the file. 
* Effects: The program will terminate in a checked runtime error if the 
*          file to be read was supplied but has a problem opening. 
* Returns: a FILE pointer that will point to the beginning of the input 
*          stream whether it is the supplied file or standard input
*
*/
FILE *get_file(const char *file_name)
{
    FILE *file;
    if (file_name != NULL) {
        file = fopen(file_name, "r");
        assert(file != NULL);
    } else {
        file = stdin;
    }
    
    return file;
}



/* decide_transformation
* Purpose: An auxillary function that will take in all the necessary 
*          information needed to perform a transformation on image 
*          with the indicated map (row/col/block-major) and rotation directed
*          by the int degrees argument.
* Parameters: char *time_file_name - a pointer to the name of the 
*               file that will store the timing data of the duration of a 
*               transformation using an indicated map type
*             int degrees - the rotation number in degrees that the user 
*               wants to transform an image, indicated at the execution 
*               time of the program
*             A2Methods_T methods - Stores the correct type of methods 
*               that will act on the 2D array raster transformation
*             A2Methods_mapfun *map - stores the proper mapping operation 
*               as will have likely been indicated at execution time 
*               of the program, if not is set to default, but is necessary 
*               in order to know how the degree transformation should be 
*               performed on the pixmap  
*             Pnm_ppm *pixmap - The PNM data (including the 2D raster)
*               that is storing the original image in the proper method type
*             bool *zero_degrees - If the degree rotation is not 90 or 180, 
*               then the 0 degree rotation will be checked and it will 
*               enter this function again to time how long it takes to return
*               the orignal pixmap (aka no rotation).       
* Effects: The program will terminate in a checked runtime error if the 
*            indicated method is NULL, the map type is NULL or the 2D image 
*            raster (pixmap) is NULL.
*          The program will terminate in a cheked runtime error if there 
*            is a problem allocating memory for a cl_methods_arr struct that
*            will hold the proper methods as well as the array of the 
*            raster being rotated.
* Returns: A2Methods_UArray2 - The 2D raster containing the tranformed image
*
*/
A2Methods_UArray2 decide_transformation(char *time_file_name, int degrees,
                         A2Methods_T methods, A2Methods_mapfun *map,
                         Pnm_ppm *pixmap, bool* zero_degrees, 
                         bool* time_called)
{
    assert(methods != NULL && map != NULL && pixmap != NULL);
    
    int width = (*pixmap)->width;
    int height = (*pixmap)->height;

    /* Struct used only for mapping (closure variable)*/
    cl_methods_arr *methods_and_arr = malloc(sizeof(struct cl_methods_arr));
    assert(methods_and_arr != NULL);
    methods_and_arr->methods = methods;


    if (degrees == 90) {
        return transformation(width, height, map, methods_and_arr, pixmap,
                             time_file_name, rotate90, time_called);
    } 
    else if (degrees == 180) {
        return transformation(height, width, map, methods_and_arr, pixmap,
                             time_file_name, rotate180, time_called);
    } 
    else if (degrees == 270) {
         return transformation(width, height, map, methods_and_arr, pixmap,
                              time_file_name, rotate270, time_called);
     }
    *zero_degrees = true;
    return (*pixmap)->pixels;
}



/* transformation
* Purpose: This function performs the actual transformation on the original 
*          image using the proper method type with the proper mapping 
*          function. This function uses modularity in the sense that 
*          it can perform any rotation so long as it is provided in a valid 
*          function pointer that will be passed into the mapping function 
*          as an "apply function". This function also takes care of timing 
*          how long the mapping takes to execute for the image transformation 
*          to be complete. The CPU timer memory is alliocated and freed.
* Parameters: int width - the width (col #s) of the original image raster
*             int height - the height (row #s) of the original image raster
*             A2Methods_mapfun *map - indicates the mapping function that 
*               will be used to perform the tranformation
*             cl_methods_arr - a struct that will be passed into the 
*               mapping function and will store the final output, aka the 
*               the transformed array
*             Pnm_ppm *pixmap - Will hold the data for the image input
*             char *time_file_name - the name of the file that will store 
*               the CPU timing data to measure the duration of the 
*               transformation with some indicated mapping type
*             void (*rotate_func) - a function pointer that represents 
*               the apply function that will be passed into the mapping 
*               function to perform some change on every pixel of the raster.
*               The rotate_func supplied will handle the nitty gritty 
*               transformation, defined for each degree transformation outside
* Effects: The program will terminate in a checked-runtime-error if the 
*          map provided is NULL, methods_and_arr struct pointer is NULL, 
*          the supplied Pnm_ppm pixmap pointer is NULL, or the function 
*          pointer is invalid, points to NULL.
* Returns: a FILE pointer that will point to the beginning of the input 
*          stream whether it is the supplied file or standard input
*
*/
A2Methods_UArray2 transformation(int width,
                               int height,
                               A2Methods_mapfun *map, 
                               cl_methods_arr *methods_and_arr,
                               Pnm_ppm *pixmap,
                               char *time_file_name,
                               void (*rotate_func) (int col, int row,
                                                   A2Methods_UArray2 array2,
                                                   void *elem,
                                                   void *methods_and_arr),
                                bool* time_called
                                )
{
    assert(map != NULL && methods_and_arr != NULL && pixmap != NULL 
           && rotate_func != NULL);
    CPUTime_T timer = CPUTime_New();
    double time_used;

    /*setting it to the correct type of array (uarray2 or uarray2b)*/
    A2Methods_T methods = methods_and_arr->methods;
    
    /*creates a 2D array of the image raster */
    A2Methods_UArray2 rotated_pixmap = methods->new(height, width, 
                                (sizeof (struct Pnm_rgb)));
    
    methods_and_arr->rotated = rotated_pixmap;

    /*begin and end the timer for timing data*/
    CPUTime_Start(timer);
    
    map((*pixmap)->pixels, rotate_func, methods_and_arr);
    
    time_used = CPUTime_Stop(timer);
    
    /* if 90 degrees or 270 degrees then swap dimensions */
    /* but for 180 degrees the dimensions stay the same so it is handled
    by the function call */
    (*pixmap)->width = height;
    (*pixmap)->height = width;
    if (*time_called == true) {
        output_time(time_file_name, time_used, width * height, width, height);
    }

    CPUTime_Free(&timer);
    methods->free(&((*pixmap)->pixels));
    free(methods_and_arr);

    return rotated_pixmap;
}



/* rotate90
* Purpose: An "apply" function that will be passed into a mapping function 
*          as a function pointer to change the pixel of an image 
*          into the pixel it represents after a 90 degree rotation of the 
*          entire image. 
* Parameters: int col - the current COLUMN index of the pixmap 2D-array
*             int row - the current ROW index of the pixmap 2D-array 
*             A2Methods_UArray2 array2 - the 2D array that stores the data 
*               of the image raster (such as width and height)
*             void *elem - represents the current element or pixel 
*               of the 2D image raster that will be transformed
*             void *methods_and_arr - represents the closure in this 
*               apply function that essentially stores the transformed array 
*               and is repeatedly being passed into this function and 
*               updated until it holds the completed transformed image. It 
*               also stores the information for the method type of the 
*               transformation.
* Effects: The program will terminate in a checked runtime error if the 
*          array2 representing the 2d pixel image raster is NULL or if 
*          the struct pointer containing the methods and the transformed 
*          array is passed in as a pointer to NULL.
* Returns: nothing - void
*
*/
void rotate90(int col, int row, A2Methods_UArray2 array2, void *elem,
              void *methods_and_arr)
{
    assert(array2 != NULL && methods_and_arr != NULL);

    A2Methods_T methods = ((cl_methods_arr *)methods_and_arr)->methods;
    A2Methods_UArray2 rotated = ((cl_methods_arr *)methods_and_arr)->rotated;
    assert(rotated != NULL && methods != NULL);

    int height = methods->height(array2);
    int x_coord = height - row - 1;
    int y_coord = col;
    
    *((Pnm_rgb)methods->at(rotated, x_coord, y_coord)) = *((Pnm_rgb)elem);
}


/* rotate180
* Purpose: An "apply" function that will be passed into a mapping function 
*          as a function pointer to change the pixel of an image 
*          into the pixel it represents after a 180 degree rotation of the 
*          entire image. 
* Parameters: int col - the current COLUMN index of the pixmap 2D-array
*             int row - the current ROW index of the pixmap 2D-array 
*             A2Methods_UArray2 array2 - the 2D array that stores the data 
*               of the image raster (such as width and height)
*             void *elem - represents the current element or pixel 
*               of the 2D image raster that will be transformed
*             void *methodsAndArray - represents the closure in this 
*               apply function that essentially stores the transformed array 
*               and is repeatedly being passed into this function and 
*               updated until it holds the completed transformed image. It 
*               also stores the information for the method type of the 
*               transformation.
* Effects: The program will terminate in a checked runtime error if the 
*          array2 representing the 2d pixel image raster is NULL or if 
*          the struct pointer containing the methods and the transformed 
*          array is passed in as a pointer to NULL.
* Returns: nothing - void
*
*/
void rotate180(int col, int row, A2Methods_UArray2 array2, void *elem,
              void *methods_and_arr)
{
    assert(array2 != NULL && methods_and_arr != NULL);

    A2Methods_T methods = ((cl_methods_arr *)methods_and_arr)->methods;
    A2Methods_UArray2 rotated = ((cl_methods_arr *)methods_and_arr)->rotated;
    assert(rotated != NULL && methods != NULL);

    int height = methods->height(array2);
    int width = methods->width(array2);

    int x_coord = width - col - 1;
    int y_coord = height - row - 1;
    
    *((Pnm_rgb)methods->at(rotated, x_coord, y_coord)) = *((Pnm_rgb)elem);
}



/* rotate270
* Purpose: An "apply" function that will be passed into a mapping function 
*          as a function pointer to change the pixel of an image 
*          into the pixel it represents after a 270 degree rotation of the 
*          entire image. 
* Parameters: int col - the current COLUMN index of the raster 2D-array
*             int row - the current ROW index of the raster 2D-array 
*             A2Methods_UArray2 array2 - the 2D array that stores the data 
*               of the image raster (such as width and height)
*             void *elem - represents the current element or pixel 
*               of the 2D image raster that will be transformed
*             void *methodsAndArray - represents the closure in this 
*               apply function that essentially stores the transformed array 
*               and is repeatedly being passed into this function and 
*               updated until it holds the completed transformed image. It 
*               also stores the information for the method type of the 
*               transformation.
* Effects: The program will terminate in a checked runtime error if the 
*          array2 representing the 2d pixel image raster is NULL or if 
*          the struct pointer containing the methods and the transformed 
*          array is passed in as a pointer to NULL.
* Returns: nothing - void
*
*/


void rotate270(int col, int row, A2Methods_UArray2 array2, void *elem,
              void *methodsAndArray)
{
    assert(array2 != NULL && methodsAndArray != NULL);

    A2Methods_T methods = ((cl_methods_arr *)methodsAndArray)->methods;
    A2Methods_UArray2 rotated = ((cl_methods_arr *)methodsAndArray)->rotated;
    assert(rotated != NULL && methods != NULL);

    int width = methods->width(array2);
    int x_coord = row;
    int y_coord = width - col - 1;

    *((Pnm_rgb)methods->at(rotated, x_coord, y_coord)) = *((Pnm_rgb)elem);
}



/* output_time
* Purpose: This function prints out the time data after the completed
*          image transformation. The data that was acquired from the 
*          CPUTime functions is passed into this function along with 
*          any relevant information about the overall image that was 
*          was transformed to make a conclusion about the 
*          1) Total CPU Time it took for the transformation to complete 
*          2) The AVERAGE CPu Time per pixel the transformation took. 
*          The data is stored into a supplied file and measured in NANOSECONDS
* Parameters: const char *time_file_name - a character pointer pointing to
*               the name of a file that will store the timing data
*             double duration - the time it took for the transformation 
*               to complete (time between CPU Timer starting and stopping)
*             int num_pixels - the total number of pixels inside of the 
*               2D image raster that was transformed
*             int width - the width (# of columns) of the 2D image raster 
*             int height - the height (# of rows) of the 2D image raster
* Effects: The program will not terminate if there was never timing file 
*          supplied when the program executed. instead there will be no 
*          file written to and this function will immediately terminate
* Returns: nothing - only upates a file if one was supplied and writes into it
*
*/
void output_time(char *time_file_name, double duration, int num_pixels, 
               int width, int height)
{
        assert(time_file_name != NULL);
        FILE *time_fp = fopen(time_file_name, "w");
        assert(time_fp != NULL);
        
        fprintf(time_fp, "\nheight: %d \n", width);
        fprintf(time_fp, "widtht: %d \n", height);
        fprintf(time_fp, "Total Time for transformation: %.0f nanoseconds\n",
                duration);
        fprintf(time_fp, "Pixel Average: %.0f nanoseconds\n",
                duration / num_pixels);
                
        /*Close the file after finished writing*/        
        fclose(time_fp);
}