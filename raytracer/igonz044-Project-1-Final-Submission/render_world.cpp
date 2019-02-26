#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"

extern bool disable_hierarchy;

Render_World::Render_World()
    :background_shader(0),
    ambient_intensity(0),
    enable_shadows(true),
    recursion_depth_limit(3)
    {}

Render_World::~Render_World()
{
    delete background_shader;
    for(size_t i=0;i<objects.size();i++) delete objects[i];
    for(size_t i=0;i<lights.size();i++) delete lights[i];
}

Hit Render_World::Closest_Intersection(const Ray& ray)
{
    //TODO;
    double min_t = std::numeric_limits<double>::max();
    Hit mainHit = {NULL, 0, 0};
    mainHit.dist = 0;

    for(unsigned int i = 0; i < objects.size(); i++)
    {
      Hit possibleHit = objects[i]->Intersection(ray, objects[i]->number_parts);
      if(possibleHit.dist > small_t && possibleHit.dist < min_t)
      {
        mainHit = possibleHit;
        min_t = possibleHit.dist;
      }
    }
    return mainHit;
}

void Render_World::Render_Pixel(const ivec2& pixel_index)
{
    vec3 endpoint = camera.position;
    vec3 direction = (camera.World_Position(pixel_index) - endpoint);
    Ray ray(endpoint, direction);
    vec3 color=Cast_Ray(ray,1);
    camera.Set_Pixel(pixel_index,Pixel_Color(color));
}

void Render_World::Render()
{
    if(!disable_hierarchy)
        Initialize_Hierarchy();

    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));
}

vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth)
{
vec3 color;
    Hit closestHit = Closest_Intersection(ray);
    if(recursion_depth > recursion_depth_limit)
    {
      color = background_shader->Shade_Surface(ray, ray.endpoint, ray.endpoint, recursion_depth);
    }
    else if( closestHit.object != NULL && closestHit.dist >=small_t)
    {
      vec3 normal = closestHit.object->Normal(ray.Point(closestHit.dist),closestHit.part);
      color = closestHit.object->material_shader->Shade_Surface(ray,ray.Point(closestHit.dist),normal, recursion_depth);
    }
    else
    {
      color = background_shader->Shade_Surface(ray, ray.endpoint, ray.endpoint, recursion_depth);
    }
    return color;
}

void Render_World::Initialize_Hierarchy()
{
    TODO;
    hierarchy.Reorder_Entries();
    hierarchy.Build_Tree();
}

