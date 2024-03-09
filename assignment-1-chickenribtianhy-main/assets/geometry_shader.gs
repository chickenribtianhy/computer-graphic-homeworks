#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=100) out;

in VS_OUT{
    vec3 normal;
    vec3 position;
}vs_out[];


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 frag_pos;
out vec3 normal;

void vertex_emit(vec3 x, vec3 y) {
    frag_pos = x;
    normal = y;
    gl_Position = projection * view * vec4(frag_pos, 1.0);
    EmitVertex();
}


void main() {
      vec3 center_position = (vs_out[0].position + vs_out[1].position + vs_out[2].position) / 3;
    vec3 center_normal = (vs_out[0].normal + vs_out[1].normal + vs_out[2].normal) / 3;

    vertex_emit(vs_out[0].position,vs_out[0].normal);
    vertex_emit(vs_out[1].position,vs_out[1].normal);
    vertex_emit(vs_out[2].position,vs_out[2].normal);

    frag_pos = center_position + 0.003 * center_normal;
    normal = center_normal;
    gl_Position = projection * view * vec4(frag_pos, 1.0);
    EmitVertex();

    EndPrimitive();
}