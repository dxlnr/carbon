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

/* Rendering Engine */
typedef struct c_renderer {
  void setup(const c_scene_t &scene, const cam_t &cam, const c_state_t &state);
  void render();
  void cleanup();
} c_renderer_t;

/* stack of rendering functions */
vec3d random_unit_vec();
vec3d random_vec_on_hemisphere(vec3d& n);
vec3d reflect(vec3d &v, vec3d &n);
vec3d refract(vec3d &d, vec3d &n, double refr);
double reflect(double cosine, double i);
int intersect(c_ray_t ray, c_scene_t *scene, double *t, int *id);
bool collide(c_ray_t r, c_scene_t *s, c_hit_t *h);
vec3d ray_color(c_ray_t r, c_scene_t *s, int depth = 0, int max_depth = 50);
vec3d radiance(c_ray_t &r, c_scene_t *scene, int depth, unsigned short *Xi);
void pt(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene, cam_t *cam);
void rt(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene, cam_t *cam, int maxd);
