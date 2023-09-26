/* 
 * Copyright 2023 Daniel Illner <illner.daniel@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * */

#include "carbon.h"
#include "scene.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 1280
#define HEIGHT 960

uint32_t IBUF[HEIGHT*WIDTH];

vec3d random_unit_vec_hemisphere(vec3d n)
{
  vec3d u = vec3d(1, 0, 0);
  if (fabs(n.x) > 0.1) {
    u = vec3d(0, 1, 0);
  }
  vec3d v = n.prod(&u);
  u = v.prod(&n);
  double r1 = 2 * M_PI * (double) rand() / RAND_MAX;
  double r2 = (double) rand() / RAND_MAX;
  double r2s = sqrt(r2);
  return u * cos(r1) * r2s + v * sin(r1) * r2s + n * sqrt(1 - r2);
}

int intersect(c_ray ray, c_scene_t *scene, double *t, int *id)
{
  double dt;
  double inf=*t=1e20;

  for (int k = 0; k < scene->num_spheres; ++k) {
    if ((dt = scene->spheres[k].intersect(ray)) && dt < *t) {
      *t = dt;
      *id = k;
    }
  }
  return *t < inf;
}

vec3d radiance(c_ray r, c_scene_t *scene, int depth)
{
  int id = 0;
  double t;

  if (depth >= 5) return vec3d(0, 0, 0);
  if (!intersect(r, scene, &t, &id)) return vec3d(0, 0, 0);

  c_sphere obj = scene->spheres[id];

  /* // Pick a random direction from here and keep going. */
  /* Ray newRay; */
  /* newRay.origin = ray.pointWhereObjWasHit; */
  vec3d nl = (r.o + r.d * t - obj.pos).norm();
  vec3d nd = random_unit_vec_hemisphere(nl);
  c_ray nray = c_ray(nl, nd);

  /* // This is NOT a cosine-weighted distribution! */
  /* newRay.direction = RandomUnitVectorInHemisphereOf(ray.normalWhereObjWasHit); */

  /* Probability of the newRay */
  const double p = 1 / (2 * M_PI);

  if (obj.material == DIFF) {
  }

  /* // Compute the BRDF for this ray (assuming Lambertian reflection) */
  double theta = nray.d.dot(&nl);
  vec3d BRDF = obj.emission / M_PI;

  /* // Recursively trace reflected light sources. */
  vec3d incoming = radiance(nray, scene, depth + 1);
  /* Color incoming = TracePath(newRay, depth + 1); */

  /* // Apply the Rendering Equation here. */
  return obj.emission + (BRDF.mul(&incoming) * theta / p);
  /* vec3d c = vec3d(0, 0, 0); */
  /* return c; */
}

void c_path_tracer(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene)
{
  vec3d c = vec3d(0, 0, 0);
  int id = 0;
  double t;

  cam cam;
  cam.init(w, h);

  for (int j = 0; j < h; ++j) {
    fprintf(stderr,"\rRendering (%d spp) %5.2f%%", cam.samples_per_pixel*4, 100.* j / (h-1));
    for (int i = 0; i < w; ++i) {
      for (int s= 0; s < cam.samples_per_pixel; ++s) {
        c_ray r = cam.get_ray(i, j);

        c = c + radiance(r, scene, 0);
        /* if (intersect(r, scene, &t, &id)) { */
        /*   c = c + scene->spheres[id].color; */
        /* } */
      }
      c = c / cam.samples_per_pixel;
      img[j*w + i] = C_RGBA((int) c.x, (int) c.y, (int) c.z, 255);

      c = vec3d(0, 0, 0);
    }
  }
}

int main() 
{
  c_sphere spheres[] = {
    {.20, vec3d(1, 0, 1), vec3d(5, 45, 240), vec3d(10, 10, 10), DIFF },
    {.15, vec3d(0.2, 0, 0.75), vec3d(90, 45, 20), vec3d(10, 10, 10), DIFF },
    {.25, vec3d(0, 0, 1), vec3d(255, 255, 255), vec3d(10, 10, 10), DIFF },
    {.25, vec3d(0, 0, 1), vec3d(255, 255, 255), vec3d(10, 10, 10), DIFF },
    /* {1e5, vec3d(50, 1e5, 81.6), vec3d(10, 10, 10), vec3d(.75,75,.75), DIFF }, */ 
  };
  c_scene_t scene = {
    .spheres = spheres,
    .num_spheres = sizeof(spheres) / sizeof(spheres[0]),
  };

  c_path_tracer(IBUF, WIDTH, HEIGHT, &scene);

  const char *file_path = "out.png";
  if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, IBUF, sizeof(uint32_t)*WIDTH)) {
        fprintf(stderr, "ERROR: could not write %s\n", file_path);
        return 1;
  }
  return 0;
}
