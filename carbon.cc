#include "carbon.h"
#include "scene.h"


void c_path_tracer(uint32_t *img, uint32_t w, uint32_t h, int samps)
{
  vec3d c = vec3d(0, 0, 0);
  double t;

  cam cam;
  cam.init(w, h);

  c_sphere sp = c_sphere {.5, vec3d(0, 0, 1), vec3d(255, 255, 255), vec3d(10, 10, 10), SOLID };

  for (int j = 0; j < h; ++j) {
    fprintf(stderr,"\rRendering (%d spp) %5.2f%%", cam.samples_per_pixel*4, 100.* j/(h-1));
    for (int i = 0; i < w; ++i) {
      for (int s= 0; s < cam.samples_per_pixel; ++s) {
        c_ray r = cam.get_ray(i, j);
        t = sp.intersect(r);
        if (t != -1.0)
          c = c + sp.color;
      }
      c = c / cam.samples_per_pixel;
      img[j*w + i] = C_RGBA((int) c.x, (int) c.y, (int) c.z, 255);
      c = vec3d(0, 0, 0);
    }
  }
}
