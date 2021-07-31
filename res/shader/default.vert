#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec4 a_color;

out vec4 vertColor;
out vec2 textCoord;

uniform mat4 u_projMat;
uniform mat3 u_viewMat;

void main() 
{
    vec3 v = u_viewMat * vec3(a_pos.x, a_pos.y, 1.f);
    gl_Position =  u_projMat * vec4(v.x, v.y, 0, 1.f);    
    vertColor = a_color;
    textCoord = a_uv;
}
