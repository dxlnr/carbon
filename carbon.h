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

#include <stdint.h>
#include <math.h>

#define C_RGBA(r, g, b, a) ((((r)&0xFF)<<(8*0)) |\
                            (((g)&0xFF)<<(8*1)) |\
                            (((b)&0xFF)<<(8*2)) |\
                            (((a)&0xFF)<<(8*3)))

/* Basic data structures */
struct vec3d {        
  double x, y, z;                  
  vec3d(double x_=0, double y_=0, double z_=0){ x=x_; y=y_; z=z_; }
  /* provide operators */
  vec3d operator+(const vec3d &v) const { return vec3d(x + v.x, y + v.y, z + v.z); }
  vec3d operator-(const vec3d &v) const { return vec3d(x - v.x, y - v.y, z - v.z); }
  vec3d operator*(double v) const { return vec3d(x * v, y * v, z * v); }
  vec3d operator/(double v) const { return vec3d(x * 1/v, y * 1/v, z * 1/v); }
  /* normalize vector */
  vec3d norm() const { return *this / sqrt(x*x + y*y + z*z); }
  /* multiple & power function */
  vec3d mul(vec3d *v) const { return vec3d(x*v->x, y*v->y, z*v->z); }
  vec3d pow() const { return vec3d(x*x, y*y, z*z); }

  double dot(vec3d *v) const { return (x*v->x + y*v->y + z*v->z); }
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

void c_path_tracer(uint32_t *img, uint32_t w, uint32_t h, int samps);

#endif
