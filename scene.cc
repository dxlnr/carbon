#include "scene.h"
#include "carbon.h"


int c_sphere_ray_intersect(c_ray r, c_sphere sp) 
{
  vec3d oc = r.o - sp.pos;
  double a = r.d.dot(&r.d); 
  double b = (r.d * 2.0).dot(&oc);
  double c = oc.dot(&oc) - (sp.radius * sp.radius);

  double sd = b * b - (a * 4.0 * c);
  /* if (sd < 0.0) return -1.0; */
  /* else return (-b - sqrt(sd)) / (2.0 * a); */
   return (sd >= 0);
}
