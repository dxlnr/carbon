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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/* parse_args return codes: */
#define ARG_HELP_R          1

static const char show_help[] =
  "Carbon Rendering Engine "" \n"
  "\n"
  "Usage: carbon [options...] [-o outfile] ...\n"
  "General options:\n"
  "  -o <file>           Place the output into <file>.\n"
  "  -help               Display available options (-help-hidden for more).\n"
  "  -pt                 Use the pathtracing algorithm.\n"
  "  -w                  Width of the output image.\n"
  "  -h                  Height of the output image.\n"
  "  -s                  Number of samples per pixel used in rendering algorithm.\n"
  "  -maxd               Maximum depth of the raytracing algorithm.\n"
  "  -cuda               Use CUDA for rendering.\n"
  "  -t                  Number of threads used in rendering algorithm.\n"
  "  -v                  Verbose mode.\n"
;

char *concat_strs(char *s1, char *s2)
{
  char *con = (char *) malloc(strlen(s1) + strlen(s2) + 1);  
  if (!con) {
    perror("Unable to allocate memory.");
    return NULL;
  }
  strcpy(con, s1);
  strcat(con, s2);
  return con;
}

int get_arg_type(const char* arg) 
{
  if (!strcmp(arg, "-help")) return ARG_HELP;
  if (!strcmp(arg, "-pt"))   return ARG_PT;
  if (!strcmp(arg, "-s"))    return ARG_S;
  if (!strcmp(arg, "-w"))    return ARG_W;
  if (!strcmp(arg, "-h"))    return ARG_H;
  if (!strcmp(arg, "-maxd")) return ARG_MAXD;
  if (!strcmp(arg, "-o"))    return ARG_O;
  if (!strcmp(arg, "-cuda")) return ARG_CUDA;
  return ARG_UNKNOWN;
}

int parse_args(state_t *s, int *argc, char ***argv) 
{
  for (int i = 1; i < *argc; ++i) {
    switch (get_arg_type((*argv)[i])) {
      case ARG_HELP:
        return ARG_HELP_R;
      case ARG_PT:
        s->pt= 1;
        break;
      case ARG_S:
        if (++i >= *argc) goto check_arg_err;
        s->spp = atoi((*argv)[i]);
        break;
      case ARG_O:
        if (++i >= *argc) goto check_arg_err;
        s->outfile = (*argv)[i];
        break;
      case ARG_W:
        if (++i >= *argc) goto check_arg_err;
        s->w = atoi((*argv)[i]);
        break;
      case ARG_H:
        if (++i >= *argc) goto check_arg_err;
        s->h = atoi((*argv)[i]);
        break;
      case ARG_MAXD:
        if (++i >= *argc) goto check_arg_err;
        s->maxd = atoi((*argv)[i]);
        break;
      case ARG_CUDA:
        s->cuda = 1;
        break;
      default:
        fprintf(stderr, "ERROR: unknown option %s\n", (*argv)[i-1]);
        return -1;

    check_arg_err:
        fprintf(stderr, "ERROR: %s requires an argument.\n", (*argv)[i-1]);
        return -1;
    }
  }
  return 0;
}

vec3d random_unit_vec() 
{
  vec3d p;
  while (true) {
    p = vec3d::rand(-1, 1);
    if (p.len() < 1)
      return vec3d::unit(p);
  }
}

vec3d random_vec_on_hemisphere(vec3d& n) {
  vec3d p = random_unit_vec();
  if (p.dot(&n) > 0.0)
    return p;
  else
    return p * -1;
}

vec3d reflect(vec3d &v, vec3d &n) {
  return v - n * (2 * v.dot(&n));
}

vec3d refract(vec3d &d, vec3d &n, double refr) {
  double cosa = fmin((d * -1.).dot(&n), 1.0);
  vec3d rpe = (d + (n * cosa)) * refr;
  vec3d rpa = n * -sqrt(fabs(1 - rpe.dot(&rpe)));
  return rpe + rpa;
}

double reflect(double cos, double i) {
  /* Schlick's approximation for reflectance. */
  double r0 = (1 - i) / (1 + i);
  r0 = r0 * r0;
  return r0 + (1 - r0) * pow((1 - cos), 5);
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

bool collide(c_ray_t r, c_scene_t *s, c_hit_t *h)
{
  c_hit_t dh;
  bool found_hit = false;
  double dt = 1e20;

  for (int k = 0; k < s->num_spheres; ++k) {
    if (s->spheres[k].hit(r, &dh, 0.001, dt)) {
      found_hit = true;
      dt = dh.t;
      dh.mat = s->spheres[k].material;
      dh.col = s->spheres[k].color;
      *h = dh;
    }
  }
  return found_hit;
}

vec3d ray_color(c_ray_t r, c_scene_t *s, int depth = 0, int max_depth = 50)
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
      // TODO set the param for the material. This is basically glass now.
      double rr = h.ff ? (1.0/1.4) : 1.4;
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
  vec3d ud = vec3d::unit(r.d);
  double a = (ud.y + 1.0) * 0.5;
  return vec3d(1.0, 1.0, 1.0) * (1.0 - a) + vec3d(0.5, 0.7, 1.0) * a;
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

void pt(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene, cam *cam)
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

void rt(uint32_t *img, uint32_t w, uint32_t h, c_scene_t *scene, cam *cam, int maxd)
{
  int id = 0;
  vec3d c;
  double t, dt;

  for (int j = 0; j < h; ++j) {
    fprintf(stderr,"\r(rt) Rendering %5.2f%%", 100.* j / (h-1));
    for (int i = 0; i < w; ++i, c=vec3d(0, 0, 0)) {
      for (int s = 0; s < cam->spp; ++s, t=0, dt=1e20, id=-1) {
        c_ray r = cam->get_ray(i, j);
        c = c + ray_color(r, scene, 0, maxd);
      }
      c = c / cam->spp;
      img[j*w + i] = C_RGBA(toInt(c.x), toInt(c.y), toInt(c.z), 255);
    }
  }
}

int main(int argc, char **argv) 
{
  state_t s = state();

  int args = parse_args(&s, &argc, &argv);
  if (args < 0) return 1;

  if (args == ARG_HELP_R){
    fputs(show_help, stdout);
    return 0;
  }
  uint32_t im_buf[s.h*s.w];

  c_sphere spheres[] = {
    /* radius, pos, color, emission, material */
    { 1000,vec3d(0,-1000.5,-1),vec3d(.1,.6,.9),vec3d(.8,.3, 0),DIFF },
    { .5,  vec3d(0,0,-1),      vec3d(.7,.3,.3),vec3d(.8,.8,.3),DIFF },
    { .5,  vec3d(1,0,-1),      vec3d(.8,.6,.2),vec3d(.8,.8,.8),REFL },
    { .5,  vec3d(-1,0,-1),     vec3d(1.,1.,1.),vec3d(.8,.8,.8),REFR },
  };
  c_scene_t scene = {
    .spheres = spheres,
    .num_spheres = sizeof(spheres) / sizeof(spheres[0]),
  };

  cam cam; cam.init(s.w, s.h, s.spp);

  if (s.rt)
    rt(im_buf, s.w, s.h, &scene, &cam, s.maxd);
  else if (s.pt)
    pt(im_buf, s.w, s.h, &scene, &cam);

  char *out_file = concat_strs(s.outfile, (char *) ".png");
  if (out_file == NULL) {
    return 0;
  }

  if (!stbi_write_png(out_file, s.w, s.h, 4, im_buf, sizeof(uint32_t)*s.w)) {
    fprintf(stderr, "ERROR: could not write %s\n", out_file);
    return 1;
  }
  return 0;
}
