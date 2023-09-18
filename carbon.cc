#include "carbon.h"

vec3 radiance(c_ray ray, int isect, int t)
{
}

void c_path_tracer(uint32_t *img, uint32_t w, uint32_t h, int samps)
{
  for (int i = 0; i < w*h; ++i) {
    vec3 c(200, 200, 200);
    /* for (int k = 0; k < samps; ++k) { */
    /* } */
    /* c = c / samps; */
    img[i] = C_RGBA((int) c.x, (int) c.y, (int) c.z, 255);
  }

  /* ray.find_nearest_obj(); */
  /* if (ray.hit == false) { */
  /*   IBUF[i_idx] = 0x0; */
  /* } */

/* for each pixel(i,j) do */
/*   Vec3 C = 0 */
/*   for (k=0; k < samplesPerPixel; k++) do */
/*     Create random ray in pixel: */
/*       Choose random point on len P_lens */
/*       Choose random point on image plane P_images */
/*       D = normalize(P_image - P_lens) */
/*       Ray ray = Ray(P_lens, D) */
/*     castRay(ray, isect) */
/*     if ray hits something then */
/*       C += radiance(ray, iscect, 0) */
/*     else */
/*       C += backgroundColor(D) */ 
/*     end if */
/*   end for */ 
/*   image(i,j) = C/samplesPerPixel */
/* end for */
}
