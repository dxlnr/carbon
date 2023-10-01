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

#define C_RGB(r, g, b)     ((((r)&0xFF)<<(8*0)) |\
                            (((g)&0xFF)<<(8*1)) |\
                            (((b)&0xFF)<<(8*2)))

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
  double len()                       const { return x * x + y * y + z * z; }
  /* random vector */
  static vec3d rand()                      { return vec3d(randd(), randd(), randd()); }
  static vec3d rand(double l, double h)    { return vec3d(randd(l,h), randd(l,h), randd(l,h)); }
  /* unit vector */
  static vec3d unit(vec3d v)               { return v / v.len(); }
};

/* 
 * c_ray
 *
 * A ray is a parametric line with an origin (o) and a direction (d). 
 * A point along the ray can be defined using a parameter, t:
 * p(t) = o + t*d
*/
struct c_ray { 
  vec3d o, d; 
  c_ray(vec3d o_, vec3d d_) : o(o_), d(d_) {} 
};

/* Camera */
struct cam {
  uint32_t w, h;
  /* Camera origin */
  vec3d origin;
  /* Count of random samples for each pixel */
  uint32_t samples_per_pixel = 10;
  /* Maximum number of ray bounces into scene */
  uint32_t maxd              = 10;
  /* Vertical view angle (field of view) */
  double vfov                = 90;

  void init(uint32_t w_, uint32_t h_) {
    w = w_;
    h = h_;
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

#endif
