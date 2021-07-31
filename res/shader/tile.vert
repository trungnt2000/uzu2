#version 330 core
layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_uv;

out vec2 texCoord;

uniform mat4 u_vpMat;

void main() 
{
    gl_Position = u_vpMat * vec4(a_pos, 1.0);
    texCoord = a_uv;
}

