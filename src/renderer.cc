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

#include "renderer.h"


vec3d random_unit_vec() 
{
  vec3d p;
  while (true) {
    p = vec3d::rand(-1, 1);
    if (p.len() < 1)
      return vec3d::unit(p);
  }
}

vec3d random_vec_on_hemisphere(vec3d& n) 
{
  vec3d p = random_unit_vec();
  if (p.dot(&n) > 0.0)
    return p;
  else
    return p * -1;
}

vec3d reflect(vec3d &v, vec3d &n) 
{
  return v - n * (2 * v.dot(&n));
}

vec3d refract(vec3d &d, vec3d &n, double refr) 
{
  double cosa = fmin((d * -1.).dot(&n), 1.0);
  vec3d rpe = (d + (n * cosa)) * refr;
  vec3d rpa = n * -sqrt(fabs(1 - rpe.dot(&rpe)));
  return rpe + rpa;
}

double reflect(double cosine, double i) 
{
  /* Schlick's approximation for reflectance. */
  double r0 = (1 - i) / (1 + i);
  r0 = r0 * r0;
  return r0 + (1 - r0) * pow((1 - cosine), 5);
}

int intersect(c_ray_t ray, c_scene_t *scene, double *t, int *id)
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

bool collide(c_ray_t r, c_scene_t *s, c_hit_t *h)
{
  c_hit_t dh;
  bool found_hit = false;
  double dt = 1e20;

  for (int k = 0; k < s->num_spheres; ++k) {
    if (s->spheres[k].hit(r, &dh, 0.001, dt)) {
      found_hit = true;
      dt     = dh.t;
      dh.mat = s->spheres[k].material;
      dh.col = s->spheres[k].color;
      dh.ir  = s->spheres[k].ir;
      *h     = dh;
    }
  }
  return found_hit;
}

vec3d ray_color(c_ray_t r, c_scene_t *s, int depth, int max_depth)
{
  c_hit h;
  vec3d nd;

  if (++depth >= max_depth) return vec3d(0, 0, 0);

  if (collide(r, s, &h)) {
    if (h.mat == DIFF) {
      nd = h.n + random_unit_vec();
      if (nd.zero())
        nd = h.n;
    } else if (h.mat == REFL) {
      vec3d urd = vec3d::unit(r.d);
      nd = reflect(urd, h.n);
    } else if (h.mat == REFR) {
      double rr = h.ff ? (1.0/h.ir) : h.ir;
      vec3d urd = vec3d::unit(r.d);

      double c = fmin((urd * -1).dot(&h.n), 1.0);
      double s = sqrt(1.0 - (c * c));

      if ((rr * s > 1.0) || reflect(c, rr) > randd()) 
        nd = reflect(urd, h.n);
      else 
        nd = refract(r.d, h.n, rr);
    } else {
      return vec3d(0, 0, 0);
    }
    return ray_color(c_ray(h.o, nd), s, depth).mul(&h.col);
  }
  /* vec3d ud = vec3d::unit(r.d); */
  /* double a = (ud.y + 1.0) * 0.5; */
  /* return vec3d(1.0, 1.0, 1.0) * (1.0 - a) + vec3d(0.5, 0.7, 1.0) * a; */
  return vec3d(.15, .15, .15);
}

vec3d radiance(c_ray_t &r, c_scene_t *scene, int depth, unsigned short *Xi)
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

void pt(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene, cam_t *cam)
{
  int id = 0;
  vec3d c;
  double t;

  for (int j = 0; j < h; ++j) {
    fprintf(stderr,"\r(pt) Rendering (%d spp) %5.2f%%", cam->spp*4, 100.* j / (h-1));
    for (int i = 0; i < w; ++i, c=vec3d(0, 0, 0)) {
      for (int s= 0; s < cam->spp; ++s) {
        unsigned short Xi[3]={0,0, 5*5*5};
        c_ray r = cam->get_ray(i, j);
        c = c + radiance(r, scene, 0, Xi);
      }
      c = c / cam->spp;
      img[j*w + i] = C_RGBA(toInt(c.x), toInt(c.y), toInt(c.z), 255);
    }
  }
}

void rt(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene, cam_t *cam, int maxd)
{
  vec3d c;
  c_ray_t r = c_ray(vec3d(0, 0, 0), vec3d(0, 0, 0));

#if MT
  std::vector<uint32_t> hi(h);
  std::iota(hi.begin(), hi.end(), 0);

  std::for_each(std::execution::par, std::begin(hi), std::end(hi), 
    [&](uint32_t j) 
    {
      for (int i = 0; i < w; ++i, c=vec3d(0, 0, 0)) {
        for (int s = 0; s < cam->spp; ++s) {
          r = cam->get_ray(i, j);
          c = c + ray_color(r, scene, 0, maxd);
        }
        c = c / cam->spp;
        img[j*w + i] = C_RGBA(toInt(c.x), toInt(c.y), toInt(c.z), 255);
      }
    });
#else
  for (int j = 0; j < h; ++j) {
    fprintf(stderr,"\r(rt) Rendering %5.2f%%", 100.* j / (h-1));
    for (int i = 0; i < w; ++i, c=vec3d(0, 0, 0)) {
      for (int s = 0; s < cam->spp; ++s) {
        r = cam->get_ray(i, j);
        c = c + ray_color(r, scene, 0, maxd);
      }
      c = c / cam->spp;
      img[j*w + i] = C_RGBA(toInt(c.x), toInt(c.y), toInt(c.z), 255);
    }
  }
#endif
}
