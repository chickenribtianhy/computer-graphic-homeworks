#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;

out VS_OUT{
    vec3 normal;
    vec3 position;
}vs_out;


//out vec3 frag_pos;
//out vec3 normal;
//out vec3 vs_frag_pos;
//out vec3 vs_normal;

//out VS_OUT vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vs_out.position = vec3(model * vec4(in_pos, 1.0f));
    vs_out.normal = vec3(model * vec4(in_normal, 0.0f));


    //gl_Position = projection * view * model * vec4(vs_out.position, 1.0f);
    gl_Position =  view * model * vec4(in_pos, 1.0f);
}