#include "carbon.h"
#include "scene.h"


void c_path_tracer(uint32_t *img, uint32_t w, uint32_t h, int samps)
{
  double ratio = (double) w / h;
  /* c_ray cam = c_ray(vec3d(w * 0.5, h * 0.5, 100), vec3d(0, -0.4, -1).norm()); */
  
  double focal_l = 1.0;
  double vp_h = 2.0;
  double vp_w = vp_h * (static_cast<double> (w) / h);
  c_ray cam = c_ray(vec3d(0, 0, 0), vec3d(0, 0, 0));

  auto vp_u = vec3d(vp_w, 0, 0);
  auto vp_v = vec3d(0, -vp_h, 0);

  auto pixel_du = vp_u / w;
  auto pixel_dv = vp_v / h;

  vec3d vp_up_left = cam.o - vec3d(0, 0, focal_l) - vp_u/2 - vp_v/2;
  vec3d p00_loc = vp_up_left + (pixel_du + pixel_dv) * 0.5;

  vec3d c;
  c_sphere sp = c_sphere {.5, vec3d(0, 0, 1), vec3d(255, 255, 255), vec3d(10, 10, 10), SOLID };

  for (int j = 0; j < h; ++j) {
    for (int i = 0; i < w; ++i) {
      vec3d pcenter = p00_loc + (pixel_du * i) + (pixel_dv * j);
      vec3d rdir = pcenter - cam.o;

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
