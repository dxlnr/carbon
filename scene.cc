#include "scene.h"
#include "carbon.h"


double c_sphere_intersect(c_ray r, c_sphere sp, double &t) 
{
  vec3 oc = r.o - sp.pos;
  vec3 a = r.d.pow(); 
  vec3 b = (r.d * 2.0).mul(&oc);
  vec3 c = oc.pow() - (sp.radius * sp.radius);

  double sd = b.mul(&b) - (a * 4.0).mul(&c);
  if (sd < 0.0) return t;
}
