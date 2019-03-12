#include "driver_state.h"
#include <cstring>

driver_state::driver_state()
{
}

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
    state.image_width=width;
    state.image_height=height;
    state.image_color= new pixel[height * width];
    pixel temp_pix = make_pixel(0,0,0);
    for (int i = 0; i < (width*height);i++){
        state.image_color[i] = temp_pix;
    }
    state.image_depth= new float[height * width];
    for (int i = 0; i < (height * width); i++){
        //state.image_depth[i] = std::numeric_limits<float>::max();
        state.image_depth[i] = 1;
        
    }
    //std::cout<<"TODO: allocate and initialize state.image_color and state.image_depth."<<std::endl;
}

// This function will be called to render the data that has been stored in this class.
// Valid values of type are:
//   render_type::triangle - Each group of three vertices corresponds to a triangle.
//   render_type::indexed -  Each group of three indices in index_data corresponds
//                           to a triangle.  These numbers are indices into vertex_data.
//   render_type::fan -      The vertices are to be interpreted as a triangle fan.
//   render_type::strip -    The vertices are to be interpreted as a triangle strip.
void render(driver_state& state, render_type type)
{
    //std::cout<<"TODO: implement rendering."<<std::endl;
    switch(type){
        case render_type::triangle:
        {
            const data_geometry * out[3];
            data_geometry d_geo[3];
            data_vertex v[3];

            int numTriangles = state.num_vertices  / 3.0;
            int p = 0;
           
            for(int c = 0; c < numTriangles; c++){
                for(int i = 0; i < 3; i++){
                    v[i].data = &state.vertex_data[p];
    
                    d_geo[i].data = v[i].data;

                    state.vertex_shader(v[i], d_geo[i], state.uniform_data);

                    out[i] = &d_geo[i];

                    p += state.floats_per_vertex; 
  
                        
                }
                //std::cout << c << std::endl;
                //rasterize_triangle(state,out);
                clip_triangle(state,out,0);
            }
            break;
        }

        case render_type::indexed:
        {
            const data_geometry * out[3];
            data_geometry d_geo[3];
            data_vertex v[3];        

            for(int i = 0; i < 3*state.num_triangles ; i += 3){
                for (int j = 0; j < 3; j++) {
                v[j].data = &state.vertex_data[state.index_data[i + j] * state.floats_per_vertex];
    
                d_geo[j].data = v[j].data;

                state.vertex_shader(v[j], d_geo[j], state.uniform_data);

                out[j] = &d_geo[j];

                }
                clip_triangle(state,out,0);
            }
            break;

        }
        case render_type::fan:
        {
            const data_geometry * out[3];
            data_geometry d_geo[3];
            data_vertex v[3];        

            for(int i = 0; i < state.num_vertices; i++){
                for (int j = 0; j < 3; j++) {
                    int index = i + j;
                    if(j == 0){
                        index = 0;
                    }
                    v[j].data = &state.vertex_data[index * state.floats_per_vertex];
        
                    d_geo[j].data = v[j].data;
    
                    state.vertex_shader(v[j], d_geo[j], state.uniform_data);

                    out[j] = &d_geo[j];

                }
                clip_triangle(state,out,0);
            }
            break;
        }
        case render_type::strip:
        {
            const data_geometry * out[3];
            data_geometry d_geo[3];
            data_vertex v[3];  
            for(int i = 0; i < state.num_vertices-2; i++){
                for(int j = 0; j < 3; j++){
                    v[j].data = &state.vertex_data[(i + j)*state.floats_per_vertex];
    
                    d_geo[j].data = v[j].data;

                    state.vertex_shader(v[j], d_geo[j], state.uniform_data);

                    out[j] = &d_geo[j];
                }
                clip_triangle(state,out,0);
            }
            break;
        }

        case render_type::invalid:

        break;
    }
}


void left_bottom_near(driver_state & state, const data_geometry * in[3], int i, int face, int flag){
    const data_geometry * new_data[3] = {in[0], in[1], in[2]};
    //data_geometry temp[3] = {*in[0], *in[1], *in[2]};
    vec4 a = in[0]->gl_Position;
    vec4 b = in[1]->gl_Position;
    vec4 c = in[2]->gl_Position;

    if(a[i] >= -a[3] && b[i] < -b[3] && c[i] < -c[3]){ //a is in
        float ab;
        float ca;
        ab = (-1*b[3] - b[i]) / (a[i] + a[3] - b[3] - b[i]);
        vec4 a_to_b = ab * a + (1-ab) * b; // a to b
        ca = (-1*a[3] - a[i]) / (c[i] + c[3] - a[3] - a[i]);
        vec4 c_to_a = ca * c + (1-ca) * a; // c->a

        data_geometry tempB, tempC;
        tempB.data = new float[state.floats_per_vertex];
        tempC.data = new float[state.floats_per_vertex];
       // if(flag == 2){
            for(int k = 0; k < state.floats_per_vertex; k++){
                if(state.interp_rules[k] == interp_type::flat){
                    tempB.data[k] = in[1]->data[k];
                    tempC.data[k] = in[2]->data[k];
                }
                if(state.interp_rules[k] == interp_type::smooth){
                    tempB.data[k] = ab * in[0]->data[k] + (1-ab) * in[1]->data[k];
                    tempC.data[k] = ca * in[2]->data[k] + (1-ca) * in[0]->data[k];
                } 
                if(state.interp_rules[k] == interp_type::noperspective){
                    float b1 = (ab * in[0]->gl_Position[3]) / ((1-ab) * in[1]->gl_Position[3] + ab * in[0]->gl_Position[3]);
                    float b2 = (ca * in[2]->gl_Position[3]) / ((1-ca) * in[0]->gl_Position[3] + ca * in[2]->gl_Position[3]);
                    tempB.data[k] = b1 * in[0]->data[k] + (1-b1) * in[1]->data[k];
                    tempC.data[k] = b2 * in[2]->data[k] + (1-b2) * in[0]->data[k];
                }
            }
            tempB.gl_Position = a_to_b;
            tempC.gl_Position = c_to_a;
            new_data[0] = in[0];
            new_data[1] = &tempB;
            new_data[2] = &tempC;
            clip_triangle(state, new_data, face+1);
        //}       
    }

    if(a[i] < -a[3] && b[i] >= -b[3] && c[i] < -c[3]){ //b is in
        float ab;
        float bc;
        ab = (-1*b[3] - b[i]) / (a[i] + a[3] - b[3] - b[i]);
        vec4 a_to_b = ab * a + (1-ab) * b; // a to b
        bc = (-1*c[3] - c[i]) / (b[i] + b[3] - c[3] - c[i]);
        vec4 b_to_c = bc * b + (1-bc) * c; // c->a

        data_geometry tempA, tempC;
        tempA.data = new float[state.floats_per_vertex];
        tempC.data = new float[state.floats_per_vertex];
       // if(flag == 2){
            for(int k = 0; k < state.floats_per_vertex; k++){
                if(state.interp_rules[k] == interp_type::flat){
                    tempA.data[k] = in[0]->data[k];
                    tempC.data[k] = in[2]->data[k];
                }
                if(state.interp_rules[k] == interp_type::smooth){
                    tempA.data[k] = ab * in[0]->data[k] + (1-ab) * in[1]->data[k];
                    tempC.data[k] = bc * in[1]->data[k] + (1-bc) * in[2]->data[k];
                } 
                if(state.interp_rules[k] == interp_type::noperspective){
                    float b1 = (ab * in[0]->gl_Position[3]) / ((1-ab) * in[1]->gl_Position[3] + ab * in[0]->gl_Position[3]);
                    float b2 = (bc * in[1]->gl_Position[3]) / ((1-bc) * in[2]->gl_Position[3] + bc * in[1]->gl_Position[3]);
                    tempA.data[k] = b1 * in[0]->data[k] + (1-b1) * in[1]->data[k];
                    tempC.data[k] = b2 * in[1]->data[k] + (1-b2) * in[2]->data[k];
                }
            }
            tempA.gl_Position = a_to_b;
            tempC.gl_Position = b_to_c;
            new_data[0] = &tempA;
            new_data[1] = in[1];
            new_data[2] = &tempC;
            clip_triangle(state, new_data, face+1);
       // }
    }

    if(a[i] < -a[3] && b[i] < -b[3] && c[i] >= -c[3]){ //c is in
        float ca;
        float bc;
        ca = (-1*a[3] - a[i]) / (c[i] + c[3] - a[3] - a[i]);
        vec4 c_to_a = ca * c + (1-ca) * a; // b -> c
        bc = (-1*c[3] - c[i]) / (b[i] + b[3] - c[3] - c[i]);
        vec4 b_to_c = bc * b + (1-bc) * c; // c->a

        data_geometry tempA, tempB;
        tempA.data = new float[state.floats_per_vertex];
        tempB.data = new float[state.floats_per_vertex];
      //  if(flag == 2){
            for(int k = 0; k < state.floats_per_vertex; k++){
                if(state.interp_rules[k] == interp_type::flat){
                    tempA.data[k] = in[0]->data[k];
                    tempB.data[k] = in[1]->data[k];
                }
                if(state.interp_rules[k] == interp_type::smooth){
                    tempA.data[k] = ca * in[2]->data[k] + (1-ca) * in[0]->data[k];
                    tempB.data[k] = bc * in[1]->data[k] + (1-bc) * in[2]->data[k];
                } 
                if(state.interp_rules[k] == interp_type::noperspective){
                    float b1 = (bc * in[1]->gl_Position[3]) / ((1-bc) * in[2]->gl_Position[3] + bc * in[1]->gl_Position[3]);
                    float b2 = (ca * in[2]->gl_Position[3]) / ((1-ca) * in[0]->gl_Position[3] + ca * in[2]->gl_Position[3]);
                    tempA.data[k] = b2 * in[2]->data[k] + (1-b2) * in[0]->data[k];
                    tempB.data[k] = b1 * in[1]->data[k] + (1-b1) * in[2]->data[k];
                }
            }
            tempA.gl_Position = c_to_a;
            tempB.gl_Position = b_to_c;
            new_data[0] = &tempA;
            new_data[1] = &tempB;
            new_data[2] = in[2];
            clip_triangle(state, new_data, face+1);
       // }
    }

    if(a[i] >= -a[3] && b[i] >= -b[3] && c[i] < -c[3]){ // a and b in
        //std::cout << "a, b in near \n";
        float bc;
        float ca;
        ca = (-1*a[3] - a[i]) / (c[i] + c[3] - a[3] - a[i]);
        vec4 c_to_a = ca * c + (1-ca) * a; // c -> a
        bc = (-1*c[3] - c[i]) / (b[i] + b[3] - c[3] - c[i]);
        vec4 b_to_c = bc * b + (1-bc) * c; // b to c

        data_geometry tempB, tempC; 
        tempB.data = new float[state.floats_per_vertex];
        tempC.data = new float[state.floats_per_vertex];
        if(flag == 1){
            for(int k = 0; k < state.floats_per_vertex; k++){
                if(state.interp_rules[k] == interp_type::flat){
                    tempC.data[k] = in[2]->data[k];
                }
                if(state.interp_rules[k] == interp_type::smooth){
                    tempC.data[k] = bc * in[1]->data[k] + (1-bc) * in[2]->data[k];
                } 
                if(state.interp_rules[k] == interp_type::noperspective){
                    float b1 = (bc * in[1]->gl_Position[3]) / (bc * in[1]->gl_Position[3] + (1-bc) * in[2]->gl_Position[3]);
                    tempC.data[k] = b1 * in[1]->data[k] + (1-b1) * in[2]->data[k];
                }
            }

            tempC.gl_Position = b_to_c;
            new_data[0] = in[0];
            new_data[1] = in[1];
            new_data[2] =  &tempC;
            
            clip_triangle(state, new_data, face+1);
        }
        if(flag == 2){
            for(int k = 0; k < state.floats_per_vertex; k++){
                if(state.interp_rules[k] == interp_type::flat){
                    tempB.data[k] = in[1]->data[k];
                    tempC.data[k] = in[2]->data[k];
                }
                if(state.interp_rules[k] == interp_type::smooth){
                    tempB.data[k] = bc * in[1]->data[k] + (1-bc) * in[2]->data[k];
                    tempC.data[k] = ca * in[2]->data[k] + (1-ca) * in[0]->data[k];
                } 
                if(state.interp_rules[k] == interp_type::noperspective){
                    float b1 = (bc * in[1]->gl_Position[3]) / ((1-bc) * in[2]->gl_Position[3] + bc * in[1]->gl_Position[3]);
                    float b2 = (ca * in[2]->gl_Position[3]) / ((1-ca) * in[0]->gl_Position[3] + ca * in[2]->gl_Position[3]);
                    tempB.data[k] = b1 * in[1]->data[k] + (1-b1) * in[2]->data[k];
                    tempC.data[k] = b2 * in[2]->data[k] + (1-b2) * in[0]->data[k];
                }
            }
            tempB.gl_Position = b_to_c;
            tempC.gl_Position = c_to_a;
            new_data[0] = in[0];
            new_data[1] = &tempB;
            new_data[2] = &tempC;
            clip_triangle(state, new_data, face+1);
        }
    }

    if(a[i] >= -a[3] && b[i] < -b[3] && c[i] >= -c[3]){ //a and c in
        float bc;
        float ab;
        bc = (-1*c[3] - c[i]) / (b[i] + b[3] - c[3] - c[i]);
        vec4 b_to_c = bc * b + (1-bc) * c; // b -> c
        ab = (-1*b[3] - b[i]) / (a[i] + a[3] - b[3] - b[i]);
        vec4 a_to_b = ab * a + (1-ab) * b; // a->b
        data_geometry temp1, temp2;
        
        temp1.data = new float[state.floats_per_vertex];
        temp2.data = new float[state.floats_per_vertex];
        if(flag == 1){
            for(int k = 0; k < state.floats_per_vertex; k++){
                if(state.interp_rules[k] == interp_type::flat){
                    temp1.data[k] = in[1]->data[k];
                }
                if(state.interp_rules[k] == interp_type::smooth){
                    temp1.data[k] = ab * in[0]->data[k] + (1-ab) * in[1]->data[k];
                } 
                if(state.interp_rules[k] == interp_type::noperspective){
                    float b1 = (ab * in[0]->gl_Position[3]) / (ab * in[0]->gl_Position[3] + (1-ab) * in[1]->gl_Position[3]);
                    temp1.data[k] = b1 * in[0]->data[k] + (1-b1) * in[1]->data[k];
                }
            }
            temp1.gl_Position = a_to_b;
            new_data[0] = in[0];
            new_data[1] =  &temp1;
            new_data[2] = in[2];
            clip_triangle(state, new_data, face+1);
        }
        if(flag == 2){
            for(int k = 0; k < state.floats_per_vertex; k++){
                if(state.interp_rules[k] == interp_type::flat){
                    temp1.data[k] = in[0]->data[k];
                    temp2.data[k] = in[1]->data[k];
                }
                if(state.interp_rules[k] == interp_type::smooth){
                    temp1.data[k] = ab * in[0]->data[k] + (1-ab) * in[1]->data[k];
                    temp2.data[k] = bc * in[1]->data[k] + (1-bc) * in[2]->data[k];
                } 
                if(state.interp_rules[k] == interp_type::noperspective){
                    float b1 = (ab * in[0]->gl_Position[3]) / ((1-ab) * in[1]->gl_Position[3] + ab * in[0]->gl_Position[3]);
                    float b2 = (bc * in[1]->gl_Position[3]) / ((1-bc) * in[2]->gl_Position[3] + bc * in[1]->gl_Position[3]);
                    temp1.data[k] = b1 * in[0]->data[k] + (1-b1) * in[1]->data[k];
                    temp2.data[k] = b2 * in[1]->data[k] + (1-b2) * in[2]->data[k];
                }
            }
            temp1.gl_Position = a_to_b;
            temp2.gl_Position = b_to_c;
            new_data[0] = &temp1;
            new_data[1] = &temp2;
            new_data[2] = in[2];
            clip_triangle(state, new_data, face+1);
        }
    }

    if(a[i] < -a[3] && b[i] >= -b[3] && c[i] >= -c[3]){
      //  std::cout << "face: " << face << " b and c in\n";
        float ab = (-1*b[3] - b[i]) / (a[i] + a[3] - b[3] - b[i]);
        vec4 a_to_b = ab * a + (1-ab) * b;
        float ca = (-1*a[3] - a[i]) / (c[i] + c[3] - a[3] - a[i]);
        vec4 c_to_a = ca * c + (1-ca) * a;

        data_geometry temp1, temp2;
        temp1.data = new float[state.floats_per_vertex];
        temp2.data = new float[state.floats_per_vertex];

        if (flag == 1){
            for(int k = 0; k < state.floats_per_vertex; k++){
                if(state.interp_rules[k] == interp_type::flat){
                    temp1.data[k] = in[0]->data[k];
                }
                if(state.interp_rules[k] == interp_type::smooth){
                    temp1.data[k] = ca * in[2]->data[k] + (1-ca) * in[0]->data[k];
                }
                if(state.interp_rules[k] == interp_type::noperspective){
                    float b1 = (ca * c[3]) / ((1-ca) * a[3] + ca * c[3]);
                    temp1.data[k] = b1 * in[2]->data[k] + (1-b1) * in[0]->data[k];
                }
            }
            temp1.gl_Position = c_to_a;
            new_data[0] = &temp1;
            new_data[1] = in[1];
            new_data[2] = in[2];
            //std::cout << "you made it here 1\n";
            clip_triangle(state, new_data, face+1);
        }
        if(flag == 2){
            for(int k = 0; k < state.floats_per_vertex; k++){
                if(state.interp_rules[k] == interp_type::flat){
                    temp1.data[k] = in[0]->data[k];
                    temp2.data[k] = in[2]->data[k];
                }
                if(state.interp_rules[k] == interp_type::smooth){
                    temp1.data[k] = ab*in[0]->data[k] + (1-ab)*in[1]->data[k];
                    temp2.data[k] = ca*in[2]->data[k] + (1-ca)*in[0]->data[k];
                }
                if(state.interp_rules[k] == interp_type::noperspective){
                    float b1 = ab*in[0]->gl_Position[3] / (ab*in[0]->gl_Position[3] + (1-ab)*in[1]->gl_Position[3]);
                    float b2 = ca*in[2]->gl_Position[3] / (ca*in[2]->gl_Position[3] + (1-ca)*in[0]->gl_Position[3]);
                    temp1.data[k] = b1*in[0]->data[k] + (1-b1)*in[1]->data[k];
                    temp2.data[k] = b2*in[2]->data[k] + (1-b2)*in[0]->data[k];
                }

            }
            temp1.gl_Position = a_to_b;
            temp2.gl_Position = c_to_a;
            new_data[0] = &temp1;
            new_data[1] = in[1];
            new_data[2] = &temp2;
            clip_triangle(state,new_data,face+1);
        }
    }
}
// This function clips a triangle (defined by the three vertices in the "in" array).
// It will be called recursively, once for each clipping face (face=0, 1, ..., 5) to
// clip against each of the clipping faces in turn.  When face=6, clip_triangle should
// simply pass the call on to rasterize_triangle.

void clip_triangle(driver_state& state, const data_geometry* in[3], int face)
{
    //const data_geometry * new_data[3] = {in[0], in[1], in[2]};
    //data_geometry temp[3] = {*in[0], *in[1], *in[2]};
    vec4 a = in[0]->gl_Position;
    vec4 b = in[1]->gl_Position;
    vec4 c = in[2]->gl_Position;

    switch(face){

        case 0: //right plane case
        {
            clip_triangle(state, in, face+1);
            break;
        }
        
        case 1: //left plane case
        {   
            if(a[0] >= -a[3] && b[0] >= -b[3] && c[0] >= -c[3]){//all in
                clip_triangle(state,in,face+1);
            }
            if(a[0] >= -a[3] && b[0] < -b[3] && c[0] < -c[3]){ //a is in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 0, 1, 2); //i = 0, face = 1
            }
            if(a[0] < -a[3] && b[0] >= -b[3] && c[0] < -c[3]){ //b is in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 0, 1, 2);
            }
            if(a[0] < -a[3] && b[0] < -b[3] && c[0] >= -c[3]){ //c is in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 0, 1, 2);
            }
            if(a[0] >= -a[3] && b[0] >= -b[3] && c[0] < -c[3]){ //a,b in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 0, 1, 1);
                left_bottom_near(state, in, 0, 1, 2);
            }
            if(a[0] >= -a[3] && b[0] < -b[3] && c[0] >= -c[3]){//a, c in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 0, 1, 1);
                left_bottom_near(state, in, 0, 1, 2);
            }
            if(a[0] < -a[3] && b[0] >= -b[3] && c[0] >= -c[3]){//b, c in
                left_bottom_near(state, in, 0, 1, 1);
                left_bottom_near(state, in, 0, 1, 2);
            }
            if(a[0] < -a[3] && b[0] < -b[3] && c[0] < -c[3]){//none in
                return;
            }
            break;
        }

        case 2: //top case
        {
            clip_triangle(state, in, face+1);
            break;
        }

        case 3: //bottom case
        {
            if(a[1] >= -a[3] && b[1] >= -b[3] && c[1] >= -c[3]){//all in
                clip_triangle(state,in,face+1);
            }
            if(a[1] >= -a[3] && b[1] < -b[3] && c[1] < -c[3]){ //a is in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 1, 3, 2);
            }
            if(a[1] < -a[3] && b[1] >= -b[3] && c[1] < -c[3]){ //b is in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 1, 3, 2);
            }
            if(a[1] < -a[3] && b[1] < -b[3] && c[1] >= -c[3]){ //c is in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 1, 3, 2);
            }
            if(a[1] >= -a[3] && b[1] >= -b[3] && c[1] < -c[3]){ //a,b in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 1, 3, 1);
                left_bottom_near(state, in, 1, 3, 2);
            }
            if(a[1] >= -a[3] && b[1] < -b[3] && c[1] >= -c[3]){//a, c in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 1, 3, 1);
                left_bottom_near(state, in, 1, 3, 2);
            }
            if(a[1] < -a[3] && b[1] >= -b[3] && c[1] >= -c[3]){//b, c in
                left_bottom_near(state, in, 1, 3, 1);
                left_bottom_near(state, in, 1, 3, 2);
            }
            if(a[1] < -a[3] && b[1] < -b[3] && c[1] < -c[3]){//none in
                return;
            }
            break;
        }

        case 4: //far 
        {
            clip_triangle(state, in, face+1);
            //right_top_far(state,in,2,4,1);
            //right_top_far(state,in,2,4,2);
            break;
        }

        case 5: //near case
        {
            if(a[2] >= -a[3] && b[2] >= -b[3] && c[2] >= -c[3]){//all in
                clip_triangle(state,in,face+1);
            }
            if(a[2] >= -a[3] && b[2] < -b[3] && c[2] < -c[3]){ //a is in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 2, 5, 2);
            }
            if(a[2] < -a[3] && b[2] >= -b[3] && c[2] < -c[3]){ //b is in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 2, 5, 2);
            }
            if(a[2] < -a[3] && b[2] < -b[3] && c[2] >= -c[3]){ //c is in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 2, 5, 2);
            }
            if(a[2] >= -a[3] && b[2] >= -b[3] && c[2] < -c[3]){ //a,b in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 2, 5, 1);
                left_bottom_near(state, in, 2, 5, 2);
            }
            if(a[2] >= -a[3] && b[2] < -b[3] && c[2] >= -c[3]){//a, c in
                //clip_triangle(state,in,face+1);
                left_bottom_near(state, in, 2, 5, 1);
                left_bottom_near(state, in, 2, 5, 2);
            }
            if(a[2] < -a[3] && b[2] >= -b[3] && c[2] >= -c[3]){//b, c in
                left_bottom_near(state, in, 2, 5, 1);
                left_bottom_near(state, in, 2, 5, 2);
            }
            if(a[2] < -a[3] && b[2] < -b[3] && c[2] < -c[3]){//none in
                return;
            }
            break;
        } 
    }

    if(face==6)
    {
        //std::cout << "i'm gonna rasterize now\n";
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
    //for converting to NDC
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

    // std::cout << "vertex a:" << std::endl;
    // for (int i = 0; i < state.floats_per_vertex; i++){
    //     std::cout << in[0]->gl_Position[i] << " ";
    // }
    // std::cout << std::endl;

    vec3 Z = {z0, z1, z2};

   

    float Ax = x0 * (state.image_width/2) + ((state.image_width/2) - 0.5);
    float Ay = y0 * (state.image_height/2) + ((state.image_height/2) - 0.5);
   

    float Bx = x1 * (state.image_width/2) + ((state.image_width/2) - 0.5);
    float By = y1 * (state.image_height/2) + ((state.image_height/2) - 0.5);
   

    float Cx = x2 * (state.image_width/2) + ((state.image_width/2) - 0.5);
    float Cy = y2 * (state.image_height/2) + ((state.image_height/2) - 0.5);


    vec2 A = {Ax,Ay};
    vec2 B = {Bx,By};
    vec2 C = {Cx,Cy};

    float minX = std::min(Ax, std::min(Bx, Cx));
    float maxX = std::max(Ax, std::max(Bx, Cx));
    float minY = std::min(Ay, std::min(By, Cy));
    float maxY = std::max(Ay, std::max(By, Cy));


    float area_abc = (C[0] - A[0]) * (B[1] - A[1]) - (C[1] - A[1]) * (B[0] - A[0]);

    if(minX < 0)
        minX = 0;
    if(maxX > state.image_width)
        maxX = state.image_width;
    if(minY < 0)
        minY = 0;
    if(maxY > state.image_height)
        maxY = state.image_height;


    for(int i = minX; i < maxX; i++){
        for (int j = minY; j < maxY; j++){

            vec2 P = {(float)i,(float)j};
            int index = i + j * state.image_width;

            float area_pbc = (P[0] - B[0]) * (C[1] - B[1]) - (P[1] - B[1]) * (C[0] - B[0]); // cross((p - b), (c-b))
            float area_pca = (P[0] - C[0]) * (A[1] - C[1]) - (P[1] - C[1]) * (A[0] - C[0]); // cross ((P-C),(A-C))
            float area_pba = (P[0] - A[0]) * (B[1] - A[1]) - (P[1] - A[1]) * (B[0] - A[0]); // cross ((P-A), (B-A))

            float alpha_p = area_pbc / area_abc;
            float beta_p = area_pca / area_abc;
            float gamma_p = area_pba / area_abc;


            float alpha,beta,gamma;

            if(alpha_p >= 0 && beta_p >= 0 && gamma_p >= 0){
                data_output out;
                data_fragment frag;
                frag.data = new float[MAX_FLOATS_PER_VERTEX];
                float depth = (alpha_p * Z[0]) + (beta_p * Z[1]) + (gamma_p * Z[2]);
                if(state.image_depth[index] > depth){
                    for(int c = 0; c < state.floats_per_vertex; c++){
                        if (state.interp_rules[c] == interp_type::flat){
                            frag.data[c] = in[0]->data[c];
                        }
                        if (state.interp_rules[c] == interp_type::noperspective){
                            frag.data[c] = (alpha_p * in[0]->data[c]) + (beta_p * in[1]->data[c]) + (gamma_p * in[2]->data[c]);
                        }
                        if (state.interp_rules[c] == interp_type::smooth){
                            float k = (alpha_p / w0) + (beta_p / w1) + (gamma_p / w2);
                            alpha = alpha_p / w0 / k; 
                            beta = beta_p / w1 / k; 
                            gamma = gamma_p / w2 / k; 
                            frag.data[c] = (alpha * in[0]->data[c]) + (beta * in[1]->data[c]) + (gamma * in[2]->data[c]);

                        }

                    }
                    state.fragment_shader(frag, out, state.uniform_data);
                    float r = out.output_color[0] * 255;
                    float   g = out.output_color[1] * 255;
                    float b = out.output_color[2] * 255;
                    state.image_color[index] = make_pixel(r,g,b);
                    state.image_depth[index] = depth;
                }
            }
        }
    }


    //std::cout<<"TODO: implement rasterization"<<std::endl;
}
