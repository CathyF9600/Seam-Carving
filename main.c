/* FILE main.c
 *    Test the functionality of c_img.c implementation for PROJECT 2.
 * Author: Cathy 2021.
 */

/******************************************************************************
 *  Types and Constants.                                                      *
 ******************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "c_img.h"
// #include "c_img.c"

/* FUNCTION main
 *    Run some tests of the functionality of the bst_bag implementation.
 * Parameters and preconditions:  none
 * Return value:  exit status
 * Side-effects:  the main program is executed
 */


int main(void)
{
    char *filename = "6x5.bin";
    struct rgb_img* im;
    struct rgb_img *cur_im;
    read_in_img(&im,filename);

    struct rgb_img *grad;
    calc_energy(im,  &grad);
    print_grad(grad);

    double *best_arr;
    dynamic_seam(grad,&best_arr);
    // for (int i=0;i<im->height*im->width;i++){
    //     printf("%f\t", best_arr[i]);
    // }
    printf("\n");
    
    int *path;
    recover_path(best_arr, grad->height, grad->width, &path);
    for (int j=0;j<im->height;j++){
        printf("%d\t",path[j]);

    }

    remove_seam(im, &cur_im, path);
    char filename2[200];
    sprintf(filename2, "IMAGE.bin");
    write_img(cur_im, filename2);


    destroy_image(im);
    destroy_image(grad);

    return 0;
}
