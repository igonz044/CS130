#include "light.h"
#include "phong_shader.h"
#include "ray.h"
#include "render_world.h"
#include "object.h"

vec3 Phong_Shader::Shade_Surface(
const Ray&  ray,
const vec3& intersection_point,
const vec3& normal,int recursion_depth)
const {

  vec3 color(0,0,0), colorAmbient(0,0,0), colorDiffuse(0,0,0), colorSpecular(0,0,0);
  vec3 R, C, I, n, l;
  vec3 light_intersect;

  double ln, RC;
  double specular_intensity;

  //Ambient: Ia = Ra*La
  colorAmbient += world.ambient_color * world.ambient_intensity * color_ambient;
  color = colorAmbient;

  for(unsigned int i = 0; i < world.lights.size(); i++)
  {
    light_intersect = ((world.lights[i]->position)-(intersection_point));
    Ray shadowRay(intersection_point, light_intersect.normalized());
    Hit shadowIntersection = world.Closest_Intersection(shadowRay);


    if (shadowIntersection.object != NULL)
    {
      double light_t = light_intersect.magnitude();
      if(shadowIntersection.dist > light_t)
      {
        shadowIntersection.object = NULL;
      }
    }

    if(shadowIntersection.object == NULL || !world.enable_shadows)//if ray does not hit an object in the scene
    {
      //light_intersect = ((world.lights[i]->position)-intersection_point);

      I = world.lights[i]->Emitted_Light((world.lights[i]->position)-intersection_point);
      n = normal.normalized();
      l = light_intersect.normalized();

      //Calculating Diffuse
      ln = std::max(dot(l,n), 0.0);

      //Diffuse: Id = Rd*Ld*max(n*1,0)
      colorDiffuse = color_diffuse * I* ln;

      //Specular: Is = Rs*Ls*max(cos, 0)^a
      R = (l - n*(2 * dot(l,n))).normalized();
      C = (intersection_point - ray.endpoint).normalized();

      RC = std::max(dot(R,C), 0.0);

      specular_intensity = pow(RC, specular_power);
      colorSpecular = color_specular * I* specular_intensity;

      //TODO; //Determine color
      color += colorDiffuse + colorSpecular;
    }
  }
  return color;
}
