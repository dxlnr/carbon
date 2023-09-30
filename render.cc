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

inline double clamp(double x){ return x < 0 ? 0 : x > 1 ? 1 : x; } 
inline int toInt(double x){ return int(pow(clamp(x), 1/2.2) * 255 + .5); } 


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

vec3d radiance(c_ray &r, c_scene_t *scene, int depth)
{
  int id = 0;
  double t;

  unsigned short Xi[3]={0,0, 5*5*5};
  /* if (depth >= 5) return vec3d(0, 0, 0); */
  
  if (!intersect(r, scene, &t, &id)) return vec3d(0, 0, 0);

  c_sphere obj = scene->spheres[id];
  vec3d f = obj.color;
  double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z;

  if (++depth>5) {
    if (erand48(Xi) < p) {
      f = f*(1 / p); 
    } else { 
      return obj.emission;
    }
  }

  vec3d no = r.o + r.d * t; 
  vec3d nn = (no - obj.pos).norm(); 
  vec3d nl = nn.dot(&r.d) < 0 ? nn : nn * -1; 

  /* // Pick a random direction from here and keep going. */
  /* Ray newRay; */
  /* newRay.origin = ray.pointWhereObjWasHit; */
  /* vec3d nl = (r.o + r.d * t - obj.pos).norm(); */

  double r1  = 2 * M_PI * erand48(Xi);
  double r2  = erand48(Xi);
  double r2s = sqrt(r2); 

  vec3d w = nl; 
  vec3d u = ((fabs(w.x) > .1 ? vec3d(0,1) : vec3d(1)).prod(&w)).norm(); 
  vec3d v = w.prod(&u); 

  vec3d nd = (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1-r2)).norm(); 

  /* vec3d nd = random_unit_vec_hemisphere(nl); */
  /* printf("nd %f %f %f\n", nd.x, nd.y, nd.z); */
  c_ray nray = c_ray(nl, nd);

  vec3d li = radiance(nray, scene, depth + 1);
  /* printf("li %f %f %f\n", li.x, li.y, li.z); */
  /* printf("incoming %f %f %f\n", incoming.x, incoming.y, incoming.z); */
  return obj.emission + obj.color.mul(&li);

  /* // This is NOT a cosine-weighted distribution! */
  /* newRay.direction = RandomUnitVectorInHemisphereOf(ray.normalWhereObjWasHit); */

  /* /1* Probability of the newRay *1/ */
  /* const double p = 1 / (2 * M_PI); */

  /* if (obj.material == DIFF) { */
  /* } */

  /* /1* // Compute the BRDF for this ray (assuming Lambertian reflection) *1/ */
  /* double theta = nray.d.dot(&nl); */
  /* vec3d BRDF = obj.color / M_PI; */

  /* /1* // Recursively trace reflected light sources. *1/ */
  /* vec3d incoming = radiance(nray, scene, depth + 1); */
  /* /1* Color incoming = TracePath(newRay, depth + 1); *1/ */

  /* printf("incoming %f %f %f\n", incoming.x, incoming.y, incoming.z); */
  /* /1* // Apply the Rendering Equation here. *1/ */
  /* return obj.emission + (BRDF.mul(&incoming) * theta / p); */
  /* /1* vec3d c = vec3d(0, 0, 0); *1/ */
  /* /1* return c; *1/ */
}

void c_path_tracer(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene)
{
  vec3d c;
  int id = 0;
  double t;

  cam cam;
  cam.init(w, h);

  for (int j = 0; j < h; ++j) {
    fprintf(stderr,"\rRendering (%d spp) %5.2f%%", cam.samples_per_pixel*4, 100.* j / (h-1));
    for (int i = 0; i < w; ++i, c=vec3d(0, 0, 0)) {
      for (int s= 0; s < cam.samples_per_pixel; ++s) {
        c_ray r = cam.get_ray(i, j);

        c = c + radiance(r, scene, 0);
        /* if (intersect(r, scene, &t, &id)) { */
        /*   c = c + scene->spheres[id].color; */
        /* } */
      }
      c = c / cam.samples_per_pixel;
      /* img[j*w + i] = C_RGBA((int) c.x, (int) c.y, (int) c.z, 255); */
      /* if (j == 48) { */
      /*   printf("c %d %d %d\n", toInt(c.x), toInt(c.y), toInt(c.z)); */
      /*   printf("c %f %f %f\n", c.x, c.y, (c.z)); */
      /* } */
      /* c = c - vec3d(0.5, 0.5, 0.5); */
      img[j*w + i] = C_RGBA(toInt(c.x), toInt(c.y), toInt(c.z), 255);
    }
  }
}

int main() 
{
  c_sphere spheres[] = {
    /* radius, pos, color, emission, material */
    {.40, vec3d(0, 0, 1),   vec3d(.05, .15, .75), vec3d(0, 0, 0),       DIFF },
    {1,   vec3d(0, -3, 1),  vec3d(0, 0, 0),       vec3d(10, 10, 10),    DIFF },
    {1,   vec3d(2, -3, 1),  vec3d(0, 0, 0),       vec3d(10, 10, 10),    DIFF },

    {5,   vec3d(0, -10, 0), vec3d(.1,.1,.6),      vec3d(0, 0, 0),       DIFF }, // top
    {5,   vec3d(0, 10, 0),  vec3d(.1,.1,.6),      vec3d(0, 0, 0),       DIFF },
    {5,   vec3d(10, 0, 0),  vec3d(.1,.1,.6),      vec3d(0, 0, 0),       DIFF }, // right
    {5,   vec3d(-10, 0, 0), vec3d(.1,.1,.6),      vec3d(0, 0, 0),       DIFF }, // left
    {5,   vec3d(0, 0, -10), vec3d(0, 0, 0),       vec3d(0, 0, 0),       DIFF }, // front
    /* {5,   vec3d(0, 0, 10),  vec3d(0, 0, 0),       vec3d(0, 0, 0),       DIFF }, // back */
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
