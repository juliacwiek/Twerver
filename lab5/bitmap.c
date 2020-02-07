#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"


/*
 * Read in the location of the pixel array, the image width, and the image 
 * height in the given bitmap file.
 */
void read_bitmap_metadata(FILE *image, int *pixel_array_offset, int *width, int *height) {

    /* read in location of the pixel array */
    fseek(image, 10, SEEK_SET);
    int error1 = fread(pixel_array_offset, 4, 1, image);
    if (error1 != 1) {
        fprintf(stderr, "Error: reading pixels"); 
    }

    /* read in image width */
    fseek(image, 4, SEEK_CUR);
    int error2 = fread(width, 4, 1, image);
    if (error2 != 1) {
        fprintf(stderr, "Error: reading pixels"); 
    }

    /* read in image height */
    int error3 = fread(height, 4, 1, image);
    if (error3 != 1) {
        fprintf(stderr, "Error: reading pixels"); 
    }
    

}

/*
 * Read in pixel array by following these instructions:
 *
 * 1. First, allocate space for m `struct pixel *` values, where m is the 
 *    height of the image.  Each pointer will eventually point to one row of 
 *    pixel data.
 * 2. For each pointer you just allocated, initialize it to point to 
 *    heap-allocated space for an entire row of pixel data.
 * 3. Use the given file and pixel_array_offset to initialize the actual 
 *    struct pixel values. Assume that `sizeof(struct pixel) == 3`, which is 
 *    consistent with the bitmap file format.
 *    NOTE: We've tested this assumption on the Teaching Lab machines, but 
 *    if you're trying to work on your own computer, we strongly recommend 
 *    checking this assumption!
 * 4. Return the address of the first `struct pixel *` you initialized.
 */
struct pixel **read_pixel_array(FILE *image, int pixel_array_offset, int width, int height) {
    
    /* 1 */
    struct pixel **array = malloc(sizeof(struct pixel*) * height);
    
    /* 2 */
    for (int i = 0; i < height; i++) {
        array[i] = malloc(sizeof(struct pixel) * width);
    }

    /* setting curr position to pixel_array_offset */
    fseek(image, pixel_array_offset, SEEK_SET);
    
    /* 3 */
    for (int a = 0; a < height; a++) {
        for (int b = 0; b < width; b++) {
            int errorA = 0;
            errorA += fread(&(array[a][b].blue), sizeof(unsigned char), 1, image);
            errorA += fread(&(array[a][b].green), sizeof(unsigned char), 1, image);
            errorA += fread(&(array[a][b].red), sizeof(unsigned char), 1, image);
            if (errorA != 3) {
                fprintf(stderr, "Error: reading pixels"); 
            }
        }
    }

    /* 4 */ 
    return array;

}


/*
 * Print the blue, green, and red colour values of a pixel.
 * You don't need to change this function.
 */
void print_pixel(struct pixel p) {
    printf("(%u, %u, %u)\n", p.blue, p.green, p.red);
}
