#include "sphere.h"
#include "ray.h"

// Determine if the ray intersects with the sphere
Hit Sphere::Intersection(const Ray& ray, int part) const
{Hit hit{this, 0, part};
  vec3 d = ray.direction;
  vec3 ec = ray.endpoint - center;
  double b = dot(d, ec);
  double a = dot(d,d);
  double c = dot(ec,ec) - pow(radius,2);
  double disc = pow(b,2) - (a * c);

  if(disc == 0)
  {
    std::cout << "if: " << std::endl;
    double t = (-1 * b / a);
    if(t > small_t) {hit.dist = t;}
  }
   else if(disc > 0)
   {
     double t0 = (-1 * b + sqrt(disc)) / a;
     double t1 = (-1 * b - sqrt(disc)) / a;
     if(t0 > small_t && t1 > small_t) {hit.dist = std::min(t0,t1);}
    }
 return hit;
}

vec3 Sphere::Normal(const vec3& point, int part) const
{
    vec3 normal;
    normal = (point - center).normalized();
    return normal;
}

Box Sphere::Bounding_Box(int part) const
{
    Box box;
    TODO;
    return box;
}
