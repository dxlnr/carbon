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

#ifndef CARBON_C_
#define CARBON_C_ 

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define C_RGBA(r, g, b, a) ((((r)&0xFF)<<(8*0)) |\
                            (((g)&0xFF)<<(8*1)) |\
                            (((b)&0xFF)<<(8*2)) |\
                            (((a)&0xFF)<<(8*3)))

/* Helper functions */
inline double degr_to_rad(double degrees)   { return degrees * M_PI / 180.0; }
inline double randd()                       { return rand() / (RAND_MAX + 1.0); }
inline double randd(double min, double max) { return min + (max-min) * (rand() / (RAND_MAX + 1.0)); }
inline double clamp(double x)               { return x < 0 ? 0 : x > 1 ? 1 : x; } 
inline int toInt(double x)                  { return int(pow(clamp(x), 1/2.2) * 255 + .5); } 

/* Basic data structures */
struct vec3d {      
  double x, y, z;
  vec3d(double x_=0, double y_=0, double z_=0){ x=x_; y=y_; z=z_; }
  /* operators */
  vec3d operator +  (const vec3d &v) const { return vec3d(x + v.x, y + v.y, z + v.z); }
  vec3d operator += (const vec3d &v) const { return vec3d(x + v.x, y + v.y, z + v.z); }
  vec3d operator +  (double v)       const { return vec3d(x + v, y + v, z + v); }
  vec3d operator -  (const vec3d &v) const { return vec3d(x - v.x, y - v.y, z - v.z); }
  vec3d operator *  (double v)       const { return vec3d(x * v, y * v, z * v); }
  vec3d operator /  (double v)       const { return vec3d(x * 1/v, y * 1/v, z * 1/v); }
  vec3d operator /= (double v)       const { return vec3d(x * 1/v, y * 1/v, z * 1/v); }
  /* normalize vector */
  vec3d norm()                       const { return *this / sqrt(x * x + y * y + z * z); }
  /* additional functions */
  vec3d mul(vec3d *v)                const { return vec3d(x * v->x, y * v->y, z * v->z); }
  vec3d pow()                        const { return vec3d(x * x, y * y, z * z); }
  /* dot product */
  double dot(vec3d *v)               const { return (x * v->x + y * v->y + z * v->z); }
  /* cross product */
  vec3d prod(vec3d *v)               const { return vec3d(y * v->z - z * v->y, 
                                                          z * v->x - x * v->z, x * v->y - y * v->x); }
  /* length of vector */
  double len()                       const { return sqrt(x * x + y * y + z * z); }
  /* random vector */
  static vec3d rand()                      { return vec3d(randd(), randd(), randd()); }
  static vec3d rand(double l, double h)    { return vec3d(randd(l,h), randd(l,h), randd(l,h)); }
  /* unit vector */
  static vec3d unit(vec3d v)               { return v / v.len(); }
  /* Return true if the vector is close to zero in all dimensions. */
  bool zero(double s = 1e-8) { return (fabs(x) < s) && (fabs(y) < s) && (fabs(z) < s); }
};

typedef enum c_material {
  DIFF,  
  REFL,
  SPEC,
  REFR,
} c_material_t;

/* c_ray
 *
 * A ray is a parametric line with an origin (o) and a direction (d). 
 * A point along the ray can be defined using a parameter, t:
 * p(t) = o + t*d
*/
typedef struct c_ray { 
  vec3d o, d; 
  c_ray(vec3d o_, vec3d d_) : o(o_), d(d_) {} 
} c_ray_t;


typedef struct c_hit {
  /* hit point origin and normal */
  vec3d o, n;
  /* distance */
  double t;
  /* color */
  vec3d col;
  /* material */
  c_material_t mat;
  /* Front face of the hit */
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
struct cam {
  uint32_t w, h;
  /* Camera origin */
  vec3d origin;
  /* Count of random samples for each pixel */
  uint32_t spp        = 10;
  /* Vertical view angle (field of view) */
  double vfov                = 90;

  void init(uint32_t w_, uint32_t h_, uint32_t spp_) {
    w = w_;
    h = h_;
    spp = spp_;
    origin = vec3d(0, 0, 0);

    double focal_l = 1.0;
    double vp_h =  2.0;
    double vp_w = vp_h * (static_cast<double>(w) / h);

    this->vu = vec3d(vp_w, 0, 0);
    this->vv = vec3d(0, -vp_h, 0);

    vec3d vp_up_left = origin - vec3d(0, 0, focal_l) - vu / 2 - vv / 2;
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
};

typedef enum arg_types {
  ARG_HELP    = 0,
  ARG_S       = 1,
  ARG_W       = 2,
  ARG_H       = 3,
  ARG_MAXD    = 4,
  ARG_O       = 5,
  ARG_CUDA    = 6,
  ARG_UNKNOWN = 7,
} arg_types_t;

typedef struct state {
  /* image width and height */
  uint32_t w          = 1280;
  uint32_t h          = 960;
  /* Count of random samples for each pixel */
  uint32_t spp        = 10;
  /* Maximum number of ray bounces into scene */
  uint32_t maxd       = 10;
  /* use cuda */
  unsigned char cuda;
  /* output filename */
  char *outfile; 
  /* image buffer */
  uint32_t *im_buffer;

  state(){ outfile = (char *) "out"; }
} state_t;

#endif
