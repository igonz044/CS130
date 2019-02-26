#include "reflective_shader.h"
#include "ray.h"
#include "render_world.h"

vec3 Reflective_Shader::Shade_Surface(
    const Ray&  ray,
    const vec3& intersection_point,
    const vec3& normal, int recursion_depth)
    const {

    vec3 color;
    //vec3 light_intersect = ((world.lights[i]->position)-(intersection_point));
    //calculate r = 2(n · v)*n - v


    color = (1 - reflectivity)*shader->
    Shade_Surface(ray, intersection_point, normal, recursion_depth);
    
    if(recursion_depth < world.recursion_depth_limit)
    {
      vec3 r = 2*dot(normal,-ray.direction)*normal+(ray.direction);
      Ray reflectiveRay(intersection_point, r);
      color += reflectivity * world.Cast_Ray(reflectiveRay,recursion_depth+1);
    }
    //TODO determine the color
    return color;
}
