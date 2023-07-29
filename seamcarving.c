#include "seamcarving.h"
#include <stdio.h>
#include <math.h>

/*
struct rgb_img{
    uint8_t *raster;
    size_t height;
    size_t width;
};
*/


int y_diff(struct rgb_img *im, int y, int x)
{
  //image 256 x 256 == (0-255, 0-255)
  //*1: if at edge go to other side 
  //*2: if in middle do normal top bot left right 
  int height = (im->height) -1 ;
  int width = (im->width) - 1;
  int ry_diff;
  int gy_diff;
  int by_diff;
  //!y diffs
  if (y == 0)
  {
    // uint8_t get_pixel(struct rgb_img *im, int y, int x, int col){
    ry_diff = get_pixel(im, height, x, 0) - get_pixel(im, 1, x, 0);
    gy_diff = get_pixel(im, height, x, 1) - get_pixel(im, 1, x, 1);
    by_diff = get_pixel(im, height, x, 2) - get_pixel(im, 1, x, 2);
  }
  else if (y == height)
  {
    ry_diff = get_pixel(im, 0, x, 0) - get_pixel(im, height-1, x, 0);
    gy_diff = get_pixel(im, 0, x, 1) - get_pixel(im, height-1, x, 1);
    by_diff = get_pixel(im, 0, x, 2) - get_pixel(im, height-1, x, 2);
  }
  else
  {
    ry_diff = get_pixel(im, y-1, x,0) - get_pixel(im, y+1, x, 0);
    gy_diff = get_pixel(im, y-1, x, 1) - get_pixel(im, y+1, x, 1);
    by_diff = get_pixel(im, y-1, x, 2) - get_pixel(im, y+1, x, 2);
  }
  int delta_y2 = ry_diff * ry_diff + gy_diff * gy_diff + by_diff * by_diff;
  return delta_y2; //Δ2y(y,x)=Ry(y,x)2+Gy(y,x)2+By(y,x)2
}

int x_diff(struct rgb_img *im, int y, int x)
{
  //image 256 x 256 == (0-255, 0-255)
  //*1: if at edge go to other side 
  //*2: if in middle do normal top bot left right 
  int height = (im->height) -1 ;
  int width = (im->width) - 1;
  int rx_diff;
  int bx_diff;
  int gx_diff;
  //!y diffs
  if (x == 0)
  {
    // uint8_t get_pixel(struct rgb_img *im, int y, int x, int col){
    rx_diff = get_pixel(im, y, width, 0) - get_pixel(im, y, 1, 0);
    gx_diff = get_pixel(im, y, width, 1) - get_pixel(im, y, 1, 1);
    bx_diff = get_pixel(im, y, width, 2) - get_pixel(im, y, 1, 2);
  }
  else if (x == width)
  {
    rx_diff = get_pixel(im, y, 0, 0) - get_pixel(im, y, width-1, 0);
    gx_diff = get_pixel(im, y, 0, 1) - get_pixel(im, y, width-1, 1);
    bx_diff = get_pixel(im, y, 0, 2) - get_pixel(im, y, width-1, 2);
  }
  else
  {
    rx_diff = get_pixel(im, y, x- 1,0) - get_pixel(im, y, x+1, 0); //prev column column ahead 
    gx_diff = get_pixel(im, y, x- 1,1) - get_pixel(im, y, x+1, 1);
    bx_diff = get_pixel(im, y, x- 1,2) - get_pixel(im, y, x+1, 2);
  }
  int delta_x2 = rx_diff*rx_diff + bx_diff*bx_diff + gx_diff * gx_diff;
  return delta_x2;
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad)
{
  // row, col = (y,x)
  int y = 0;
  int x = 0;
  int height = im->height;
  int width = im->width;
  uint8_t energy;
  create_img(grad, height, width);
  for (y; y < height; y++)
  {
    for (x; x < width; x++)
    {
      double delta_x2 = (double)x_diff(im, y, x);
      double delta_y2 = (double)y_diff(im, y , x); // sqrt takes in double input
      energy = (uint8_t)(sqrt(delta_x2 + delta_y2)/10);
      set_pixel(*grad,y,x,energy, energy, energy);
    }
  }

}

int dynamic_min(struct rgb_img *grad, double *best_arr, int y, int x)
{
  /*
  returns the min option for a given coordinate by looking at 3 cases:
  1. edge @ x = 0 
  2. edge at x = width
  3. normal*/
  int width = grad->width;
  if (x- 1 == -1) //if at edge just need to see middle and right side 
  {
    if (best_arr[(y-1)*width + 1] < best_arr[(y-1)*width])
    {
      return best_arr[(y-1)*width + 1]; //case where moving to the right is better
    }
    else
    {
      return best_arr[(y-1)*width]; //case where middle lane is better
    }
  }
  else if (x == width -1)
  {
    if (best_arr[(y-1)*width + width - 1 ] < best_arr[(y-1)*width +width -2])
    {
    return best_arr[(y-1)*width + width -1]; //case where staying in middle lane is better
    }
    else
    {
    return best_arr[(y-1)*width+width-2]; //case where moving to left is better
    }
  }
  else
  {
    if (best_arr[(y-1)*width + x] < best_arr[(y-1)*width + x-1]) //if staying in middle better than moving left 
    {
      if (best_arr[(y-1)*width + x] < best_arr[(y-1)*width + x + 1]) //if staying in middle better than moving right 
      {
        return best_arr[(y-1)*width + x];                               //return middle MLR/MRL
      }
      else                                                           //if moving right better than/equal staying middle
      {
        return best_arr[(y-1)*width + x + 1];                           //return right RML
      }
    }
    else if (best_arr[(y-1)*width + x] > best_arr[(y-1)*width + x-1]) 
                                                                //if moving left better than staying in middle 
    {
      if (best_arr[(y-1)*width+x-1] < best_arr[(y-1)*width+x+1])     //if staying left better than moving right
      {
        return best_arr[(y-1)*width + x -1];                             //return left LMR/LRM
      }
      else                                                           //if moving right better than/equal staying left
      {
        return best_arr[(y-1)*width + x +1];                              //return right RLM
      }
    }//!dont forget to think about equality
    else if (best_arr[(y-1)*width + x + 1] < best_arr[(y-1)*width + x])
                                                                //if moving right better than middle
    {
      if (best_arr[(y-1)*width+x+1] < best_arr[(y-1)*width+x-1])     //if staying right better than moving left
      {
        return best_arr[(y-1)*width+x+1];                                 //return right RLM/RML
      }
      else                                                          //if moving left better than/equal staying right
      {
        return best_arr[(y-1)*width+x-1];                                 //return left LRM      
      }
    }
    else                                                       //equality cases between LM, MR, LMR
    {
      if (best_arr[(y-1)*width + x] == best_arr[(y-1)*width + x -1])
      {
        return best_arr[(y-1)*width+x];                                   //return middle if M = L
      }
      if (best_arr[(y-1)*width+x] == best_arr[(y-1)*width+x+1])           //return middle if M = R
      {
        return best_arr[(y-1)*width+x];
      }
    }
  }
} 

void dynamic_seam(struct rgb_img *grad, double **best_arr)
{
  /*
  Problem: find the min energy path (sum of energies
at nodes) from top to bottom
Subproblem: min. energy path
from any top node to node (j, i)
𝑂𝑃𝑇 𝑗, 𝑖 = 𝐸 𝑗, 𝑖 + min𝑖′∈{𝑖−1,𝑖,𝑖+1} 𝑂𝑃𝑇((𝑗 − 1, 𝑖′)ji
(Need to take care of edge cases where the seam is near the boundary 
*/

  int height = grad->height;
  int width = grad->width;
  *best_arr = (double *)malloc(height * width * sizeof(double));//256 x 256 array given grad is a 256 x 256 picture 0-255 
  int y = 0;
  int x = 0;
// (*best_arr)[i*width+j] contains the minimum cost of a seam from the top of grad to the point 
// basically best way to get to specific coord from top down 
//*1. dual gradient image 
//*2. dynamic program from top down 
  for (y; y < height; y++) //row
  {
    for (x; x < width; x++) //col
    {
      // base case: row 0, starting from row 0 each column goes down 
      if (y == 0)
      { //grad gotten from last part 
        (*best_arr)[x] = get_pixel(grad, y, x, 0); //if y = 0, looking at first row and fills out the first row
        //if not first row need to find the least energy path from left right middle paths 
      }
      else //if y not 0, need to find min path
      {
        // (*best_arr)[i*width+j] need to work laterally for x-1, x, x+1
        // if (x-1 == -1) //edge of pic @ x = 0 (column 0)
        // {
        //   break;
        // }
       (*best_arr)[y*width+x] = (double)get_pixel(grad, y, x, 0) + dynamic_min(grad, *best_arr, y, x); //energy of 
      }
    }
  }
}

void recover_path(double *best, int height, int width, int **path) //best total, best path array
//This function allocates a path through the minimum seam as defined by the array best //y,x row col height width
//best is array from prev function, path is the list u make
/* (5 rows, 6 columns) --> (0-4, 0-5) height: 5, width: 6 
24.000000       22.000000       30.000000       15.000000       18.000000       19.000000 
34.000000       45.000000       30.000000       38.000000       25.000000       33.000000
45.000000       43.000000       52.000000       38.000000       46.000000       39.000000
56.000000       58.000000       55.000000       66.000000       57.000000       60.000000
73.000000       72.000000       62.000000       82.000000       77.000000       76.000000
*/
{
  *path = (int *)malloc(height * sizeof(int));
  double bot_min = 1.0/0.0;
  int y = height-1; //y = 4
  int x = 0; //x = 0
  int x_bot;
  int temp_x;

  
for (y; y > -1; y--)
  {
  double temp = 1.0/0.0;

  for (x; x < width; x++) // starts at y = 255, x = 0 goes until y= 0, x = 256 
    {
      if (y == height-1)    //should only run 1 time in first loop when y = 255
      {
        if (best[y*width+x] < bot_min)      //if the x index in the last row is less than the current min
        {
          bot_min = best[y*width+x];        //set the current min to that min
          x_bot =  x;                       //then set the current x_bot to that x index value
        }
        if (x == width-1)                   //if you have ran through the entire row
        {
        (*path)[y] = x_bot;                 //then set the array value of y (in first case this should be height-1), to the x_bot value which
                                            //is the index of the lowest energy value     
        }
      }
      else                                  //if not the first time you are looping
      {
        if (best[y*width+x] < temp && x_bot - x <= 1 && x_bot - x >= -1) //if the index value of that row of x is less than the current min
                                                                        // and the x value of that row is within 1 of the x value of the previous row 
        {
          temp = best[y*width+x];                                        // set the temp then to the value at that index
          temp_x = x;                                                    // set the current x index value to that index too                                        
        }
        if (x==width-1)                                                  //once you have run through the entire row 
        {
          (*path)[y] = temp_x;                                             //set the x value in tha array to the current and now final x_bot index 
          x_bot = temp_x;
        }
      }
    }
  }  

}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path);


