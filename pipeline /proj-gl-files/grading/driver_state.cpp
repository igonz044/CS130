#include "driver_state.h"
#include <cstring>

driver_state::driver_state(){}

driver_state::~driver_state()
{delete [] image_color;
 delete [] image_depth;}
// This function should allocate and initialize the arrays that store color and
// depth.  This is not done during the constructor since the width and height
// are not known when this class is constructed.
void initialize_render(driver_state& state, int width, int height)
{
    state.image_width =width;
    state.image_height=height;
    state.image_color =0;
    state.image_depth =0;
    state.image_color = new pixel[width*height];

    std::cout<<"TODO: allocate and initialize state.image_color and state.image_depth."<<std::endl;
    for(int i = 0; i < width*height; i++)
    {
        state.image_color[i] = make_pixel(0,0,0);
    }
}
// This function will be called to render the data that has been stored in this class.
// Valid values of type are:
//   render_type::triangle - Each group of three vertices corresponds to a triangle.
//   render_type::indexed  - Each group of three indices in index_data corresponds
//                           to a triangle.  These numbers are indices into vertex_data.
//   render_type::fan      - The vertices are to be interpreted as a triangle fan.
//   render_type::strip    - The vertices are to be interpreted as a triangle strip.
void render(driver_state& state, render_type type)
{
  switch(type)
  {
    case render_type::triangle:
    {
      data_geometry g_arr[3];
      data_vertex   vertex_d[3];

      const data_geometry *g[3];

      for (int i = 0; i< state.num_vertices*state.floats_per_vertex; i+=3*state.floats_per_vertex)
      {
        vertex_d[0].data = &state.vertex_data[i];
        vertex_d[1].data = &state.vertex_data[i+state.floats_per_vertex*1];
        vertex_d[2].data = &state.vertex_data[i+state.floats_per_vertex*2];
      }

      for (int j = 0; j < 3; j++)
      {
        state.vertex_shader(vertex_d[j], g_arr[j], state.uniform_data);
        g[j] = &g_arr[j];
      }
      rasterize_triangle(state, g);
    }
    break;

    /*case render_type::indexed:
    break;

    case render_type::fan:
    break;

    case render_type::strip:
    break;*/

    default:
    std::cout << "No render type" << std::endl;
  }
}
// This function clips a triangle (defined by the three vertices in the "in" array).
// It will be called recursively, once for each clipping face (face=0, 1, ..., 5) to
// clip against each of the clipping faces in turn.  When face=6, clip_triangle should
// simply pass the call on to rasterize_triangle.
void clip_triangle(driver_state& state, const data_geometry* in[3],int face)
{
    if(face==6)
    {
        rasterize_triangle(state, in);
        return;
    }
    std::cout<<"TODO: implement clipping. (The current code passes the triangle through without clipping them.)"<<std::endl;
    clip_triangle(state,in,face+1);
}
// Rasterize the triangle defined by the three vertices in the "in" array.  This
// function is responsible for rasterization, interpolation of data to
// fragments, calling the fragment shader, and z-buffering.
void rasterize_triangle(driver_state& state, const data_geometry* in[3])
{

  //Calculations of the areas for the barycentric coordinates
  //using pixel coordinates for each vertex of the triangle
  //std::cout<<"TODO: implement rasterization"<<std::endl;
  float Ai = (((state.image_width/2) * in[0]->gl_Position[0]) + ((state.image_width/2) - 0.5));
  float Aj = (((state.image_height/2)* in[0]->gl_Position[1]) + ((state.image_height/2) - 0.5));
  float Bi = (((state.image_width/2) * in[1]->gl_Position[0]) + ((state.image_width/2) - 0.5));
  float Bj = (((state.image_height/2)* in[1]->gl_Position[1]) + ((state.image_height/2) - 0.5));
  float Ci = (((state.image_width/2) * in[2]->gl_Position[0]) + ((state.image_width/2) - 0.5));
  float Cj = (((state.image_height/2)* in[2]->gl_Position[1]) + ((state.image_height/2) - 0.5));

  int x_min = std::min(std::min(Ai,Bi),Ci);
  int x_max = std::max(std::max(Ai,Bi),Ci);
  int y_min = std::min(std::min(Aj,Bj),Cj);
  int y_max = std::max(std::max(Aj,Bj),Cj);
  float totalArea = 0.5 * (((Bi * Cj) - (Ci * Bj)) - ((Ai * Cj) - (Ci * Aj)) + ((Ai * Bj) - (Bi * Aj)));

  //Iterating through each pixel
  for (int i = x_min; i < x_max; i++)
  {
    for (int j = y_min; j < y_max ; j++)
    {
      float a = 0.5 * (((Bi * Cj) - (Ci * Bj)) - ((i * Cj) - (Ci * j)) + ((i * Bj) - (Bi * j)));
      float b = 0.5 * (((i * Cj) - (Ci * j)) - ((Ai * Cj) - (Ci * Aj)) + ((Ai * j) - (i * Aj)));
      float g = 0.5 * (((Bi * j) - (i * Bj)) - ((Ai * j) - (i * Aj)) + ((Ai * Bj) - (Bi * Aj)));
      a = a/totalArea;
      b = b/totalArea;
      g = g/totalArea;

      if (a >= 0 && b >= 0 && g<= 1)
      {
        int index = (state.image_width * j) + i;
        state.image_color[index] = make_pixel(255,255,255);
      }
    }
  }
}
