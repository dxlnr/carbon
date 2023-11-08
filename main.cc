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
#include "renderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
  "  -vfov               Vertical field of view.\n"
  "  -s                  Number of samples per pixel used in rendering algorithm.\n"
  "  -maxd               Maximum depth of the raytracing algorithm.\n"
  "  -cuda               Use CUDA for rendering.\n"
  "  -v                  Verbose mode.\n"
;

int main(int argc, char **argv) 
{
  c_state_t s = c_state();

  int args = parse_args(&s, &argc, &argv);
  if (args < 0) return 1;

  if (args == ARG_HELP_R){
    fputs(show_help, stdout);
    return 0;
  }

  s.im_buffer = (uint32_t *)malloc(s.h * s.w * sizeof(uint32_t));
  if (s.im_buffer == NULL) {
    perror("Unable to allocate memory for image buffer.");
    return 1;
  }

  c_sphere spheres[] = {
    /* radius, pos, color, emission, index of refraction, material */
    { 1000,vec3d(0,-1000.5,-1),vec3d(.82,.82,.82),vec3d(.8,.3, 0),1.,DIFF },
    { .5,  vec3d(0,0,-3),      vec3d(.7,.3,.3),vec3d(.8,.8,.3),1.,DIFF },
    { .5,  vec3d(1,0,-3),      vec3d(.8,.6,.2),vec3d(.8,.8,.8),1.,REFL },
    { .5,  vec3d(-1,0,-3),     vec3d(.9,.9,.9),vec3d(.8,.8,.8),1.,REFL },
    /* { .5,  vec3d(-1,0,-3),     vec3d(.9,.9,.9),vec3d(.8,.8,.8),1.5,REFR }, */
  };
  c_scene_t scene = {
    .spheres = spheres,
    .num_spheres = sizeof(spheres) / sizeof(spheres[0]),
  };

  cam_t cam; cam.init(s.w, s.h, s.spp, s.vfov);

  if (s.rt) {
    rt(s.im_buffer, s.w, s.h, &scene, &cam, s.maxd);
  } else if (s.pt) {
    pt(s.im_buffer, s.w, s.h, &scene, &cam);
  } else {
    fprintf(stderr, "ERROR: no algorithm selected.\n");
    return 1;
  }

  char *out_file = concat_strs(s.outfile, (char *) ".png");
  if (out_file == NULL) {
    return 0;
  }
  printf("\nSave as : %s\n", out_file);

  if (!stbi_write_png(out_file, s.w, s.h, 4, s.im_buffer, sizeof(uint32_t)*s.w)) {
    fprintf(stderr, "ERROR: could not write %s\n", out_file);
    return 1;
  }
  return 0;
}
