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

#ifndef SCENE_H
#define SCENE_H

#include "carbon.h"

typedef enum c_material {
  /*  Diffusion */
  DIFF,  
  /*  Reflection */
  REFL,
  SPEC,
  /*  Refraction described by Snell’s law */
  REFR,
} c_material_t;

/* c_hit
 *
 * Cache for storing the latest intersected object.
 */
typedef struct c_hit {
  /* hit point origin and normal */
  vec3d o, n;
  /* distance */
  double t;
  /* color */
  vec3d col;
  /* idx of refraction */
  double ir;
  /* material */
  c_material_t mat;
  /* front face of the hit */
  bool ff;

  void set_ff_n(c_ray& r, vec3d& on) {
    ff = r.d.dot(&on) < 0;
    this->n = ff ? on : on * -1;
  }
} c_hit_t;

struct c_sphere {
  double radius;
  /* center */
  vec3d pos;
  vec3d color;
  vec3d emission;
  double ir       = 1.0;
  c_material_t material;

  int hit(c_ray_t r, c_hit_t *ch, double tmin = 0, double tmax = 1e20) {
    vec3d oc = r.o - this->pos;
    double a = r.d.dot(&r.d); 
    double b = (r.d).dot(&oc);
    double c = oc.dot(&oc) - (this->radius * this->radius);

    double sd = b * b - (a * c);
    if (sd < 0) return 0;

    double sqrtd = sqrt(sd);
    double root = (-b - sqrtd) / a;
    if (root <= tmin || tmax <= root) {
      root = (-b + sqrtd) / a;
      if (root <= tmin || tmax <= root)
        return 0;
    }

    ch->t = root;
    ch->o = r.o + r.d * root;
    vec3d on = (ch->o - pos) / radius;
    ch->set_ff_n(r, on);

    return 1;
  }

  double intersect(c_ray r) {
    vec3d oc = r.o - this->pos;
    double a = r.d.dot(&r.d); 
    double b = (r.d * 2.0).dot(&oc);
    double c = oc.dot(&oc) - (this->radius * this->radius);

    double sd = b * b - (a * 4.0 * c);
    if (sd < 0) return 0;
    else return (-b - sqrt(sd)) / (2.0 * a);
  }
};

struct c_plane {
  vec3d normal;
  vec3d pos;
  vec3d color;
  vec3d emission;
  c_material_t material;
};

/* Scenery */
typedef struct c_scene {
  c_sphere *spheres;
  uint32_t num_spheres;
} c_scene_t;

/* Camera */
typedef struct cam {
  uint32_t w, h;
  /* Camera origin */
  vec3d origin   = vec3d(0, 0, 0);
  /* Vertical view angle (field of view) */
  double vfov    = 90;
  /* Reference or proxy point where the camera looks at. */
  vec3d refp     = vec3d(0, 0, -1);
  /* Camera-relative "up" direction */
  vec3d vup      = vec3d(0, 1, 0);
  /* Count of random samples for each pixel */
  uint32_t spp   = 10;

  void init(uint32_t w_, uint32_t h_, uint32_t spp_, double vfov_) {
    w = w_;
    h = h_;
    spp = spp_;
    vfov = vfov_;

    double focal_l = (origin - refp).len();
    double theta = degr_to_rad(vfov);
    double th = tan(theta / 2);

    double vp_h = 2 * th * focal_l;
    double vp_w = vp_h * (static_cast<double>(w) / h);

    cw = vec3d::unit(origin - refp);
    cu = vec3d::unit(vup.prod(&cw));
    cv = cw.prod(&cu);

    this->vu = cu * vp_w;
    this->vv = cv * -vp_h;

    vec3d vp_up_left = origin - (cw * focal_l) - (vu / 2) - (vv / 2);
    this->p0 = vp_up_left + (vu / w + vv / h) * 0.5;
  }

  /* Sample around each pixel */
  vec3d sample_pixel_sqr() const {
    double px = -0.5 + randd();
    double py = -0.5 + randd();
    return (this->vu / w * px) + (this->vv / h * py);
  }

  /* Get ray for pixel at (x_, y_) */
  c_ray get_ray(int x_, int y_) {
    vec3d r = this->p0 + (this->vu / w * x_) + (this->vv/ h * y_) - this->origin;
    vec3d s = sample_pixel_sqr();
    return c_ray(s - this->origin, r.norm());
  }

private:
  vec3d p0;
  vec3d vu, vv;
  /* Camera frame basis vectors */
  vec3d cu, cv, cw;        
} cam_t;

#endif // SCENE_H
