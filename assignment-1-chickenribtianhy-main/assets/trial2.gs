#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 200) out;

in VS_OUT {
    vec3 normal;
    vec3 position;
} vs_out[];

const float MAGNITUDE = 0.4;

uniform mat4 projection;

void GenerateLine(int index)
{
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    
    
    //gl_Position = projection * (gl_in[index].gl_Position + 
     //                           vec4(vs_out[index].normal, 0.0) * MAGNITUDE);
    //EmitVertex();
}

void main()
{
    GenerateLine(0); // 第一个顶点法线
    GenerateLine(1); // 第二个顶点法线
    GenerateLine(2); // 第三个顶点法线
    EndPrimitive();

}