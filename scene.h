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

#ifndef SCENE_C_
#define SCENE_C_ 

#include "carbon.h"

typedef enum c_material {
  GLASS,
  SOLID,
  METALIC,
} c_material_t;

struct c_sphere {
  double radius;
  vec3d pos;
  vec3d color;
  vec3d emission;
  c_material_t material;

  double intersect(c_ray r) {
    vec3d oc = r.o - this->pos;
    double a = r.d.dot(&r.d); 
    double b = (r.d * 2.0).dot(&oc);
    double c = oc.dot(&oc) - (this->radius * this->radius);

    double sd = b * b - (a * 4.0 * c);
    if (sd < 0.0) return -1.0;
    else return (-b - sqrt(sd)) / (2.0 * a);
  }
};

int c_sphere_ray_intersect(c_ray ray, c_sphere sphere);

struct c_plane {
  vec3d normal;
  vec3d pos;
  vec3d color;
  vec3d emission;
  c_material_t material;
};

typedef struct c_scene {
} c_scene_t;

#endif // SCENE_C_
