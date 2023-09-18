#include "carbon.h"


void c_naive_path_tracer(int i_idx, Ray ray, int depth, int MaxDepth=5000)
{
  if (depth >= MaxDepth) {
    IBUF[i_idx] = 0x0;
  }

  /* ray.find_nearest_obj(); */
  if (ray.hit == false) {
    IBUF[i_idx] = 0x0;
  }
}
