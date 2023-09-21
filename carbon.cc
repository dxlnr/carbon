#include "carbon.h"
#include "scene.h"


void c_path_tracer(uint32_t *img, uint32_t w, uint32_t h, int samps)
{
  vec3d c;

  cam camera;
  camera.init(w, h);

  c_sphere sp = c_sphere {.5, vec3d(0, 0, 1), vec3d(255, 255, 255), vec3d(10, 10, 10), SOLID };

  for (int j = 0; j < h; ++j) {
    for (int i = 0; i < w; ++i) {
      c_ray r = camera.get_ray(i, j);
      double t = sp.intersect(r);

      if (t != -1.0) {
        c = sp.color;
      } else {
        c = vec3d(0, 0, 0);
      }
      img[j*w + i] = C_RGBA((int) c.x, (int) c.y, (int) c.z, 255);
    }
  }
}
