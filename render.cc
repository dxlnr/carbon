#include "carbon.h"
#include "scene.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define WIDTH 400
#define HEIGHT 225

uint32_t IBUF[HEIGHT*WIDTH];

int main() 
{
  c_sphere spheres[] = {
    {40.8, vec3d(1e5+1, 40.8, 81.6), vec3d(.75,.25,.25), vec3d(10, 10, 10), SOLID},
  };

  c_path_tracer(IBUF, WIDTH, HEIGHT, 100);

  const char *file_path = "out.png";
  if (!stbi_write_png(file_path, WIDTH, HEIGHT, 4, IBUF, sizeof(uint32_t)*WIDTH)) {
        fprintf(stderr, "ERROR: could not write %s\n", file_path);
        return 1;
  }
  return 0;
}
