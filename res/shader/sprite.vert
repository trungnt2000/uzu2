#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec4 a_color;
out vec4 vertColor;
out vec2 texCoord;

uniform mat4 u_viewProjectionMatrix;

void main() 
{
    gl_Position =  u_viewProjectionMatrix * vec4(a_pos.x, a_pos.y, 0.f, 1.0);    
    gl_Position.z = 0.f;
    vertColor = a_color;
    texCoord = a_uv;
}

