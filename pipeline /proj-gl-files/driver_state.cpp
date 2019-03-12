#include "driver_state.h"
#include <cstring>

driver_state::driver_state(){}
driver_state::~driver_state()
{
  delete [] image_color;
  delete [] image_depth;
}

// This function should allocate and initialize the arrays that store color and
// depth.  This is not done during the constructor since the width and height
// are not known when this class is constructed.
void initialize_render(driver_state& state, int width, int height)
{
    state.image_width =width;
    state.image_height=height;

    state.image_depth = new float[height*width];
    state.image_color = new pixel[height*width];

    pixel tempPixel = make_pixel(0,0,0);

    std::cout<<"TODO: allocate and initialize state.image_color and state.image_depth."<<std::endl;
    for(int i = 0; i < (width*height); i++)
    {
        state.image_color[i] = tempPixel;
        //state.image_depth[i] = 1;
    }

    for(int i = 0; i < (width*height); i++)
    {
        //state.image_color[i] = make_pixel(0,0,0);
        state.image_depth[i] = 1;
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

      const data_geometry * g[3];

      int numTriangles = state.num_vertices/3.0;
      int x = 0;

      for(int i = 0; i < numTriangles; i++)
      {
        // vertex_d[0].data = &state.vertex_data[i];
        // vertex_d[1].data = &state.vertex_data[i+state.floats_per_vertex*1];
        // vertex_d[2].data = &state.vertex_data[i+state.floats_per_vertex*2];
        for(int j = 0; j < 3; j++)
        {
          vertex_d[j].data = &state.vertex_data[x];
          g_arr[j].data = vertex_d[j].data;
          state.vertex_shader(vertex_d[j], g_arr[j], state.uniform_data);
          g[j] = &g_arr[j];
          x += state.floats_per_vertex;
        }
        //rasterize_triangle(state, g);
        clip_triangle(state, g, 0);
      }
      // for (int j = 0; j < 3; j++)
      // {
      //   state.vertex_shader(vertex_d[j], g_arr[j], state.uniform_data);
      //   g[j] = &g_arr[j];
      // }
      // rasterize_triangle(state, g);
      break;
    }

    case render_type::indexed:
    {
      data_geometry g_arr[3];
      data_vertex   vertex_d[3];

      const data_geometry *g[3];

      for(int i = 0; i < 3*state.num_triangles; i+=3)
      {
        for(int j = 0; j < 3; j++)
        {
          vertex_d[j].data = &state.vertex_data[state.index_data[i + j] * state.floats_per_vertex];
          g_arr[j].data = vertex_d[j].data;
          state.vertex_shader(vertex_d[j], g_arr[j], state.uniform_data);
          g[j] = &g_arr[j];
        }
        clip_triangle(state, g, 0);
      }
      break;
    }

    case render_type::fan:
    {
      data_geometry g_arr[3];
      data_vertex   vertex_d[3];

      const data_geometry *g[3];

      for(int i = 0; i < state.num_vertices; i++)
      {
        for(int j = 0; j < 3; j++)
        {
          int index = i + j;
          if(j == 0) { index = 0; }

          vertex_d[j].data = &state.vertex_data[index * state.floats_per_vertex];
          g_arr[j].data = vertex_d[j].data;
          state.vertex_shader(vertex_d[j], g_arr[j], state.uniform_data);
          g[j] = &g_arr[j];
        }
        clip_triangle(state, g, 0);
      }
      break;
    }

    case render_type::strip:
    {
      data_geometry g_arr[3];
      data_vertex   vertex_d[3];

      const data_geometry *g[3];

      for(int i = 0; i < 3*state.num_vertices-2; i++)
      {
        for(int j = 0; j < 3; j++)
        {
          vertex_d[j].data = &state.vertex_data[(i + j)*state.floats_per_vertex];
          g_arr[j].data = vertex_d[j].data;
          state.vertex_shader(vertex_d[j], g_arr[j], state.uniform_data);
          g[j] = &g_arr[j];
        }
        clip_triangle(state, g, 0);
      }
      break;
    }/**/

    case render_type::invalid:{ break; }

    default:
    std::cout << "No render type" << std::endl;
  }
}

//Helper Function
void helperFunc(driver_state & state, const data_geometry * in[3], int face, int o, int i)
{
  vec4 a = in[0]->gl_Position;
  vec4 b = in[1]->gl_Position;
  vec4 c = in[2]->gl_Position;

  const data_geometry * newData[3] = {in[0], in[1], in[2]};
  //A inside
  if(a[i] >= -a[3] && b[i] < -b[3] && c[i] < -c[3])
  {
    float ab, ca;
    //A to B
    ab = (-1*b[3] - b[i]) / (a[i] + a[3] - b[3] - b[i]); vec4 AtoB = ab * a + (1-ab) * b;
    //C to A
    ca = (-1*a[3] - a[i]) / (c[i] + c[3] - a[3] - a[i]); vec4 CtoA = ca * c + (1-ca) * a;

    data_geometry tempB, tempC;
    tempB.data = new float[state.floats_per_vertex]; tempC.data = new float[state.floats_per_vertex];

    for(int j = 0; j < state.floats_per_vertex; j++)
    {
      if(state.interp_rules[j] == interp_type::flat){tempB.data[j] = in[1]->data[j]; tempC.data[j] = in[2]->data[j];}
      if(state.interp_rules[j] == interp_type::smooth){tempB.data[j] = ab * in[0]->data[j] + (1-ab) * in[1]->data[j]; tempC.data[j] = ca * in[2]->data[j] + (1-ca) * in[0]->data[j];}
      if(state.interp_rules[j] == interp_type::noperspective){
        float b1 = (ab * in[0]->gl_Position[3]) / ((1-ab) * in[1]->gl_Position[3] + ab * in[0]->gl_Position[3]);
        float b2 = (ca * in[2]->gl_Position[3]) / ((1-ca) * in[0]->gl_Position[3] + ca * in[2]->gl_Position[3]);
        tempB.data[j] = b1 * in[0]->data[j] + (1-b1) * in[1]->data[j]; tempC.data[j] = b2 * in[2]->data[j] + (1-b2) * in[0]->data[j];}
    }

    tempB.gl_Position = AtoB; tempC.gl_Position = CtoA;
    newData[0] = in[0]; newData[1] = &tempB; newData[2] = &tempC;
    clip_triangle(state, newData, face+1);
  }

  //B inside
  if(a[i] < -a[3] && b[i] >= -b[3] && c[i] < -c[3])
  {
    float ab, bc;
    //A to B
    ab = (-1*b[3] - b[i]) / (a[i] + a[3] - b[3] - b[i]); vec4 AtoB = ab * a + (1-ab) * b;
    //B to C
    bc = (-1*c[3] - c[i]) / (b[i] + b[3] - c[3] - c[i]); vec4 BtoC = bc * b + (1-bc) * c;

    data_geometry tempA, tempC;
    tempA.data = new float[state.floats_per_vertex]; tempC.data = new float[state.floats_per_vertex];

    for(int j = 0; j < state.floats_per_vertex; j++)
    {
      if(state.interp_rules[j] == interp_type::flat){tempA.data[j] = in[0]->data[j]; tempC.data[j] = in[2]->data[j];}
      if(state.interp_rules[j] == interp_type::smooth){tempA.data[j] = ab * in[0]->data[j] + (1-ab) * in[1]->data[j];tempC.data[j] = bc * in[1]->data[j] + (1-bc) * in[2]->data[j];}
      if(state.interp_rules[j] == interp_type::noperspective){
        float b1 = (ab * in[0]->gl_Position[3]) / ((1-ab) * in[1]->gl_Position[3] + ab * in[0]->gl_Position[3]);
        float b2 = (bc * in[1]->gl_Position[3]) / ((1-bc) * in[2]->gl_Position[3] + bc * in[1]->gl_Position[3]);
        tempA.data[j] = b1 * in[0]->data[j] + (1-b1) * in[1]->data[j];
        tempC.data[j] = b2 * in[1]->data[j] + (1-b2) * in[2]->data[j];}
    }
    tempA.gl_Position = AtoB; tempC.gl_Position = BtoC;
    newData[0] = &tempA; newData[1] = in[1]; newData[2] = &tempC;
    clip_triangle(state, newData, face+1);
  }

  //C inside
  if(a[i] < -a[3] && b[i] < -b[3] && c[i] >= -c[3])
  {
    float ca, bc;
    ca = (-1*a[3] - a[i]) / (c[i] + c[3] - a[3] - a[i]); vec4 CtoA = ca * c + (1-ca) * a;
    bc = (-1*c[3] - c[i]) / (b[i] + b[3]-c[3] - c[i]); vec4 BtoC = bc * b + (1-bc) * c;

    data_geometry tempA, tempB;
    tempA.data = new float[state.floats_per_vertex]; tempB.data = new float[state.floats_per_vertex];

    for(int j = 0; j < state.floats_per_vertex; j++)
    {
      if(state.interp_rules[j] == interp_type::flat){tempA.data[j] = in[0]->data[j];tempB.data[j] = in[1]->data[j];}
      if(state.interp_rules[j] == interp_type::smooth){tempA.data[j] = ca * in[2]->data[j] + (1-ca) * in[0]->data[j]; tempB.data[j] = bc * in[1]->data[j] + (1-bc) * in[2]->data[j];}
      if(state.interp_rules[j] == interp_type::noperspective){
        float b1 = (bc * in[1]->gl_Position[3]) / ((1-bc) * in[2]->gl_Position[3] + bc * in[1]->gl_Position[3]);
        float b2 = (ca * in[2]->gl_Position[3]) / ((1-ca) * in[0]->gl_Position[3] + ca * in[2]->gl_Position[3]);
        tempA.data[j] = b2 * in[2]->data[j] + (1-b2) * in[0]->data[j]; tempB.data[j] = b1 * in[1]->data[j] + (1-b1) * in[2]->data[j];}
    }
    tempA.gl_Position = CtoA; tempB.gl_Position = BtoC;
    newData[0] = &tempA; newData[1] = &tempB; newData[2] = in[2];
    clip_triangle(state, newData, face+1);
  }

  //A and B inside
  if(a[i] >= -a[3] && b[i] >= -b[3] && c[i] < -c[3])
  {
    float bc, ca;
    ca = (-1*a[3] - a[i]) / (c[i] + c[3] - a[3] - a[i]); vec4 CtoA = ca * c + (1-ca) * a;
    //B to C
    bc = (-1*c[3] - c[i]) / (b[i] + b[3] - c[3] - c[i]); vec4 BtoC = bc * b + (1-bc) * c;

    data_geometry tempB, tempC;
    tempB.data = new float[state.floats_per_vertex]; tempC.data = new float[state.floats_per_vertex];

    if(o == 1)
    {
      for(int j = 0; j < state.floats_per_vertex; j++)
      {
        if(state.interp_rules[j] == interp_type::flat){tempC.data[j] = in[2]->data[j];}
        if(state.interp_rules[j] == interp_type::smooth){tempC.data[j] = bc * in[1]->data[j] + (1-bc) * in[2]->data[j];}
        if(state.interp_rules[j] == interp_type::noperspective){
          float b1 = (bc * in[1]->gl_Position[3]) / (bc * in[1]->gl_Position[3] + (1-bc) * in[2]->gl_Position[3]);
          tempC.data[j] = b1 * in[1]->data[j] + (1-b1) * in[2]->data[j];}
      }
      tempC.gl_Position = BtoC;
      newData[0] = in[0]; newData[1] = in[1]; newData[2] =  &tempC;
      clip_triangle(state, newData, face+1);
    }
    if(o == 2)
    {
      for(int j = 0; j < state.floats_per_vertex; j++)
      {
        if(state.interp_rules[j] == interp_type::flat){tempB.data[j] = in[1]->data[j];tempC.data[j] = in[2]->data[j];}
        if(state.interp_rules[j] == interp_type::smooth){tempB.data[j] = bc * in[1]->data[j] + (1-bc) * in[2]->data[j];tempC.data[j] = ca * in[2]->data[j] + (1-ca) * in[0]->data[j];}
        if(state.interp_rules[j] == interp_type::noperspective){
          float b1 = (bc * in[1]->gl_Position[3]) / ((1-bc) * in[2]->gl_Position[3] + bc * in[1]->gl_Position[3]);
          float b2 = (ca * in[2]->gl_Position[3]) / ((1-ca) * in[0]->gl_Position[3] + ca * in[2]->gl_Position[3]);
          tempB.data[j] = b1 * in[1]->data[j] + (1-b1) * in[2]->data[j]; tempC.data[j] = b2 * in[2]->data[j] + (1-b2) * in[0]->data[j];}
      }
      tempB.gl_Position = BtoC; tempC.gl_Position = CtoA;
      newData[0] = in[0]; newData[1] = &tempB; newData[2] = &tempC;
      clip_triangle(state, newData, face+1);
    }
  }

  //A and C inside
  if(a[i] >= -a[3] && b[i] < -b[3] && c[i] >= -c[3])
  {
    float bc, ab;
    //B to C
    bc = (-1*c[3] - c[i]) / (b[i] + b[3] - c[3] - c[i]); vec4 BtoC = bc * b + (1-bc) * c;
    //A to B
    ab = (-1*b[3] - b[i]) / (a[i] + a[3] - b[3] - b[i]); vec4 AtoB = ab * a + (1-ab) * b;

    data_geometry temp1, temp2;
    temp1.data = new float[state.floats_per_vertex];
    temp2.data = new float[state.floats_per_vertex];

    if(o == 1)
    {
      for(int j = 0; j < state.floats_per_vertex; j++)
      {
        if(state.interp_rules[j] == interp_type::flat){temp1.data[j] = in[1]->data[j];}
        if(state.interp_rules[j] == interp_type::smooth){temp1.data[j] = ab * in[0]->data[j] + (1-ab) * in[1]->data[j];}
        if(state.interp_rules[j] == interp_type::noperspective){
          float b1 = (ab * in[0]->gl_Position[3])/(ab * in[0]->gl_Position[3] + (1-ab) * in[1]->gl_Position[3]);
          temp1.data[j] = b1 * in[0]->data[j] + (1-b1) * in[1]->data[j];}
      }
      temp1.gl_Position = AtoB;
      newData[0] = in[0]; newData[1] =  &temp1; newData[2] = in[2];
      clip_triangle(state, newData, face+1);
    }
    if(o == 2)
    {
      for(int j = 0; j < state.floats_per_vertex; j++)
      {
        if(state.interp_rules[j] == interp_type::flat){temp1.data[j] = in[0]->data[j];temp2.data[j] = in[1]->data[j];}
        if(state.interp_rules[j] == interp_type::smooth){temp1.data[j] = ab * in[0]->data[j] + (1-ab) * in[1]->data[j];temp2.data[j] = bc * in[1]->data[j] + (1-bc) * in[2]->data[j];}
        if(state.interp_rules[j] == interp_type::noperspective){
          float b1 = (ab * in[0]->gl_Position[3]) / ((1-ab) * in[1]->gl_Position[3] + ab * in[0]->gl_Position[3]);
          float b2 = (bc * in[1]->gl_Position[3]) / ((1-bc) * in[2]->gl_Position[3] + bc * in[1]->gl_Position[3]);
          temp1.data[j] = b1 * in[0]->data[j] + (1-b1) * in[1]->data[j]; temp2.data[j] = b2 * in[1]->data[j] + (1-b2) * in[2]->data[j];}
      }
      temp1.gl_Position = AtoB; temp2.gl_Position = BtoC;
      newData[0] = &temp1; newData[1] = &temp2; newData[2] = in[2];
      clip_triangle(state, newData, face+1);
    }
  }

  if(a[i] < -a[3] && b[i] >= -b[3] && c[i] >= -c[3])
  {
    float ab = (-1*b[3] - b[i]) / (a[i] + a[3] - b[3] - b[i]); vec4 AtoB = ab * a + (1-ab) * b;
    float ca = (-1*a[3] - a[i]) / (c[i] + c[3] - a[3] - a[i]); vec4 CtoA = ca * c + (1-ca) * a;

    data_geometry temp1, temp2;
    temp1.data = new float[state.floats_per_vertex]; temp2.data = new float[state.floats_per_vertex];

    if (o == 1)
    {
      for(int j = 0; j < state.floats_per_vertex; j++)
      {
        if(state.interp_rules[j] == interp_type::flat)  {temp1.data[j] = in[0]->data[j];}
        if(state.interp_rules[j] == interp_type::smooth){temp1.data[j] = ca * in[2]->data[j] + (1-ca) * in[0]->data[j];}
        if(state.interp_rules[j] == interp_type::noperspective){
          float b1 = (ca * c[3]) / ((1-ca) * a[3] + ca * c[3]);
          temp1.data[j] = b1 * in[2]->data[j] + (1-b1) * in[0]->data[j];}
      }
      temp1.gl_Position = CtoA;
      newData[0] = &temp1; newData[1] = in[1]; newData[2] = in[2];
      clip_triangle(state, newData, face+1);
    }
    if(o == 2)
    {
      for(int j = 0; j < state.floats_per_vertex; j++)
      {
        if(state.interp_rules[j] == interp_type::flat){temp1.data[j] = in[0]->data[j]; temp2.data[j] = in[2]->data[j];}
        if(state.interp_rules[j] == interp_type::smooth){temp1.data[j] = ab*in[0]->data[j] + (1-ab)*in[1]->data[j]; temp2.data[j] = ca*in[2]->data[j] + (1-ca)*in[0]->data[j];}
        if(state.interp_rules[j] == interp_type::noperspective){
          float b1 = ab*in[0]->gl_Position[3] / (ab*in[0]->gl_Position[3] + (1-ab)*in[1]->gl_Position[3]);
          float b2 = ca*in[2]->gl_Position[3] / (ca*in[2]->gl_Position[3] + (1-ca)*in[0]->gl_Position[3]);
          temp1.data[j] = b1*in[0]->data[j] + (1-b1)*in[1]->data[j]; temp2.data[j] = b2*in[2]->data[j] + (1-b2)*in[0]->data[j];}
      }
      temp1.gl_Position = AtoB; temp2.gl_Position = CtoA;
      newData[0] = &temp1; newData[1] = in[1]; newData[2] = &temp2;
      clip_triangle(state,newData,face+1);
    }
  }
}

// This function clips a triangle (defined by the three vertices in the "in" array).
// It will be called recursively, once for each clipping face (face=0, 1, ..., 5) to
// clip against each of the clipping faces in turn.  When face=6, clip_triangle should
// simply pass the call on to rasterize_triangle.
void clip_triangle(driver_state& state, const data_geometry* in[3],int face)
{
  vec4 a = in[0]->gl_Position;
  vec4 b = in[1]->gl_Position;
  vec4 c = in[2]->gl_Position;

  switch (face)
  {
    //Near Plane
    case 0:
    {
      //none
      if(a[2] < -a[3] && b[2] < -b[3] && c[2] < -c[3])
      {
        return;
      }
      //a || b || c Inside
      if((a[2] >= -a[3] && b[2] < -b[3] && c[2] < -c[3]) ||
         (a[2] < -a[3] && b[2] >= -b[3] && c[2] < -c[3]) ||
         (a[2] < -a[3] && b[2] < -b[3] && c[2] >= -c[3]))
      {
        helperFunc(state, in, 5, 2, 2);
      }
      //a, b || a, c || b, c Inside
      if((a[2] >= -a[3] && b[2] < -b[3] && c[2] >= -c[3]) ||
         (a[2] >= -a[3] && b[2] >= -b[3] && c[2] < -c[3]) ||
         (a[2] < -a[3] && b[2] >= -b[3] && c[2] >= -c[3]))
      {
        helperFunc(state, in, 5, 1, 2);
        helperFunc(state, in, 5, 2, 2);
      }
      //a, b, and c Inside
      if(a[2] >= -a[3] && b[2] >= -b[3] && c[2] >= -c[3])
      {
        clip_triangle(state,in,face+1);
      }
      break;
    }



    //Far Plane
    case 1:
    {
      //none
      if(a[2] < -a[3] && b[2] < -b[3] && c[2] < -c[3])
      {
        return;
      }
      //a || b || c Inside
      if((a[2] >= -a[3] && b[2] < -b[3] && c[2] < -c[3]) ||
         (a[2] < -a[3] && b[2] >= -b[3] && c[2] < -c[3]) ||
         (a[2] < -a[3] && b[2] < -b[3] && c[2] >= -c[3]))
      {
        helperFunc(state, in, 5, 2, 2);
      }
      //a, b || a, c || b, c Inside
      if((a[2] >= -a[3] && b[2] < -b[3] && c[2] >= -c[3]) ||
         (a[2] >= -a[3] && b[2] >= -b[3] && c[2] < -c[3]) ||
         (a[2] < -a[3] && b[2] >= -b[3] && c[2] >= -c[3]))
      {
        helperFunc(state, in, 5, 1, 2);
        helperFunc(state, in, 5, 2, 2);
      }
      //a, b, and c Inside
      if(a[2] >= -a[3] && b[2] >= -b[3] && c[2] >= -c[3])
      {
        clip_triangle(state,in,face+1);
      }
      break;

    }


    //Left Plane
    case 2:
    {
      //none
      if(a[0] < -a[3] && b[0] < -b[3] && c[0] < -c[3])
      {
        return;
      }
      //a || b || c Inside
      if((a[0] >= -a[3] && b[0] < -b[3] && c[0] < -c[3]) ||
         (a[0] < -a[3] && b[0] >= -b[3] && c[0] < -c[3]) ||
         (a[0] < -a[3] && b[0] < -b[3] && c[0] >= -c[3]))
      {
        helperFunc(state, in, 1, 2, 0);
      }
      //a, b || a, c || b, c Inside
      if((a[2] >= -a[3] && b[2] < -b[3] && c[2] >= -c[3]) ||
         (a[2] >= -a[3] && b[2] >= -b[3] && c[2] < -c[3]) ||
         (a[2] < -a[3] && b[2] >= -b[3] && c[2] >= -c[3]))
      {
        helperFunc(state, in, 1, 1, 0);
        helperFunc(state, in, 1, 2, 0);
      }
      //a, b, and c Inside
      if(a[0] >= -a[3] && b[0] >= -b[3] && c[0] >= -c[3])
      {
        clip_triangle(state, in, face+1);
      }
      break;
    }

    //Right Plane
    case 3:
    {
      //none
      if(a[1] < -a[3] && b[1] < -b[3] && c[1] < -c[3])
      {
        return;
      }
      //a || b || c Inside
      if((a[1] >= -a[3] && b[1] < -b[3] && c[1] < -c[3]) ||
         (a[1] < -a[3] && b[1] >= -b[3] && c[1] < -c[3]) ||
         (a[1] < -a[3] && b[1] < -b[3] && c[1] >= -c[3]))
      {
        helperFunc(state, in, 3, 2, 1);
      }
      //a, b || a, c || b, c Inside
      if((a[1] >= -a[3] && b[1] < -b[3] && c[1] >= -c[3]) ||
         (a[1] >= -a[3] && b[1] >= -b[3] && c[1] < -c[3]) ||
         (a[1] < -a[3] && b[1] >= -b[3] && c[1] >= -c[3]))
      {
        helperFunc(state, in, 4, 2, 1);
        helperFunc(state,in,4,2,2);
      }
      //a, b, and c Inside
      if(a[0] >= -a[3] && b[0] >= -b[3] && c[0] >= -c[3])
      {
        clip_triangle(state, in, face+1);
      }
      break;
    }


    //Top Plane
    case 4:
    {
      //none
      if(a[1] < -a[3] && b[1] < -b[3] && c[1] < -c[3])
      {
        return;
      }
      //a || b || c Inside
      if((a[1] >= -a[3] && b[1] < -b[3] && c[1] < -c[3]) ||
         (a[1] < -a[3] && b[1] >= -b[3] && c[1] < -c[3]) ||
         (a[1] < -a[3] && b[1] < -b[3] && c[1] >= -c[3]))
      {
        helperFunc(state, in, 3, 2, 1);
      }
      //a, b || a, c || b, c Inside
      if((a[1] >= -a[3] && b[1] < -b[3] && c[1] >= -c[3]) ||
         (a[1] >= -a[3] && b[1] >= -b[3] && c[1] < -c[3]) ||
         (a[1] < -a[3] && b[1] >= -b[3] && c[1] >= -c[3]))
      {
        helperFunc(state, in, 3, 1, 1);
        helperFunc(state, in, 3, 2, 1);
      }
      //a, b, and c Inside
      if(a[0] >= -a[3] && b[0] >= -b[3] && c[0] >= -c[3])
      {
        clip_triangle(state, in, face+1);
      }
      break;
    }


    //Bottom Plane
    case 5:
    {
      //none
      if(a[1] < -a[3] && b[1] < -b[3] && c[1] < -c[3])
      {
        return;
      }
      //a || b || c Inside
      if((a[1] >= -a[3] && b[1] < -b[3] && c[1] < -c[3]) ||
         (a[1] < -a[3] && b[1] >= -b[3] && c[1] < -c[3]) ||
         (a[1] < -a[3] && b[1] < -b[3] && c[1] >= -c[3]))
      {
        helperFunc(state, in, 3, 2, 1);
      }
      //a, b || a, c || b, c Inside
      if((a[1] >= -a[3] && b[1] < -b[3] && c[1] >= -c[3]) ||
         (a[1] >= -a[3] && b[1] >= -b[3] && c[1] < -c[3]) ||
         (a[1] < -a[3] && b[1] >= -b[3] && c[1] >= -c[3]))
      {
        helperFunc(state, in, 3, 1, 1);
        helperFunc(state, in, 3, 2, 1);
      }
      //a, b, and c Inside
      if(a[0] >= -a[3] && b[0] >= -b[3] && c[0] >= -c[3])
      {
        clip_triangle(state, in, face+1);
      }
      break;
    }

  }//outside switch statement


  if(face==6)
  {
    rasterize_triangle(state, in);
    return;
  }
  //std::cout<<"TODO: implement clipping. (The current code passes the triangle through without clipping them.)"<<std::endl;
  //clip_triangle(state,in,face+1);
}
// Rasterize the triangle defined by the three vertices in the "in" array.  This
// function is responsible for rasterization, interpolation of data to
// fragments, calling the fragment shader, and z-buffering.
void rasterize_triangle(driver_state& state, const data_geometry* in[3])
{
  //Calculations of the areas for the barycentric coordinates
  //using pixel coordinates for each vertex of the triangle
  //std::cout<<"TODO: implement rasterization"<<std::endl;
  vec4 positiona = in[0]->gl_Position/in[0]->gl_Position[3];
  //std::cout<< positiona << std::endl;
  vec4 positionb = in[1]->gl_Position/in[1]->gl_Position[3];
  //std::cout<< in[0]->gl_Position << std::endl;
  vec4 positionc = in[2]->gl_Position/in[2]->gl_Position[3];
  //
  // float z0 = in[0]->gl_Position[2] / in[0]->gl_Position[3];;
  // float z1 = in[1]->gl_Position[2] / in[1]->gl_Position[3];;
  // float z2 = in[2]->gl_Position[2] / in[2]->gl_Position[3];;

  float w0 = in[0]->gl_Position[3];
  float w1 = in[1]->gl_Position[3];
  float w2 = in[2]->gl_Position[3];

  float x0 = in[0]->gl_Position[0] / w0;
  float x1 = in[1]->gl_Position[0] / w1;
  float x2 = in[2]->gl_Position[0] / w2;

  float y0 = in[0]->gl_Position[1] / w0;
  float y1 = in[1]->gl_Position[1] / w1;
  float y2 = in[2]->gl_Position[1] / w2;

  float z0 = in[0]->gl_Position[2] / w0;
  float z1 = in[1]->gl_Position[2] / w1;
  float z2 = in[2]->gl_Position[2] / w2;
  vec3 Z = {z0, z1, z2};

  float Ai = (state.image_width/2) * x0 + (state.image_width/2 - 0.5);
  float Aj = (state.image_height/2)* y0 + (state.image_height/2 - 0.5);
  float Bi = (state.image_width/2) * x1 + (state.image_width/2 - 0.5);
  float Bj = (state.image_height/2)* y1 + (state.image_height/2 - 0.5);
  float Ci = (state.image_width/2) * x2 + (state.image_width/2 - 0.5);
  float Cj = (state.image_height/2)* y2 + (state.image_height/2 - 0.5);

  vec2 A = {Ai,Aj};
  vec2 B = {Bi,Bj};
  vec2 C = {Ci,Cj};

  int x_min = std::min(std::min(Ai,Bi),Ci);
  int x_max = std::max(std::max(Ai,Bi),Ci);
  int y_min = std::min(std::min(Aj,Bj),Cj);
  int y_max = std::max(std::max(Aj,Bj),Cj);
  float totalArea = (C[0] - A[0]) * (B[1] - A[1]) - (C[1] - A[1]) * (B[0] - A[0]);

  if(x_min< 0) {x_min = 0;}
  if(x_max > state.image_width) {x_max = state.image_width;}
  if(y_min < 0) { y_min = 0;}
  if(y_max > state.image_height) {y_max = state.image_height;}

  //Iterating through each pixel
  for (int i = x_min; i <= x_max; i++)
  {
    for (int j = y_min; j <= y_max; j++)
    {
      vec2 P = {(float)i,(float)j};
      int index = (state.image_width * j) + i;
      //int index = i + j * state.image_width;
      float a = ((P[0] - B[0]) * (C[1] - B[1]) - (P[1] - B[1]) * (C[0] - B[0]))/totalArea;
      float b = ((P[0] - C[0]) * (A[1] - C[1]) - (P[1] - C[1]) * (A[0] - C[0]))/totalArea;
      float g = ((P[0] - A[0]) * (B[1] - A[1]) - (P[1] - A[1]) * (B[0] - A[0]))/totalArea;

      float alpha, beta, gamma;

      if (a >= 0 && b >= 0 && g>= 0)
      {
        data_output out_data;
        data_fragment fragment;
        fragment.data = new float[state.floats_per_vertex];

        float depth = (a * Z[0]) + (b * Z[1]) + (g* Z[2]);

        if(state.image_depth[index] > depth)
        {
        for(int i = 0; i < state.floats_per_vertex; i++)
        {

          if(state.interp_rules[i] == interp_type::flat)
          {
            fragment.data[i] = in[0]->data[i];
          }
          if(state.interp_rules[i] == interp_type::smooth)
          {
            //fragment.data[i] = in[0]->data[i];
            float k = (a/ w0) + (b/ w1) + (g/ w2);

            // float ap = a/ (k * in[0]->gl_Position[3]);
            // float bp = b/ (k * in[1]->gl_Position[3]);
            // float gp = g/ (k * in[2]->gl_Position[3]);

            alpha = a / w0 / k;
            beta = b/ w1 / k;
            gamma = g/ w2 / k;
            fragment.data[i] = (alpha*in[0]->data[i])
            + (beta*in[1]->data[i])
            + (gamma*in[2]->data[i]);
          }
          if(state.interp_rules[i] == interp_type::noperspective)
          {
            fragment.data[i] = a*in[0]->data[i]
                             + b*in[1]->data[i]
                             + g*in[2]->data[i];
          }
          else if(state.interp_rules[i] == interp_type::invalid)
          {
            std::cout << "Error" << std::endl;
          }
        }
      }

        state.fragment_shader(fragment, out_data, state.uniform_data);
        out_data.output_color = out_data.output_color*255;
        int red   = out_data.output_color[0];
        int green = out_data.output_color[1];
        int blue  = out_data.output_color[2];

        float z = a*positiona[2]+b*positionb[2]+g*positionc[2];
        int index = (state.image_width * j) + i;

        //if zbuffer[index] > z
        if(state.image_depth[index] > z)
        {
          //zbuffer[index] = z
          state.image_depth[index] = z;
          state.image_color[index] = make_pixel(red,green,blue);
          //state.image_depth[index] = depth;
        }
      }
    }
  }
}

