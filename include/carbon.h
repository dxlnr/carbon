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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <omp.h>

/* Multithreaded */
#define MT                  0
/* parse_args return codes: */
#define ARG_HELP_R          1

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

typedef enum arg_types {
  ARG_HELP    =  0,
  ARG_RT      =  1,
  ARG_PT      =  2,
  ARG_S       =  3,
  ARG_W       =  4,
  ARG_H       =  5,
  ARG_VFOV    =  6,
  ARG_MAXD    =  7,
  ARG_O       =  8,
  ARG_CUDA    =  9,
  ARG_UNKNOWN = 10,
} arg_types_t;

typedef struct c_state {
  /* image width and height */
  uint32_t w          = 1280;
  uint32_t h          = 960;
  /* Count of random samples for each pixel */
  uint32_t spp        = 10;
  /* Maximum number of ray bounces into scene */
  uint32_t maxd       = 10;
  /* Using the raytracing algorithm. */
  unsigned char rt    = 1;
  /* Using the pathtracing algorithm. */
  unsigned char pt    = 0;
  /* Camera params */
  double vfov         = 90;
  /* use cuda */
  unsigned char cuda  = 0;
  /* output filename */
  char *outfile; 
  /* image buffer */
  uint32_t *im_buffer;

  c_state(){ outfile = (char *) "out"; }
} c_state_t;

char *concat_strs(char *s1, char *s2);
int get_arg_type(const char* arg);
int parse_args(c_state_t *s, int *argc, char ***argv);

#endif
