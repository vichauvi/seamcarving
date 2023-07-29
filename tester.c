#include <stdio.h>
#include "c_img.h"
#include "seamcarving.h"

int main()
{
  struct rgb_img im;
  struct rgb_img *grad;
  
  calc_energy(&im,  &grad);
  
  print_grad(grad);
}
