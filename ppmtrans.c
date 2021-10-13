#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assert.h"
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
    A2Methods_UArray2 rotated; /*represents the array being rotatedI*/
    A2Methods_T methods; /*These are the methods that act on it*/
} cl_methods_arr;

A2Methods_UArray2 rotate(char *time_file_name, int degrees,
                         A2Methods_T methods, A2Methods_mapfun *map,
                         Pnm_ppm *pixmap, bool* zero_degrees);
                         
A2Methods_UArray2 actualRotation(int width,
                            int height,
                            A2Methods_mapfun *map,
                            cl_methods_arr *methodsAndArray,
                            Pnm_ppm *pixmap,
                            char *time_file_name,
                            void (*rotate_func) (int col, int row,
                                                A2Methods_UArray2 array2,
                                                void *elem,
                                                void *methodsAndArray)
                             );
                             
void rotate90(int col, int row, A2Methods_UArray2 array2, void *elem,
              void *methodsAndArray);

void rotate180(int col, int row, A2Methods_UArray2 array2, void *elem,
              void *methodsAndArray);

void printTime(char *time_file_name, double duration, int num_pixels, 
             int width, int height);
FILE *get_file_to_read(const char *file_name);




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
        int   rotation       = 0;  //the number of degrees we are rotating
        int   i;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; //methods 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; //mapping function
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

    FILE* fp = get_file_to_read(argv[i]);
    assert(fp);
    /* Read a file using the given methods and return a pixmap
     * containing a 2D array of the type returned by 'methods->new'.
     * 'methods' field of the result is the same as the argument.
     */
    
    Pnm_ppm pixmap = Pnm_ppmread(fp, methods);
    /* A2Methods_UArray2 is the type of pixmap->pixels
    */
    bool zero_degrees = false;
    pixmap->pixels = rotate(time_file_name, rotation, methods, map,
                            &pixmap, &zero_degrees);
    if (zero_degrees == true) {
        // printf("zero degrees\n");
        CPUTime_T timer = CPUTime_New();
        double time_used;

        CPUTime_Start(timer);
        pixmap->pixels = rotate(time_file_name, rotation, methods, map,
                                &pixmap, &zero_degrees);
        time_used = CPUTime_Stop(timer);
        printTime(time_file_name, time_used, pixmap->width * 
                 pixmap->height, pixmap->width, pixmap->height);
        CPUTime_Free(&timer);        
    }
    
    Pnm_ppmwrite(stdout, pixmap);
    fclose(fp);
    Pnm_ppmfree(&pixmap);
    exit(0);
}

FILE *get_file_to_read(const char *file_name)
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

/*
* Call the actualRotation function with the associated function pointer
* (i.e.) rotate90 if the rotation we're executing is 90 degrees.
* C.R.E.: if methods is NULL, if map is NULL, or if pixmap is NULL
* Memory allocation: for the cl_methods_arr struct. freed in actualRotation.
*/
A2Methods_UArray2 rotate(char *time_file_name, int degrees,
                         A2Methods_T methods, A2Methods_mapfun *map,
                         Pnm_ppm *pixmap, bool* zero_degrees)
{
    assert(methods != NULL && map != NULL && pixmap != NULL);
    
    //getting the width and height of the pixelmap
    int width = (*pixmap)->width;
    int height = (*pixmap)->height;

    /*
    * Struct used only for mapping (closure variable)
    * Holds the array that will be written to in the mapping functions (i.e.
    * stores the rotated image) and the methods set in main
    */
    cl_methods_arr *methodsAndArray = malloc(sizeof(struct cl_methods_arr));
    assert(methodsAndArray != NULL);
    methodsAndArray->methods = methods;


    if (degrees == 90) {
        return actualRotation(width, height, map, methodsAndArray, pixmap,
                             time_file_name, rotate90);
    } 
    else if (degrees == 180) {
        return actualRotation(height, width, map, methodsAndArray, pixmap,
                             time_file_name, rotate180);
    } 
    // else if (degrees == 270) {
    //     return actualRotation(width, height, map, methodsAndArray, pixmap,
    //                          time_file_name, rotate270);
    // 
    // }
    *zero_degrees = true;
    return (*pixmap)->pixels; /* if rotation == 0 return original array*/
}

/*
* Call mapping functions, use CPUTime_T to measure how long it takes to
* rotate or flip images
* C.R.E.: if any of the following are NULL: map, methodsAndArray struct,
*         pixmap, or rotate_func (function pointer)
* Memory allocation: for the CPU timer, freed in this function, and the new
*        array that will store the final rotated image, freed in this function
*        along with the rest of the struct
*/
A2Methods_UArray2 actualRotation(int width,
                               int height,
                               A2Methods_mapfun *map, //which mapping function we are going to use
                               cl_methods_arr *methodsAndArray, //holds output
                               Pnm_ppm *pixmap,
                               char *time_file_name,
                               void (*rotate_func) (int col, int row,
                                                   A2Methods_UArray2 array2,
                                                   void *elem,
                                                   void *methodsAndArray)
                                )
{
    assert(map != NULL && methodsAndArray != NULL && pixmap != NULL 
           && rotate_func != NULL);
    CPUTime_T timer = CPUTime_New();
    double time_used;

    //setting it to the correct type of array (uarray2 or uarray2b)
    //separated into three lines for readability
    A2Methods_T methods = methodsAndArray->methods;
    //creates a 2D array of the pixmap 
    A2Methods_UArray2 rotated_pixmap = methods->new(height, width, 
                                (sizeof (struct Pnm_rgb)));
    
    methodsAndArray->rotated = rotated_pixmap;

    CPUTime_Start(timer);
    map((*pixmap)->pixels, rotate_func, methodsAndArray); //calling the mapping function we passed in with the correct function pointer
    time_used = CPUTime_Stop(timer);

    (*pixmap)->width = height;
    (*pixmap)->height = width;

    printTime(time_file_name, time_used, width * height, width, height);

    CPUTime_Free(&timer);
    methods->free(&((*pixmap)->pixels));
    free(methodsAndArray);

    return rotated_pixmap;
}

void rotate90(int col, int row, A2Methods_UArray2 array2, void *elem,
              void *methodsAndArray)
{
    assert(array2 != NULL && methodsAndArray != NULL);

    A2Methods_T methods = ((cl_methods_arr *)methodsAndArray)->methods;
    A2Methods_UArray2 rotated = ((cl_methods_arr *)methodsAndArray)->rotated;
    assert(rotated != NULL && methods != NULL);

    int height = methods->height(array2);

    *((Pnm_rgb)methods->at(rotated, height - row - 1, col)) = *((Pnm_rgb)elem);
}

void rotate180(int col, int row, A2Methods_UArray2 array2, void *elem,
              void *methodsAndArray)
{
    assert(array2 != NULL && methodsAndArray != NULL);

    A2Methods_T methods = ((cl_methods_arr *)methodsAndArray)->methods;
    A2Methods_UArray2 rotated = ((cl_methods_arr *)methodsAndArray)->rotated;
    assert(rotated != NULL && methods != NULL);

    int height = methods->height(array2);
    int width = methods->width(array2);

    *((Pnm_rgb)methods->at(rotated, width - col - 1,  height - row - 1)) 
                                                            = *((Pnm_rgb)elem);
}

/*
* Intializes a file pointer for timing output file and prints duration of
* mapping the entire image and of one pixel in nanoseconds.
* Memory allocation: file pointer created for timing file, freed in scope
*/
void printTime(char *time_file_name, double duration, int num_pixels, 
               int width, int height)
{
    if (time_file_name != NULL) {
        FILE *time_fp = NULL;
        time_fp = fopen(time_file_name, "w");
        fprintf(time_fp, "height: %d \n", width);
        fprintf(time_fp, "widtht: %d \n", height);
        fprintf(time_fp, "Transformation completed in %.0f nanoseconds\n",
                duration);
        fprintf(time_fp, "Each pixel mapped in average %.0f nanoseconds\n",
                duration / num_pixels);
        fclose(time_fp);
    }
}


//clock rate = cpu mhz
