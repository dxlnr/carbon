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


void c_path_tracer(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene)
{
  vec3d c = vec3d(0, 0, 0);
  double t;

  cam cam;
  cam.init(w, h);

  for (int j = 0; j < h; ++j) {
    fprintf(stderr,"\rRendering (%d spp) %5.2f%%", cam.samples_per_pixel*4, 100.* j / (h-1));
    for (int i = 0; i < w; ++i) {
      for (int s= 0; s < cam.samples_per_pixel; ++s) {
        c_ray r = cam.get_ray(i, j);
        for (int k = 0; k < scene->num_spheres; ++k) {
          t = scene->spheres[k].intersect(r);
          if (t != -1.0)
            c = c + scene->spheres[k].color;
        }
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
    {.25, vec3d(0, 0, 1), vec3d(255, 255, 255), vec3d(10, 10, 10), SOLID },
    {.20, vec3d(1, 0, 1), vec3d(5, 45, 240), vec3d(10, 10, 10), SOLID }
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
