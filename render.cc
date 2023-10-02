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

vec3d random_vec_on_hemisphere(vec3d& n) {
  vec3d p;
  while (true) {
    p = vec3d::rand(-1, 1);
    if (p.len() < 1)
      return vec3d::unit(p);
  }
  if (p.dot(&n) > 0.0)
    return p;
  else
    return p * -1;
}

vec3d ray_color(c_ray r, c_scene_t *s, int depth = 0)
{
  int id = -1;
  double t  = 0;
  double dt = 1e20;
  vec3d c;

  if (++depth > 5) return vec3d(0, 0, 0);

  for (int k = 0; k < s->num_spheres; ++k) {
    if ((t = s->spheres[k].intersect(r)) && t > 0) {
      if (t < dt) {
        dt = t;
        id = k;
        vec3d no = r.o + r.d * t; 
        vec3d nn = (no - s->spheres[id].pos).norm();
        vec3d nd = random_vec_on_hemisphere(nn);
        return ray_color(c_ray(no, nd), s, depth) * 0.5;
      }
    }
  }
  vec3d nn = vec3d::unit(r.o + r.d * t - vec3d(0, 0, -1));
  return vec3d(nn.x + 1, nn.y + 1, nn.z + 1);
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

vec3d radiance(c_ray &r, c_scene_t *scene, int depth, unsigned short *Xi)
{
  int id = 0;
  double t;

  if (!intersect(r, scene, &t, &id)) return vec3d(0, 0, 0);

  c_sphere obj = scene->spheres[id]; 
  vec3d c = obj.color;
  double p = c.x > c.y && c.x > c.z ? c.x : c.y > c.z ? c.y : c.z;

  if (++depth > 5) {
    if (erand48(Xi) < p)
      c = c * (1 / p); 
    else 
      return obj.emission;
  }
  vec3d no = r.o + r.d * t; 
  vec3d nn = (no - obj.pos).norm(); 
  vec3d nl = nn.dot(&r.d) < 0 ? nn : nn * -1; 

  if (obj.material == DIFF) { 
    /* DIFFUSE reflection */
    double r1  = 2 * M_PI * erand48(Xi), r2  = erand48(Xi), r2s = sqrt(r2); 

    vec3d w = nl; 
    vec3d u = ((fabs(w.x) > .1 ? vec3d(0,1) : vec3d(1)).prod(&w)).norm(); 
    vec3d v = w.prod(&u); 

    vec3d nd = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1-r2)).norm(); 

    c_ray nray = c_ray(nl, nd);

    vec3d li = radiance(nray, scene, depth, Xi);
    return obj.emission + obj.color.mul(&li);
  } else if (obj.material == SPEC) 
  { 
    /* SPECULAR reflection */
  } 
  /* dielectric REFRACTION */
  bool into = nn.dot(&nl) > 0; // ray from outside going in?
  return obj.emission;
}

void pt(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene)
{
  vec3d c;
  int id = 0;
  double t;

  cam cam;
  cam.init(w, h);

  for (int j = 0; j < h; ++j) {
    fprintf(stderr,"\r(pt) Rendering (%d spp) %5.2f%%", cam.samples_per_pixel*4, 100.* j / (h-1));
    for (int i = 0; i < w; ++i, c=vec3d(0, 0, 0)) {
      for (int s= 0; s < cam.samples_per_pixel; ++s) {
        unsigned short Xi[3]={0,0, 5*5*5};
        c_ray r = cam.get_ray(i, j);
        c = c + radiance(r, scene, 0, Xi);
      }
      c = c / cam.samples_per_pixel;
      img[j*w + i] = C_RGBA(toInt(c.x), toInt(c.y), toInt(c.z), 255);
    }
  }
}

void rt(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene)
{
  int id = 0;
  vec3d c;
  double t, dt;

  cam cam;
  cam.init(w, h);

  for (int j = 0; j < h; ++j) {
    fprintf(stderr,"\r(rt) Rendering %5.2f%%", 100.* j / (h-1));
    for (int i = 0; i < w; ++i, c=vec3d(0, 0, 0)) {
      for (int s = 0; s < cam.samples_per_pixel; ++s, t=0, dt=1e20, id=-1) {
        c_ray r = cam.get_ray(i, j);
        c = c + ray_color(r, scene);
        /* for (int k = 0; k < scene->num_spheres; ++k) { */
        /*   if ((t = scene->spheres[k].intersect(r)) && t > 0) { */
        /*     if (t < dt) { */
        /*       dt = t; */
        /*       id = k; */
        /*     } */
        /*   } */
        /* } */
        /* if (id >= 0) { */
        /*   /1* vec3d nn = vec3d::unit(r.o + r.d * t - vec3d(0, 0, -1)); *1/ */
        /*   /1* c = c + vec3d(nn.x + 1, nn.y + 1, nn.z + 1); *1/ */
        /*   c = c + scene->spheres[id].color; */
        /* } */
      }
      c = c / cam.samples_per_pixel;
      img[j*w + i] = C_RGBA(toInt(c.x), toInt(c.y), toInt(c.z), 255);
    }
  }
}

int main() 
{
  c_sphere spheres[] = {
    /* radius, pos, color, emission, material */
    { .5, vec3d(0,0,-1),     vec3d(.05,.15,.75),vec3d(0, 0, 0),DIFF },
    { 1000,vec3d(0,-1000.5,-1),vec3d(.85,.15,.75),vec3d(0, 0, 0),DIFF },
  };
  c_scene_t scene = {
    .spheres = spheres,
    .num_spheres = sizeof(spheres) / sizeof(spheres[0]),
  };

  /* pt(IBUF, WIDTH, HEIGHT, &scene); */
  rt(IBUF, WIDTH, HEIGHT, &scene);

  const char *file_path = "out.png";
  if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, IBUF, sizeof(uint32_t)*WIDTH)) {
        fprintf(stderr, "ERROR: could not write %s\n", file_path);
        return 1;
  }
  return 0;
}
