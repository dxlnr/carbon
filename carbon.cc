#include "carbon.h"
#include "scene.h"


void c_path_tracer(uint32_t *img, uint32_t w, uint32_t h, int samps)
{
  vec3d c;

  double focal_l = 1.0;
  c_ray cam = c_ray(vec3d(0, 0, 0), vec3d(0, 0, 0));

  vec3d vp_u = vec3d(2.0 * (static_cast<double> (w) / h), 0, 0);
  vec3d vp_v = vec3d(0, -2.0, 0);
  vec3d vp_up_left = cam.o - vec3d(0, 0, focal_l) - vp_u / 2 - vp_v / 2;
  vec3d p0 = vp_up_left + (vp_u / w + vp_v / h) * 0.5;

  c_sphere sp = c_sphere {.5, vec3d(0, 0, 1), vec3d(255, 255, 255), vec3d(10, 10, 10), SOLID };

  for (int j = 0; j < h; ++j) {
    for (int i = 0; i < w; ++i) {
      vec3d pc = p0 + (vp_u / w * i) + (vp_v / h * j);
      vec3d rdir = pc - cam.o;

      c_ray r(cam.o, rdir);
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
