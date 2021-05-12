#include "seamcarving.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Project 2 Begins */
void calc_energy(struct rgb_img *im, struct rgb_img **grad)
{
    /* compute the dual-gradient energy function, and place it in the struct rgb_img *grad. */
    size_t h,w,y,x,x_right,x_left,y_above,y_below;
    double rx, gx, bx, ry, gy, by; /* rbg components of pixels surrounding the central pixel, along the x and y-axis */
    h = im->height;
    w = im->width;
    create_img(grad,h,w);
    for(y=0;y<h;y++) /* row */
    {
        for(x=0;x<w;x++) /* column */
        {
            if (x == 0){
                x_right = x + 1;
                x_left = w - 1;
            }
            else if (x == (w - 1)){
                x_right = 0;
                x_left = x - 1;
            }
            else{
                x_right = x + 1;
                x_left = x - 1;
            }
            
            // special cond for y
            if (y == 0){
                y_above = y + 1;
                y_below = h - 1;
            }
            else if (y == (h - 1)){
                y_below = y - 1;
                y_above = 0;
            }
            else{
                y_above = y + 1;
                y_below = y - 1;
            }
            rx = (double)get_pixel(im,y,x_right,0)-(double)get_pixel(im,y,x_left,0);
            gx = (double)get_pixel(im,y,x_right,1)-(double)get_pixel(im,y,x_left,1);
            bx = (double)get_pixel(im,y,x_right,2)-(double)get_pixel(im,y,x_left,2);
            ry = (double)get_pixel(im,y_above,x,0)-(double)get_pixel(im,y_below,x,0);
            gy = (double)get_pixel(im,y_above,x,1)-(double)get_pixel(im,y_below,x,1);
            by = (double)get_pixel(im,y_above,x,2)-(double)get_pixel(im,y_below,x,2);
            double sum_sqrt = sqrt(pow(rx,2)+pow(gx,2)+pow(bx,2)+pow(ry,2)+pow(gy,2)+pow(by,2));
            uint8_t E = (uint8_t) (sum_sqrt/10);
            set_pixel(*grad,y,x,E,E,E);
        }
    }
}

int min2(int x, int y){return x < y ? x : y;}
int min3(int x, int y, int z){return x < min2(y,z) ? x : min2(y,z);}

int min_index2(int a, int b, int n, double *arr){return (arr)[n+a] <= (arr)[n+b] ? a : b;} /* n = i*width */
int min_index3(int a, int b, int c, int n, double *arr){return (arr)[n+a] <= (arr)[n+min_index2(b,c,n,arr)] ? a : min_index2(b,c,n,arr);}

void dynamic_seam(struct rgb_img *grad, double **best_arr)
{
    int i,j;
    size_t h = grad->height;
    size_t w = grad->width;
    int minVal = -1;
    // printf("HEIGHT*WIDTH: %d\n",h*w);
    *best_arr = (double *)malloc((h*w) * sizeof(double));
    //best_arr = (double **)malloc((h*w) * sizeof(double*));
    //*best_arr = (double *)malloc((h*w) * sizeof(double*));
    for(i=0;i<h;i++) /* row */
    {
        for(j=0;j<w;j++) /* column */
        {
            if(i==0) /* (0,0), (0,1), (0,2) etc */
            {
                (*best_arr)[i*w+j]=(double)get_pixel(grad,i,j,0);
                //printf("%d\t%lf\n",i*w+j,(*best_arr)[i*w+j]);
            }
            else
            {
                if(j==0)
                {
                    //minVal = min2(get_pixel(grad,i-1,j,0),get_pixel(grad,i-1,j+1,0));
                    minVal = min2((*best_arr)[(i-1)*w+j],(*best_arr)[(i-1)*w+j+1]);
                }
                else if(j==w-1)
                {
                    //minVal = min2(get_pixel(grad,i-1,j,0),get_pixel(grad,i-1,j-1,0));
                    minVal = min2((*best_arr)[(i-1)*w+j],(*best_arr)[(i-1)*w+j-1]);
                }
                else
                {
                    //minVal = min3(get_pixel(grad,i-1,j-1,0),get_pixel(grad,i-1,j,0),get_pixel(grad,i-1,j+1,0));
                    minVal = min3((*best_arr)[(i-1)*w+j],(*best_arr)[(i-1)*w+j+1],(*best_arr)[(i-1)*w+j-1]);
                }
                (*best_arr)[i*w+j]=(double)(minVal+get_pixel(grad,i,j,0));
                //printf("%d\t%d\t%d\t%d\t%lf\n",i,j,get_pixel(grad,i,j,0),minVal,(*best_arr)[i*w+j]);
                //printf("%d\t%lf\n",i*w+j,(*best_arr)[i*w+j]);
            }
        }
    }
    // int *path;
    // recover_path(*best_arr,h,w,&path);
}


void recover_path(double *best, int height, int width, int **path)
{
    int i,j,k,m,n;
    double temp_min = INFINITY;
    *path = (int *)malloc(height * sizeof(int));
    for(i=height-1, n=1;i>=0;i--,n++) /* row */
    {
        if(i==height-1) /* find column index of the min of the last row */
        {
            for(j=0;j<width;j++) /* column */
            {
                if(best[i*width+j]<temp_min){temp_min=best[i*width+j];k=j;}
            }
            (*path)[height-n]=k; /* n=1 */
        }
        else
        {
            // printf("K: %d",k);
            if(k==0){m=min_index2(k,k+1,i*width,best);}
            else if(k==width-1){m=min_index2(k,k-1,i*width,best);}
            else{m=min_index3(k-1,k,k+1,i*width,best);}
            k = m;
            (*path)[height-n]=k; /* n:[2,height] */
        }
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path)
{
    int i,j,remove;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    size_t h = src->height;
    size_t w = src->width;
    create_img(dest,h,w-1);
    for(i=0;i<h;i++) /* row */
    {
        remove = path[i];
        // printf("%d\n",remove);
        for(j=0;j<w;j++) /* column */
        {
            if(j<remove)
            {
                r = get_pixel(src,i,j,0);
                g = get_pixel(src,i,j,1);
                b = get_pixel(src,i,j,2);
                set_pixel(*dest,i,j,r,g,b);
            }
            if(j>remove)
            {
                r = get_pixel(src,i,j,0);
                g = get_pixel(src,i,j,1);
                b = get_pixel(src,i,j,2);
                set_pixel(*dest,i,j-1,r,g,b);
            }
        }
    }
}